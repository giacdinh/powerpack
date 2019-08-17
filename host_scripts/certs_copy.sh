#!/usr/bin/expect -f

set PASSPH "support\$01"
send_user "\n"
stty -echo

spawn rsync -apv -e ssh unit-*.pem "bacson@$1:/mnt/sysdata/certs"
expect "password:"
send "$PASSPH\n"
expect "#"

