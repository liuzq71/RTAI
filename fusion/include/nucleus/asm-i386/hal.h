/**
 *   @ingroup hal
 *   @file
 *
 *   Adeos-based Real-Time Hardware Abstraction Layer for x86.
 *
 *   Original RTAI/x86 layer implementation: \n
 *   Copyright &copy; 2000 Paolo Mantegazza, \n
 *   Copyright &copy; 2000 Steve Papacharalambous, \n
 *   Copyright &copy; 2000 Stuart Hughes, \n
 *   and others.
 *
 *   RTAI/x86 rewrite over Adeos: \n
 *   Copyright &copy; 2002,2003 Philippe Gerum.
 *   Major refactoring for RTAI/fusion: \n
 *   Copyright &copy; 2004 Philippe Gerum.
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, Inc., 675 Mass Ave, Cambridge MA 02139,
 *   USA; either version 2 of the License, or (at your option) any later
 *   version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/**
 * @addtogroup hal
 *@{*/

#ifndef _RTAI_ASM_I386_HAL_H
#define _RTAI_ASM_I386_HAL_H

#include <rtai_config.h>

#define RTHAL_NR_CPUS  ADEOS_NR_CPUS

typedef unsigned long long rthal_time_t;

/* FIXME: temporary fix pasted from include/linux/compiler*.h, in order to have
   nucleus/lib/fusion.c. The proper fix is to implement llimd in user-space,
   since this is the only routine needed. */
#ifndef __KERNEL__
#if __GNUC__ == 2 && __GNUC_MINOR >= 96 || __GNUC__ >= 3
#define __attribute_const__ __attribute__((__const__))
#else
#define __attribute_const__
#endif
#if __GNUC__ > 3 || __GNUC__ == 3 && __GNUC_MINOR__ >= 1
# define inline         inline          __attribute__((always_inline))
#endif
#endif

#define __rthal_u64tou32(ull, h, l) ({          \
    (l) = ull & 0xffffffff;                     \
    (h) = ull >> 32;                            \
})

#define __rthal_u64fromu32(h, l) ({                     \
    union { unsigned long long _ull;                    \
        struct { unsigned long _sl, _sh; } _s; } _tmp;  \
    _tmp._s._sh=(h);                                    \
    _tmp._s._sl=(l);                                    \
    _tmp._ull;                                          \
})

/* Fast longs multiplication. */
static inline __attribute_const__ unsigned long long
rthal_ullmul(unsigned long m1, unsigned long m2) {
    /* Gcc (at least for versions 2.95 and higher) optimises correctly here. */
    return (unsigned long long) m1 * m2;
}

/* const helper for rthal_uldivrem, so that the compiler will eliminate
   multiple calls with same arguments, at no additionnal cost. */
static inline __attribute_const__ unsigned long long
__rthal_uldivrem(unsigned long long ull, unsigned long d) {

    __asm__ ("divl %1" : "=A,A"(ull) : "r,?m"(d), "A,A"(ull));
    /* Exception if quotient does not fit on unsigned long. */
    return ull;
}

/* Fast long long division: when the quotient and remainder fit on 32 bits.
   Eg.: conversion between POSIX struct timespec and nanoseconds count.
   Recent compilers remove redundant calls to this function. */
static inline unsigned long
rthal_uldivrem(unsigned long long ull, unsigned long d, unsigned long *rp) {

    unsigned long q, r;
    ull = __rthal_uldivrem(ull, d);
    __asm__ ( "": "=d"(r), "=a"(q) : "A"(ull));
    if(rp)
        *rp = r;
    return q;
}

/* Slow long long division. Uses rthal_uldivrem, hence has the same property:
   compiler removes redundant calls. It seems to inline quite well too. */
static inline unsigned long long rthal_ulldiv (unsigned long long ull,
                                               unsigned long d,
                                               unsigned long *rp) {

    unsigned long h, l, qh, rh, ql;
    __rthal_u64tou32(ull, h, l);

    qh = rthal_uldivrem(h, d, &rh);
    __asm__ ( "": "=A"(ull) : "d"(rh), "a"(l));
    ql = rthal_uldivrem(ull, d, rp);

    return __rthal_u64fromu32(qh, ql);
}

/* Replaced the helper with rthal_ulldiv: rthal_ulldiv inlines better. */
#define rthal_u64div32c rthal_ulldiv

static inline __attribute_const__ int rthal_imuldiv (int i, int mult, int div) {

    /* Returns (unsigned)i = (unsigned)i*(unsigned)(mult)/(unsigned)div. */
    unsigned long ui = (unsigned long) i, um = (unsigned long) mult;
    return __rthal_uldivrem((unsigned long long) ui * um, div);
}

