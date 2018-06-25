#include <stdio.h>
#include <math.h>
#include <pthread.h>
#include <stdlib.h>

double successes;
double failures;
int iterations;
int ready;
double previousPi;
double delta;
pthread_barrier_t *barrier;
pthread_barrier_t *barrier2;
pthread_mutex_t *rand_lock;
pthread_mutex_t *succ_lock;
pthread_mutex_t *fail_lock;
int threadAmount;

void srand(unsigned int seed);

double getRand()
{
  pthread_mutex_lock(rand_lock);
  double top = (double) rand();
  double bot = (double) RAND_MAX;
  pthread_mutex_unlock(rand_lock);
  return top / bot;
}

double getDistance(double xVal, double yVal)
{
  double A = pow(0.5 - xVal, 2);
  double B = pow(0.5 - yVal, 2);

  return (double)sqrt(A + B);
  
}

void *workerBody()
{
  while(1)
    {
          int counter = 0;
          while (counter < iterations)
            {
              double xPoint = getRand();
              double yPoint = getRand();
              if (getDistance(xPoint, yPoint) <= 0.5)
                {
		  pthread_mutex_lock(succ_lock);
	          successes++;
		  pthread_mutex_unlock(succ_lock);
	        }
              else
	        {
		  pthread_mutex_lock(fail_lock);
	          failures++;
		  pthread_mutex_unlock(fail_lock);
	        }
              counter++;
            }
      pthread_barrier_wait(barrier);
      pthread_barrier_wait(barrier2);
    }
}

void *controlBody()
{
  while (1)
    {
      pthread_barrier_wait(barrier);
	  double total = successes + failures;
	  double area = successes / total;
	  double piGuess = area * 4.0;

	  printf("%lf\n", piGuess);
          if (fabs(previousPi - piGuess) < delta)
	  {
	    pthread_exit(NULL);
	  }
	  else
	  {
	    previousPi = piGuess;
	  }
	  pthread_barrier_wait(barrier2);
    }
}


int main(int argc, char *argv[])
{
  threadAmount = atoi(argv[1]);
  iterations = atoi(argv[2]);
  delta = atof(argv[3]);
  ready = 0;
  int i = 0;
  successes = 0.0;
  failures = 0.0;
  previousPi = 100;
  pthread_t *threadID[threadAmount + 1];
  int barrierReturn;

  succ_lock = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
  pthread_mutex_init(succ_lock, NULL);
  fail_lock = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
  pthread_mutex_init(fail_lock, NULL);
  rand_lock = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
  pthread_mutex_init(rand_lock, NULL);

  barrier = (pthread_barrier_t *) malloc(sizeof(pthread_barrier_t));
  barrier2 = (pthread_barrier_t *) malloc(sizeof(pthread_barrier_t));
  barrierReturn = pthread_barrier_init(barrier, NULL, threadAmount + 1);

  if (barrierReturn)
    {
      printf("Error with Barrier");
      exit(1);
    }

  barrierReturn = pthread_barrier_init(barrier2, NULL, threadAmount + 1);

  if (barrierReturn)
    {
      printf("Error with Barrier");
      exit(1);
    }
  
  for (i = 0; i < threadAmount; i++)
    {
      threadID[i] = (pthread_t *) malloc(sizeof(pthread_t));
      if (pthread_create(threadID[i], NULL, workerBody, NULL))
	{
	  printf("Error creating thread");
	  exit(-1);
	}
    }
  
  threadID[threadAmount] = (pthread_t *) malloc(sizeof(pthread_t));
  if (pthread_create(threadID[threadAmount], NULL, controlBody , NULL))
	{
	  printf("Error creating thread");
	  exit(-1);
	}
  
  pthread_join(*threadID[threadAmount], NULL);
  return 0;
}
