/*
 * Copyright (C) 2001,2002,2003,2004 Philippe Gerum <rpm@xenomai.org>.
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
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <nucleus/heap.h>
#include <rtai/syscall.h>
#include <rtai/task.h>
#include <rtai/queue.h>

extern int __rtai_muxid;

static inline int __init_skin (void) /* FIXME: Hey, this is racy! */

{
    __rtai_muxid = XENOMAI_SYSCALL2(__xn_sys_attach,RTAI_SKIN_MAGIC,NULL);
    return __rtai_muxid;
}

static int __map_queue_memory (RT_QUEUE *q, RT_QUEUE_PLACEHOLDER *php)

{
    int err, heapfd;

    /* Open the heap device to share the message pool memory with the
       in-kernel skin and bound clients. */
    heapfd = open(XNHEAP_DEV_NAME,O_RDONLY);

    if (heapfd < 0)
	return -ENOENT;

    /* Bind this file instance to the shared heap. */
    err = ioctl(heapfd,0,php->opaque2);

    if (err)
	goto close_and_exit;

    /* Map the heap memory into our address space. */
    php->mapbase = (caddr_t)mmap(NULL,
				 php->mapsize,
				 PROT_READ|PROT_WRITE,
				 MAP_SHARED,
				 heapfd,
				 0L);

    if (php->mapbase != MAP_FAILED)
	/* Copy back a complete placeholder only if all is ok. */
	*q = *php;

 close_and_exit:

    close(heapfd);

    return err;
}

int rt_queue_create (RT_QUEUE *q,
		     const char *name,
		     size_t poolsize,
		     size_t qlimit,
		     int mode)
{
    RT_QUEUE_PLACEHOLDER ph;
    int err;

    if (__rtai_muxid < 0 && __init_skin() < 0)
	return -ENOSYS;

    err = XENOMAI_SKINCALL5(__rtai_muxid,
			    __rtai_queue_create,
			    &ph,
			    name,
			    poolsize,
			    qlimit,
			    mode|Q_SHARED);

    return err ?: __map_queue_memory(q,&ph);
}

int rt_queue_bind (RT_QUEUE *q,
		   const char *name)
{
    RT_QUEUE_PLACEHOLDER ph;
    int err;

    if (__rtai_muxid < 0 && __init_skin() < 0)
	return -ENOSYS;

    err = XENOMAI_SKINCALL2(__rtai_muxid,
			    __rtai_queue_bind,
			    &ph,
			    name);

    return err ?: __map_queue_memory(q,&ph);
}

int rt_queue_delete (RT_QUEUE *q)

{
    int err;

    err = XENOMAI_SKINCALL1(__rtai_muxid,
			    __rtai_queue_delete,
			    q);
    if (err)
	return err;

    munmap(q->mapbase,q->mapsize);

    return 0;
}

void *rt_queue_alloc (RT_QUEUE *q,
		      size_t size)
{
    void *buf;

    return XENOMAI_SKINCALL3(__rtai_muxid,
			     __rtai_queue_alloc,
			     q,
			     size,
			     &buf) ? NULL : buf;
}

int rt_queue_free (RT_QUEUE *q,
		   void *buf)
{
    return XENOMAI_SKINCALL2(__rtai_muxid,
			     __rtai_queue_free,
			     q,
			     buf);
}

int rt_queue_send (RT_QUEUE *q,
		   void *buf,
		   int mode)
{
    return XENOMAI_SKINCALL3(__rtai_muxid,
			     __rtai_queue_send,
			     q,
			     buf,
			     mode);
}

int rt_queue_recv (RT_QUEUE *q,
		   void **bufp,
		   RTIME timeout)
{
    return XENOMAI_SKINCALL3(__rtai_muxid,
			     __rtai_queue_recv,
			     q,
			     bufp,
			     &timeout);
}

int rt_queue_inquire (RT_QUEUE *q,
		      RT_QUEUE_INFO *info)
{
    return XENOMAI_SKINCALL2(__rtai_muxid,
			     __rtai_queue_inquire,
			     q,
			     info);
}