static inline __attribute_const__ long long rthal_llimd(long long ll,
                                                        int mult,
                                                        int div) {

    /* Returns (long long)ll = (int)ll*(int)(mult)/(int)div. */

    __asm__  ( \
	"movl %%edx,%%ecx\t\n" \
	"mull %%esi\t\n" \
	"movl %%eax,%%ebx\n\t" \
	"movl %%ecx,%%eax\t\n" \
        "movl %%edx,%%ecx\t\n" \
        "mull %%esi\n\t" \
	"addl %%ecx,%%eax\t\n" \
	"adcl $0,%%edx\t\n" \
        "divl %%edi\n\t" \
        "movl %%eax,%%ecx\t\n" \
        "movl %%ebx,%%eax\t\n" \
	"divl %%edi\n\t" \
	"sal $1,%%edx\t\n" \
        "cmpl %%edx,%%edi\t\n" \
        "movl %%ecx,%%edx\n\t" \
	"jge 1f\t\n" \
        "addl $1,%%eax\t\n" \
        "adcl $0,%%edx\t\n" \
	"1:\t\n" \
	: "=A" (ll) \
	: "A" (ll), "S" (mult), "D" (div) \
	: "%ebx", "%ecx");

    return ll;
}


static inline __attribute_const__ unsigned long ffnz (unsigned long word) {
    /* Derived from bitops.h's ffs() */
    __asm__("bsfl %1, %0"
	    : "=r,r" (word)
	    : "r,?m"  (word));
    return word;
}

#if defined(__KERNEL__) && !defined(__cplusplus)
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <asm/system.h>
#include <asm/io.h>
#include <asm/timex.h>
#include <nucleus/asm/atomic.h>
#include <asm/processor.h>
#include <io_ports.h>
#ifdef CONFIG_X86_LOCAL_APIC
#include <asm/fixmap.h>
#include <asm/apic.h>
#endif /* CONFIG_X86_LOCAL_APIC */

typedef void (*rthal_irq_handler_t)(unsigned irq,
				    void *cookie);

struct rthal_calibration_data {

    unsigned long cpu_freq;
    unsigned long timer_freq;
};

extern struct rthal_calibration_data rthal_tunables;

extern volatile unsigned long rthal_cpu_realtime;

extern adomain_t rthal_domain;

#define RTHAL_DOMAIN_ID  0x52544149

#define RTHAL_NR_SRQS    32

#define RTHAL_TIMER_FREQ  (rthal_tunables.timer_freq)
#define RTHAL_CPU_FREQ    (rthal_tunables.cpu_freq)
#define RTHAL_8254_IRQ    0

#ifdef CONFIG_X86_LOCAL_APIC
#define RTHAL_APIC_TIMER_VECTOR    ADEOS_SERVICE_VECTOR3
#define RTHAL_APIC_TIMER_IPI       ADEOS_SERVICE_IPI3
#define RTHAL_APIC_ICOUNT	   ((RTHAL_TIMER_FREQ + HZ/2)/HZ)
#endif /* CONFIG_X86_LOCAL_APIC */

#ifdef CONFIG_X86_TSC
static inline unsigned long long rthal_rdtsc (void) {
    unsigned long long t;
    __asm__ __volatile__( "rdtsc" : "=A" (t));
    return t;
}
#else  /* !CONFIG_X86_TSC */
#define RTHAL_8254_COUNT2LATCH  0xfffe
void rthal_setup_8254_tsc(void);
rthal_time_t rthal_get_8254_tsc(void);
#define rthal_rdtsc() rthal_get_8254_tsc()
#endif /* CONFIG_X86_TSC */

#define rthal_cli()                     adeos_stall_pipeline_from(&rthal_domain)
#define rthal_sti()                     adeos_unstall_pipeline_from(&rthal_domain)
#define rthal_local_irq_save(x)         ((x) = !!adeos_test_and_stall_pipeline_from(&rthal_domain))
#define rthal_local_irq_restore(x)      adeos_restore_pipeline_from(&rthal_domain,(x))
#define rthal_local_irq_flags(x)        ((x) = !!adeos_test_pipeline_from(&rthal_domain))
#define rthal_local_irq_test()          (!!adeos_test_pipeline_from(&rthal_domain))
#define rthal_local_irq_sync(x)         ((x) = !!adeos_test_and_unstall_pipeline_from(&rthal_domain))

#define rthal_hw_lock(flags)            adeos_hw_local_irq_save(flags)
#define rthal_hw_unlock(flags)          adeos_hw_local_irq_restore(flags)
#define rthal_hw_enable()               adeos_hw_sti()
#define rthal_hw_disable()              adeos_hw_cli()

#define rthal_linux_sti()                adeos_unstall_pipeline_from(adp_root)
#define rthal_linux_cli()                adeos_stall_pipeline_from(adp_root)
#define rthal_linux_local_irq_save(x)    ((x) = !!adeos_test_and_stall_pipeline_from(adp_root))
#define rthal_linux_local_irq_restore(x) adeos_restore_pipeline_from(adp_root,x)
#define rthal_linux_local_irq_restore_nosync(x,cpuid) adeos_restore_pipeline_nosync(adp_root,x,cpuid)

