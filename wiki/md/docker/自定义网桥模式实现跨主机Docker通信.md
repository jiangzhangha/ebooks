---
title: 自定义网桥模式实现跨主机Docker通信
date: 2020-07-08 13:43:45
categories: docker
index_img: /img/articles/docker.jpg
tags:
	- docker
	- 跨主机通信
	- 自定义网桥
---

## 概念

不同主机上的Docker容器通过某些方法可实现跨主机通信，但是一般不同主机上的Docker容器内的IP地址不是处于同一个网段，但是可以通过自定义网桥的方式，将不同主机的自定义网桥设定为同一个网段，并桥接主机上的网卡，实现跨主机通信。


## 地址划分

### 主控：

ip地址：192.168.84.83

网卡：eth20

网关：192.168.84.1

自定义网桥名称：br1

自定义网桥地址：192.168.84.1

主控上容器ip地址池：192.168.84.224/28

### 线卡

ip地址：192.168.84.85

网卡：eth20

网关：192.168.84.1

自定义网桥名称：br1

自定义网桥地址：192.168.84.1

主控上容器ip地址池：192.168.84.240/28

## 主控设置

在设置之前，主控中没有br网桥，ip路由显示192.168.84.0/24网段路由通过网卡eth20端：

~~~shell
[root@jamza_vm_master_litepaas master]# ip addr
1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN qlen 1
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
    inet 127.0.0.1/8 scope host lo
       valid_lft forever preferred_lft forever
    inet6 ::1/128 scope host
       valid_lft forever preferred_lft forever
2: eth0: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc pfifo_fast state UP qlen 1000
    link/ether 52:54:83:aa:bb:00 brd ff:ff:ff:ff:ff:ff
    inet 192.168.83.83/24 brd 192.168.83.255 scope global eth0
       valid_lft forever preferred_lft forever
    inet6 fe80::5054:83ff:feaa:bb00/64 scope link
       valid_lft forever preferred_lft forever
3: eth20: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc pfifo_fast state UP qlen 1000
    link/ether 52:54:83:aa:bb:20 brd ff:ff:ff:ff:ff:ff
    inet 192.168.84.83/24 brd 192.168.84.255 scope global eth20
       valid_lft forever preferred_lft forever
    inet6 fe80::5054:83ff:feaa:bb20/64 scope link
       valid_lft forever preferred_lft forever
4: eth21: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc pfifo_fast state UP qlen 1000
    link/ether 52:54:83:aa:bb:21 brd ff:ff:ff:ff:ff:ff
    inet 192.168.85.83/24 brd 192.168.85.255 scope global eth21
       valid_lft forever preferred_lft forever
    inet6 fe80::5054:83ff:feaa:bb21/64 scope link
       valid_lft forever preferred_lft forever
5: docker_gwbridge: <NO-CARRIER,BROADCAST,MULTICAST,UP> mtu 1500 qdisc noqueue state DOWN
    link/ether 02:42:0c:8b:82:cc brd ff:ff:ff:ff:ff:ff
    inet 172.18.0.1/20 scope global docker_gwbridge
       valid_lft forever preferred_lft forever
6: docker0: <NO-CARRIER,BROADCAST,MULTICAST,UP> mtu 1500 qdisc noqueue state DOWN
    link/ether 02:42:0f:34:eb:4b brd ff:ff:ff:ff:ff:ff
    inet 10.76.84.11/24 scope global docker0
       valid_lft forever preferred_lft forever
    inet6 fe80::42:fff:fe34:eb4b/64 scope link
       valid_lft forever preferred_lft forever
