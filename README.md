# FTPClient
FTP Client for Linux base vsftpd server

### 一、 Linux安装FTP服务器  

##### 1. 安装  

- centos7环境下使用yum直接安装vsfftpd：  

`yum -y install vsftpd`  

- 设置开机启动:  

```C

[root@localhost vsftpd]# systemctl enable vsftpd

Created symlink from /etc/systemd/system/multi-user.target.wants/vsftpd.service to /usr/lib/systemd/system/vsftpd.service.

```

- 启动vsftp服务:  

`systemctl start vsftpd.service`  

- 重新启动vsftp服务:  

`systemctl restart vsftpd.service`  

- 打开防火墙，开放21端口（centos7.4以后默认关闭Firewall防火墙，关闭防火墙后就无需输入以下命令）  

```

firewall-cmd --zone=public --add-port=21/tcp --permanent

firewall-cmd --permanent --zone=public --add-service=ftp

firewall-cmd --reload



关闭防火墙：

https://blog.csdn.net/u013514928/article/details/80411110

```



- - -



##### 2. 配置  

- 安装完成后在/etc/vsftpd目录下会产生4个文件：  

```

[root@localhost ftp]# cd /etc/vsftpd

[root@localhost vsftpd]# ls

ftpusers  user_list  vsftpd.conf  vsftpd_conf_migrate.sh

```

- 为了使用ftp,先创建一个文件夹用来作为进行ftp服务的根目录:  

`mkdir -p /home/ftpfiles`  

- 然后利用该目录创建vsftpd的登录用户和主目录:  

`useradd -g root -d /home/ftpfiles -s /sbin/nologin myftp`  

- 注：表示新增一个myftp（用户名），且指定上传目录在/home/ftpfiles下,  

如果后期想变更此用户的上传目录到（/home/xxx），请使用下面的命令：  

`usermod -d /home/xxx myftp`  

- 修改用户密码:  

`passwd myftp`  

```

强密码：Ftp2233...

```

-  配置selinux 允许ftp访问home和外网访问（默认关闭centos7.4）：  

```

setsebool -P tftp_home_dir on

setsebool -P allow_ftpd_full_access on

```

- 设置权限（此用户可以访问整个主机目录，后面备注设置权限问题）：  

`chown -R myftp:root /home/ftpfiles`  

- 修改vsftp配置文件，禁用匿名登录：  

路径：/etc/vsftpd/vsftpd.conf  

`anonymous_enable=YES` 改为： `anonymous_enable=NO`  

```

// 修改 vi /etc/vsftpd/vsftpd.conf如下：（去掉注释！！！）  可选配置！！！！

chroot_local_user=NO

chroot_list_enable=YES

# (default follows)

chroot_list_file=/etc/vsftpd/chroot_list

```

编辑上面的内容  

```

第一行：chroot_local_user=NO

第二行：chroot_list_enable=YES　　//限制访问自身目录

第四行：编辑chroot_list根据第三行所指定的目录，找到chroot_list文件。（因主机不同，文件名也许略有不同）

编辑chroot_list，将受限制的用户添加进去，每个用户名一行，这里没有限制需求，就不编辑

```

说明：  

`chroot_local_user=NO`则所有用户不被限定在主目录内  

`chroot_list_enable=YES`表示要启用`chroot_list_file`  

因为`chroot_local_user=NO`，即全体用户都“不被限定在主目录内”,所以总是作为“例外列表”的chroot_list_file这时列出的是那些“会被限制在主目录下”的用户。  



- 配置完成后重启vsftp服务：  

`service vsftpd restart`  

此时利用之前的账户和密码就能够进行ftp连接了。  

```

账户：myftp

密码：Ftp2233...

```

- 无法登录问题  

```C

https://blog.csdn.net/zhouzhiwengang/article/details/88343904

Liunx搭建 FTP之后，本地机器无法连接，输入密码FTP服务器没有返回任何信息：

[root@codecage FTP]# touch /etc/vsftpd/chroot_list

[root@codecage FTP]# cat /etc/pam.d/vsftpd

#%PAM-1.0

session    optional     pam_keyinit.so    force revoke

auth       required	pam_listfile.so item=user sense=deny file=/etc/vsftpd/ftpusers onerr=succeed

#auth       required	pam_shells.so #注释这一行！！！

auth       include	password-auth

account    include	password-auth

session    required     pam_loginuid.so

session    include	password-auth

[root@codecage FTP]# systemctl restart vsftpd.service

[root@codecage FTP]# 

```

- 下载空字节文件的问题(即服务器读取服务器端文件失败)  

```C

[root@master ftpfiles]# ls -l

总用量 1556

-rw-r--r--. 1 myftp root 1592978 7月  13 21:01 1.mp3



上面的ftpfiles是连接ftp服务器后的根目录，ftpfile属于myftp用户所有，里面的任何一个文件的拥有者必须是myftp用户，否则myftp用户读取失败。  

useradd -g root -d /home/ftpfiles -s /sbin/nologin myftp

    

 综上，最好是先上传，再下载上传之后的文件！！！

```



- 常用配置如下:  

