#!/bin/bash
# Program:
#	This program is designed to get the host information.
# History:
# 2015/09/30/ Fang Wei First release
PATH=/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/sbin:~/bin
export PATH
#get cpu information
echo "获取CPU信息" >> GetLocalInfo
cat /proc/cpuinfo >> GetLocalInfo
#get disk information
echo "获取磁盘信息" >> GetLocalInfo
cat /proc/partitions >> GetLocalInfo
#get memory information
echo "获取内存信息" >> GetLocalInfo
cat /proc/meminfo >> GetLocalInfo
#查看当前操作系统内核信息
echo "获取当前操作系统内核信息" >> GetLocalInfo
uname -a >> GetLocalInfo