[root@jamza_vm_master_litepaas master]#
[root@jamza_vm_master_litepaas master]# ip route
default via 192.168.83.1 dev eth0
10.76.84.0/24 dev docker0 proto kernel scope link src 10.76.84.11
169.254.0.0/16 dev eth0 scope link metric 1002
169.254.0.0/16 dev eth20 scope link metric 1003
169.254.0.0/16 dev eth21 scope link metric 1004
172.18.0.0/20 dev docker_gwbridge proto kernel scope link src 172.18.0.1
192.168.83.0/24 dev eth0 proto kernel scope link src 192.168.83.83
192.168.84.0/24 dev eth20 proto kernel scope link src 192.168.84.83
192.168.85.0/24 dev eth21 proto kernel scope link src 192.168.85.83
[root@jamza_vm_master_litepaas master]#
~~~

在主控上创建一个网桥，为网桥分配ip地址，并将自定义的网桥桥接本地的网卡eth20：

~~~shell
[root@jamza_vm_master_litepaas master]#
[root@jamza_vm_master_litepaas master]# brctl addbr br1
[root@jamza_vm_master_litepaas master]#
[root@jamza_vm_master_litepaas master]# ifconfig br1 192.168.84.1 netmask 255.255.255.0
[root@jamza_vm_master_litepaas master]#
[root@jamza_vm_master_litepaas master]# brctl addif br1 eth20
[root@jamza_vm_master_litepaas master]#
[root@jamza_vm_master_litepaas master]#
~~~

修改docker的服务端配置，使得docker默认连接自定义的网桥br1，而不是默认的docker0网桥，并配置主控上的容器ip地址池为192.168.84.224/28，以防止主控上的容器ip地址与线卡上的容器ip地址冲突，注意，设置默认网桥与地址池的字段分别是bridge与fixed-cidr，然后重启docker服务端：

~~~shell
[root@jamza_vm_master_litepaas master]#
[root@jamza_vm_master_litepaas master]# vi /etc/docker/daemon.json
[root@jamza_vm_master_litepaas master]#
[root@jamza_vm_master_litepaas master]# cat /etc/docker/daemon.json
{
"bridge":"br1",
"fixed-cidr":"192.168.84.224/28",
"cpu-rt-runtime":800000,
"cpu-rt-period":1000000,
"storage-driver":"overlay2",
"storage-opts":["overlay2.override_kernel_check=true"]
}
{ "insecure-registries":["172.18.0.3:5000"]}
[root@jamza_vm_master_litepaas master]#
[root@jamza_vm_master_litepaas master]#
[root@jamza_vm_master_litepaas master]# systemctl daemon-reload
[root@jamza_vm_master_litepaas master]# service docker restart
Redirecting to /bin/systemctl restart docker.service
[root@jamza_vm_master_litepaas master]#
~~~

此时查看主控上的ip地址，出现br1网桥：

~~~shell
[root@jamza_vm_master_litepaas master]# ip addr
1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN qlen 1
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
    inet 127.0.0.1/8 scope host lo
       valid_lft forever preferred_lft forever
    inet6 ::1/128 scope host
       valid_lft forever preferred_lft forever
2: eth0: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc pfifo_fast state UP qlen 1000
    link/ether 52:54:83:aa:bb:00 brd ff:ff:ff:ff:ff:ff
    inet 192.168.83.83/24 brd 192.168.83.255 scope global eth0
       valid_lft forever preferred_lft forever
    inet6 fe80::5054:83ff:feaa:bb00/64 scope link
       valid_lft forever preferred_lft forever
3: eth20: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc pfifo_fast master br1 state UP qlen 1000
    link/ether 52:54:83:aa:bb:20 brd ff:ff:ff:ff:ff:ff
    inet 192.168.84.83/24 brd 192.168.84.255 scope global eth20
       valid_lft forever preferred_lft forever
    inet6 fe80::5054:83ff:feaa:bb20/64 scope link
       valid_lft forever preferred_lft forever
4: eth21: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc pfifo_fast state UP qlen 1000
    link/ether 52:54:83:aa:bb:21 brd ff:ff:ff:ff:ff:ff
    inet 192.168.85.83/24 brd 192.168.85.255 scope global eth21
       valid_lft forever preferred_lft forever
    inet6 fe80::5054:83ff:feaa:bb21/64 scope link
       valid_lft forever preferred_lft forever
