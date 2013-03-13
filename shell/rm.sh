#!/bin/sh  

dirpath=/data/Recycle # 选择回收站所在的分区目录 
now=`date +%Y%m%d_%H_%M_%S_`   
delfile=`basename $1`
filename=${now}${delfile}  # 给已经删除的文件加一个时间前缀来标识删除时的精准时间 
if [ ! -d ${dirpath} ];then  
    /bin/mkdir -p ${dirpath}  
fi  
/bin/mv $1 ${dirpath}/${filename}  
