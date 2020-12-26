---
title: macvlan网络模式实现跨主机Docker通信
date: 2020-07-08 13:52:45
categories: docker
index_img: /img/articles/docker.jpg
tags:
	- docker
	- 跨主机通信
	- macvlan
---

## macvlan网络模式概念

macvlan本身是linux内核的模块，本质上是一种网卡虚拟化的技术，其功能是允许在同一个物理网卡上虚拟出多个网卡，通过不同的MAC地址在数据链路层进行网络数据的转发。

一块网卡上配置多个MAC地址，即多个接口，每个接口都可以配置自身的IP地址。Docker的macvlan网络实际上就是使用了Linux提供的macvlan驱动功能。

因为多个MAC地址的网络数据包都是从同一个网卡上传输，因此需要打开网卡的混杂模式，即ip link set eth0 promisc on

在Docker下创建macvlan网络不同于创建bridge网络模式，需要指定网段与网关，且网段与网关需是物理上存在的。

macvlan网络模式不依赖于网桥，所以使用brctl show将看到并没有创建新的网桥。但是查看容器内的网络，将会看到每个虚拟网卡都对应一个容器外部的接口。

~~~shell
/ # ip addr
1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue qlen 1
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
    inet 127.0.0.1/8 scope host lo
       valid_lft forever preferred_lft forever
    inet6 ::1/128 scope host
       valid_lft forever preferred_lft forever
627: eth2@if3: <BROADCAST,MULTICAST,UP,LOWER_UP,M-DOWN> mtu 1500 qdisc noqueue
    link/ether 02:42:c0:a8:54:e1 brd ff:ff:ff:ff:ff:ff
    inet 192.168.84.225/24 scope global eth2
       valid_lft forever preferred_lft forever
    inet6 fe80::42:c0ff:fea8:54e1/64 scope link
       valid_lft forever preferred_lft forever
628: eth1@if4: <BROADCAST,MULTICAST,UP,LOWER_UP,M-DOWN> mtu 1500 qdisc noqueue
    link/ether 02:42:c0:a8:55:e1 brd ff:ff:ff:ff:ff:ff
    inet 192.168.85.225/24 scope global eth1
       valid_lft forever preferred_lft forever
    inet6 fe80::42:c0ff:fea8:55e1/64 scope link
       valid_lft forever preferred_lft forever
629: eth0@if2: <BROADCAST,MULTICAST,UP,LOWER_UP,M-DOWN> mtu 1500 qdisc noqueue
    link/ether 02:42:c0:a8:53:e1 brd ff:ff:ff:ff:ff:ff
    inet 192.168.83.225/24 scope global eth0
       valid_lft forever preferred_lft forever
    inet6 fe80::42:c0ff:fea8:53e1/64 scope link
       valid_lft forever preferred_lft forever
/ #
~~~

比如，以上是在容器内执行ip addr指令后显示的各个虚拟网口内容。比如虚拟网口显示“629: eth0@if2”，其中if2表示物理网卡的编号为2的网口，即如下所示，编号为2的网口即eth0网口。

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
~~~

## 网段规划

查看宿主机的网口信息：