5: docker_gwbridge: <NO-CARRIER,BROADCAST,MULTICAST,UP> mtu 1500 qdisc noqueue state DOWN
    link/ether 02:42:0c:8b:82:cc brd ff:ff:ff:ff:ff:ff
    inet 172.18.0.1/20 scope global docker_gwbridge
       valid_lft forever preferred_lft forever
6: docker0: <NO-CARRIER,BROADCAST,MULTICAST,UP> mtu 1500 qdisc noqueue state DOWN
    link/ether 02:42:0f:34:eb:4b brd ff:ff:ff:ff:ff:ff
    inet 10.76.84.11/24 scope global docker0
       valid_lft forever preferred_lft forever
    inet6 fe80::42:fff:fe34:eb4b/64 scope link
       valid_lft forever preferred_lft forever
135: br1: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue state UP qlen 1000
    link/ether 52:54:83:aa:bb:20 brd ff:ff:ff:ff:ff:ff
    inet 192.168.84.1/24 brd 192.168.84.255 scope global br1
       valid_lft forever preferred_lft forever
    inet6 fe80::f0e1:fbff:fe3d:c088/64 scope link
       valid_lft forever preferred_lft forever
[root@jamza_vm_master_litepaas master]#
~~~

查看主控上的ip路由信息，注意这里出现了两个192.168.84.0/24网段的路由路径：

~~~shell
[root@jamza_vm_master_litepaas master]# ip route
default via 192.168.83.1 dev eth0
10.76.84.0/24 dev docker0 proto kernel scope link src 10.76.84.11
169.254.0.0/16 dev eth0 scope link metric 1002
169.254.0.0/16 dev eth20 scope link metric 1003
169.254.0.0/16 dev eth21 scope link metric 1004
172.18.0.0/20 dev docker_gwbridge proto kernel scope link src 172.18.0.1
192.168.83.0/24 dev eth0 proto kernel scope link src 192.168.83.83
192.168.84.0/24 dev eth20 proto kernel scope link src 192.168.84.83
192.168.84.0/24 dev br1 proto kernel scope link src 192.168.84.1
192.168.85.0/24 dev eth21 proto kernel scope link src 192.168.85.83
[root@jamza_vm_master_litepaas master]#
~~~

## 线卡设置

在设置之前，主控中没有br网桥，ip路由显示192.168.84.0/24网段路由通过网卡eth20端：

~~~shell
[root@jamza_vm_lp0_litepaas master]# ip addr
1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN qlen 1
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
    inet 127.0.0.1/8 scope host lo
       valid_lft forever preferred_lft forever
    inet6 ::1/128 scope host
       valid_lft forever preferred_lft forever
2: eth0: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc pfifo_fast state UP qlen 1000
    link/ether 52:54:83:cc:dd:00 brd ff:ff:ff:ff:ff:ff
    inet 192.168.83.85/24 brd 192.168.83.255 scope global eth0
       valid_lft forever preferred_lft forever
    inet6 fe80::5054:83ff:fecc:dd00/64 scope link
       valid_lft forever preferred_lft forever
3: eth20: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc pfifo_fast state UP qlen 1000
    link/ether 52:54:83:cc:dd:20 brd ff:ff:ff:ff:ff:ff
    inet 192.168.84.85/24 brd 192.168.84.255 scope global eth20
       valid_lft forever preferred_lft forever
    inet6 fe80::5054:83ff:fecc:dd20/64 scope link
       valid_lft forever preferred_lft forever
4: eth21: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc pfifo_fast state UP qlen 1000
    link/ether 52:54:83:cc:dd:21 brd ff:ff:ff:ff:ff:ff
    inet 192.168.85.85/24 brd 192.168.85.255 scope global eth21
       valid_lft forever preferred_lft forever
    inet6 fe80::5054:83ff:fecc:dd21/64 scope link
       valid_lft forever preferred_lft forever
