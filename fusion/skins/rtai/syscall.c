/**
 * This file is part of the RTAI project.
 *
 * @note Copyright (C) 2004 Philippe Gerum <rpm@xenomai.org> 
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <nucleus/pod.h>
#include <nucleus/heap.h>
#include <nucleus/shadow.h>
#include <rtai/syscall.h>
#include <rtai/registry.h>
#include <rtai/task.h>
#include <rtai/timer.h>
#include <rtai/sem.h>

/* This file implements the syscall wrappers. Unchecked uaccess is
   used since the syslib is trusted. */

static int __muxid;

/*
 * int __rt_task_create(struct rt_arg_bulk *bulk,
 *                      pid_t syncpid,
 *                      int *u_syncp)
 *
 * bulk = {
 * a1: RT_TASK_PLACEHOLDER *task;
 * a2: const char *name;
 * a3: int prio;
 * }
 */

static int __rt_task_create (struct task_struct *curr, struct pt_regs *regs)

{
    char name[XNOBJECT_NAME_LEN];
    struct rt_arg_bulk bulk;
    int *u_syncp, err, prio;
    RT_TASK_PLACEHOLDER *ph;
    pid_t syncpid;
    RT_TASK *task;
    spl_t s;

    __xn_copy_from_user(curr,&bulk,(void *)__xn_reg_arg1(regs),sizeof(bulk));

    ph = (RT_TASK_PLACEHOLDER *)bulk.a1;

    if (bulk.a2)
	{
	if (!__xn_access_ok(curr,VERIFY_READ,bulk.a2,sizeof(name)))
	    return -EFAULT;

	__xn_copy_from_user(curr,name,(const char *)bulk.a2,sizeof(name) - 1);
	name[sizeof(name) - 1] = '\0';
	strncpy(curr->comm,name,sizeof(curr->comm));
	curr->comm[sizeof(curr->comm) - 1] = '\0';
	}
    else
	*name = '\0';

    /* Task priority. */
    prio = bulk.a3;
    /* PID of parent thread waiting for sync. */
    syncpid = __xn_reg_arg2(regs);
    /* Semaphore address. */
    u_syncp = (int *)__xn_reg_arg3(regs);

    task = (RT_TASK *)xnmalloc(sizeof(*task));

    if (!task)
	return -ENOMEM;

    /* We don't want some funny guy to rip the new TCB off while two
       user-space threads are being synchronized on it, so enter a
       critical section. */

    splhigh(s);

    /* Force FPU support in user-space. This will lead to a no-op if
       the platform does not support it. */

    err = rt_task_create(task,name,0,prio,XNFPU|XNSHADOW);

    if (err == 0)
	{
	/* Copy back the registry handle to the ph struct
	   _before_ current is suspended in xnshadow_map(). */

	__xn_put_user(curr,task->handle,&ph->opaque);

	/* The following service blocks until rt_task_start() is
	   issued. */

	xnshadow_map(&task->thread_base,
		     syncpid,
		     u_syncp);

	/* Pass back the entry and the cookie pointers obtained from
	   rt_task_start(). */

	bulk.a1 = (u_long)task->thread_base.entry;
	bulk.a2 = (u_long)task->thread_base.cookie;
	__xn_copy_to_user(curr,(void *)__xn_reg_arg1(regs),&bulk,sizeof(bulk));
	}
    else
	{
	xnfree(task);
	xnshadow_sync_post(syncpid,u_syncp,err);
	}

    splexit(s);

    return err;
}

/*
 * int __rt_task_bind (RT_TASK_PLACEHOLDER *ph,
 *                     const char *name)
 */

static int __rt_task_bind (struct task_struct *curr, struct pt_regs *regs)

{
    return 0;
}

/*
 * int __rt_task_start(RT_TASK_PLACEHOLDER *ph,
 *                     void (*entry)(void *cookie),
 *                     void *cookie)
 */

static int __rt_task_start (struct task_struct *curr, struct pt_regs *regs)

{
    RT_TASK_PLACEHOLDER *ph;
    RT_TASK *task;
    int err;

    ph = (RT_TASK_PLACEHOLDER *)__xn_reg_arg1(regs);
    task = (RT_TASK *)rt_registry_get(ph->opaque);

    if (!task)
	return -ENOENT;

    err = rt_task_start(task,
			(void (*)(void *))__xn_reg_arg2(regs),
			(void *)__xn_reg_arg3(regs));

    rt_registry_put(ph->opaque);

    return err;
}

/*
 * int __rt_task_set_periodic(RT_TASK_PLACEHOLDER *ph,
 *			         RTIME idate,
 *			         RTIME period)
 */

static int __rt_task_set_periodic (struct task_struct *curr, struct pt_regs *regs)

