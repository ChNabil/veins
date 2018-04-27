#!/bin/bash
if [ -z "$1" ]; then
  echo "Usage: $0 <OMNET configuration name> <OMNET INI file> [<first run number>]"
  exit
fi

CONFIG=$1
INI_FILE=$2

declare -i CURR_CONFIG
CURR_CONFIG=0
if [ "${#}" == "3" ]; then
 CURR_CONFIG=$3
fi 
 
# Queue variable. Currently no jobs are in the queue.
declare -i QUEUE
QUEUE=$(showq | grep "Total jobs" | sed -r "s/Total jobs:[ ]*([0-9]*)/\1/g")
UPPER_LIMIT_DAY=49
UPPER_LIMIT_NIGHT=49

# Job submission script
JOB_SUBMISSION_SCRIPT=$HOME/veins/job.moab
 
# load omnet, so we can run opp_run
module unload cae/omnet/4.6_inet3.0.0
module load mpi/openmpi/1.8-gnu-4.9
module load cae/omnet/4.6
#load python2
module load devel/python/2.7.10

if test -z "$OMNET_HOME"; then
 echo "ERROR: Failed to load module 'cae/omnet/4.6'."
 exit 101
fi
echo "OMNET_BIN_DIR = $OMNET_HOME"
 
# Use SED to get the amount of configs.
CONFIG_NUMBER=$(opp_run -x "${CONFIG}" -f "${INI_FILE}" | sed -r -n 's/Number of runs: ([0-9]*)/\1/p')
if [ -z ${CONFIG_NUMBER} ]; then
  echo "No configurations for ${CONFIG}!"
  exit
fi
 
echo "Number of Configs: ${CONFIG_NUMBER}"
 
# cd into workspace
WS_DIR="${WORK}/`whoami`-veins-sims-0"
mkdir -vp "$WS_DIR"
cd $WS_DIR
 
echo "Switched to ${WS_DIR}"
 
# Outer control loop
let "SLEEP_TIME=60"
while true 
do
 declare -i UPPER_LIMIT
 h=$(date +%H)
 if [ $h -lt 8 -o $h -gt 20 ]; then
  UPPER_LIMIT=$UPPER_LIMIT_NIGHT
 else
  UPPER_LIMIT=$UPPER_LIMIT_DAY
 fi
 
 # Determine how many jobs are in the queue at the moment
 QUEUE=$(showq -v | grep "`whoami`" | wc -l) 
 
 while [ $QUEUE -lt $UPPER_LIMIT ]
 do
  # Add jobs until we reach the upper limit.
  L=$CURR_CONFIG
 
  echo "msub $JOB_SUBMISSION_SCRIPT $CONFIG $L $L"
  JOBID=$(msub $JOB_SUBMISSION_SCRIPT $CONFIG $L)
  if [ 0 -eq $? ]; then
    let "QUEUE++"
    let "CURR_CONFIG+=1"
  fi

  echo "Added JOB: ${JOBID} for RUN ${L}"
  sleep 1
  if [ ${CONFIG_NUMBER} -le ${CURR_CONFIG} ]; then
    break
  fi
 done
 
 if [ ${CONFIG_NUMBER} -le ${CURR_CONFIG} ]; then
  # We are finished, jump out of the loop.
  echo "Finished running all jobs."
  break;
 fi 
 
 sleep $SLEEP_TIME; # Only check every minute if there is capacity available
done