5: docker_gwbridge: <NO-CARRIER,BROADCAST,MULTICAST,UP> mtu 1500 qdisc noqueue state DOWN
    link/ether 02:42:4f:bd:6d:e3 brd ff:ff:ff:ff:ff:ff
    inet 172.18.0.1/20 scope global docker_gwbridge
       valid_lft forever preferred_lft forever
8: docker0: <NO-CARRIER,BROADCAST,MULTICAST,UP> mtu 1500 qdisc noqueue state DOWN
    link/ether 02:42:40:2d:14:72 brd ff:ff:ff:ff:ff:ff
    inet 10.76.84.22/24 scope global docker0
       valid_lft forever preferred_lft forever
    inet6 fe80::42:40ff:fe2d:1472/64 scope link
       valid_lft forever preferred_lft forever
[root@jamza_vm_lp0_litepaas master]#
[root@jamza_vm_lp0_litepaas master]# ip route
default via 192.168.83.1 dev eth0
10.76.84.0/24 dev docker0 proto kernel scope link src 10.76.84.22
169.254.0.0/16 dev eth0 scope link metric 1002
169.254.0.0/16 dev eth20 scope link metric 1003
169.254.0.0/16 dev eth21 scope link metric 1004
172.18.0.0/20 dev docker_gwbridge proto kernel scope link src 172.18.0.1
192.168.83.0/24 dev eth0 proto kernel scope link src 192.168.83.85
192.168.84.0/24 dev eth20 proto kernel scope link src 192.168.84.85
192.168.85.0/24 dev eth21 proto kernel scope link src 192.168.85.85
[root@jamza_vm_lp0_litepaas master]#
~~~

在主控上创建一个网桥，为网桥分配ip地址，并将自定义的网桥桥接本地的网卡eth20：

~~~shell
[root@jamza_vm_lp0_litepaas master]#
[root@jamza_vm_lp0_litepaas master]# brctl addbr br1
[root@jamza_vm_lp0_litepaas master]#
[root@jamza_vm_lp0_litepaas master]#
[root@jamza_vm_lp0_litepaas master]# ifconfig br1 192.168.84.1 netmask 255.255.255.0
[root@jamza_vm_lp0_litepaas master]#
[root@jamza_vm_lp0_litepaas master]# brctl addif br1 eth20
[root@jamza_vm_lp0_litepaas master]#
~~~

修改docker的服务端配置，使得docker默认连接自定义的网桥br1，而不是默认的docker0网桥，并配置线卡上的容器ip地址池为192.168.84.240/28，以防止线卡上的容器ip地址与主控上的容器ip地址冲突，注意，设置默认网桥与地址池的字段分别是bridge与fixed-cidr，然后重启docker服务端：

~~~shell
[root@jamza_vm_lp0_litepaas master]# vi /etc/docker/daemon.json
[root@jamza_vm_lp0_litepaas master]#
[root@jamza_vm_lp0_litepaas master]# cat /etc/docker/daemon.json
{
"bridge":"br1",
"fixed-cidr":"192.168.84.240/28",
"cpu-rt-runtime":800000,
"cpu-rt-period":1000000,
"storage-driver":"overlay2",
"storage-opts":["overlay2.override_kernel_check=true"]
}
{ "insecure-registries":["192.168.83.241:5000"]}
[root@jamza_vm_lp0_litepaas master]#
[root@jamza_vm_lp0_litepaas master]# systemctl daemon-reload
[root@jamza_vm_lp0_litepaas master]#
[root@jamza_vm_lp0_litepaas master]# service docker restart
Redirecting to /bin/systemctl restart docker.service
[root@jamza_vm_lp0_litepaas master]#
~~~

此时查看线卡上的ip地址，出现br1网桥：

~~~shell
[root@jamza_vm_lp0_litepaas master]# ip addr
1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN qlen 1
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
    inet 127.0.0.1/8 scope host lo
       valid_lft forever preferred_lft forever
    inet6 ::1/128 scope host
       valid_lft forever preferred_lft forever
