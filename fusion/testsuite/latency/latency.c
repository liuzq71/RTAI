#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <rtai/task.h>
#include <rtai/timer.h>
#include <rtai/sem.h>

#define TASK_PERIOD_NS 100000LL

#define SAMPLE_COUNT (1000000000 / TASK_PERIOD_NS)

RT_TASK latency_task, display_task;

RT_SEM display_sem;

int minjitter = 10000000,
    maxjitter = -10000000,
    alljitter = 0,
    overrun = 0;

void latency (void *cookie)

{
    int minj = 10000000, maxj = -10000000;
    RTIME itime, expected, period;
    int dt, err, count;
    long long sumj = 0;

    err = rt_timer_start(RT_TIMER_ONESHOT);

    if (err)
	{
	printf("latency: cannot start timer, code %d\n",err);
	return;
	}

    period = rt_timer_ns2ticks(TASK_PERIOD_NS);
    itime = rt_timer_read() + TASK_PERIOD_NS * 5;
    expected = rt_timer_ns2ticks(itime);
    err = rt_task_set_periodic(NULL,itime,TASK_PERIOD_NS);

    if (err)
	{
	printf("latency: failed to set periodic, code %d\n",err);
	return;
	}

    for (;;)
	{
	for (count = sumj = 0; count < SAMPLE_COUNT; count++)
	    {
	    expected += period;
	    err = rt_task_wait_period();

	    if (err)
		overrun++;
	    else
		{
		dt = (int)(rt_timer_tsc() - expected);
		if (dt > maxj) maxj = dt;
		if (dt < minj) minj = dt;
		sumj += dt;
		}
	    }

	minjitter = minj;
	minj = 10000000;
	maxjitter = maxj;
	alljitter = sumj / SAMPLE_COUNT;
	rt_sem_v(&display_sem);
	}

    rt_timer_stop();
}

void display (void *cookie)

{
    int err;

    err = rt_sem_create(&display_sem,"dispsem",0,S_FIFO);

    if (err)
	{
        printf("latency: cannot create semaphore: %s\n",strerror(-err));
	return;
	}

    for (;;)
	{
	err = rt_sem_p(&display_sem,RT_TIME_INFINITE);

	if (err)
	    {
	    if (err != -EIDRM)
		printf("latency: failed to pend on semaphore, code %d\n",err);

	    return;
	    }

	printf("min = %Ld ns, max = %Ld ns, avg = %Ld ns, overrun = %d\n",
	       rt_timer_ticks2ns(minjitter),
	       rt_timer_ticks2ns(maxjitter),
	       rt_timer_ticks2ns(alljitter),
	       overrun);
	}
}

void cleanup_upon_sig(int sig __attribute__((unused)))

{
    int err = rt_sem_delete(&display_sem);

    if(err)
        fprintf(stderr, "Warning: could not delete semaphore: %s.\n",
                strerror(-err));
}

int main (int argc, char **argv)

{
    int err;

    signal(SIGINT, cleanup_upon_sig);
    signal(SIGTERM, cleanup_upon_sig);
    
    err = rt_task_create(&display_task,"display",0,2,0);

    if (err)
	{
	printf("latency: failed to create display task, code %d\n",err);
	return 0;
	}

    err = rt_task_start(&display_task,&display,NULL);

    if (err)
	{
	printf("latency: failed to start display task, code %d\n",err);
	return 0;
	}

    err = rt_task_create(&latency_task,"sampling",0,1,T_FPU);

    if (err)
	{
	printf("latency: failed to create latency task, code %d\n",err);
	return 0;
	}

    err = rt_task_start(&latency_task,&latency,NULL);

    if (err)
	{
	printf("latency: failed to start latency task, code %d\n",err);
	return 0;
	}

    pause();

    return 0;
}
