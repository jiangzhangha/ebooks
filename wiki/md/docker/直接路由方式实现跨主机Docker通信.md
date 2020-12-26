---
title: 直接路由方式实现跨主机Docker通信
date: 2020-07-08 13:41:45
categories: docker
index_img: /img/articles/docker.jpg
tags:
	- docker
	- 跨主机通信
	- 直接路由
---

## 概述

在两台宿主机分别创建自定义的docker网络，网络类型为bridge，在各自的宿主机上创建docker容器，容器连接至各自主机上的自定义网络。在宿主机上创建路由规则，实现两台宿主机上的容器之间的互联通信。

通过宿主机上的网络命名空间ID，向容器中添加多个网卡。

## 网络地址划分

### 主控主机

IP：192.168.83.83

网段：10.76.1.0/24

网关：10.76.1.1

自定义网络网桥名称：br01

容器IP地址：10.76.1.83、10.76.1.84、10.76.1.85

### 线卡主机

IP：192.168.83.85

网段：10.76.2.0/24

网关：10.76.2.1

自定义网络网桥名称：br02

容器IP地址：10.76.2.83、10.76.2.84、10.76.2.85

## 创建网络与容器

在主控主机上创建自定义网络的步骤流程如下：

~~~shell
[root@jamza_vm_master_litepaas route_net_test]# docker network ls
NETWORK ID          NAME                DRIVER              SCOPE
5b591ed369ac        bridge              bridge              local
b919f69968ec        docker_gwbridge     bridge              local
032c767b11de        host                host                local
cad8d79e6d05        none                null                local
[root@jamza_vm_master_litepaas route_net_test]#
[root@jamza_vm_master_litepaas route_net_test]# docker network create --subnet=10.76.1.0/24 --gateway=10.76.1.1 -o "com.docker.network.bridge.name"=br01 -o "com.docker.network.bridge.enable_icc"=true net1
52318642206e51c48599d06f2d8a4473e23aa485084cc9ca48618cdfc6b867fd
[root@jamza_vm_master_litepaas route_net_test]#
[root@jamza_vm_master_litepaas route_net_test]# docker network ls
NETWORK ID          NAME                DRIVER              SCOPE
5b591ed369ac        bridge              bridge              local
b919f69968ec        docker_gwbridge     bridge              local
032c767b11de        host                host                local
52318642206e        net1                bridge              local
cad8d79e6d05        none                null                local
[root@jamza_vm_master_litepaas route_net_test]#
[root@jamza_vm_master_litepaas route_net_test]# docker network inspect net1
[
    {
        "Name": "net1",
        "Id": "52318642206e51c48599d06f2d8a4473e23aa485084cc9ca48618cdfc6b867fd",
        "Created": "2019-08-06T14:18:03.256967046+08:00",
        "Scope": "local",
        "Driver": "bridge",
        "EnableIPv6": false,
        "IPAM": {
            "Driver": "default",
            "Options": {},
            "Config": [
                {
                    "Subnet": "10.76.1.0/24",
                    "Gateway": "10.76.1.1"
                }
            ]
        },
        "Internal": false,
        "Attachable": false,
        "Containers": {},
        "Options": {
            "com.docker.network.bridge.enable_icc": "true",
            "com.docker.network.bridge.name": "br01"
        },
        "Labels": {}
    }
]
[root@jamza_vm_master_litepaas route_net_test]#
~~~

在主控主机上创建静态路由，表示访问10.76.2.0/24网段需经过192.168.83.85网卡（线卡主机），且经过本机eth0网卡。

