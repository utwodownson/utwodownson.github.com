#!/bin/sh  

# ����վ
# chmod 755 /bin/rm.sh
# echo "alias rm='/bin/rm.sh'" >> /etc/bashrc

dirpath=/data/Recycle # ѡ�����վ���ڵķ���Ŀ¼ 
now=`date +%Y%m%d_%H_%M_%S_`   
delfile=`basename $1`
filename=${now}${delfile}  # ���Ѿ�ɾ�����ļ���һ��ʱ��ǰ׺����ʶɾ��ʱ�ľ�׼ʱ�� 
if [ ! -d ${dirpath} ];then  
    /bin/mkdir -p ${dirpath}  
fi  
/bin/mv $1 ${dirpath}/${filename}  
