#!/bin/sh
awk '/^Revision/ { print $3 }' /proc/cpuinfo
