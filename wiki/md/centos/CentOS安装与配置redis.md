---
title: 在CentOS环境下安装配置redis服务
date: 2020-07-08 09:20:45
categories: redis
index_img: /img/articles/redis.jpg
tags:
	- CentOS
	- 安装配置
	- redis
---

## 下载redis安装包

从redis官方网站下载redis的安装包：https://redis.io/download



## 安装redis

下载获取redis的安装包`redis-5.0.8.tar.gz`。

将redis安装包拷贝到`/opt`目录下，然后解压安装包，解压后，会生成目录`redis-5.0.8`。

~~~shell
[root@jamza-vm opt]# tar -zxvf redis-5.0.8.tar.gz
[root@jamza-vm opt]# ll
total 2184
drwxrwxr-x  7 root root    4096 Jun  9 18:19 redis-5.0.8
-rwxr--r--  1 root root 2217666 Jun 23 14:10 redis-5.0.8.tar.gz

~~~

然后安装gcc依赖，若环境已安装gcc工具链，可跳过。

~~~shell
[root@jamza-vm opt]# yum install gcc gcc-c++
~~~

进入到目录`redis-5.0.8`下，首先编译。

~~~shell
[root@jamza-vm opt]# cd redis-5.0.8/
[root@jamza-vm redis-5.0.8]# make
~~~

上一步编译无错后，执行安装，可通过`PREFIX`选项指定安装到路径`/usr/local/redis`。

~~~shell
[root@jamza-vm redis-6.0.5]# make PREFIX=/usr/local/redis install
~~~

或者执行`make install`命令，会将`src`下面的几个命令复制到`/usr/local/bin/`下

~~~shell
[root@jamza-vm redis-5.0.8]# make install
cd src && make install
make[1]: Entering directory `/opt/redis-5.0.8/src'
    CC Makefile.dep
make[1]: Leaving directory `/opt/redis-5.0.8/src'
make[1]: Entering directory `/opt/redis-5.0.8/src'

Hint: It's a good idea to run 'make test' ;)

    INSTALL install
    INSTALL install
    INSTALL install
    INSTALL install
    INSTALL install
make[1]: Leaving directory `/opt/redis-5.0.8/src'
[root@jamza-vm opt]# redis-
redis-benchmark  redis-check-aof  redis-check-rdb  redis-cli        redis-sentinel   redis-server
[root@jamza-vm opt]#
[root@jamza-vm opt]# which redis-server
/usr/local/bin/redis-server
[root@jamza-vm opt]#

~~~



## 配置redis

redis的配置文件为`redis.conf`，就在`/opt/redis-5.0.8`目录下。

配置的主要内容包括：

1. 配置允许所有`ip`都可以访问`redis`,在`bind 127.0.0.1`前加“#”将其注释掉
2. 默认为保护模式，把 `protected-mode yes` 改为 `protected-mode no`
3. 默认为不守护进程模式，把`daemonize no`改为`daemonize yes`
4. 将`requirepass foobared`前的`#`去掉，密码改为你想要设置的密码，比如`requirepass PassWord`
5. 设置`maxclients`为 `128`， 表示同一时间最大客户端连接数为`128`，默认无限制，`Redis` 可以同时打开的客户端连接数为` Redis `进程可以打开的最大文件描述符数，如果设置 `maxclients 0`，表示不作限制。当客户端连接数到达限制时，`Redis `会关闭新的连接并向客户端返回 `max number of clients reached` 错误信息
6. 将`stop-writes-on-bgsave-error`改为`no`
7. `maxmemory`设置为`2147483648`，即2GB，指定` Redis `最大内存限制，`Redis` 在启动时会把数据加载到内存中，达到最大内存后，`Redis `会先尝试清除已到期或即将到期的 `Key`，当此方法处理后，仍然到达最大内存设置，将无法再进行写入操作，但仍然可以进行读取操作。在内存不够用的情况下，可以适当考虑开启`Redis`的`vm`机制，`Redis` 新的` vm `机制，会把 `Key` 存放内存，`Value `会存放在 `swap` 区



## 启动redis

指定`redis.conf`文件启动：

~~~shell
[root@jamza-vm redis-5.0.8]# redis-server ./redis.conf
23181:C 23 Jun 2020 15:08:21.685 # oO0OoO0OoO0Oo Redis is starting oO0OoO0OoO0Oo
23181:C 23 Jun 2020 15:08:21.685 # Redis version=5.0.8, bits=64, commit=00000000, modified=0, pid=23181, just started
23181:C 23 Jun 2020 15:08:21.685 # Configuration loaded
[root@jamza-vm redis-5.0.8]#
[root@jamza-vm redis-5.0.8]#
[root@jamza-vm redis-5.0.8]# ps -ef | grep redis
root     23182     1  0 15:08 ?        00:00:00 redis-server *:6379
root     23189 14276  0 15:08 pts/1    00:00:00 grep --color=auto redis
[root@jamza-vm redis-5.0.8]#
~~~



## 关闭redis

使用`kill`指令直接杀死redis进程：

~~~shell
[root@jamza-vm redis-5.0.8]# ps -ef | grep redis
root     23182     1  0 15:08 ?        00:00:00 redis-server *:6379
root     23189 14276  0 15:08 pts/1    00:00:00 grep --color=auto redis
[root@jamza-vm redis-5.0.8]#
[root@jamza-vm redis-5.0.8]# kill -9 23182
~~~



## 查看是否开启所有IP地址访问

使用命令`netstat -lunpt`查看端口的IP地址访问类型，如果`redis`端口号`6379`前面显示的是*或者0.0.0.0则说明客户端可以访问了，如果是127.0.0.1，表示只能本机访问，配置文件中没将其注释掉。