```

1）local_root=/ftpfile(当本地用户登入时，将被更换到定义的目录下，默认值为各用户的家目录)

2）anon_root=/ftpfile(使用匿名登入时，所登入的目录)

3）use_localtime=YES(默认是GMT时间，改成使用本机系统时间)

4）anonymous_enable=NO(不允许匿名用户登录)

5）local_enable=YES(允许本地用户登录)

6）write_enable=YES(本地用户可以在自己家目录中进行读写操作)

7）local_umask=022(本地用户新增档案时的umask值)

8）dirmessage_enable=YES(如果启动这个选项，那么使用者第一次进入一个目录时，会检查该目录下是否有.message这个档案，如果有，则会出现此档案的内容，通常这个档案会放置欢迎话语，或是对该目录的说明。默认值为开启)

9）xferlog_enable=YES(是否启用上传/下载日志记录。如果启用，则上传与下载的信息将被完整纪录在xferlog_file 所定义的档案中。预设为开启。)

10）connect_from_port_20=YES(指定FTP使用20端口进行数据传输，默认值为YES)

11）xferlog_std_format=YES(如果启用，则日志文件将会写成xferlog的标准格式)

12）ftpd_banner=Welcome to mmall FTP Server(这里用来定义欢迎话语的字符串)

13）chroot_local_user=NO(用于指定用户列表文件中的用户是否允许切换到上级目录)

14）chroot_list_enable=YES(设置是否启用chroot_list_file配置项指定的用户列表文件)

15）chroot_list_file=/etc/vsftpd/chroot_list(用于指定用户列表文件)

16）listen=YES(设置vsftpd服务器是否以standalone模式运行，以standalone模式运行是一种较好的方式，此时listen必须设置为YES，此为默认值。建议不要更改，有很多与服务器运行相关的配置命令，需要在此模式下才有效，若设置为NO，则vsftpd不是以独立的服务运行，要受到xinetd服务的管控，功能上会受到限制)

17）pam_service_name=vsftpd(虚拟用户使用PAM认证方式，这里是设置PAM使用的名称，默认即可，与/etc/pam.d/vsftpd对应) userlist_enable=YES(是否启用vsftpd.user_list文件，黑名单,白名单都可以

18)pasv_min_port=61001(被动模式使用端口范围最小值)

19)pasv_max_port=62000(被动模式使用端口范围最大值)

20)pasv_enable=YES(pasv_enable=YES/NO（YES）

若设置为YES，则使用PASV工作模式；若设置为NO，则使用PORT模式。默认值为YES，即使用PASV工作模式。

```

FTP协议有两种工作方式：PORT方式和PASV方式，中文意思为主动式和被动式。  



- - -



- ① PORT（主动）方式的连接过程是：客户端向服务器的FTP端口（默认是21）发送连接请求，服务器接受连接，建立一条命令链路。  

&emsp;当需要传送数据时，客户端在命令链路上用 PORT命令告诉服务器：“我打开了端口，你过来连接我”。于是服务器从20端口向客户端的端口发送连接请求，建立一条数据链路来传送数据。  

- ② PASV（被动）方式的连接过程是：客户端向服务器的FTP端口（默认是21）发送连接请求，服务器接受连接，建立一条命令链路。  

&emsp;当需要传送数据时，服务器在命令链路上用 PASV命令告诉客户端：“我打开了端口，你过来连接我”。于是客户端向服务器的端口发送连接请求，建立一条数据链路来传送数据。  

-  从上面可以看出，两种方式的命令链路连接方法是一样的，而数据链路的建立方法就完全不同。而FTP的复杂性就在于此。  



- - -

### 二、 命令码以及响应码  

```
ABOR 中断数据连接程序 
ACCT <account> 系统特权帐号 
ALLO <bytes> 为服务器上的文件存储器分配字节 
APPE <filename> 添加文件到服务器同名文件 
CDUP <dir path> 改变服务器上的父目录 
CWD <dir path> 改变服务器上的工作目录
DELE <filename> 删除服务器上的指定文件 
HELP <command> 返回指定命令信息 
LIST <name> 如果是文件名列出文件信息，如果是目录则列出文件列表 
MODE <mode> 传输模式（S=流模式，B=块模式，C=压缩模式） 
MKD <directory> 在服务器上建立指定目录 
NLST <directory> 列出指定目录内容 
NOOP 无动作，除了来自服务器上的承认 
PASS <password> 系统登录密码 
PASV 请求服务器等待数据连接 
PORT <address> IP 地址和两字节的端口 ID 
PWD 显示当前工作目录 
QUIT 从 FTP 服务器上退出登录 
REIN 重新初始化登录状态连接 
REST <offset> 由特定偏移量重启文件传递 
RETR <filename> 从服务器上找回（复制）文件 
RMD <directory> 在服务器上删除指定目录 
RNFR <old path> 对旧路径重命名 
RNTO <new path> 对新路径重命名 
SITE <params> 由服务器提供的站点特殊参数 
SMNT <pathname> 挂载指定文件结构 
STAT <directory> 在当前程序或目录上返回信息 
STOR <filename> 储存（复制）文件到服务器上 
STOU <filename> 储存文件到服务器名称上
STRU <type> 数据结构（F=文件，R=记录，P=页面） 
SYST 返回服务器使用的操作系统 TYPE <data type> 数据类型（A=ASCII，E=EBCDIC，I=binary） USER <username> 系统登录的用户名
```

```
110 新文件指示器上的重启标记
120 服务器准备就绪的时间（分钟数） 
125 打开数据连接，开始传输 
150 打开连接 
200 成功
202 命令没有执行 
211 系统状态回复
212 目录状态回复 
213 文件状态回复 
214 帮助信息回复 
215 系统类型回复 
220 服务就绪
221 退出网络 
225 打开数据连接
226 结束数据连接
227 进入被动模式（IP 地址、ID 端口） 
230 登录因特网 
250 文件行为完成 
257 路径名建立
331 要求密码 
332 要求帐号 
350 文件行为暂停 
421 服务关闭 
425 无法打开数据连接
426 结束连接 
450 文件不可用 
451 遇到本地错误 
452 磁盘空间不足
500 无效命令 
501 错误参数 
502 命令没有执行
503 错误指令序列
504 无效命令参数 
530 未登录网络 
532 存储文件需要帐号 
550 文件不可用 
551 不知道的页类型
552 超过存储分配 
553 文件名不允许
```


