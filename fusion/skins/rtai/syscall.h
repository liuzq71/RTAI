/*
 * Copyright (C) 2004 Philippe Gerum <rpm@xenomai.org>.
 *
 * Xenomai is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Xenomai is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Xenomai; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * As a special exception, the RTAI project gives permission
 * for additional uses of the text contained in its release of
 * Xenomai.
 *
 * The exception is that, if you link the Xenomai libraries with other
 * files to produce an executable, this does not by itself cause the
 * resulting executable to be covered by the GNU General Public License.
 * Your use of that executable is in no way restricted on account of
 * linking the Xenomai libraries code into it.
 *
 * This exception does not however invalidate any other reasons why
 * the executable file might be covered by the GNU General Public
 * License.
 *
 * This exception applies only to the code released by the
 * RTAI project under the name Xenomai.  If you copy code from other
 * RTAI project releases into a copy of Xenomai, as the General Public
 * License permits, the exception does not apply to the code that you
 * add in this way.  To avoid misleading anyone as to the status of
 * such modified files, you must delete this exception notice from
 * them.
 *
 * If you write modifications of your own for Xenomai, it is your
 * choice whether to permit this exception to apply to your
 * modifications. If you do not wish that, delete this exception
 * notice.
 */

#ifndef _RTAI_SYSCALL_H
#define _RTAI_SYSCALL_H

#ifndef __RTAI_SIM__
#include <nucleus/asm/syscall.h>
#endif /* __RTAI_SIM__ */

#define __rtai_task_create        0
#define __rtai_task_bind          1
#define __rtai_task_start         2
#define __rtai_task_suspend       3
#define __rtai_task_resume        4
#define __rtai_task_delete        5
#define __rtai_task_yield         6
#define __rtai_task_set_periodic  7
#define __rtai_task_wait_period   8
#define __rtai_task_set_priority  9
#define __rtai_task_sleep         10
#define __rtai_task_sleep_until   11
#define __rtai_task_unblock       12
#define __rtai_task_inquire       13
#define __rtai_timer_start        14
#define __rtai_timer_stop         15
#define __rtai_timer_read         16
#define __rtai_timer_tsc          17
#define __rtai_timer_ns2ticks     18
#define __rtai_timer_ticks2ns     19
#define __rtai_timer_inquire      20
#define __rtai_sem_create         21
#define __rtai_sem_bind           22
#define __rtai_sem_delete         23
#define __rtai_sem_p              24
#define __rtai_sem_v              25
#define __rtai_sem_inquire        26
#define __rtai_event_create       27
#define __rtai_event_bind         28
#define __rtai_event_delete       29
#define __rtai_event_pend         30
#define __rtai_event_post         31
#define __rtai_event_inquire      32
#define __rtai_mutex_create       33
#define __rtai_mutex_bind         34
#define __rtai_mutex_delete       35
#define __rtai_mutex_lock         36
#define __rtai_mutex_unlock       37
#define __rtai_mutex_inquire      38
#define __rtai_cond_create        39
#define __rtai_cond_bind          40
#define __rtai_cond_delete        41
#define __rtai_cond_wait          42
#define __rtai_cond_signal        43
#define __rtai_cond_broadcast     44
#define __rtai_cond_inquire       45
#define __rtai_queue_create       46
#define __rtai_queue_bind         47
#define __rtai_queue_delete       48
#define __rtai_queue_alloc        49
#define __rtai_queue_free         50
#define __rtai_queue_send         51
#define __rtai_queue_recv         52
#define __rtai_queue_inquire      53

struct rt_arg_bulk {

    u_long a1;
    u_long a2;
    u_long a3;
    u_long a4;
    u_long a5;
};

#ifdef __KERNEL__

#ifdef __cplusplus
extern "C" {
#endif

int __syscall_pkg_init(void);

void __syscall_pkg_cleanup(void);

#ifdef __cplusplus
}
#endif

#endif /* __KERNEL__ */

#endif /* _RTAI_SYSCALL_H */