~~~shell
[root@jamza-vm redis-5.0.8]# netstat -lunpt
Active Internet connections (only servers)
Proto Recv-Q Send-Q Local Address           Foreign Address         State       PID/Program name
tcp        0      0 0.0.0.0:22              0.0.0.0:*               LISTEN      1173/sshd
tcp        0      0 127.0.0.1:25            0.0.0.0:*               LISTEN      1136/master
tcp        0      0 127.0.0.1:6010          0.0.0.0:*               LISTEN      14221/sshd: jamza@p
tcp        0      0 127.0.0.1:6011          0.0.0.0:*               LISTEN      14265/sshd: root@pt
tcp        0      0 127.0.0.1:6012          0.0.0.0:*               LISTEN      14312/sshd: jamza@p
tcp        0      0 127.0.0.1:6013          0.0.0.0:*               LISTEN      14356/sshd: root@pt
tcp        0      0 0.0.0.0:445             0.0.0.0:*               LISTEN      1929/smbd
tcp        0      0 0.0.0.0:6379            0.0.0.0:*               LISTEN      23182/redis-server
~~~



## 设置redis开机启动

在`/etc`目录下新建`redis`目录：

~~~shell
[root@CentOS7 redis-5.0.2]# mkdir -pv /etc/redis
~~~

将配置文件复制进`/etc/redis/`下,并命名为`6379.conf`

~~~shell
[root@CentOS7 redis]# cp /opt/redis-5.0.2/redis.conf /etc/redis/6379.conf
~~~

创建服务，用`service`来管理服务的时候，是在`/etc/init.d/`目录中创建一个脚本文件,来管理服务的启动和停止。

在`systemctl`中，也类似，文件目录有所不同，在`/etc/systemd/system`目录下创建一个脚本文件`redis.service`，里面的内容如下：

~~~
[Unit]
Description=Redis
After=network.target

[Service]
Type=forking
ExecStart=/opt/redis-5.0.2/src/redis-server /etc/redis/6379.conf
ExecStop=/opt/redis-5.0.2/src/redis-cli -h 127.0.0.1 -p 6379 shutdown

RestartSec=10
Restart=always
 
[Install]
WantedBy=multi-user.target
~~~

刷新配置，让`systemctl`识别得到`redis`

~~~shell
[root@CentOS7 redis]# systemctl daemon-reload
~~~

 启动关闭`redis`

~~~shell
[root@CentOS7 system]# systemctl start redis    #启动redis服务
[root@CentOS7 system]# systemctl stop redis     #关闭redis服务
~~~

设置`redis`开机启动

~~~shell
[root@CentOS7 system]# systemctl enable redis
~~~



## 编译错误处理

在新版本`redis 6.01`及以上版本的编译过程中，可能会出现如下类似的错误：

~~~
server.c:5142:19: error: ‘struct redisServer’ has no member named ‘cluster_enabled’
         if (server.cluster_enabled) {
                   ^
server.c:5150:19: error: ‘struct redisServer’ has no member named ‘ipfd_count’
         if (server.ipfd_count > 0 || server.tlsfd_count > 0)
                   ^
server.c:5150:44: error: ‘struct redisServer’ has no member named ‘tlsfd_count’
         if (server.ipfd_count > 0 || server.tlsfd_count > 0)
                                            ^
server.c:5152:19: error: ‘struct redisServer’ has no member named ‘sofd’
         if (server.sofd > 0)
                   ^
~~~

原因是`gcc`的版本问题，查看`gcc`的版本是否在`5.3`以上，`centos7`默认是`4.8.5`

~~~shell
[root@jamza-vm redis-6.0.5]# gcc -v
Using built-in specs.
COLLECT_GCC=gcc
COLLECT_LTO_WRAPPER=/usr/libexec/gcc/x86_64-redhat-linux/4.8.5/lto-wrapper
Target: x86_64-redhat-linux
Configured with: ../configure --prefix=/usr --mandir=/usr/share/man --infodir=/usr/share/info --with-bugurl=http://bugzilla.redhat.com/bugzilla --enable-bootstrap --enable-shared --enable-threads=posix --enable-checking=release --with-system-zlib --enable-__cxa_atexit --disable-libunwind-exceptions --enable-gnu-unique-object --enable-linker-build-id --with-linker-hash-style=gnu --enable-languages=c,c++,objc,obj-c++,java,fortran,ada,go,lto --enable-plugin --enable-initfini-array --disable-libgcj --with-isl=/builddir/build/BUILD/gcc-4.8.5-20150702/obj-x86_64-redhat-linux/isl-install --with-cloog=/builddir/build/BUILD/gcc-4.8.5-20150702/obj-x86_64-redhat-linux/cloog-install --enable-gnu-indirect-function --with-tune=generic --with-arch_32=x86-64 --build=x86_64-redhat-linux
Thread model: posix
gcc version 4.8.5 20150623 (Red Hat 4.8.5-39) (GCC)
~~~

解决办法为升级`gcc`版本：

~~~
#升级到 5.3及以上版本
yum -y install centos-release-scl
yum -y install devtoolset-9-gcc devtoolset-9-gcc-c++ devtoolset-9-binutils

scl enable devtoolset-9 bash
~~~

注意：`scl`命令启用只是临时的，退出`xshell`或者重启就会恢复到原来的gcc版本。

如果要长期生效的话，执行如下指令：

~~~
echo "source /opt/rh/devtoolset-9/enable" >>/etc/profile
~~~