~~~shell
[root@jamza_vm_master_litepaas route_net_test]#
[root@jamza_vm_master_litepaas route_net_test]# ip route
default via 192.168.83.1 dev eth0
10.76.1.0/24 dev br01 proto kernel scope link src 10.76.1.1
169.254.0.0/16 dev eth0 scope link metric 1002
169.254.0.0/16 dev eth20 scope link metric 1003
169.254.0.0/16 dev eth21 scope link metric 1004
172.17.6.0/24 dev docker0 proto kernel scope link src 172.17.6.252
172.18.0.0/20 dev docker_gwbridge proto kernel scope link src 172.18.0.1
192.168.83.0/24 dev eth0 proto kernel scope link src 192.168.83.83
192.168.84.0/24 dev eth20 proto kernel scope link src 192.168.84.83
192.168.85.0/24 dev eth21 proto kernel scope link src 192.168.85.83
[root@jamza_vm_master_litepaas route_net_test]#
[root@jamza_vm_master_litepaas route_net_test]# ip route add 10.76.2.0/24 via 192.168.83.85 dev eth0
[root@jamza_vm_master_litepaas route_net_test]#
[root@jamza_vm_master_litepaas route_net_test]# ip route
default via 192.168.83.1 dev eth0
10.76.1.0/24 dev br01 proto kernel scope link src 10.76.1.1
10.76.2.0/24 via 192.168.83.85 dev eth0
169.254.0.0/16 dev eth0 scope link metric 1002
169.254.0.0/16 dev eth20 scope link metric 1003
169.254.0.0/16 dev eth21 scope link metric 1004
172.17.6.0/24 dev docker0 proto kernel scope link src 172.17.6.252
172.18.0.0/20 dev docker_gwbridge proto kernel scope link src 172.18.0.1
192.168.83.0/24 dev eth0 proto kernel scope link src 192.168.83.83
192.168.84.0/24 dev eth20 proto kernel scope link src 192.168.84.83
192.168.85.0/24 dev eth21 proto kernel scope link src 192.168.85.83
[root@jamza_vm_master_litepaas route_net_test]#
~~~

在主控主机上创建容器，并启动容器：

~~~shell
[root@jamza_vm_master_litepaas route_net_test]#
[root@jamza_vm_master_litepaas route_net_test]# docker run -it --privileged=true --net net1 --name test --ip 10.76.1.83 -v /lib64:/lib64 -v /usr/sbin/tcpdump:/bin/tcpdump busybox/x86_64 /bin/sh
/ #
/ # ifconfig
eth0      Link encap:Ethernet  HWaddr 02:42:0A:4C:01:53
          inet addr:10.76.1.83  Bcast:0.0.0.0  Mask:255.255.255.0
          inet6 addr: fe80::42:aff:fe4c:153/64 Scope:Link
          UP BROADCAST RUNNING MULTICAST  MTU:1500  Metric:1
          RX packets:14 errors:0 dropped:0 overruns:0 frame:0
          TX packets:7 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:0
          RX bytes:1156 (1.1 KiB)  TX bytes:578 (578.0 B)

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

同理，在线卡主机上创建自定义网络，添加路由规则，并启动容器，连接到自定义网络：

~~~shell
[root@jamza_vm_lp0_litepaas route_net_test]#
[root@jamza_vm_lp0_litepaas route_net_test]# docker network create --subnet=10.76.2.0/24 --gateway=10.76.2.1 -o "com.docker.network.bridge.name"=br02 -o "com.docker.network.bridge.enable_icc"=true net1
0879d05f096683aba90b013620fc6e3e0ebd2b5d5896fc3a6fc3a575846e4666
[root@jamza_vm_lp0_litepaas route_net_test]#
[root@jamza_vm_lp0_litepaas route_net_test]# docker network ls
NETWORK ID          NAME                DRIVER              SCOPE
055d9dddcab8        bridge              bridge              local
bd5b745fb87f        docker_gwbridge     bridge              local
a20b3d67c74d        host                host                local
0879d05f0966        net1                bridge              local
d8bdffcb4f6e        none                null                local
[root@jamza_vm_lp0_litepaas route_net_test]#
[root@jamza_vm_lp0_litepaas route_net_test]#
[root@jamza_vm_lp0_litepaas route_net_test]# ip route
default via 192.168.83.1 dev eth0
10.76.2.0/24 dev br02 proto kernel scope link src 10.76.2.1
169.254.0.0/16 dev eth0 scope link metric 1002
169.254.0.0/16 dev eth20 scope link metric 1003
169.254.0.0/16 dev eth21 scope link metric 1004
172.17.9.0/24 dev docker0 proto kernel scope link src 172.17.9.252
172.18.0.0/20 dev docker_gwbridge proto kernel scope link src 172.18.0.1
192.168.83.0/24 dev eth0 proto kernel scope link src 192.168.83.85
192.168.84.0/24 dev eth20 proto kernel scope link src 192.168.84.85
192.168.85.0/24 dev eth21 proto kernel scope link src 192.168.85.85
[root@jamza_vm_lp0_litepaas route_net_test]#
[root@jamza_vm_lp0_litepaas route_net_test]# ip route add 10.76.1.0/24 via 192.168.83.83 dev eth0
[root@jamza_vm_lp0_litepaas route_net_test]#
[root@jamza_vm_lp0_litepaas route_net_test]# ip route
default via 192.168.83.1 dev eth0
10.76.1.0/24 via 192.168.83.83 dev eth0
10.76.2.0/24 dev br02 proto kernel scope link src 10.76.2.1
169.254.0.0/16 dev eth0 scope link metric 1002
169.254.0.0/16 dev eth20 scope link metric 1003
169.254.0.0/16 dev eth21 scope link metric 1004
172.17.9.0/24 dev docker0 proto kernel scope link src 172.17.9.252
172.18.0.0/20 dev docker_gwbridge proto kernel scope link src 172.18.0.1
192.168.83.0/24 dev eth0 proto kernel scope link src 192.168.83.85
192.168.84.0/24 dev eth20 proto kernel scope link src 192.168.84.85
192.168.85.0/24 dev eth21 proto kernel scope link src 192.168.85.85
[root@jamza_vm_lp0_litepaas route_net_test]#
[root@jamza_vm_lp0_litepaas route_net_test]#
[root@jamza_vm_lp0_litepaas route_net_test]# docker run -it --privileged=true --net net1 --name test --ip 10.76.2.83 -v /lib64:/lib64 -v /usr/sbin/tcpdump:/bin/tcpdump busybox/x86_64 /bin/sh
/ #
/ # ifconfig
eth0      Link encap:Ethernet  HWaddr 02:42:0A:4C:02:53
          inet addr:10.76.2.83  Bcast:0.0.0.0  Mask:255.255.255.0
          inet6 addr: fe80::42:aff:fe4c:253/64 Scope:Link
          UP BROADCAST RUNNING MULTICAST  MTU:1500  Metric:1
          RX packets:12 errors:0 dropped:0 overruns:0 frame:0
          TX packets:6 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:0
          RX bytes:1016 (1016.0 B)  TX bytes:508 (508.0 B)

