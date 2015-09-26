#!/bin/bash

#initialize variables
OPT_D=0
OPT_U=0
OPT_E=0

#if no arguments then print usage and exit
if [ $# -eq 0 ]; then
  echo "Usage: .hw3.sh [-hudea] file_name"
  exit 1
fi

#Process all the options
while getopts ":haude" opt; do
 case $opt in
   h)
     echo "Usage: .hw3.sh [-hudea] file_name"
	#Should exit except look for -a
        while getopts ":a" opti; do
         case $opti in
          a)
            echo "Written by Alok Arya Fall 2015"
            exit 1
            ;;
         esac
        done
     exit 1
     ;;
   a)
     echo "Written by Alok Arya Fall 2015"
        #Should exit except look for -h
        while getopts ":h" opti; do
         case $opti in
          h)
            echo "Usage: .hw3.sh [-hudea] file_name"
            exit 1
            ;;
         esac
        done
     exit 1
     ;;
   u)
     #set variable to tell option -u is set
     OPT_U=1
     ;;
   d)
     #set variable to tell option -d is set
     OPT_D=1
     ;;
   e)
     #set variable to tell option -d is set
     OPT_E=1
     ;;
   \?)
     echo "Unrecognized argument: $OPTARG"
     ;;
 esac
done

shift $((OPTIND-1))

#get the file name
FILE=$1

#retrieve only filename to check if arguments also present 
#split the sentence into an array
temp=$FILE
FILENAME=( $temp )

#check if file exists
if [ ! -f ${FILENAME[0]} ]; then
    echo "The file $FILE does not exist."
    exit 1
fi

#check if the file is an executable
if [ ! -x ${FILENAME[0]} ] ; then
    echo "The file $FILE is not an executable"
    exit 1
fi

#default log file name
logfilename="hw3.log"

#generate unique logfile name as per option u or d or both 
if [ $OPT_U -eq 1 ]; then
    logfilename=hw3.`mktemp XXXXXX`
    if [ $OPT_D -eq 1 ]; then
         now=$(date +"%Y%d%m"T"%H%M%S"Z"") 
         logfilename=$logfilename.$now.log
    else
         logfilename=$logfilename.log
    fi
else
   if [ $OPT_D -eq 1 ]; then
     now=$(date +"%Y%d%m"T"%H%M%S"Z"")
     logfilename=hw3.$now.log
   fi
fi

#run the command and output to log file if E option is set
if [ $OPT_E -eq 1 ]; then
     $FILE > $logfilename 2> $logfilename
else
     $FILE > $logfilename
fi

echo "file name is $logfilename"


