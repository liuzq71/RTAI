/*
COPYRIGHT (C) 2000  Paolo Mantegazza (mantegazza@aero.polimi.it)

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
*/


#include <stdio.h>
#include <stdlib.h>
#include <sys/poll.h>
#include <rtai_mbx.h>
#include <rtai_msg.h>

static RT_TASK *task;

int main(void)
{
	unsigned int msg;
	MBX *mbx;
	struct sample { long long min; long long max; int index, ovrn; } samp;

 	if (!(task = rt_task_init(nam2num("LATCHK"), 20, 0, 0))) {
		printf("CANNOT INIT MASTER TASK\n");
		exit(1);
	}

 	if (!(mbx = rt_get_adr(nam2num("LATMBX")))) {
		printf("CANNOT FIND MAILBOX\n");
		exit(1);
	}

	while (1) {
		struct pollfd pfd = { fd: 0, events: POLLIN|POLLERR|POLLHUP, revents: 0 };
		rt_mbx_receive(mbx, &samp, sizeof(samp));
		printf("*** min: %d, max: %d average: %d (%d) <Hit [RETURN] to stop> ***\n", (int) samp.min, (int) samp.max, samp.index, samp.ovrn);
		if (poll(&pfd, 1, 20) > 0 && (pfd.revents & (POLLIN|POLLERR|POLLHUP)) != 0)
		    break;
	}

	rt_rpc(rt_get_adr(nam2num("LATCAL")), msg, &msg);
	rt_task_delete(task);
	exit(0);
}