lo        Link encap:Local Loopback
          inet addr:127.0.0.1  Mask:255.0.0.0
          inet6 addr: ::1/128 Scope:Host
          UP LOOPBACK RUNNING  MTU:65536  Metric:1
          RX packets:0 errors:0 dropped:0 overruns:0 frame:0
          TX packets:0 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:1
          RX bytes:0 (0.0 B)  TX bytes:0 (0.0 B)

/ #
/ #
~~~

此时两个容器能够互相ping通：

~~~shell
#主控上容器ping线卡上容器
/ #
/ # ping 10.76.2.83
PING 10.76.2.83 (10.76.2.83): 56 data bytes
64 bytes from 10.76.2.83: seq=0 ttl=62 time=0.359 ms
64 bytes from 10.76.2.83: seq=1 ttl=62 time=0.485 ms
64 bytes from 10.76.2.83: seq=2 ttl=62 time=0.479 ms
64 bytes from 10.76.2.83: seq=3 ttl=62 time=0.559 ms
64 bytes from 10.76.2.83: seq=4 ttl=62 time=0.505 ms
64 bytes from 10.76.2.83: seq=5 ttl=62 time=0.510 ms
^C
--- 10.76.2.83 ping statistics ---
6 packets transmitted, 6 packets received, 0% packet loss
round-trip min/avg/max = 0.359/0.482/0.559 ms
/ #
/ #

#线卡上容器ping主控上容器
/ #
/ # ping 10.76.1.83
PING 10.76.1.83 (10.76.1.83): 56 data bytes
64 bytes from 10.76.1.83: seq=0 ttl=62 time=0.348 ms
64 bytes from 10.76.1.83: seq=1 ttl=62 time=0.401 ms
64 bytes from 10.76.1.83: seq=2 ttl=62 time=0.429 ms
64 bytes from 10.76.1.83: seq=3 ttl=62 time=0.574 ms
64 bytes from 10.76.1.83: seq=4 ttl=62 time=0.519 ms
64 bytes from 10.76.1.83: seq=5 ttl=62 time=0.584 ms
^C
--- 10.76.1.83 ping statistics ---
6 packets transmitted, 6 packets received, 0% packet loss
round-trip min/avg/max = 0.348/0.475/0.584 ms
/ #
/ #
~~~

## 向容器中添加网卡

向容器中添加网卡，需使用两个虚拟的以太网端口，即veth pair。容器中的网络命名空间与宿主机的网络命名空间是隔离的，需将veth pair的一端添加到容器的网络命名空间中，将另一端添加到宿主机的网络命名空间，即实现两个网络命名空间通过veth pair通信。