2: eth0: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc pfifo_fast state UP qlen 1000
    link/ether 52:54:83:cc:dd:00 brd ff:ff:ff:ff:ff:ff
    inet 192.168.83.85/24 brd 192.168.83.255 scope global eth0
       valid_lft forever preferred_lft forever
    inet6 fe80::5054:83ff:fecc:dd00/64 scope link
       valid_lft forever preferred_lft forever
3: eth20: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc pfifo_fast master br1 state UP qlen 1000
    link/ether 52:54:83:cc:dd:20 brd ff:ff:ff:ff:ff:ff
    inet 192.168.84.85/24 brd 192.168.84.255 scope global eth20
       valid_lft forever preferred_lft forever
    inet6 fe80::5054:83ff:fecc:dd20/64 scope link
       valid_lft forever preferred_lft forever
4: eth21: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc pfifo_fast state UP qlen 1000
    link/ether 52:54:83:cc:dd:21 brd ff:ff:ff:ff:ff:ff
    inet 192.168.85.85/24 brd 192.168.85.255 scope global eth21
       valid_lft forever preferred_lft forever
    inet6 fe80::5054:83ff:fecc:dd21/64 scope link
       valid_lft forever preferred_lft forever
5: docker_gwbridge: <NO-CARRIER,BROADCAST,MULTICAST,UP> mtu 1500 qdisc noqueue state DOWN
    link/ether 02:42:4f:bd:6d:e3 brd ff:ff:ff:ff:ff:ff
    inet 172.18.0.1/20 scope global docker_gwbridge
       valid_lft forever preferred_lft forever
8: docker0: <NO-CARRIER,BROADCAST,MULTICAST,UP> mtu 1500 qdisc noqueue state DOWN
    link/ether 02:42:40:2d:14:72 brd ff:ff:ff:ff:ff:ff
    inet 10.76.84.22/24 scope global docker0
       valid_lft forever preferred_lft forever
    inet6 fe80::42:40ff:fe2d:1472/64 scope link
       valid_lft forever preferred_lft forever
118: br1: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue state UP qlen 1000
    link/ether 52:54:83:cc:dd:20 brd ff:ff:ff:ff:ff:ff
    inet 192.168.84.1/24 brd 192.168.84.255 scope global br1
       valid_lft forever preferred_lft forever
    inet6 fe80::10c6:b7ff:fe09:ff42/64 scope link
       valid_lft forever preferred_lft forever
[root@jamza_vm_lp0_litepaas master]#
~~~

查看线卡上的ip路由信息，注意这里出现了两个192.168.84.0/24网段的路由路径：

~~~shell
[root@jamza_vm_lp0_litepaas master]# ip route
default via 192.168.83.1 dev eth0
10.76.84.0/24 dev docker0 proto kernel scope link src 10.76.84.22
169.254.0.0/16 dev eth0 scope link metric 1002
169.254.0.0/16 dev eth20 scope link metric 1003
169.254.0.0/16 dev eth21 scope link metric 1004
172.18.0.0/20 dev docker_gwbridge proto kernel scope link src 172.18.0.1
192.168.83.0/24 dev eth0 proto kernel scope link src 192.168.83.85
192.168.84.0/24 dev eth20 proto kernel scope link src 192.168.84.85
192.168.84.0/24 dev br1 proto kernel scope link src 192.168.84.1
192.168.85.0/24 dev eth21 proto kernel scope link src 192.168.85.85
[root@jamza_vm_lp0_litepaas master]#
~~~

## 容器互通测试

在主控上创建容器，网络使用默认bridge模式，根据之前的设定，容器将连接到自定义网桥br1上，且从分配的地址池中取出可用的ip地址：

