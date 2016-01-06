#include <fcntl.h>
#include <semaphore.h>
#include <errno.h>
#include <signal.h>
#include <sys/shm.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

#define BUFFER 4096

//------------print help ------------//
void help(char *file)
{
	printf("Usage: %s \n",file);
	return ;
}

//---------Shared memory structure ------------//
struct shared_mem
{
	int pid;
	int dataWritten;  
	char text[BUFFER];
	int noOfProcesses;
	int read;
	
};

//---------write to shared memeory--------------//
void writeToSharedMem(void *args, sem_t *sem_id)
{
	char* line=NULL;
  	size_t read=0;
  	size_t capacity=0;
  	struct shared_mem *ipc_mem = (struct shared_mem *)args;

	do{
		read = getline( &line, &capacity, stdin);

	  	//---------exit condition ----------//
	  	if( strcmp( line, "exit\n" ) == 0 )
	  	{
			break;
	  	}

	  	if(read !=-1 )
	  	{
  			while( ipc_mem->dataWritten == 1 )
  			 {
 				sleep(1);
  		     }

  			sem_wait(sem_id);
      		ipc_mem->pid=getpid();
  			strncpy(ipc_mem->text,line,BUFFER);
  			ipc_mem->dataWritten=1;
  			sem_post(sem_id);
  	}

  	}while(1);
}

//-----------------read from shared memory ----------//
void readFromSharedMem(void *args, sem_t *sem_id)
{

	struct shared_mem *ipc_mem = (struct shared_mem *)args;

	pid_t myPid = 0;

	while(1)
	{
		if( ipc_mem->dataWritten == 1 && 
			ipc_mem->pid != getppid() && 
			ipc_mem->pid != myPid 
		  )
			{

				sem_wait(sem_id);
				printf("Process %d sent : %s",(int)ipc_mem->pid,ipc_mem->text );
				ipc_mem->read++;
				if( ipc_mem->read == ipc_mem->noOfProcesses-1 )
				{
					ipc_mem->dataWritten=0;
					ipc_mem->read=0;
				}

			myPid = ipc_mem->pid;
			sem_post(sem_id);

		}

		if( ipc_mem->dataWritten == 0)
		{
			myPid = 0 ;
		}

	}
	
}

int main(int argc,char* argv[])
{

	//-----check for options -------------//
	if( argc > 1 )
	{

		if( strcmp(argv[1],"-h") == 0)
		{
			help(argv[0]);
		}

		exit(EXIT_SUCCESS);
	}

  	//---declarations---------//
  	struct shared_mem *ipc_mem;
  	void *shared_memory = (void *)0;
    int shmid=0;
    sem_t *sem_id;
    pid_t child;

    //----create semaphore-----//
    sem_id = sem_open("semaphore", O_CREAT, 0600, 1);

    if(sem_id == SEM_FAILED) 
    {
        perror("Error");
        exit(EXIT_FAILURE);
    }

    //------create or open shared memory -------------//
    shmid = shmget( (key_t)1234 , sizeof(struct shared_mem), 0666 |IPC_CREAT );

    if(shmid==-1)
    {

    	perror("Error"); 
    	exit(EXIT_FAILURE);
    }

    shared_memory = shmat( shmid, (void *)0, 0);

    if (shared_memory == (void *) - 1) 
    {

		perror("shmat failed");
		exit(EXIT_FAILURE);
	}

	ipc_mem = (struct shared_mem *)shared_memory;
	
    sem_wait(sem_id);
    ipc_mem->noOfProcesses++;
    sem_post(sem_id);

    if( ( child = fork() ) == -1 )
    {

		perror("Fork:");
		exit(EXIT_FAILURE);

	}
	else if( child == 0)           //parent
	{ 

		readFromSharedMem( ipc_mem, sem_id );
		
	}
	else                   //child
	{

		writeToSharedMem( ipc_mem, sem_id );
		kill( child, SIGKILL );
	}


	sem_wait(sem_id);
	ipc_mem->noOfProcesses--;
	int num_process = ipc_mem->noOfProcesses;
	sem_post(sem_id);

	if ( shmdt( shared_memory ) == -1 )
	{
			perror("shmdt(): ");
			exit(EXIT_FAILURE);
	}


	if(num_process==0)
	{

		if ( shmctl( shmid, IPC_RMID, 0 ) == -1 )
		{
			perror("shmctl()");
			exit(EXIT_FAILURE);
		}
	}
	
	exit(EXIT_SUCCESS);
}