对主控主机中的容器添加虚拟网卡，首先是获取容器所在的网络命名空间ID，其中test是已经启动的docker容器的名称，获取的ID为10281：

~~~shell
[root@jamza_vm_master_litepaas route_net_test]# docker inspect -f '{{.State.Pid}}' test
10281
[root@jamza_vm_master_litepaas route_net_test]#
~~~

后续将使用ip netns命令，而命令的有效目录是/var/run/netns/，因此需通过软链接的方式创建/var/run/netns/目录下的容器网络命名空间：

~~~shell
[root@jamza_vm_master_litepaas route_net_test]# ll /var/run/netns/
总用量 0
[root@jamza_vm_master_litepaas route_net_test]#
[root@jamza_vm_master_litepaas route_net_test]# ln -s /proc/10281/ns/net /var/run/netns/10281
[root@jamza_vm_master_litepaas route_net_test]#
[root@jamza_vm_master_litepaas route_net_test]# ll /var/run/netns/
总用量 0
lrwxrwxrwx. 1 root root 18 8月   6 14:49 10281 -> /proc/10281/ns/net
[root@jamza_vm_master_litepaas route_net_test]#
[root@jamza_vm_master_litepaas route_net_test]# ip netns
10281 (id: 0)
[root@jamza_vm_master_litepaas route_net_test]#
~~~

创建两对veth pair，其中veth0与veth1为一对，veth2与veth3为一对：

~~~shell
[root@jamza_vm_master_litepaas route_net_test]# ip link add veth0 type veth peer name veth1
[root@jamza_vm_master_litepaas route_net_test]#
[root@jamza_vm_master_litepaas route_net_test]# ip link add veth2 type veth peer name veth3
[root@jamza_vm_master_litepaas route_net_test]#
~~~

将veth pair中的veth1与veth3与宿主机上的自定义docker网络的网关br01相关联，并启动veth1与veth3端口：

~~~shell
[root@jamza_vm_master_litepaas route_net_test]# ip link set dev veth1 master br01
[root@jamza_vm_master_litepaas route_net_test]# ip link set dev veth1 up
[root@jamza_vm_master_litepaas route_net_test]#
[root@jamza_vm_master_litepaas route_net_test]# ip link set dev veth3 master br01
[root@jamza_vm_master_litepaas route_net_test]# ip link set dev veth3 up
[root@jamza_vm_master_litepaas route_net_test]#
[root@jamza_vm_master_litepaas route_net_test]# ip addr
......(略)
68: br01: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue state UP
    link/ether 02:42:bd:a4:a2:de brd ff:ff:ff:ff:ff:ff
    inet 10.76.1.1/24 scope global br01
       valid_lft forever preferred_lft forever
    inet6 fe80::42:bdff:fea4:a2de/64 scope link
       valid_lft forever preferred_lft forever
70: vethdea75f3@if69: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue master br01 state UP
    link/ether de:fb:93:07:f5:6e brd ff:ff:ff:ff:ff:ff link-netnsid 0
    inet6 fe80::dcfb:93ff:fe07:f56e/64 scope link
       valid_lft forever preferred_lft forever
71: veth1@veth0: <NO-CARRIER,BROADCAST,MULTICAST,UP,M-DOWN> mtu 1500 qdisc noqueue master br01 state LOWERLAYERDOWN qlen 1000
    link/ether 9e:0e:4d:74:c6:9c brd ff:ff:ff:ff:ff:ff
72: veth0@veth1: <BROADCAST,MULTICAST> mtu 1500 qdisc noop state DOWN qlen 1000
    link/ether 62:3c:be:c3:73:d2 brd ff:ff:ff:ff:ff:ff
73: veth3@veth2: <NO-CARRIER,BROADCAST,MULTICAST,UP,M-DOWN> mtu 1500 qdisc noqueue master br01 state LOWERLAYERDOWN qlen 1000
    link/ether 4e:15:bc:96:5a:12 brd ff:ff:ff:ff:ff:ff
74: veth2@veth3: <BROADCAST,MULTICAST> mtu 1500 qdisc noop state DOWN qlen 1000
    link/ether 6e:a7:45:2c:b5:0e brd ff:ff:ff:ff:ff:ff
~~~

然后配置veth pair中的veth0与veth2在容器中启动：

