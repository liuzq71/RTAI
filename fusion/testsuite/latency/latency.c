#include <sys/mman.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <getopt.h>
#include <time.h>
#include <rtai/task.h>
#include <rtai/timer.h>
#include <rtai/sem.h>

RT_TASK latency_task, display_task;

RT_SEM display_sem;

#define ONE_BILLION  1000000000
#define TEN_MILLION    10000000

long minjitter, maxjitter, avgjitter, overrun;
long gminjitter = TEN_MILLION,
     gmaxjitter = -TEN_MILLION,
     gavgjitter = 0,
     goverrun = 0;

int sampling_period = 0;
int test_duration = 0;	/* sec of testing, via -T <sec>, 0 is inf */
int data_lines = 21;	/* data lines per header line, -l <lines> to change */
int quiet = 0;		/* suppress printing of RTH, RTD lines when -T given */

time_t test_start, test_end;	/* report test duration */
int test_loops = 0;		/* outer loop count */

#define MEASURE_PERIOD ONE_BILLION
#define SAMPLE_COUNT (MEASURE_PERIOD / sampling_period)

#define HISTOGRAM_CELLS 100
int histogram_size = HISTOGRAM_CELLS;
unsigned long *histogram_avg = NULL,
              *histogram_max = NULL,
              *histogram_min = NULL;

int do_histogram = 0, finished = 0;
int bucketsize = 1000;	/* default = 1000ns, -B <size> to override */

static inline void add_histogram (long *histogram, long addval)
{
    /* us steps */
    long inabs = rt_timer_ticks2ns(addval >= 0 ? addval : -addval) / bucketsize;
    histogram[inabs < histogram_size ? inabs : histogram_size-1]++;
}

void latency (void *cookie)
{
    long minj = TEN_MILLION, maxj = -TEN_MILLION, dt, sumj;
    int err, count, nsamples;
    RTIME expected, period;

    err = rt_timer_start(TM_ONESHOT);

    if (err)
      {
	fprintf(stderr,"latency: cannot start timer, code %d\n",err);
	return;
      }

    overrun = 0;
    nsamples = ONE_BILLION / sampling_period;
    period = rt_timer_ns2ticks(sampling_period);
    expected = rt_timer_tsc();
    err = rt_task_set_periodic(NULL,TM_NOW,sampling_period);

    if (err)
      {
	fprintf(stderr,"latency: failed to set periodic, code %d\n",err);
	return;
      }

    for (;;)
      {
 	test_loops++;

	for (count = sumj = 0; count < nsamples; count++)
	  {
	    expected += period;
	    err = rt_task_wait_period();
	    
	    if (err)
	      {
		if (err != -ETIMEDOUT)
		  rt_task_delete(NULL); /* Timer stopped. */
		
		overrun++;
	      }
	    
	    dt = (long)(rt_timer_tsc() - expected);
	    if (dt > maxj) maxj = dt;
	    if (dt < minj) minj = dt;
	    sumj += dt;

	    if (do_histogram && !finished)
	      add_histogram(histogram_avg, dt);
	  }
	
	if (do_histogram && !finished)
	  {
	    add_histogram(histogram_max, maxj);
	    add_histogram(histogram_min, minj);
	    minj = TEN_MILLION;
	    maxj = -TEN_MILLION;
	    overrun = 0;
	  }
    
	minjitter = rt_timer_ticks2ns(minj);
	maxjitter = rt_timer_ticks2ns(maxj);
	avgjitter = rt_timer_ticks2ns(sumj / nsamples);
	rt_sem_v(&display_sem);
      }
}

void display (void *cookie)
{
    int err, n = 0;
    time_t start;

    err = rt_sem_create(&display_sem,"dispsem",0,S_FIFO);

    if (err)
	{
        fprintf(stderr,"latency: cannot create semaphore: %s\n",strerror(-err));
	return;
	}

    time(&start);

    if (quiet)
      fprintf(stderr, "running quietly for %d seconds\n", test_duration);

    for (;;)
	{
	err = rt_sem_p(&display_sem,TM_INFINITE);

	if (err)
	    {
	    if (err != -EIDRM)
		fprintf(stderr,"latency: failed to pend on semaphore, code %d\n",err);

	    rt_task_delete(NULL);
	    }

	if (!quiet)
	    {
	    if (data_lines && (n++ % data_lines)==0)
	        {
		time_t now, dt;
		time(&now);
		dt = now - start;
		printf("RTH|%12s|%12s|%12s|%12s|     %.2ld:%.2ld:%.2ld\n",
		       "lat min","lat avg","lat max","overrun",
		       dt / 3600,(dt / 60) % 60,dt % 60);
	        }
	    
	    printf("RTD|%12ld|%12ld|%12ld|%12ld\n",
		   minjitter,
		   avgjitter,
		   maxjitter,
		   overrun);
	    }

	/* update global jitters */
	if (minjitter < gminjitter) gminjitter = minjitter;
	if (maxjitter > gmaxjitter) gmaxjitter = maxjitter;
	gavgjitter += avgjitter;
	if (do_histogram)
	    goverrun += overrun;
	else
	    goverrun = overrun;
	}
}

