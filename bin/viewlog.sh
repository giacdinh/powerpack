#!/bin/bash
LOGDATE=`date -I |awk -F '-' '{print $1$2$3}'`
vi /mnt/sysdata/log/$LOGDATE'_log'
