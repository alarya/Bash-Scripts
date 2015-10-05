#!/bin/bash

#initialize variables
#Each variable represent if an option was set(0) or unset(1)
OPT_H=0
OPT_F=0
OPT_C=0
OPT_E=0
OPT_S=0

#Number of ARGS
NUM_ARGS=$#

#Set Script Name variable & formatting variables
SCRIPT=`basename ${BASH_SOURCE[0]}`
NORM=`tput sgr0`
BOLD=`tput bold`

#--------------------------FUNCTIONS---------------------------------------------#
#usage function
function usage {
  echo  "Usage for the script: $BOLD$SCRIPT$NORM"
  echo  "Command: ./$SCRIPT -[h] [-f][arg] [-c] [-e] [-s][arg] PID"
  echo  "${BOLD}Options:${NORM} "
  echo  "    h: Prints the usage of this script"
  echo  "    f: Print PIDS which have the specified file opened"
  echo  "    c: Print the command-line used to start the PID"
  echo  "    e: Print the path to the executable that was used to start the PID"
  echo  "    s: Print status information matching the supplied pattern"

}

#print PIDs of the process using the specified file 
function printPIDsWithFileOpen {
   # $1 - File
   
   #store output of finding open files in /proc/*/fd/* in a temp file
   ls -lR /proc/*/fd/* 2> /dev/null | grep "$1" > tmp.txt
   
   echo "The process Ids which have opened file $1:"
   #recurse through each line and extract the process Ids
   while IFS='' read -r line || [[ -n "$line" ]]; do
      lineTmp=( $line )
      echo ${lineTmp[8]} | awk -F'/' '{print $3}'
   done < "tmp.txt"

   #remove the temp file
   rm tmp.txt
}

#print the command that was used to start the PID
function printCommandUsed {
   # $1 - PID
   echo "Process was started as: $(cat /proc/$1/cmdline | xargs -0)"
}

#print executable info related to the PID specified {path of executable used PID}
function printExecInfo {
   # $1 - PID
   #echo "Path used to start Process $1: " `readlink -e /proc/$1/cwd/`
   if readlink -e /proc/$1/exe > /dev/null; then
       echo "Process was started by: $(readlink -e /proc/$1/exe)"
   else
       echo "Permission denied to access executable path information"
   fi
}

#print status information of the PID related to the pattern provided
function printStatusInfo {
   # $1 - PID, $2 - pattern	
   echo "Status Info matching pattern $2"
   grep "$2" "/proc/$1/status"
}


#-----------Process all the options-------------------------
while getopts ":hf:ces:" opt; do
 case $opt in
   h)
     #show usage and exit regardless of anything else
     OPT_H=1
     usage 
     exit 1
     ;;
   f)
     #set variable to tell option -f is set and it's argument
     OPT_F=1
     if [ -z $OPTARG ]; then
        echo "Error: No argument was provided for the option -f"
        exit 1
     else
        F_ARG=$OPTARG
        printPIDsWithFileOpen $F_ARG
        exit 1
     fi
     ;;
   c)
     #set variable to tell option -u is set
     OPT_C=1
     ;;
   e)
     #set variable to tell option -d is set
     OPT_E=1
     ;;
   s)
     #set variable to tell option -d is set and it's argument
     OPT_S=1
     if [ -z $OPTARG ]; then
        echo "Error: No argument was provided for the option -s"
        exit 1
     else
        S_ARG=$OPTARG
     fi
     ;;
   \?)
     echo "Unrecognized Option: -$OPTARG"
     usage
     exit 1
     ;;
 esac
done

#move to arguments of the script

shift $((OPTIND-1))

#If no arguments: print usage and exit
if [ $NUM_ARGS -eq 0 ]; then
     usage
     exit 1
fi

#get the process ID argument
PID=$1

#check if PID available to process
if [ -z $1 ]; then
        echo "No PID available"
        exit 1
fi

#Check if PID provided is an existing process
if [ ! -d /proc/$PID ]; then
    echo "Process $PID is not running"
    exit 1
else
    echo "Process $PID is running"
fi

#If -c option set: print the command used to start the PID
if [ $OPT_C -eq 1 ]; then
    printCommandUsed $PID
fi

#If -e option set: print executable info related to the PID {path of executeable used PID}
if [ $OPT_E -eq 1 ]; then
    printExecInfo $PID
fi

#if -s option set: print status information of PID based on the argument provided
if [ $OPT_S -eq 1 ]; then
    printStatusInfo $PID $S_ARG
fi
