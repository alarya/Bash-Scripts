#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <linux/limits.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <sys/inotify.h>

#define EVENT_SIZE ( sizeof (struct inotify_event) )
#define BUF_LEN ( 1024 * ( EVENT_SIZE + 16 ) )

void print_help(char *filename)
{
	printf("Usage.....\n");


}

char* getFullPath(char* fileName, char *cwd)
{
	int size = strlen(cwd) + 1;
	size += strlen(fileName);
	char* fullPath = (char *)malloc(size);
	strcat(fullPath,cwd);
	strcat(fullPath,"/");
	strcat(fullPath,fileName);
	return fullPath;
}

int main(int argc, char* argv[])
{
	char* dir;
	struct stat fileStat;
	DIR *dir_handle;
	struct dirent *ent;

	//----------check if argument passed--------------------
	if(argv[1] != NULL)
	{
		dir = argv[1] ;
		printf("Directory passed: %s\n",dir);
	}
	else
	{
		
		size_t sizeOfPath = PATH_MAX ;
		char path[sizeOfPath];
	  
		if(getcwd(path,sizeOfPath) == NULL){
			printf("Could not get current working directory \n");
	    }
	    else
	  		printf("Assume the current working directory: %s \n",path);

	  dir = path ;

	}
 

	//---------check if the directory exists----------------
	if(lstat(dir, &fileStat) == -1){
		if(errno == ENOENT){
			printf("The path provided does not exist \n");
			return -1;
		}
	}
	else
	{
		if(!S_ISDIR(fileStat.st_mode)){
			printf("The path provided is not a directory \n");
			return -1;
		}
	}

	//--------List all files in the directory-----------------
	if( (dir_handle = opendir (dir)) != NULL ){
		int count = 1 ;
		struct stat fileStat;
		printf("The files in the directory are :- \n");
		while( (ent = readdir(dir_handle)) != NULL )
		{	
			/* Print only regular files */
			char *fullPath = getFullPath(ent->d_name, dir);

			lstat(fullPath, &fileStat);

			if( S_ISREG(fileStat.st_mode) || S_ISDIR(fileStat.st_mode) )
				if( !( strcmp(ent->d_name,".") == 0 || strcmp(ent->d_name,"..") == 0 ) )
				printf("%d. %s\n", count++, ent->d_name);

		}
	}
	else
	{
		/*could not open directory*/
		printf("Could not open directory..");
		return -1;
	}

	
	//------Input file from user and add a watch -------------
	char fileName[256];
	printf("Select a file: ");
	scanf("%s", fileName);
	char* fullPath = getFullPath(fileName,dir);
	int fd = open(fullPath, O_RDONLY);
	if( fd == -1)
	{
		printf("Specified file not present..\n");
		closedir(dir_handle);
		return -1;
	}
    close(fd);

	printf("Selected file: %s \n",fullPath);

	//------Listen for events on selected file-------------
	int inotifyFd = inotify_init();
	if( inotifyFd == -1 )
	{
		printf("Could not open a inotify descriptor.. \n");
		closedir(dir_handle);
		return -1;
	}
	int wd = inotify_add_watch( inotifyFd, fullPath, IN_ALL_EVENTS );
	if( wd == -1 )
	{
		printf("Could not open a watch  descriptor... \n");
		closedir(dir_handle);
		return -1;
	}
	printf("Listening on the file.... \n");
	char buff[BUF_LEN];
	ssize_t numRead;
	char *p;
	struct inotify_event *event;
	while(1)
	{
		numRead = read( inotifyFd, buff, BUF_LEN );

		if( numRead == 0 )
		{
			closedir(dir_handle);
			return -1;
		}
		
		for( p = buff; p < buff + numRead ; )
		{
			event = (struct inotify_event *) p;

			if( event->mask & IN_ACCESS )
			{
			 	if(event->len)
			 		printf("File %s was read. \n", event->name);
			 	else
			 		printf("File was read. \n");
			}
			if( event->mask & IN_MODIFY )
			{
				if(event->len)
					printf("File %s was modified. \n", event->name);
				else
			 		printf("File was modified. \n");
			}

			p += EVENT_SIZE + event->len ;

		}
	}

	(void) inotify_rm_watch(inotifyFd, wd);
	(void) close(inotifyFd);

   	return 0;
}