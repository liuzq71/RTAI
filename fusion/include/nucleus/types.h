/*
 * Copyright (C) 2001,2002,2003 Philippe Gerum <rpm@xenomai.org>.
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

#ifndef _RTAI_NUCLEUS_TYPES_H
#define _RTAI_NUCLEUS_TYPES_H

#include <linux/errno.h>
#include <nucleus/asm/system.h>

typedef unsigned long xnsigmask_t;

typedef unsigned long long xnticks_t;

typedef long long xnsticks_t;

typedef unsigned long long xntime_t; /* ns */

typedef long long xnstime_t;

#ifdef CONFIG_RTAI_OPT_TIMESTAMPS
typedef struct xntimes {
    xnticks_t timer_entry;
    xnticks_t timer_handler;
    xnticks_t timer_handled;
    xnticks_t timer_exit;
    xnsticks_t timer_drift;
    xnsticks_t timer_drift2;
    xnticks_t intr_resched;
    xnticks_t resume_entry;
    xnticks_t resume_exit;
    xnticks_t switch_in;
    xnticks_t switch_out;
    xnticks_t periodic_wakeup;
    xnticks_t periodic_exit;
} xntimes_t;
#endif /* CONFIG_RTAI_OPT_TIMESTAMPS */

#define XN_INFINITE   (0)
#define XN_NONBLOCK   ((xnticks_t)-1)

#define testbits(flags,mask) ((flags) & (mask))
#define setbits(flags,mask)  xnarch_atomic_set_mask(&(flags),mask)
#define clrbits(flags,mask)  xnarch_atomic_clear_mask(&(flags),mask)
#define __testbits(flags,mask) testbits(flags,mask)
#define __setbits(flags,mask)  do { (flags) |= (mask); } while(0)
#define __clrbits(flags,mask)  do { (flags) &= ~(mask); } while(0)

typedef atomic_flags_t xnflags_t;

#ifndef NULL
#define NULL 0
#endif

#define XNOBJECT_NAME_LEN 32

static inline void xnobject_copy_name (char *dst,
				       const char *src)
{
    if (src)
	{
	const char *rp = src;
	char *wp = dst;
	do
	    *wp++ = *rp;
	while (*rp && rp++ - src < XNOBJECT_NAME_LEN);
	}
    else
	*dst = '\0';
}

#define minval(a,b) ((a) < (b) ? (a) : (b))
#define maxval(a,b) ((a) > (b) ? (a) : (b))

#define XN_NBBY 8 /* Assume a byte is always 8 bits :o> */

#ifdef __cplusplus
extern "C" {
#endif

const char *xnpod_fatal_helper(const char *format, ...);

#ifdef __cplusplus
}
#endif

#define xnpod_fatal(format,args...) \
do { \
   const char *panic = xnpod_fatal_helper(format,##args); \
   xnarch_halt(panic); \
} while (0)

#endif /* !_RTAI_NUCLEUS_TYPES_H */