~~~shell
[root@jamza_vm_master_litepaas route_net_test]# ip link set dev veth0 name eth1 netns 10281
[root@jamza_vm_master_litepaas route_net_test]# ip netns exec 10281 ip link set dev eth1 up
[root@jamza_vm_master_litepaas route_net_test]# ip netns exec 10281 ip addr add 10.76.1.84/24 dev eth1
[root@jamza_vm_master_litepaas route_net_test]#
[root@jamza_vm_master_litepaas route_net_test]#
[root@jamza_vm_master_litepaas route_net_test]# ip link set dev veth2 name eth2 netns 10281
[root@jamza_vm_master_litepaas route_net_test]# ip netns exec 10281 ip link set dev eth2 up
[root@jamza_vm_master_litepaas route_net_test]# ip netns exec 10281 ip addr add 10.76.1.85/24 dev eth2
[root@jamza_vm_master_litepaas route_net_test]#
~~~

然后再主控主机的容器中，可以看到新添加的虚拟网卡：

~~~shell
/ #
/ # ifconfig
eth0      Link encap:Ethernet  HWaddr 02:42:0A:4C:01:53
          inet addr:10.76.1.83  Bcast:0.0.0.0  Mask:255.255.255.0
          inet6 addr: fe80::42:aff:fe4c:153/64 Scope:Link
          UP BROADCAST RUNNING MULTICAST  MTU:1500  Metric:1
          RX packets:47 errors:0 dropped:0 overruns:0 frame:0
          TX packets:23 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:0
          RX bytes:3894 (3.8 KiB)  TX bytes:1950 (1.9 KiB)

eth1      Link encap:Ethernet  HWaddr 62:3C:BE:C3:73:D2
          inet addr:10.76.1.84  Bcast:0.0.0.0  Mask:255.255.255.0
          inet6 addr: fe80::603c:beff:fec3:73d2/64 Scope:Link
          UP BROADCAST RUNNING MULTICAST  MTU:1500  Metric:1
          RX packets:16 errors:0 dropped:0 overruns:0 frame:0
          TX packets:8 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:1000
          RX bytes:1296 (1.2 KiB)  TX bytes:648 (648.0 B)

eth2      Link encap:Ethernet  HWaddr 6E:A7:45:2C:B5:0E
          inet addr:10.76.1.85  Bcast:0.0.0.0  Mask:255.255.255.0
          inet6 addr: fe80::6ca7:45ff:fe2c:b50e/64 Scope:Link
          UP BROADCAST RUNNING MULTICAST  MTU:1500  Metric:1
          RX packets:8 errors:0 dropped:0 overruns:0 frame:0
          TX packets:8 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:1000
          RX bytes:648 (648.0 B)  TX bytes:648 (648.0 B)

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

同理，在线卡主机上向容器中添加网卡，结果如下：

~~~shell
/ # ifconfig
eth0      Link encap:Ethernet  HWaddr 02:42:0A:4C:02:53
          inet addr:10.76.2.83  Bcast:0.0.0.0  Mask:255.255.255.0
          inet6 addr: fe80::42:aff:fe4c:253/64 Scope:Link
          UP BROADCAST RUNNING MULTICAST  MTU:1500  Metric:1
          RX packets:28 errors:0 dropped:0 overruns:0 frame:0
          TX packets:8 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:0
          RX bytes:2312 (2.2 KiB)  TX bytes:648 (648.0 B)

eth1      Link encap:Ethernet  HWaddr F6:6C:37:75:A9:D4
          inet addr:10.76.2.84  Bcast:0.0.0.0  Mask:255.255.255.0
          inet6 addr: fe80::f46c:37ff:fe75:a9d4/64 Scope:Link
          UP BROADCAST RUNNING MULTICAST  MTU:1500  Metric:1
          RX packets:12 errors:0 dropped:0 overruns:0 frame:0
          TX packets:6 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:1000
          RX bytes:1016 (1016.0 B)  TX bytes:508 (508.0 B)

eth2      Link encap:Ethernet  HWaddr 76:64:D9:37:6B:98
          inet addr:10.76.2.85  Bcast:0.0.0.0  Mask:255.255.255.0
          inet6 addr: fe80::7464:d9ff:fe37:6b98/64 Scope:Link
          UP BROADCAST RUNNING MULTICAST  MTU:1500  Metric:1
          RX packets:11 errors:0 dropped:0 overruns:0 frame:0
          TX packets:6 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:1000
          RX bytes:926 (926.0 B)  TX bytes:508 (508.0 B)

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