void dump_histogram (long *histogram, char* kind)
{
    int n, total_hits = 0;
    long avg = 0;		/* used to sum hits 1st */
    double variance = 0;

    fprintf(stderr,"\nHSH-%s| latency range (usecs) | number of samples\n", kind);

    for (n = 0; n < histogram_size; n++)
        {
	long hits = histogram[n];

	if (hits) {
	    fprintf(stderr,"HSD-%s|%3d-%3d|%ld\n",kind, n, n+1, hits);
	    total_hits += hits;
	    avg += n * hits;
	}
    }

    avg /= total_hits;	/* compute avg, reuse variable */

    for (n = 0; n < histogram_size; n++)
      {
	long hits = histogram[n];
	if (hits)
	    variance += hits * (n-avg) * (n-avg);
      }

    /* compute std-deviation (unbiased form) */
    variance /= total_hits - 1;
    // variance = sqrt(variance);

    fprintf(stderr,"HSH-%s-Samples:\t%d\n", kind, total_hits);
    fprintf(stderr,"HSH-%s-Average:\t%ld\n", kind, avg);
    // fprintf(stderr,"HSH-%s-StdDev:\t%f\n", kind, variance);
    fprintf(stderr,"HSH-%s-Variance:\t%f\n", kind, variance);
}

void dump_histograms (void)
{
  dump_histogram (histogram_min, "min");
  dump_histogram (histogram_avg, "avg");
  dump_histogram (histogram_max, "max");
}

void cleanup_upon_sig(int sig __attribute__((unused)))
{
    time_t actual_duration;

    if (finished)
	return;

    finished = 1;
    rt_timer_stop();
    rt_sem_delete(&display_sem);

    if (do_histogram)
	dump_histograms();

    time(&test_end);
    actual_duration = test_end - test_start;
    if (!test_duration) test_duration = actual_duration;
    gavgjitter /= (test_loops ?: 2)-1;

    printf("---|------------|------------|------------|------------|     %.2ld:%.2ld:%.2ld/%.2d:%.2d:%.2d\n",
	   actual_duration / 3600,(actual_duration / 60) % 60,actual_duration % 60,
	   test_duration / 3600,(test_duration / 60) % 60,test_duration % 60);

    printf("RTS|%12ld|%12ld|%12ld|%12ld\n",
	   gminjitter,
	   gavgjitter,
	   gmaxjitter,
	   goverrun);

    if (histogram_avg)	free(histogram_avg);
    if (histogram_max)	free(histogram_max);
    if (histogram_min)	free(histogram_min);

    exit(0);
}

int main (int argc, char **argv)
{
    int c, err;

    while ((c = getopt(argc,argv,"hp:l:T:qH:B:")) != EOF)
	switch (c)
	    {
	    case 'h':
		/* ./latency --h[istogram] */
		do_histogram = 1;
		break;

	    case 'H':

		histogram_size = atoi(optarg);
		break;

	    case 'B':

		bucketsize = atoi(optarg);
		break;

	    case 'p':

		sampling_period = atoi(optarg) * 1000;
		break;

	    case 'l':

		data_lines = atoi(optarg);
		break;
		
	    case 'T':

		test_duration = atoi(optarg);
		alarm(test_duration);
		break;

	    case 'q':

	        quiet = 1;
		break;
		
	    default:
		
		fprintf(stderr, "usage: latency [options]\n"
			"  [-h]				# print histogram of scheduling latency\n"
			"  [-H <histogram-size>]	# default = 200, increase if your last bucket is full\n"
			"  [-B <bucket-size>]		# default = 1000ns, decrease for more resolution\n"
			"  [-p <period_us>]		# sampling period\n"
			"  [-l <data-lines per header>]	# default=21, 0 to supress headers\n"
			"  [-T <test_duration_seconds>]	# default=0, so ^C to end\n"
			"  [-q]				# supresses RTD, RTH lines if -T is used\n");
		exit(2);
	    }

    if (!test_duration && quiet)
       {
       fprintf(stderr, "latency: -q only works if -T has been given.\n");
       quiet = 0;
       }

    time(&test_start);

    histogram_avg = calloc(histogram_size, sizeof(long));
    histogram_max = calloc(histogram_size, sizeof(long));
    histogram_min = calloc(histogram_size, sizeof(long));

    if (!(histogram_avg && histogram_max && histogram_min)) 
        cleanup_upon_sig(0);

    if (sampling_period == 0)
	sampling_period = 100000; /* ns */

    signal(SIGINT, cleanup_upon_sig);
    signal(SIGTERM, cleanup_upon_sig);
    signal(SIGHUP, cleanup_upon_sig);
    signal(SIGALRM, cleanup_upon_sig);

    setlinebuf(stdout);

    printf("== Sampling period: %d us\n",sampling_period / 1000);

    mlockall(MCL_CURRENT|MCL_FUTURE);

    err = rt_task_create(&display_task,"display",0,98,0);

    if (err)
	{
	fprintf(stderr,"latency: failed to create display task, code %d\n",err);
	return 0;
	}

    err = rt_task_start(&display_task,&display,NULL);

    if (err)
	{
	fprintf(stderr,"latency: failed to start display task, code %d\n",err);
	return 0;
	}

    err = rt_task_create(&latency_task,"sampling",0,99,T_FPU);

    if (err)
	{
	fprintf(stderr,"latency: failed to create latency task, code %d\n",err);
	return 0;
	}

    err = rt_task_start(&latency_task,&latency,NULL);

    if (err)
	{
	fprintf(stderr,"latency: failed to start latency task, code %d\n",err);
	return 0;
	}

    pause();

    return 0;
}

