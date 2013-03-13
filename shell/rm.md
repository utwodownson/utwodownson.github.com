避免误删文件：Linux回收站机制
===

##[回收站脚本](rm.sh)

    chmod 755 /bin/rm.sh

    echo "alias rm='/bin/rm.sh'" >> /etc/bashrc 

##[定时删除回收站](clean_recycle.sh)
    
    crontab -e

    1 5 * * 0 sh /root/clean_recycle.sh


