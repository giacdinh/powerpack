#!/bin/bash
getdat=`date -I |cut -c 1-4,6,7,9-12`
dat=$getdat'_log'
tail -f /mnt/sysdata/log/$dat
