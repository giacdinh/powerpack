#!/bin/bash
LOGDATE=`date -I |awk -F '-' '{print $1$2$3}'`
tail -f /mnt/sysdata/log/$LOGDATE'_log'
<<<<<<< HEAD
=======

>>>>>>> d72f0b562a0fc54091eab1b9386d8e9070bf83ee
