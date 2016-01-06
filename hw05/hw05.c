#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <pwd.h>


void print_help(char* filename)
{
         printf("Usage: %s [-toe] file_name \n",filename);
         printf("Options:- \n");
         printf("t - Displays the type of file specified \n");
         printf("o - Displays the owner Id and Owner username of specified file \n");
         printf("e - Appends the program environment to the specified file \n");
}

int main(int argc, char* argv[])
{
    int opt_t = 0;
    int opt_o = 0;
    int opt_e = 0;   
    char* file_name;
    int opt ;
    int fd;
    struct stat fileStat;
    char** env;
    int fd2;
    struct passwd *pw;
    extern char** environ;

    //process all the options
    while( ( opt = getopt(argc, argv, "toe") ) != -1 ) 
    {
      switch(opt)
      {
        case 't':
          opt_t = 1 ;
 	   		  break;
	  	  case 'o':
			    opt_o = 1 ;
			    break;
	   	  case 'e':
			    opt_e = 1 ;
      		break;
        default:
          print_help(argv[0]) ;
          return -1 ;
      }
    }

    

    //---CHECK: File name passed------------------
    if(argv[optind] == NULL)
      {
        print_help(argv[0]);
        return -1;
      }   
     
    //----Print file name ------------------------
    file_name = argv[optind] ;
    printf("The file passed is: %s \n", file_name);

    //-----Check if file can be opened------------
    fd = open(file_name, O_RDONLY);
    if(fd == -1)
    {
      printf("Could not open file..\n");
    }
    
    //-----OPTION E: write environment to file----------------------------------//
    if(opt_e==1)
    {

      int fd1 = open(file_name, O_APPEND | O_WRONLY);
      if(fd1 == -1)
          printf("Could not append environment to the file..\n");
      else
      {          
        char* thisEnv;
        int buffLength;
        for(env = environ; *env != 0 ; env++)
        {
          thisEnv = strcat(*env,"\n");
          buffLength = strlen(thisEnv);
          if(write(fd1, thisEnv, buffLength) < 0)
          {
            fprintf(stderr, "Error in writing content at %s: %s\n",file_name,strerror(errno));
            return -1;
          }
        }   
      printf("Appended the environment to file..\n");
      }
      close(fd1);  
    }
    
    //----Read file status----------------------------------------------//
    if( (stat(file_name, &fileStat)) < 0 )
    {
      printf("Could not read file status...\n");
    }

    //----OPTION T: Show type of file----------------------------------------//
    struct stat fileStatLnk;
    lstat(file_name, &fileStatLnk);
    if(opt_t == 1 )
    {
      //check if symbolic link     
      if(S_ISLNK(fileStatLnk.st_mode))
      {
        printf("This is a symbolic link \n" );
      }
      else
      {
        if(S_ISREG(fileStat.st_mode))
          printf("This is a regular file\n");
        else if(S_ISDIR(fileStat.st_mode))
          printf("This is a directory\n");
        else if(S_ISCHR(fileStat.st_mode))
          printf("This is a character device\n");
        else if(S_ISBLK(fileStat.st_mode))
          printf("This is a block device\n");
        else if(S_ISFIFO(fileStat.st_mode))
          printf("This is a named pipe\n");
        else if(S_ISSOCK(fileStat.st_mode))
          printf("This is a socket\n");
      }
    }

    //-----PRINT Size of File-------------------------------------------------//
    printf("File Size: %d bytes \n", (int)fileStatLnk.st_size);

    
    //-----OPTION O: Print owner of the file----------------------------------//
    if(opt_o == 1)
    {
      uid_t owner = fileStat.st_uid;
      
      pw = getpwuid(owner);
      if(pw)
      {
        printf("Owner User Name: ");
        puts (pw->pw_name);
      }
      else
      {
        printf("Could not find User Name \n");
        printf("OwnerId: %u\n", owner);
      }

    }

    close(fd);
    return 0;

}

