#include <sys/types.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <getopt.h>
#include "latency.h"

#define HISTOGRAM_CELLS 200
int histogram_size = HISTOGRAM_CELLS;
unsigned long *histogram_avg = 0, *histogram_max = 0, *histogram_min = 0;

int do_histogram = 0, finished = 0;
int bucketsize = 1000;		/* bucketsize */

static inline void add_histogram (long *histogram, long addval)
{
    int inabs = (addval >= 0 ? addval : -addval) / bucketsize;
    histogram[inabs < histogram_size ? inabs : histogram_size-1]++;
}

void dump_histogram (long *histogram, char* kind)
{
    int n, total_hits = 0;
  
    fprintf(stderr,"HSH-%s| latency (*%d ns) | num occurrences\n", kind, bucketsize); 

    for (n = 0; n < histogram_size; n++) {

	long hits = histogram[n];
	
	if (hits) {
	    fprintf(stderr,"HSD-%s|%5d-%5d|%ld\n", kind, n, n+1, hits);
	    total_hits += hits;
	}
    }
    fprintf(stderr,"HSTotal|%d\n",total_hits);
}

void dump_histograms (void)
{
    dump_histogram (histogram_min, "min");
    dump_histogram (histogram_max, "max");
    dump_histogram (histogram_avg, "avg");
}

void cleanup_upon_sig(int sig __attribute__((unused)))
{
    finished = 1;

    if (do_histogram)
	dump_histograms();

    if (histogram_avg)	free(histogram_avg);
    if (histogram_max)	free(histogram_max);
    if (histogram_min)	free(histogram_min);

    fflush(stdout);	/* finish histogram before unloading modules */
    exit(0);
}

int main (int argc, char **argv)
{
    const char *const communication_channel = "/dev/rtp0";
    int n = 0, c, fd, err, data_lines = 21;
    struct rtai_latency_stat s;
    time_t start;
    ssize_t sz;
    int test_duration = 0, quiet = 0;

    while ((c = getopt(argc,argv,"hl:T:qH:B:")) != EOF)
	switch (c)
	    {
	    case 'h':
		/*./klatency --h[istogram] */
		do_histogram = 1;
		break;
		
	    case 'H':

		histogram_size = atoi(optarg);
		break;

	    case 'B':

		bucketsize = atoi(optarg);
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
		
		fprintf(stderr, "usage: klatency [options]\n"
			"  [-h]				# prints histogram of latencies\n"
			"  [-H <histogram-size>]	# default = 200, increase if your last bucket is full\n"
			"  [-B <bucket-size>]		# default = 1000ns, decrease for more resolution\n"
			"  [-l <data-lines per header>]	# default = 21, 0 supresses header\n"
			"  [-T <seconds_to_test>]	# default = 0, so ^C to end\n"
			"  [-q]				# supresses RTD, RTH lines if -T is used\n");
		exit(2);
	    }

    if (!test_duration && quiet)
	{
	fprintf(stderr, "klatency: -q only works if -T has been given.\n");
	quiet = 0;
	}

    signal(SIGINT, cleanup_upon_sig);
    signal(SIGTERM, cleanup_upon_sig);
    signal(SIGHUP, cleanup_upon_sig);
    signal(SIGALRM, cleanup_upon_sig);

    setlinebuf(stdout);

    histogram_max = calloc(histogram_size,sizeof(long));
    histogram_min = calloc(histogram_size,sizeof(long));
    histogram_avg = calloc(histogram_size,sizeof(long));

    if (!(histogram_avg && histogram_max && histogram_min))
        cleanup_upon_sig(0);
    
    fd = open(communication_channel, O_RDWR);
    
    if (fd < 0)
        {
        fprintf(stderr, "klatency: open(%s): %m\n", communication_channel);
        exit(1);
        }

    time(&start);

    for (;;)
        {
        sz = read(fd,&s,sizeof(s));

        if (!sz)
            break;
        
        if (sz != sizeof(s))
            {
            perror("read");
            exit(1);
            }

	if (do_histogram && !finished)
	  {
	    add_histogram(histogram_max, s.maxjitter);
	    add_histogram(histogram_avg, s.avgjitter);
	    add_histogram(histogram_min, s.minjitter);
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

	    printf("RTD|%12d|%12d|%12d|%12d\n",
		   s.minjitter,
		   s.avgjitter,
		   s.maxjitter,
		   s.overrun);
	    }
	}
    if ((err = close(fd))) {
	perror("close");
	exit(1);
    }
    return 0;
}