~~~shell
[root@jamza_vm_master_litepaas master]# docker run -it --privileged=true --name test busybox/x86_64 /bin/sh
/ #
/ # ifconfig
eth0      Link encap:Ethernet  HWaddr 02:42:C0:A8:54:E0
          inet addr:192.168.84.224  Bcast:0.0.0.0  Mask:255.255.255.0
          inet6 addr: fe80::42:c0ff:fea8:54e0/64 Scope:Link
          UP BROADCAST RUNNING MULTICAST  MTU:1500  Metric:1
          RX packets:12 errors:0 dropped:4 overruns:0 frame:0
          TX packets:6 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:0
          RX bytes:908 (908.0 B)  TX bytes:508 (508.0 B)

lo        Link encap:Local Loopback
          inet addr:127.0.0.1  Mask:255.0.0.0
          inet6 addr: ::1/128 Scope:Host
          UP LOOPBACK RUNNING  MTU:65536  Metric:1
          RX packets:0 errors:0 dropped:0 overruns:0 frame:0
          TX packets:0 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:1
          RX bytes:0 (0.0 B)  TX bytes:0 (0.0 B)

/ #
~~~

在线卡上创建容器，网络使用默认bridge模式，根据之前的设定，容器将连接到自定义网桥br1上，且从分配的地址池中取出可用的ip地址：

~~~shell
[root@jamza_vm_lp0_litepaas master]# docker run -it --privileged=true --name test busybox/x86_64 /bin/sh
/ #
/ # ifconfig
eth0      Link encap:Ethernet  HWaddr 02:42:C0:A8:54:F0
          inet addr:192.168.84.240  Bcast:0.0.0.0  Mask:255.255.255.0
          inet6 addr: fe80::42:c0ff:fea8:54f0/64 Scope:Link
          UP BROADCAST RUNNING MULTICAST  MTU:1500  Metric:1
          RX packets:10 errors:0 dropped:3 overruns:0 frame:0
          TX packets:6 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:0
          RX bytes:782 (782.0 B)  TX bytes:508 (508.0 B)

lo        Link encap:Local Loopback
          inet addr:127.0.0.1  Mask:255.0.0.0
          inet6 addr: ::1/128 Scope:Host
          UP LOOPBACK RUNNING  MTU:65536  Metric:1
          RX packets:0 errors:0 dropped:0 overruns:0 frame:0
          TX packets:0 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:1
          RX bytes:0 (0.0 B)  TX bytes:0 (0.0 B)

/ #
~~~

主控上的容器ping线卡上的容器，以及线卡上的容器ping主控上的容器，结果显示，均能够ping通。

~~~shell
#主控上的容器ping线卡上的容器
/ #
/ # ping 192.168.84.240
PING 192.168.84.240 (192.168.84.240): 56 data bytes
64 bytes from 192.168.84.240: seq=0 ttl=64 time=0.683 ms
64 bytes from 192.168.84.240: seq=1 ttl=64 time=0.403 ms
64 bytes from 192.168.84.240: seq=2 ttl=64 time=0.413 ms
64 bytes from 192.168.84.240: seq=3 ttl=64 time=0.296 ms
^C
--- 192.168.84.240 ping statistics ---
4 packets transmitted, 4 packets received, 0% packet loss
round-trip min/avg/max = 0.296/0.448/0.683 ms
/ #

#线卡上的容器ping主控上的容器
/ # ping 192.168.84.224
PING 192.168.84.224 (192.168.84.224): 56 data bytes
64 bytes from 192.168.84.224: seq=0 ttl=64 time=0.393 ms
64 bytes from 192.168.84.224: seq=1 ttl=64 time=0.421 ms
64 bytes from 192.168.84.224: seq=2 ttl=64 time=0.542 ms
64 bytes from 192.168.84.224: seq=3 ttl=64 time=0.414 ms
^C
--- 192.168.84.224 ping statistics ---
4 packets transmitted, 4 packets received, 0% packet loss
round-trip min/avg/max = 0.393/0.442/0.542 ms
/ #
~~~

若想主机也能ping主机上容器内的，则需删除主机上的路由信息：

192.168.84.0/24 dev eth20 proto kernel scope link src 192.168.84.85

192.168.84.0/24 dev eth20 proto kernel scope link src 192.168.84.83