#!/usr/bin/expect -f

set PASSPH "support\$01"
send_user "\n"
stty -echo

spawn rsync -apv -e ssh "bacson@$1:/mnt/sysdata/log/*" .
expect "password:"
send "$PASSPH\n"
expect "#"

