������ɾ�ļ���Linux����վ����
===

##[����վ�ű�](rm.sh)

    chmod 755 /bin/rm.sh

    echo "alias rm='/bin/rm.sh'" >> /etc/bashrc 

##[��ʱɾ������վ](clean_recycle.sh)
    
    crontab -e

    1 5 * * 0 sh /root/clean_recycle.sh


