#!/bin/bash
LOGDATE=`date -I |awk -F '-' '{print $1$2$3}'`
tail -f /mnt/sysdata/log/$LOGDATE'_log'
