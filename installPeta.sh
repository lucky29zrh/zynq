 vim installPeta.sh
 chmod u+x installPeta.sh
 ./installPeta.sh 


-------
#! /bin/bash

#阶段0：检测环境及更新
echo "------detect starting-------"
sudo apt-get update 
#sudo apt-get upgrade
is_finsh_detect=$?
if [ $is_finsh_detect -eq 0 ]
then
echo "------detect ending---------"
else
    echo '++++++++++detect failed+++++++++'
fi


#阶段1：
echo "------install first stage start-------"
sudo apt-get install build-essential 
sudo apt-get  install  vim
sudo apt-get install tofrodos  iproute2 gawk
sudo apt-get install gcc git make
sudo apt-get install net-tools  libncurses5-dev  tftpd
sudo apt-get install zlib1g-dev libssl-dev  flex bison libselinux1
is_finsh_first=$?
if [ $is_finsh_first -eq 0 ]
then
echo "------first stage end--------"
else
    echo '++++++++++first stage install failed+++++++++'
fi



#阶段2：
#2.1 petalinux 2016 版本安装额外的库
sudo apt-get install lib32z1 lib32ncurses5 libbz2-1.0:i386 lib32stdc++6
sudo apt-get install libssl-dev
sudo apt-get install libncurses5 libncurses5-dev
sudo apt-get install libc6:i386
sudo apt-get install libstdc++6:i386
sudo apt-get install zlib1g:i386
sudo apt-get install wget diffstat chrpath socat xterm autoconf \
libtool unzip texinfo zlib1g-dev gcc-multilib build-essential libsdl1.2-dev libglib2.0-dev

#2.2 petalinux 2017.2所需依赖库安装
sudo apt-get install tofrodos  iproute2 gawk
sudo apt-get install gcc git make

sudo apt-get install xvfb

sudo apt-get install net-tools  libncurses5-dev  tftpd
sudo apt-get install zlib1g-dev zlib1g-dev:i386 libssl-dev  flex bison libselinux1
sudo apt-get install gnupg wget diffstat chrpath socat xterm
sudo apt-get install autoconf libtool tar unzip texinfo zlib1g-dev \
gcc-multilib build-essential libsdl1.2-dev libglib2.0-dev
sudo apt-get install screen pax gzip tar

is_finsh_second=$?
if [ $is_finsh_second -eq 0 ]
then
echo "------second stage end--------"
else
    echo '++++++++++second stage install failed+++++++++'
fi


#阶段3：安装tftp相关的功能及改进
#不安装petalinux时出现警告，提示No tftp server found 需要安装下面3个库，记得root权限
sudo  apt-get install tftp-hpa tftpd-hpa xinetd

sudo mkdir /tftpboot
sudo chmod -R 777 /tftpboot

# sudo gedit /etc/default/tftpd-hpa
##在文件中增加以下内容
## /etc/default/tftpd-hpa
#TFTP_USERNAME="tftp"
#TFTP_ADDRESS="0.0.0.0:69"
#TFTP_DIRECTORY="/tftpboot"    #服务器目录,需要设置权限为777
#TFTP_OPTIONS="-l-c -s"
##保存并退出
#sudo  service tftpd-hpa restart
#netstat -an | more | grep udp
##看到有如下输出，即表示tftp安装成功
##udp        0      0 0.0.0.0:69              0.0.0.0:*

sudo dpkg-reconfigure dash
#并在弹出界面选“否”来禁用dash，选择bash
ls -al /bin/sh
is_finsh_third=$?
if [ $is_finsh_third -eq 0 ]
then
echo "------third stage end--------"
else
    echo '++++++++++third stage install failed+++++++++'
fi


#阶段4：
#之后便是安装petalinux了，祝您成功。
#4.1 安装
#直接在home目录下建（普通权限）
#mkdir -p opt/pkg/petalinux1604
#千万不要使用root或者sudo
#./petalinux-v2016.4-final-installer.run /home/pp/opt/pkg/petalinux1604
#安装过程中总共只有3个y,要小心。 
#每当选择一个y,小心按q表示关闭查看license文件，此处不要按Enter键终止安装！ 

#4.2 优化改进
#关闭统计信息
#petalinux-util --webtalk off
#验证并设置自启动
#gedit ~/.bashrc
#在末尾添加：
#source  /home/pp/opt/pkg/petalinux1604/settings.sh