{
    RT_TASK_PLACEHOLDER *ph;
    RTIME idate, period;
    RT_TASK *task;
    int err;

    ph = (RT_TASK_PLACEHOLDER *)__xn_reg_arg1(regs);
    task = (RT_TASK *)rt_registry_get(ph->opaque);

    if (!task)
	return -ENOENT;

    __xn_copy_from_user(curr,&idate,(void *)__xn_reg_arg2(regs),sizeof(idate));
    __xn_copy_from_user(curr,&period,(void *)__xn_reg_arg3(regs),sizeof(period));

    err = rt_task_set_periodic(task,idate,period);

    rt_registry_put(ph->opaque);

    return err;
}

/*
 * int __rt_task_wait_period(void)
 */

static int __rt_task_wait_period (struct task_struct *curr, struct pt_regs *regs)

{
    int err;

    rt_registry_get(RT_REGISTRY_SELF);

    err = rt_task_wait_period();

    rt_registry_put(RT_REGISTRY_SELF);

    return err;
}

/*
 * int __rt_task_sleep(RTIME delay)
 */

static int __rt_task_sleep (struct task_struct *curr, struct pt_regs *regs)

{
    RTIME delay;

    __xn_copy_from_user(curr,&delay,(void *)__xn_reg_arg1(regs),sizeof(delay));

    return rt_task_sleep(delay);
}

/*
 * int __rt_timer_start(RTIME *tickvalp)
 */

static int __rt_timer_start (struct task_struct *curr, struct pt_regs *regs)

{
    RTIME tickval;

    __xn_copy_from_user(curr,&tickval,(void *)__xn_reg_arg1(regs),sizeof(tickval));

    if (testbits(nkpod->status,XNTIMED))
	{
	if ((tickval == FUSION_APERIODIC_TIMER && xnpod_get_tickval() == 1) ||
	    (tickval != FUSION_APERIODIC_TIMER && xnpod_get_tickval() == tickval))
	    return 0;

	xnpod_stop_timer();
	}

    if (xnpod_start_timer(tickval,XNPOD_DEFAULT_TICKHANDLER) != 0)
	return -ETIME;

    return 0;
}

/*
 * int __rt_timer_stop (void)
 */

static int __rt_timer_stop (struct task_struct *curr, struct pt_regs *regs)

{
    rt_timer_stop();
    return 0;
}

/*
 * int __rt_timer_read (RTIME *timep)
 */

static int __rt_timer_read (struct task_struct *curr, struct pt_regs *regs)

{
    RTIME now = rt_timer_read();
    __xn_copy_to_user(curr,(void *)__xn_reg_arg1(regs),&now,sizeof(now));
    return 0;
}

/*
 * int __rt_timer_tsc (RTIME *tscp)
 */

static int __rt_timer_tsc (struct task_struct *curr, struct pt_regs *regs)

{
    RTIME tsc = rt_timer_tsc();
    __xn_copy_to_user(curr,(void *)__xn_reg_arg1(regs),&tsc,sizeof(tsc));
    return 0;
}

/*
 * int __rt_timer_ns2ticks (RTIME *ticksp, RTIME *nsp)
 */

static int __rt_timer_ns2ticks (struct task_struct *curr, struct pt_regs *regs)

{
    RTIME ns, ticks;

    __xn_copy_from_user(curr,&ns,(void *)__xn_reg_arg2(regs),sizeof(ns));
    ticks = rt_timer_ns2ticks(ns);
    __xn_copy_to_user(curr,(void *)__xn_reg_arg1(regs),&ticks,sizeof(ticks));

    return 0;
}

/*
 * int __rt_timer_ticks2ns (RTIME *nsp, RTIME *ticksp)
 */

static int __rt_timer_ticks2ns (struct task_struct *curr, struct pt_regs *regs)

{
    RTIME ticks, ns;

    __xn_copy_from_user(curr,&ticks,(void *)__xn_reg_arg2(regs),sizeof(ticks));
    ns = rt_timer_ticks2ns(ticks);
    __xn_copy_to_user(curr,(void *)__xn_reg_arg1(regs),&ns,sizeof(ns));

    return 0;
}

/*
 * int __rt_sem_create (RT_SEM_PLACEHOLDER *ph,
 *                      const char *name,
 *                      unsigned icount,
 *                      int mode)
 */

static int __rt_sem_create (struct task_struct *curr, struct pt_regs *regs)

