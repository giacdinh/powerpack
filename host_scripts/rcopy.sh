#!/usr/bin/expect -f

set PASSPH "support\$01"
send_user "\n"
stty -echo

spawn rsync -apv -e ssh $1 "bacson@192.168.1.227:~/."
expect "password:"
send "$PASSPH\n"
expect "#"

