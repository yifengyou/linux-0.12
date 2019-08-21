#!/bin/bash

if [ ! -e linux-0.12-080324.zip ];then
    wget -c http://oldlinux.org/Linux.old/bochs/linux-0.12-080324.zip
    if [ $? -eq 0 ];then
   	 unzip linux-0.12-080324.zip
    fi
fi


cd linux-0.12-080324

sed  -i 's/vga_update_interval: 300000/#vga_update_interval: 300000/g'  bochsrc-0.12-hd.bxrc

sed  -i 's/vga_update_interval: 300000/#vga_update_interval: 300000/g'  bochsrc-0.12-fd.bxrc

bochs -qf bochsrc-0.12-hd.bxrc
