#include <stdlib.h>
#include <stdbool.h>
#include "pthread.h"




pthread_mutex_t mMutex;
pthread_cond_t cond;

typedef struct element {
   void* data;
   size_t size;
   struct element* next;
} element_t;

typedef struct {
   element_t* head;
   element_t* tail;
} queue_t;

void queue_init(queue_t* queue)
{
   queue->head = NULL;
   queue->tail = NULL;
      //initialize mutex

   if(pthread_mutex_init(&mMutex, NULL) !=0){

      perror("mutex init error");
      exit (EXIT_FAILURE);
   }

   if(pthread_cond_init(&cond, NULL) !=0){

      perror("con var init error");
      exit (EXIT_FAILURE);
   }     

   return;
}

void queue_destroy(queue_t* queue)
{
      //destroy mutex
   if(   pthread_mutex_destroy(&mMutex) !=0){

      perror("mutex destroy error");
      exit (EXIT_FAILURE);
   }

   if(pthread_cond_destroy(&cond) !=0){

      perror("cond var error");
      exit(EXIT_FAILURE);
   }     

   if (queue != NULL)
   {
      while(queue->head != NULL)
      {
	      element_t* temp = queue->head->next;
	      free(queue->head);
	      queue->head = temp;
      }
   }
   return;
}

bool push(queue_t* queue, void* data, size_t size)
{
   pthread_mutex_lock(&mMutex);
   element_t* next = malloc(sizeof(element_t));
   next->data = data;
   next->size = size;
   next->next = NULL;

   if (queue->head == NULL)
   {
      queue->head = next;
   }
   if (queue->tail != NULL)
   {
      queue->tail->next = next;
   }
   queue->tail = next;

   pthread_cond_signal(&cond);
   pthread_mutex_unlock(&mMutex);

   return true;
}

bool pop(queue_t* queue, void** data, size_t* size)
{
   pthread_mutex_lock(&mMutex);
   if(queue->head ==NULL){

         pthread_cond_wait(&cond, &mMutex);
   }

   element_t* top = queue->head;
   if (queue->head != NULL)
   {
      if (queue->tail == top)
      {
         queue->tail = NULL;
      }
      queue->head = top->next;
      *data = top->data;
      *size = top->size;
      free(top);
      pthread_mutex_unlock(&mMutex);
      return true;
   }
   return false;
}