~~~shell
[root@jamza_vm_master_litepaas master]# ifconfig
docker0: flags=4099<UP,BROADCAST,MULTICAST>  mtu 1500
        inet 172.17.6.252  netmask 255.255.255.0  broadcast 0.0.0.0
        inet6 fe80::42:c6ff:fe05:2363  prefixlen 64  scopeid 0x20<link>
        ether 02:42:c6:05:23:63  txqueuelen 0  (Ethernet)
        RX packets 570  bytes 35680 (34.8 KiB)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 223  bytes 19338 (18.8 KiB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

docker_gwbridge: flags=4099<UP,BROADCAST,MULTICAST>  mtu 1500
        inet 172.18.0.1  netmask 255.255.240.0  broadcast 0.0.0.0
        inet6 fe80::42:42ff:fe41:eebc  prefixlen 64  scopeid 0x20<link>
        ether 02:42:42:41:ee:bc  txqueuelen 0  (Ethernet)
        RX packets 3574777  bytes 185891316 (177.2 MiB)
        RX errors 0  dropped 15  overruns 0  frame 0
        TX packets 1530  bytes 99712 (97.3 KiB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

eth0: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
        inet 192.168.83.83  netmask 255.255.255.0  broadcast 192.168.83.255
        inet6 fe80::5054:83ff:feaa:bb00  prefixlen 64  scopeid 0x20<link>
        ether 52:54:83:aa:bb:00  txqueuelen 1000  (Ethernet)
        RX packets 83683524  bytes 364816891747 (339.7 GiB)
        RX errors 0  dropped 15  overruns 0  frame 0
        TX packets 94961933  bytes 213362123667 (198.7 GiB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

eth20: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
        inet 192.168.84.83  netmask 255.255.255.0  broadcast 192.168.84.255
        inet6 fe80::5054:83ff:feaa:bb20  prefixlen 64  scopeid 0x20<link>
        ether 52:54:83:aa:bb:20  txqueuelen 1000  (Ethernet)
        RX packets 10698672  bytes 712975406 (679.9 MiB)
        RX errors 0  dropped 7120862  overruns 0  frame 0
        TX packets 1773  bytes 122490 (119.6 KiB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

eth21: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
        inet 192.168.85.83  netmask 255.255.255.0  broadcast 192.168.85.255
        inet6 fe80::5054:83ff:feaa:bb21  prefixlen 64  scopeid 0x20<link>
        ether 52:54:83:aa:bb:21  txqueuelen 1000  (Ethernet)
        RX packets 3574777  bytes 185891316 (177.2 MiB)
        RX errors 0  dropped 15  overruns 0  frame 0
        TX packets 1530  bytes 99712 (97.3 KiB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

lo: flags=73<UP,LOOPBACK,RUNNING>  mtu 65536
        inet 127.0.0.1  netmask 255.0.0.0
        inet6 ::1  prefixlen 128  scopeid 0x10<host>
        loop  txqueuelen 1  (Local Loopback)
        RX packets 34518600  bytes 4376071963 (4.0 GiB)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 34518600  bytes 4376071963 (4.0 GiB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0
~~~

宿主机包含三个物理网卡，分别是eth0、eth20与eth21，因此规划建立3个macvlan模式网络，考虑主控与线卡的macvlan模式网络为同一网段，为防止主控与线卡上的网段重合，通过ip-range选项设定主控与线卡的ip地址池，具体规划如下：

### 网络1规划

net_name：rcm_net1

subnet：192.168.83.0/24

gateway：192.168.83.1

parent_interface：eth0

ip_range：192.168.83.240/28（master） 192.168.83.224/28（lp）

### 网络2规划

net_name：rcm_net2

subnet：192.168.84.0/24

gateway：192.168.84.1

parent_interface：eth20

ip_range：192.168.84.240/28（master） 192.168.84.224/28（lp）

### 网络3规划

net_name：rcm_net3

subnet：192.168.85.0/24

gateway：192.168.85.1

parent_interface：eth21

ip_range：192.168.85.240/28（master） 192.168.85.224/28（lp）

## 网络建立

网络建立之前，主控系统的docker网络列表如下：

~~~shell
[root@jamza_vm_master_litepaas master]# docker network ls
NETWORK ID          NAME                DRIVER              SCOPE
a78327e048e0        bridge              bridge              local
b919f69968ec        docker_gwbridge     bridge              local
032c767b11de        host                host                local
cad8d79e6d05        none                null                local
[root@jamza_vm_master_litepaas master]#
~~~

通过docker network create命令，建立macvlan网络模式，如下：

~~~shell
[root@jamza_vm_master_litepaas master]# docker network create -d macvlan --subnet=192.168.83.0/24 --gateway=192.168.83.1 --ip-range=192.168.83.240/28 -o parent=eth0 rcm_net1
a3db1d77c949c20638d04fc0049e6522e318ff0dc6f647b609ab3196ee9ef50b
[root@jamza_vm_master_litepaas master]#
[root@jamza_vm_master_litepaas master]# docker network ls
NETWORK ID          NAME                DRIVER              SCOPE
a78327e048e0        bridge              bridge              local
b919f69968ec        docker_gwbridge     bridge              local
032c767b11de        host                host                local
cad8d79e6d05        none                null                local
a3db1d77c949        rcm_net1            macvlan             local
[root@jamza_vm_master_litepaas master]#
[root@jamza_vm_master_litepaas master]# docker network inspect rcm_net1
[
    {
        "Name": "rcm_net1",
        "Id": "a3db1d77c949c20638d04fc0049e6522e318ff0dc6f647b609ab3196ee9ef50b",
        "Created": "2019-07-31T13:38:59.184319993+08:00",
        "Scope": "local",
        "Driver": "macvlan",
        "EnableIPv6": false,
        "IPAM": {
            "Driver": "default",
            "Options": {},
            "Config": [
                {
                    "Subnet": "192.168.83.0/24",
                    "IPRange": "192.168.83.240/28",
                    "Gateway": "192.168.83.1"
                }
            ]
        },
        "Internal": false,
        "Attachable": false,
        "Containers": {},
        "Options": {
            "parent": "eth0"
        },
        "Labels": {}
    }
]
[root@jamza_vm_master_litepaas master]#
~~~

同理，创建另外两个macvlan网络：

~~~shell
[root@jamza_vm_master_litepaas master]# docker network create -d macvlan --subnet=192.168.84.0/24 --gateway=192.168.84.1 --ip-range=192.168.84.240/28 -o parent=eth20 rcm_net2
b95e405b383d0b3c6035c9f7ea556f315def34d9d4cf290f7ca2ba6272341d20
[root@jamza_vm_master_litepaas master]#
[root@jamza_vm_master_litepaas master]# docker network create -d macvlan --subnet=192.168.85.0/24 --gateway=192.168.85.1 --ip-range=192.168.85.240/28 -o parent=eth21 rcm_net3
2fe74eed7b69033582cfa44ba4d16f06feee0b1f56234d0640948da81f9674bc
[root@jamza_vm_master_litepaas master]#
[root@jamza_vm_master_litepaas master]# docker network ls
NETWORK ID          NAME                DRIVER              SCOPE
a78327e048e0        bridge              bridge              local
b919f69968ec        docker_gwbridge     bridge              local
032c767b11de        host                host                local
cad8d79e6d05        none                null                local
a3db1d77c949        rcm_net1            macvlan             local
b95e405b383d        rcm_net2            macvlan             local
2fe74eed7b69        rcm_net3            macvlan             local
[root@jamza_vm_master_litepaas master]#
~~~

在线卡主机上也创建同样的三个网络，区别是ip地址池不同：

~~~shell
[root@jamza_vm_lp0_litepaas master]# docker network ls
NETWORK ID          NAME                DRIVER              SCOPE
eae88252b139        bridge              bridge              local
bd5b745fb87f        docker_gwbridge     bridge              local
a20b3d67c74d        host                host                local
d8bdffcb4f6e        none                null                local
[root@jamza_vm_lp0_litepaas master]#
[root@jamza_vm_lp0_litepaas master]# docker network create -d macvlan --subnet=192.168.83.0/24 --gateway=192.168.83.1 --ip-range=192.168.83.224/28 -o parent=eth0 rcm_net1
afc8a5c72b2a87b89837cc096b8b4d6fcf312b83ef578bf05bc6be8fce4c0b7c
[root@jamza_vm_lp0_litepaas master]#
[root@jamza_vm_lp0_litepaas master]# docker network create -d macvlan --subnet=192.168.84.0/24 --gateway=192.168.84.1 --ip-range=192.168.84.224/28 -o parent=eth20 rcm_net2
9a6fda4d7da07317dad8edb3fdb2595a986631a5e2211ecf02f422bddc6de12b
[root@jamza_vm_lp0_litepaas master]#
[root@jamza_vm_lp0_litepaas master]# docker network create -d macvlan --subnet=192.168.85.0/24 --gateway=192.168.85.1 --ip-range=192.168.85.224/28 -o parent=eth21 rcm_net3
13f9679e4f1a521ed1506bb41b65340f9775e6bde0669192874b0c58f6822b33
[root@jamza_vm_lp0_litepaas master]#
[root@jamza_vm_lp0_litepaas master]# docker network ls
NETWORK ID          NAME                DRIVER              SCOPE
eae88252b139        bridge              bridge              local
bd5b745fb87f        docker_gwbridge     bridge              local
a20b3d67c74d        host                host                local
d8bdffcb4f6e        none                null                local
afc8a5c72b2a        rcm_net1            macvlan             local
9a6fda4d7da0        rcm_net2            macvlan             local
13f9679e4f1a        rcm_net3            macvlan             local
[root@jamza_vm_lp0_litepaas master]#
[root@jamza_vm_lp0_litepaas master]# docker network inspect rcm_net1
[
    {
        "Name": "rcm_net1",
        "Id": "afc8a5c72b2a87b89837cc096b8b4d6fcf312b83ef578bf05bc6be8fce4c0b7c",
        "Created": "2019-07-31T13:46:03.832634708+08:00",
        "Scope": "local",
        "Driver": "macvlan",
        "EnableIPv6": false,
        "IPAM": {
            "Driver": "default",
            "Options": {},
            "Config": [
                {
                    "Subnet": "192.168.83.0/24",
                    "IPRange": "192.168.83.224/28",
                    "Gateway": "192.168.83.1"
                }
            ]
        },
        "Internal": false,
        "Attachable": false,
        "Containers": {},
        "Options": {
            "parent": "eth0"
        },
        "Labels": {}
    }
]
[root@jamza_vm_lp0_litepaas master]#
~~~

## 创建连接至macvlan的Docker容器

在主控环境创建连接至三个macvlan网络的Docker容器，容器中已经创建了三个网卡，分别对应连接的三个macvlan网络，其ip地址也是对应的网段的ip地址池范围内的：

~~~shell
[root@jamza_vm_master_litepaas master]# docker ps -a
CONTAINER ID        IMAGE               COMMAND             CREATED             STATUS              PORTS               NAMES
[root@jamza_vm_master_litepaas master]#
[root@jamza_vm_master_litepaas master]# docker create -it --privileged=true --net rcm_net1 --name test busybox/x86_64:latest /bin/sh
4c4a70764b65f043a5ab6d37d5a49a38eab4fcaa3d2fb46adffcd0afe8f55559
[root@jamza_vm_master_litepaas master]#
[root@jamza_vm_master_litepaas master]# docker network connect rcm_net2 test
[root@jamza_vm_master_litepaas master]#
[root@jamza_vm_master_litepaas master]# docker network connect rcm_net3 test
[root@jamza_vm_master_litepaas master]#
[root@jamza_vm_master_litepaas master]# docker start -i test
/ #
/ # ifconfig
eth0      Link encap:Ethernet  HWaddr 02:42:C0:A8:53:F0
          inet addr:192.168.83.240  Bcast:0.0.0.0  Mask:255.255.255.0
          inet6 addr: fe80::42:c0ff:fea8:53f0/64 Scope:Link
          UP BROADCAST RUNNING MULTICAST  MTU:1500  Metric:1
          RX packets:0 errors:0 dropped:0 overruns:0 frame:0
          TX packets:7 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:0
          RX bytes:0 (0.0 B)  TX bytes:578 (578.0 B)

eth1      Link encap:Ethernet  HWaddr 02:42:C0:A8:54:F0
          inet addr:192.168.84.240  Bcast:0.0.0.0  Mask:255.255.255.0
          inet6 addr: fe80::42:c0ff:fea8:54f0/64 Scope:Link
          UP BROADCAST RUNNING MULTICAST  MTU:1500  Metric:1
          RX packets:0 errors:0 dropped:0 overruns:0 frame:0
          TX packets:7 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:0
          RX bytes:0 (0.0 B)  TX bytes:578 (578.0 B)

eth2      Link encap:Ethernet  HWaddr 02:42:C0:A8:55:F0
          inet addr:192.168.85.240  Bcast:0.0.0.0  Mask:255.255.255.0
          inet6 addr: fe80::42:c0ff:fea8:55f0/64 Scope:Link
          UP BROADCAST RUNNING MULTICAST  MTU:1500  Metric:1
          RX packets:0 errors:0 dropped:0 overruns:0 frame:0
          TX packets:7 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:0
          RX bytes:0 (0.0 B)  TX bytes:578 (578.0 B)

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

同理，在线卡主机环境中，创建连接至三个macvlan网络的Docker容器：

~~~shell
[root@jamza_vm_lp0_litepaas master]# docker ps -a
CONTAINER ID        IMAGE               COMMAND             CREATED             STATUS              PORTS               NAMES
[root@jamza_vm_lp0_litepaas master]#
[root@jamza_vm_lp0_litepaas master]# docker create -it --privileged=true --net rcm_net1 --name test busybox/x86_64:latest /bin/sh
9335b9268b7554476727749823e15530fb94b424ec8fb122f308724ac3250036
[root@jamza_vm_lp0_litepaas master]#
[root@jamza_vm_lp0_litepaas master]# docker network connect rcm_net2 test
[root@jamza_vm_lp0_litepaas master]#
[root@jamza_vm_lp0_litepaas master]# docker network connect rcm_net3 test
[root@jamza_vm_lp0_litepaas master]#
[root@jamza_vm_lp0_litepaas master]# docker start -i test
/ #
/ # ifconfig
eth0      Link encap:Ethernet  HWaddr 02:42:C0:A8:53:E0
          inet addr:192.168.83.224  Bcast:0.0.0.0  Mask:255.255.255.0
          inet6 addr: fe80::42:c0ff:fea8:53e0/64 Scope:Link
          UP BROADCAST RUNNING MULTICAST  MTU:1500  Metric:1
          RX packets:0 errors:0 dropped:0 overruns:0 frame:0
          TX packets:6 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:0
          RX bytes:0 (0.0 B)  TX bytes:508 (508.0 B)

eth1      Link encap:Ethernet  HWaddr 02:42:C0:A8:54:E0
          inet addr:192.168.84.224  Bcast:0.0.0.0  Mask:255.255.255.0
          inet6 addr: fe80::42:c0ff:fea8:54e0/64 Scope:Link
          UP BROADCAST RUNNING MULTICAST  MTU:1500  Metric:1
          RX packets:0 errors:0 dropped:0 overruns:0 frame:0
          TX packets:6 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:0
          RX bytes:0 (0.0 B)  TX bytes:508 (508.0 B)

eth2      Link encap:Ethernet  HWaddr 02:42:C0:A8:55:E0
          inet addr:192.168.85.224  Bcast:0.0.0.0  Mask:255.255.255.0
          inet6 addr: fe80::42:c0ff:fea8:55e0/64 Scope:Link
          UP BROADCAST RUNNING MULTICAST  MTU:1500  Metric:1
          RX packets:0 errors:0 dropped:0 overruns:0 frame:0
          TX packets:6 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:0
          RX bytes:0 (0.0 B)  TX bytes:508 (508.0 B)

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

## 主控与线卡环境中容器ping测试

在主控环境中的容器内，ping线卡环境中的容器ip地址，以及在线卡环境中的容器内，ping主控环境中的容器ip地址，显示均能够ping通：

~~~shell
#主控环境中的容器内，ping线卡环境上的容器ip地址
/ #
/ # ping 192.168.83.224
PING 192.168.83.224 (192.168.83.224): 56 data bytes
64 bytes from 192.168.83.224: seq=0 ttl=64 time=0.421 ms
64 bytes from 192.168.83.224: seq=1 ttl=64 time=0.544 ms
64 bytes from 192.168.83.224: seq=2 ttl=64 time=0.412 ms
64 bytes from 192.168.83.224: seq=3 ttl=64 time=0.383 ms
^C
--- 192.168.83.224 ping statistics ---
4 packets transmitted, 4 packets received, 0% packet loss
round-trip min/avg/max = 0.383/0.440/0.544 ms
/ #
/ # ping 192.168.84.224
PING 192.168.84.224 (192.168.84.224): 56 data bytes
64 bytes from 192.168.84.224: seq=0 ttl=64 time=0.458 ms
64 bytes from 192.168.84.224: seq=1 ttl=64 time=0.586 ms
64 bytes from 192.168.84.224: seq=2 ttl=64 time=0.560 ms
64 bytes from 192.168.84.224: seq=3 ttl=64 time=0.333 ms
^C
--- 192.168.84.224 ping statistics ---
4 packets transmitted, 4 packets received, 0% packet loss
round-trip min/avg/max = 0.333/0.484/0.586 ms
/ #
/ # ping 192.168.85.224
PING 192.168.85.224 (192.168.85.224): 56 data bytes
64 bytes from 192.168.85.224: seq=0 ttl=64 time=0.341 ms
64 bytes from 192.168.85.224: seq=1 ttl=64 time=0.465 ms
64 bytes from 192.168.85.224: seq=2 ttl=64 time=0.266 ms
64 bytes from 192.168.85.224: seq=3 ttl=64 time=0.514 ms
^C
--- 192.168.85.224 ping statistics ---
4 packets transmitted, 4 packets received, 0% packet loss
round-trip min/avg/max = 0.266/0.396/0.514 ms
/ #

#线卡环境中的容器内，ping主控环境上的容器ip地址
/ #
/ # ping 192.168.83.240
PING 192.168.83.240 (192.168.83.240): 56 data bytes
64 bytes from 192.168.83.240: seq=0 ttl=64 time=0.428 ms
64 bytes from 192.168.83.240: seq=1 ttl=64 time=0.415 ms
64 bytes from 192.168.83.240: seq=2 ttl=64 time=0.384 ms
64 bytes from 192.168.83.240: seq=3 ttl=64 time=0.455 ms
^C
--- 192.168.83.240 ping statistics ---
4 packets transmitted, 4 packets received, 0% packet loss
round-trip min/avg/max = 0.384/0.420/0.455 ms
/ #
/ # ping 192.168.84.240
PING 192.168.84.240 (192.168.84.240): 56 data bytes
64 bytes from 192.168.84.240: seq=0 ttl=64 time=0.356 ms
64 bytes from 192.168.84.240: seq=1 ttl=64 time=0.372 ms
64 bytes from 192.168.84.240: seq=2 ttl=64 time=0.415 ms
64 bytes from 192.168.84.240: seq=3 ttl=64 time=0.501 ms
^C
--- 192.168.84.240 ping statistics ---
4 packets transmitted, 4 packets received, 0% packet loss
round-trip min/avg/max = 0.356/0.411/0.501 ms
/ #
/ # ping 192.168.85.240
PING 192.168.85.240 (192.168.85.240): 56 data bytes
64 bytes from 192.168.85.240: seq=0 ttl=64 time=0.241 ms
64 bytes from 192.168.85.240: seq=1 ttl=64 time=0.340 ms
64 bytes from 192.168.85.240: seq=2 ttl=64 time=0.449 ms
64 bytes from 192.168.85.240: seq=3 ttl=64 time=0.286 ms
^C
--- 192.168.85.240 ping statistics ---
4 packets transmitted, 4 packets received, 0% packet loss
round-trip min/avg/max = 0.241/0.329/0.449 ms
/ #
~~~