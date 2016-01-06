#include "stdio.h"
#include "stdlib.h"
#include "pthread.h"
#include "string.h"
#include "unistd.h"
#include "queue.h"

pthread_mutex_t myMutex;
pthread_cond_t condc;

void* producer(void* arg)
{
	queue_t* queue = (queue_t*)arg;
	size_t i = 0;
	for(i = 0; i < 100000; ++i)
	{
		push(queue, (void*)i, sizeof(i));
		if ((i % 10000) == 0) { sleep(1); }
	}


	return NULL;
}

void* consumer(void* arg)
{
	queue_t* queue = (queue_t*)arg;
	size_t i = 0;
	for(i = 0; i < 100000; ++i)
	{
		size_t i = 0;
		size_t i_size = 0;

		if (pop(queue, (void**)&i, (void*)&i_size))
		{
			/*assert(i_size == sizeof(i));*/
			fprintf(stderr, "got %d from queue\n",(int) i);
		}

	}

	return NULL;
}

int main(int argc, char* argv[])
{
	pthread_t pro_th, con_th;
	queue_t queue;
	queue_init(&queue);


	//producer
	pthread_create (&pro_th, NULL, (void *) &producer, (void *) &queue);

	//consumer
	pthread_create (&con_th, NULL, (void *) &consumer, (void *) &queue);

	pthread_join(con_th, NULL);
  	pthread_join(pro_th, NULL);

	queue_destroy(&queue);

	return EXIT_SUCCESS;
}