{
    char name[XNOBJECT_NAME_LEN];
    RT_SEM_PLACEHOLDER *ph;
    unsigned icount;
    int err, mode;
    RT_SEM *sem;

    ph = (RT_SEM_PLACEHOLDER *)__xn_reg_arg1(regs);

    if (__xn_reg_arg2(regs))
	{
	if (!__xn_access_ok(curr,VERIFY_READ,__xn_reg_arg2(regs),sizeof(name)))
	    return -EFAULT;

	__xn_copy_from_user(curr,name,(const char *)__xn_reg_arg2(regs),sizeof(name) - 1);
	name[sizeof(name) - 1] = '\0';
	}
    else
	*name = '\0';

    /* Initial semaphore value. */
    icount = (unsigned)__xn_reg_arg3(regs);
    /* Creation mode. */
    mode = (int)__xn_reg_arg4(regs);

    sem = (RT_SEM *)xnmalloc(sizeof(*sem));

    if (!sem)
	return -ENOMEM;

    err = rt_sem_create(sem,name,icount,mode);

    if (err == 0)
	/* Copy back the registry handle to the ph struct. */
	__xn_put_user(curr,sem->handle,&ph->opaque);
    else
	xnfree(sem);

    return err;
}

/*
 * int __rt_sem_bind (RT_SEM_PLACEHOLDER *ph,
 *                    const char *name)
 */

static int __rt_sem_bind (struct task_struct *curr, struct pt_regs *regs)

{
    return 0;
}

/*
 * int __rt_sem_delete (RT_SEM_PLACEHOLDER *ph)
 */

static int __rt_sem_delete (struct task_struct *curr, struct pt_regs *regs)

{
    return 0;
}

/*
 * int __rt_sem_p (RT_SEM_PLACEHOLDER *ph,
 *                 RTIME *timeoutp)
 */

static int __rt_sem_p (struct task_struct *curr, struct pt_regs *regs)

{
    RT_SEM_PLACEHOLDER *ph;
    RTIME timeout;
    RT_SEM *sem;
    int err;

    ph = (RT_SEM_PLACEHOLDER *)__xn_reg_arg1(regs);
    sem = (RT_SEM *)rt_registry_get(ph->opaque);

    if (!sem)
	return -ENOENT;

    __xn_copy_from_user(curr,&timeout,(void *)__xn_reg_arg2(regs),sizeof(timeout));

    err = rt_sem_p(sem,timeout);

    rt_registry_put(ph->opaque);

    return err;
}

/*
 * int __rt_sem_p (RT_SEM_PLACEHOLDER *ph)
 */

static int __rt_sem_v (struct task_struct *curr, struct pt_regs *regs)

{
    RT_SEM_PLACEHOLDER *ph;
    RT_SEM *sem;
    int err;

    ph = (RT_SEM_PLACEHOLDER *)__xn_reg_arg1(regs);
    sem = (RT_SEM *)rt_registry_get(ph->opaque);

    if (!sem)
	return -ENOENT;

    err = rt_sem_v(sem);

    rt_registry_put(ph->opaque);

    return err;
}

/*
 * int __rt_sem_inquire (RT_SEM_PLACEHOLDER *ph,
 *                       RT_SEM_INFO *infop)
 */

static int __rt_sem_inquire (struct task_struct *curr, struct pt_regs *regs)

{
    return 0;
}

static xnsysent_t __systab[] = {
    { &__rt_task_create, __xn_flag_init },
    { &__rt_task_bind, __xn_flag_init },
    { &__rt_task_start, __xn_flag_anycall },
    { &__rt_task_set_periodic, __xn_flag_regular },
    { &__rt_task_wait_period, __xn_flag_regular },
    { &__rt_task_sleep, __xn_flag_regular },
    { &__rt_timer_start, __xn_flag_anycall  },
    { &__rt_timer_stop, __xn_flag_anycall  },
    { &__rt_timer_read, __xn_flag_anycall  },
    { &__rt_timer_tsc, __xn_flag_anycall  },
    { &__rt_timer_ns2ticks, __xn_flag_anycall  },
    { &__rt_timer_ticks2ns, __xn_flag_anycall  },
    { &__rt_sem_create, __xn_flag_anycall  },
    { &__rt_sem_bind, __xn_flag_regular },
    { &__rt_sem_delete, __xn_flag_anycall  },
    { &__rt_sem_p, __xn_flag_regular  },
    { &__rt_sem_v, __xn_flag_anycall  },
    { &__rt_sem_inquire, __xn_flag_anycall  },
};

static void __shadow_delete_hook (xnthread_t *thread)

{
    if (xnthread_get_magic(thread) == RTAI_SKIN_MAGIC &&
	testbits(thread->status,XNSHADOW))
	xnshadow_unmap(thread);
}

int __syscall_pkg_init (void)

{
    __muxid = xnshadow_register_skin("native",
				     RTAI_SKIN_MAGIC,
				     sizeof(__systab) / sizeof(__systab[0]),
				     __systab,
				     NULL);
    if (__muxid < 0)
	return -ENOSYS;

    xnpod_add_hook(XNHOOK_THREAD_DELETE,&__shadow_delete_hook);
    
    return 0;
}

void __syscall_pkg_cleanup (void)

{
    xnpod_remove_hook(XNHOOK_THREAD_DELETE,&__shadow_delete_hook);
    xnshadow_unregister_skin(__muxid);
}
