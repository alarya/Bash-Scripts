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
#include <utime.h>

#define BUF_SIZE 1024


int cloneFile(char* source, char* destination, int destExists, int overwrite)
{
    
    int destFd;
    int sourceFd;
    char buf[BUF_SIZE];
    struct stat fileStat;
    ssize_t numRead;
    mode_t filePerms;
    struct utimbuf lastModifiedTime;

    //-------get source permissions ----------------------//
    sourceFd = open( source, O_RDONLY );
    if( stat( source, &fileStat ) == -1 )
    {
        perror("Error ");
    }
    filePerms = fileStat.st_mode ;

    //-------create destination file-------------------//
    if(!destExists)
    {
        printf("Destination does not exist...Creating \n");
        destFd = open( destination, O_CREAT, filePerms );
        if(destFd == -1)
        {
            perror("Error: ");
            close(sourceFd);
            return -1;
        }
    }
    else
    {  
        printf("Destination exist..\n") ;
        if(overwrite)
        {
            printf("Overwriting...\n");
            destFd = open( destination, O_RDWR | O_TRUNC, filePerms );
            if(destFd == -1)
            {
                perror("Error: ");
                close(sourceFd);
                return -1;
            }

        }
        else
        {
            printf("Use option -f to overwrite \n");
            close(sourceFd);
            return -1;
        }
    }

    destFd = open( destination, O_RDWR | O_TRUNC ) ;

    //-----copy file data ------------------------------//
    while( ( numRead = read( sourceFd, buf, BUF_SIZE) ) > 0 )
    {
        if( write( destFd, buf, numRead) != numRead )
            printf("couldn't write whole buffer");
    }

    //------change permissions of destination with  source ----//
    if( fchmod( destFd, filePerms ) == -1 )
    {
        perror("Could not change permissions: ");
    }

    //------Updating owner of destination---------------//
    if( fchown(destFd, fileStat.st_uid, fileStat.st_gid) == -1 )
    {
        perror("Could not change owner: ");
    } 
 
    //-----update Last modified timestamp --------------//
    lastModifiedTime.actime = fileStat.st_atime;
    lastModifiedTime.modtime = fileStat.st_mtime;
    if( utime(destination, &lastModifiedTime) == -1 )
    {
        perror("Could not update timestamp: ");
    }

    close(destFd);
    close(sourceFd);

    return 0;
}


int main(int argc, char* argv[])
{
	int opt_f = 0;
	int opt;
	char* source;
	char* destination;
    struct stat fileStat;
    int fd;
    int destExists = 0;

	//----------process all the options------------------//
    while( ( opt = getopt( argc, argv, "f" ) ) != -1 ) 
    {
      switch(opt)
      {
        case 'f':
        	opt_f = 1 ;
 	   		break;
 	    default :
 	      /*do nothing */
 	    	break;
      }
    }

    //-----------check if arguments present --------------//
    if( argv[optind] == NULL )
    {
    	printf("Source file not specified \n");
    	return -1;
    }
    else
    	source = argv[optind];

    if( argv[++optind] == NULL )
    {
    	printf("Destination file not specified \n");
    	return -1;
    }
    else 
    	destination = argv[optind];


    //-----check if source file exists and it's type ----//
    if( lstat( source, &fileStat ) == -1 )
    {
        if( errno == ENOENT )
        {
            printf("The source file provided does not exist... \n");
            return -1;
        }
    }

    //-----check if destination exists ----------------//
    fd = open( destination, O_RDONLY );
    if( fd == -1 )
    {
        destExists = 0;       
    }
    else
    {
        destExists = 1; 
        close(fd);
    }  
    
    //------clone file based on it's type ---------------//
    if( S_ISREG( fileStat.st_mode ) )
    {
        printf("Cloning a regular file \n");
        int result = cloneFile( source, destination, destExists, opt_f );
        if(result == -1)
            printf("Clone failed \n");
        else
            printf("Clone succeeded \n");
    }
    else if( S_ISDIR( fileStat.st_mode ) )
    {
        printf("Cloning a dir \n");
        int result = cloneFile( source, destination, destExists, opt_f );
        if(result == -1)
            printf("Clone failed \n");
        else
            printf("Clone succeeded \n");
    }
    else if( S_ISLNK( fileStat.st_mode ) )
    {
        printf("Cloning a symbolic link \n");
        int result = cloneFile( source, destination, destExists, opt_f );
        if(result == -1)
            printf("Clone failed \n");
        else
            printf("Clone succeeded \n");
    }
    else
    {
        printf("cannot clone this type of file \n");
    }

    return 0;    
}