/*
 * Copyright (C) 2005 Philippe Gerum <rpm@xenomai.org>.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
 */

#include <sys/types.h>
#include <stdio.h>
#include <rtai/syscall.h>
#include <rtai/intr.h>

extern int __rtai_muxid;

int __init_skin(void);

int rt_intr_create (RT_INTR *intr,
		    unsigned irq)
{
    if (__rtai_muxid < 0 && __init_skin() < 0)
	return -ENOSYS;

    return XENOMAI_SKINCALL2(__rtai_muxid,
			     __rtai_intr_create,
			     intr,
			     irq);
}

int rt_intr_bind (RT_INTR *intr,
		  unsigned irq)
{
    char name[XNOBJECT_NAME_LEN];

    if (__rtai_muxid < 0 && __init_skin() < 0)
	return -ENOSYS;

    snprintf(name,sizeof(name),"interrupt/%u",irq);

    return XENOMAI_SKINCALL2(__rtai_muxid,
			     __rtai_intr_bind,
			     intr,
			     name);
}

int rt_intr_delete (RT_INTR *intr)

{
    return XENOMAI_SKINCALL1(__rtai_muxid,
			     __rtai_intr_delete,
			     intr);
}

int rt_intr_wait (RT_INTR *intr,
		  RTIME timeout)
{
    return XENOMAI_SKINCALL2(__rtai_muxid,
			     __rtai_intr_wait,
			     intr,
			     &timeout);
}

int rt_intr_inquire (RT_INTR *intr,
		     RT_INTR_INFO *info)
{
    return XENOMAI_SKINCALL2(__rtai_muxid,
			     __rtai_intr_inquire,
			     intr,
			     info);
}