#define rthal_spin_lock(lock)    adeos_spin_lock(lock)
#define rthal_spin_unlock(lock)  adeos_spin_unlock(lock)

static inline void rthal_spin_lock_irq(spinlock_t *lock) {

    rthal_cli();
    rthal_spin_lock(lock);
}

static inline void rthal_spin_unlock_irq(spinlock_t *lock) {

    rthal_spin_unlock(lock);
    rthal_sti();
}

static inline unsigned long rthal_spin_lock_irqsave(spinlock_t *lock) {

    unsigned long flags;
    rthal_local_irq_save(flags);
    rthal_spin_lock(lock);
    return flags;
}

static inline void rthal_spin_unlock_irqrestore(unsigned long flags,
						spinlock_t *lock) {
    rthal_spin_unlock(lock);
    rthal_local_irq_restore(flags);
}

#ifdef CONFIG_SMP
#define rthal_cpu_relax(x) \
do { \
   int i = 0; \
   do \
     cpu_relax(); \
   while (++i < x); \
} while(0)
#endif /* CONFIG_SMP */

#if !defined(CONFIG_ADEOS_NOTHREADS)

/* Since real-time interrupt handlers are called on behalf of the RTAI
   domain stack, we cannot infere the "current" Linux task address
   using %esp. We must use the suspended Linux domain's stack pointer
   instead. */

static inline struct task_struct *rthal_get_root_current (int cpuid) {
    return ((struct thread_info *)(((u_long)adp_root->esp[cpuid]) & (~8191UL)))->task;
}

static inline struct task_struct *rthal_get_current (int cpuid)

{
    int *esp;

    __asm__ ("movl %%esp, %0" : "=r,?m" (esp));

    if (esp >= rthal_domain.estackbase[cpuid] && esp < rthal_domain.estackbase[cpuid] + 2048)
	return rthal_get_root_current(cpuid);

    return get_current();
}

#else /* CONFIG_ADEOS_NOTHREADS */

static inline struct task_struct *rthal_get_root_current (int cpuid) {
    return current;
}

static inline struct task_struct *rthal_get_current (int cpuid) {
    return current;
}

#endif /* !CONFIG_ADEOS_NOTHREADS */

static inline void rthal_set_timer_shot (unsigned long delay) {

    if (delay) {
        unsigned long flags;
        rthal_hw_lock(flags);
#ifdef CONFIG_X86_LOCAL_APIC
	apic_read(APIC_LVTT);
	apic_write_around(APIC_LVTT,RTHAL_APIC_TIMER_VECTOR);
	apic_read(APIC_TMICT);
	apic_write_around(APIC_TMICT,delay);
#else /* !CONFIG_X86_LOCAL_APIC */
	outb(delay & 0xff,0x40);
	outb(delay >> 8,0x40);
#endif /* CONFIG_X86_LOCAL_APIC */
        rthal_hw_unlock(flags);
    }
}

    /* Private interface -- Internal use only */

unsigned long rthal_critical_enter(void (*synch)(void));

void rthal_critical_exit(unsigned long flags);

void rthal_set_linux_task_priority(struct task_struct *task,
				   int policy,
				   int prio);

#endif /* __KERNEL__ && !__cplusplus */

    /* Public interface */

#ifdef __KERNEL__

#include <linux/kernel.h>

typedef int (*rthal_trap_handler_t)(adevinfo_t *evinfo);

#define rthal_printk    printk /* This is safe over Adeos */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int rthal_request_irq(unsigned irq,
		      void (*handler)(unsigned irq, void *cookie),
		      void *cookie);

int rthal_release_irq(unsigned irq);

/**
 * @name Programmable Interrupt Controllers (PIC) management functions.
 *
 *@{*/

int rthal_enable_irq(unsigned irq);

int rthal_disable_irq(unsigned irq);

/*@}*/

int rthal_request_linux_irq(unsigned irq,
			    irqreturn_t (*handler)(int irq,
						   void *dev_id,
						   struct pt_regs *regs), 
			    char *name,
			    void *dev_id);

int rthal_release_linux_irq(unsigned irq,
			    void *dev_id);

int rthal_pend_linux_irq(unsigned irq);

int rthal_pend_linux_srq(unsigned srq);

int rthal_request_srq(unsigned label,
		      void (*handler)(void));

int rthal_release_srq(unsigned srq);

int rthal_set_irq_affinity(unsigned irq,
			   cpumask_t cpumask,
			   cpumask_t *oldmask);

int rthal_request_timer(void (*handler)(void),
			unsigned long nstick);

void rthal_release_timer(void);

rthal_trap_handler_t rthal_set_trap_handler(rthal_trap_handler_t handler);

unsigned long rthal_calibrate_timer(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __KERNEL__ */

/*@}*/

#endif /* !_RTAI_ASM_I386_HAL_H */