测试两个主机上的容器之间是否能够ping通。在主控主机上的容器中测试：

~~~shell
/ #
/ # ping 10.76.2.83
PING 10.76.2.83 (10.76.2.83): 56 data bytes
64 bytes from 10.76.2.83: seq=0 ttl=62 time=0.436 ms
64 bytes from 10.76.2.83: seq=1 ttl=62 time=0.560 ms
64 bytes from 10.76.2.83: seq=2 ttl=62 time=0.519 ms
64 bytes from 10.76.2.83: seq=3 ttl=62 time=0.516 ms
64 bytes from 10.76.2.83: seq=4 ttl=62 time=0.443 ms
^C
--- 10.76.2.83 ping statistics ---
5 packets transmitted, 5 packets received, 0% packet loss
round-trip min/avg/max = 0.436/0.494/0.560 ms
/ #
/ # ping 10.76.2.84
PING 10.76.2.84 (10.76.2.84): 56 data bytes
64 bytes from 10.76.2.84: seq=0 ttl=62 time=0.355 ms
64 bytes from 10.76.2.84: seq=1 ttl=62 time=0.537 ms
64 bytes from 10.76.2.84: seq=2 ttl=62 time=0.540 ms
64 bytes from 10.76.2.84: seq=3 ttl=62 time=0.435 ms
^C
--- 10.76.2.84 ping statistics ---
4 packets transmitted, 4 packets received, 0% packet loss
round-trip min/avg/max = 0.355/0.466/0.540 ms
/ #
/ # ping 10.76.2.85
PING 10.76.2.85 (10.76.2.85): 56 data bytes
64 bytes from 10.76.2.85: seq=0 ttl=62 time=0.394 ms
64 bytes from 10.76.2.85: seq=1 ttl=62 time=0.334 ms
64 bytes from 10.76.2.85: seq=2 ttl=62 time=0.438 ms
64 bytes from 10.76.2.85: seq=3 ttl=62 time=0.321 ms
^C
--- 10.76.2.85 ping statistics ---
4 packets transmitted, 4 packets received, 0% packet loss
round-trip min/avg/max = 0.321/0.371/0.438 ms
/ #
~~~

在线卡主机的容器中测试，结果显示，各个网络地址互相之间能够ping通：

~~~shell
/ #
/ # ping 10.76.1.83
PING 10.76.1.83 (10.76.1.83): 56 data bytes
64 bytes from 10.76.1.83: seq=0 ttl=62 time=0.332 ms
64 bytes from 10.76.1.83: seq=1 ttl=62 time=0.584 ms
64 bytes from 10.76.1.83: seq=2 ttl=62 time=0.524 ms
64 bytes from 10.76.1.83: seq=3 ttl=62 time=0.541 ms
^C
--- 10.76.1.83 ping statistics ---
4 packets transmitted, 4 packets received, 0% packet loss
round-trip min/avg/max = 0.332/0.495/0.584 ms
/ #
/ #
/ # ping 10.76.1.84
PING 10.76.1.84 (10.76.1.84): 56 data bytes
64 bytes from 10.76.1.84: seq=0 ttl=62 time=0.355 ms
64 bytes from 10.76.1.84: seq=1 ttl=62 time=0.563 ms
64 bytes from 10.76.1.84: seq=2 ttl=62 time=0.559 ms
64 bytes from 10.76.1.84: seq=3 ttl=62 time=0.422 ms
^C
--- 10.76.1.84 ping statistics ---
4 packets transmitted, 4 packets received, 0% packet loss
round-trip min/avg/max = 0.355/0.474/0.563 ms
/ #
/ #
/ # ping 10.76.1.85
PING 10.76.1.85 (10.76.1.85): 56 data bytes
64 bytes from 10.76.1.85: seq=0 ttl=62 time=0.354 ms
64 bytes from 10.76.1.85: seq=1 ttl=62 time=0.413 ms
64 bytes from 10.76.1.85: seq=2 ttl=62 time=0.520 ms
64 bytes from 10.76.1.85: seq=3 ttl=62 time=0.561 ms
^C
--- 10.76.1.85 ping statistics ---
4 packets transmitted, 4 packets received, 0% packet loss
round-trip min/avg/max = 0.354/0.462/0.561 ms
/ #
/ #
~~~