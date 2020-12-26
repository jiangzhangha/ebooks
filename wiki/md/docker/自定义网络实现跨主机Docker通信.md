---
title: 自定义网络实现跨主机Docker通信
date: 2020-07-08 13:42:45
categories: docker
index_img: /img/articles/docker.jpg
tags:
	- docker
	- 跨主机通信
	- 自定义网络
---

## 环境参数分配

### 主控master

宿主机ip：eth0	192.168.83.83

容器分配网段：172.172.0.0/24

启动容器指定ip：172.172.0.10

### 线卡lp

宿主机ip：eth0	192.168.83.85

容器分配网段：172.172.1.0/24

启动容器指定ip：172.172.1.10

## 主控宿主机操作

创建自定义网络之前，主机环境：

~~~shell
[root@jamza_vm_master_litepaas master]# ifconfig
docker0: flags=4099<UP,BROADCAST,MULTICAST>  mtu 1500
        inet 172.17.6.252  netmask 255.255.255.0  broadcast 0.0.0.0
        inet6 fe80::42:c6ff:fe05:2363  prefixlen 64  scopeid 0x20<link>
        ether 02:42:c6:05:23:63  txqueuelen 0  (Ethernet)
        RX packets 562  bytes 35144 (34.3 KiB)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 223  bytes 19338 (18.8 KiB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

docker_gwbridge: flags=4099<UP,BROADCAST,MULTICAST>  mtu 1500
        inet 172.18.0.1  netmask 255.255.240.0  broadcast 0.0.0.0
        inet6 fe80::42:42ff:fe41:eebc  prefixlen 64  scopeid 0x20<link>
        ether 02:42:42:41:ee:bc  txqueuelen 0  (Ethernet)
        RX packets 3527525  bytes 183431812 (174.9 MiB)
        RX errors 0  dropped 15  overruns 0  frame 0
        TX packets 1455  bytes 93698 (91.5 KiB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

eth0: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
        inet 192.168.83.83  netmask 255.255.255.0  broadcast 192.168.83.255
        inet6 fe80::5054:83ff:feaa:bb00  prefixlen 64  scopeid 0x20<link>
        ether 52:54:83:aa:bb:00  txqueuelen 1000  (Ethernet)
        RX packets 83274207  bytes 364679223968 (339.6 GiB)
        RX errors 0  dropped 15  overruns 0  frame 0
        TX packets 94593718  bytes 213226263982 (198.5 GiB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

eth20: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
        inet 192.168.84.83  netmask 255.255.255.0  broadcast 192.168.84.255
        inet6 fe80::5054:83ff:feaa:bb20  prefixlen 64  scopeid 0x20<link>
        ether 52:54:83:aa:bb:20  txqueuelen 1000  (Ethernet)
        RX packets 10557267  bytes 703540934 (670.9 MiB)
        RX errors 0  dropped 7026900  overruns 0  frame 0
        TX packets 1441  bytes 92630 (90.4 KiB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

eth21: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
        inet 192.168.85.83  netmask 255.255.255.0  broadcast 192.168.85.255
        inet6 fe80::5054:83ff:feaa:bb21  prefixlen 64  scopeid 0x20<link>
        ether 52:54:83:aa:bb:21  txqueuelen 1000  (Ethernet)
        RX packets 3527525  bytes 183431812 (174.9 MiB)
        RX errors 0  dropped 15  overruns 0  frame 0
        TX packets 1455  bytes 93698 (91.5 KiB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

lo: flags=73<UP,LOOPBACK,RUNNING>  mtu 65536
        inet 127.0.0.1  netmask 255.0.0.0
        inet6 ::1  prefixlen 128  scopeid 0x10<host>
        loop  txqueuelen 1  (Local Loopback)
        RX packets 34438746  bytes 4372079263 (4.0 GiB)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 34438746  bytes 4372079263 (4.0 GiB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

[root@jamza_vm_master_litepaas master]# docker network ls
NETWORK ID          NAME                DRIVER              SCOPE
a78327e048e0        bridge              bridge              local
b919f69968ec        docker_gwbridge     bridge              local
032c767b11de        host                host                local
cad8d79e6d05        none                null                local
~~~

创建自定义网络，指定网桥名称为docker-br0，当然也可以取其他的名字，指定网桥的网段为172.172.0.0/24：

~~~shell
[root@jamza_vm_master_litepaas master]# docker network create --subnet=172.172.0.0/24 docker-br0
760119ea59076b90d02d0d8ee69fff176ac07bb30b8c78b4f4466fd7c46c2f1d
[root@jamza_vm_master_litepaas master]#
~~~

创建网络后，查看当前主机的网络环境，环境中多出了名称为br-760119ea5907的网卡，且docker网络出现刚创建的名称为docker-br0的网络：

~~~shell
[root@jamza_vm_master_litepaas master]# ifconfig
br-760119ea5907: flags=4099<UP,BROADCAST,MULTICAST>  mtu 1500
        inet 172.172.0.1  netmask 255.255.255.0  broadcast 0.0.0.0
        ether 02:42:0c:76:ad:5d  txqueuelen 0  (Ethernet)
        RX packets 10557312  bytes 703543934 (670.9 MiB)
        RX errors 0  dropped 7026930  overruns 0  frame 0
        TX packets 1441  bytes 92630 (90.4 KiB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

docker0: flags=4099<UP,BROADCAST,MULTICAST>  mtu 1500
        inet 172.17.6.252  netmask 255.255.255.0  broadcast 0.0.0.0
        inet6 fe80::42:c6ff:fe05:2363  prefixlen 64  scopeid 0x20<link>
        ether 02:42:c6:05:23:63  txqueuelen 0  (Ethernet)
        RX packets 562  bytes 35144 (34.3 KiB)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 223  bytes 19338 (18.8 KiB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

docker_gwbridge: flags=4099<UP,BROADCAST,MULTICAST>  mtu 1500
        inet 172.18.0.1  netmask 255.255.240.0  broadcast 0.0.0.0
        inet6 fe80::42:42ff:fe41:eebc  prefixlen 64  scopeid 0x20<link>
        ether 02:42:42:41:ee:bc  txqueuelen 0  (Ethernet)
        RX packets 3527540  bytes 183432592 (174.9 MiB)
        RX errors 0  dropped 15  overruns 0  frame 0
        TX packets 1455  bytes 93698 (91.5 KiB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

eth0: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
        inet 192.168.83.83  netmask 255.255.255.0  broadcast 192.168.83.255
        inet6 fe80::5054:83ff:feaa:bb00  prefixlen 64  scopeid 0x20<link>
        ether 52:54:83:aa:bb:00  txqueuelen 1000  (Ethernet)
        RX packets 83274510  bytes 364679281561 (339.6 GiB)
        RX errors 0  dropped 15  overruns 0  frame 0
        TX packets 94593962  bytes 213226325066 (198.5 GiB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

eth20: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
        inet 192.168.84.83  netmask 255.255.255.0  broadcast 192.168.84.255
        inet6 fe80::5054:83ff:feaa:bb20  prefixlen 64  scopeid 0x20<link>
        ether 52:54:83:aa:bb:20  txqueuelen 1000  (Ethernet)
        RX packets 10557312  bytes 703543934 (670.9 MiB)
        RX errors 0  dropped 7026930  overruns 0  frame 0
        TX packets 1441  bytes 92630 (90.4 KiB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

eth21: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
        inet 192.168.85.83  netmask 255.255.255.0  broadcast 192.168.85.255
        inet6 fe80::5054:83ff:feaa:bb21  prefixlen 64  scopeid 0x20<link>
        ether 52:54:83:aa:bb:21  txqueuelen 1000  (Ethernet)
        RX packets 3527540  bytes 183432592 (174.9 MiB)
        RX errors 0  dropped 15  overruns 0  frame 0
        TX packets 1455  bytes 93698 (91.5 KiB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

lo: flags=73<UP,LOOPBACK,RUNNING>  mtu 65536
        inet 127.0.0.1  netmask 255.0.0.0
        inet6 ::1  prefixlen 128  scopeid 0x10<host>
        loop  txqueuelen 1  (Local Loopback)
        RX packets 34438790  bytes 4372081463 (4.0 GiB)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 34438790  bytes 4372081463 (4.0 GiB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

[root@jamza_vm_master_litepaas master]# docker network ls
NETWORK ID          NAME                DRIVER              SCOPE
a78327e048e0        bridge              bridge              local
760119ea5907        docker-br0          bridge              local
b919f69968ec        docker_gwbridge     bridge              local
032c767b11de        host                host                local
cad8d79e6d05        none                null                local
~~~

创建容器，并使用自定义的网络，指定容器内的ip地址为172.172.0.10，查看容器内的网络环境，ip配置为设定值：

~~~shell
[root@jamza_vm_master_litepaas master]# docker run -it --net docker-br0 --ip 172.172.0.10 --name master busybox/x86_64 /bin/sh
/ # ifconfig
eth0      Link encap:Ethernet  HWaddr 02:42:AC:AC:00:0A
          inet addr:172.172.0.10  Bcast:0.0.0.0  Mask:255.255.255.0
          inet6 addr: fe80::42:acff:feac:a/64 Scope:Link
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

~~~

在容器内ping当前宿主机的网络地址，显示可以ping通：

~~~shell
/ #
/ # ping 192.168.83.83
PING 192.168.83.83 (192.168.83.83): 56 data bytes
64 bytes from 192.168.83.83: seq=0 ttl=64 time=0.059 ms
64 bytes from 192.168.83.83: seq=1 ttl=64 time=0.058 ms
64 bytes from 192.168.83.83: seq=2 ttl=64 time=0.064 ms
64 bytes from 192.168.83.83: seq=3 ttl=64 time=0.048 ms
64 bytes from 192.168.83.83: seq=4 ttl=64 time=0.048 ms
^C
--- 192.168.83.83 ping statistics ---
5 packets transmitted, 5 packets received, 0% packet loss
round-trip min/avg/max = 0.048/0.055/0.064 ms
/ #
/ #
/ # ping 192.168.84.83
PING 192.168.84.83 (192.168.84.83): 56 data bytes
64 bytes from 192.168.84.83: seq=0 ttl=64 time=0.061 ms
64 bytes from 192.168.84.83: seq=1 ttl=64 time=0.057 ms
64 bytes from 192.168.84.83: seq=2 ttl=64 time=0.047 ms
64 bytes from 192.168.84.83: seq=3 ttl=64 time=0.046 ms
64 bytes from 192.168.84.83: seq=4 ttl=64 time=0.046 ms
^C
--- 192.168.84.83 ping statistics ---
5 packets transmitted, 5 packets received, 0% packet loss
round-trip min/avg/max = 0.046/0.051/0.061 ms
/ #
/ #
/ #
/ # ping 192.168.85.83
PING 192.168.85.83 (192.168.85.83): 56 data bytes
64 bytes from 192.168.85.83: seq=0 ttl=64 time=0.092 ms
64 bytes from 192.168.85.83: seq=1 ttl=64 time=0.049 ms
64 bytes from 192.168.85.83: seq=2 ttl=64 time=0.050 ms
64 bytes from 192.168.85.83: seq=3 ttl=64 time=0.050 ms
64 bytes from 192.168.85.83: seq=4 ttl=64 time=0.057 ms
64 bytes from 192.168.85.83: seq=5 ttl=64 time=0.049 ms
^C
--- 192.168.85.83 ping statistics ---
6 packets transmitted, 6 packets received, 0% packet loss
round-trip min/avg/max = 0.049/0.057/0.092 ms
/ #
/ #
~~~

## 线卡宿主机操作

创建自定义网络之前，主机环境：

~~~shell
[root@jamza_vm_lp0_litepaas master]# ifconfig
docker0: flags=4099<UP,BROADCAST,MULTICAST>  mtu 1500
        inet 172.17.9.252  netmask 255.255.255.0  broadcast 0.0.0.0
        inet6 fe80::42:aeff:feb6:63d3  prefixlen 64  scopeid 0x20<link>
        ether 02:42:ae:b6:63:d3  txqueuelen 0  (Ethernet)
        RX packets 517  bytes 32240 (31.4 KiB)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 244  bytes 21080 (20.5 KiB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

docker_gwbridge: flags=4099<UP,BROADCAST,MULTICAST>  mtu 1500
        inet 172.18.0.1  netmask 255.255.240.0  broadcast 0.0.0.0
        inet6 fe80::42:e1ff:fe2d:fc02  prefixlen 64  scopeid 0x20<link>
        ether 02:42:e1:2d:fc:02  txqueuelen 0  (Ethernet)
        RX packets 6230639  bytes 324090678 (309.0 MiB)
        RX errors 0  dropped 13  overruns 0  frame 0
        TX packets 41  bytes 2798 (2.7 KiB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

eth0: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
        inet 192.168.83.85  netmask 255.255.255.0  broadcast 192.168.83.255
        inet6 fe80::5054:83ff:fecc:dd00  prefixlen 64  scopeid 0x20<link>
        ether 52:54:83:cc:dd:00  txqueuelen 1000  (Ethernet)
        RX packets 60862286  bytes 102008047050 (95.0 GiB)
        RX errors 0  dropped 13  overruns 0  frame 0
        TX packets 54156229  bytes 36551403265 (34.0 GiB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

eth20: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
        inet 192.168.84.85  netmask 255.255.255.0  broadcast 192.168.84.255
        inet6 fe80::5054:83ff:fecc:dd20  prefixlen 64  scopeid 0x20<link>
        ether 52:54:83:cc:dd:20  txqueuelen 1000  (Ethernet)
        RX packets 37009669  bytes 8716621422 (8.1 GiB)
        RX errors 0  dropped 13277522  overruns 0  frame 0
        TX packets 15165149  bytes 13005633643 (12.1 GiB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

eth21: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
        inet 192.168.85.85  netmask 255.255.255.0  broadcast 192.168.85.255
        inet6 fe80::5054:83ff:fecc:dd21  prefixlen 64  scopeid 0x20<link>
        ether 52:54:83:cc:dd:21  txqueuelen 1000  (Ethernet)
        RX packets 6230639  bytes 324090678 (309.0 MiB)
        RX errors 0  dropped 13  overruns 0  frame 0
        TX packets 41  bytes 2798 (2.7 KiB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

lo: flags=73<UP,LOOPBACK,RUNNING>  mtu 65536
        inet 127.0.0.1  netmask 255.0.0.0
        inet6 ::1  prefixlen 128  scopeid 0x10<host>
        loop  txqueuelen 1  (Local Loopback)
        RX packets 37422659  bytes 9979561045 (9.2 GiB)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 37422659  bytes 9979561045 (9.2 GiB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0
        
[root@jamza_vm_lp0_litepaas master]# docker network ls
NETWORK ID          NAME                DRIVER              SCOPE
eae88252b139        bridge              bridge              local
bd5b745fb87f        docker_gwbridge     bridge              local
a20b3d67c74d        host                host                local
d8bdffcb4f6e        none                null                local
~~~

创建自定义网络，指定网桥名称为docker-br0，当然也可以取其他的名字，指定网桥的网段为172.172.1.0/24：

~~~shell
[root@jamza_vm_lp0_litepaas master]# docker network create --subnet=172.172.1.0/24 docker-br0
563243e33ad94e59bdec3a8cb7b150c1157a7890c13489acd063c23cc2e4f51a
[root@jamza_vm_lp0_litepaas master]#
~~~

创建网络后，查看当前主机的网络环境，环境中多出了名称为br-563243e33ad9的网卡，且docker网络出现刚创建的名称为docker-br0的网络：

~~~shell
[root@jamza_vm_lp0_litepaas master]# ifconfig
br-563243e33ad9: flags=4099<UP,BROADCAST,MULTICAST>  mtu 1500
        inet 172.172.1.1  netmask 255.255.255.0  broadcast 0.0.0.0
        ether 02:42:54:27:29:5a  txqueuelen 0  (Ethernet)
        RX packets 37009669  bytes 8716621422 (8.1 GiB)
        RX errors 0  dropped 13277522  overruns 0  frame 0
        TX packets 15165149  bytes 13005633643 (12.1 GiB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

docker0: flags=4099<UP,BROADCAST,MULTICAST>  mtu 1500
        inet 172.17.9.252  netmask 255.255.255.0  broadcast 0.0.0.0
        inet6 fe80::42:aeff:feb6:63d3  prefixlen 64  scopeid 0x20<link>
        ether 02:42:ae:b6:63:d3  txqueuelen 0  (Ethernet)
        RX packets 517  bytes 32240 (31.4 KiB)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 244  bytes 21080 (20.5 KiB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

docker_gwbridge: flags=4099<UP,BROADCAST,MULTICAST>  mtu 1500
        inet 172.18.0.1  netmask 255.255.240.0  broadcast 0.0.0.0
        inet6 fe80::42:e1ff:fe2d:fc02  prefixlen 64  scopeid 0x20<link>
        ether 02:42:e1:2d:fc:02  txqueuelen 0  (Ethernet)
        RX packets 6230639  bytes 324090678 (309.0 MiB)
        RX errors 0  dropped 13  overruns 0  frame 0
        TX packets 41  bytes 2798 (2.7 KiB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

eth0: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
        inet 192.168.83.85  netmask 255.255.255.0  broadcast 192.168.83.255
        inet6 fe80::5054:83ff:fecc:dd00  prefixlen 64  scopeid 0x20<link>
        ether 52:54:83:cc:dd:00  txqueuelen 1000  (Ethernet)
        RX packets 60862286  bytes 102008047050 (95.0 GiB)
        RX errors 0  dropped 13  overruns 0  frame 0
        TX packets 54156229  bytes 36551403265 (34.0 GiB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

eth20: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
        inet 192.168.84.85  netmask 255.255.255.0  broadcast 192.168.84.255
        inet6 fe80::5054:83ff:fecc:dd20  prefixlen 64  scopeid 0x20<link>
        ether 52:54:83:cc:dd:20  txqueuelen 1000  (Ethernet)
        RX packets 37009669  bytes 8716621422 (8.1 GiB)
        RX errors 0  dropped 13277522  overruns 0  frame 0
        TX packets 15165149  bytes 13005633643 (12.1 GiB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

eth21: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
        inet 192.168.85.85  netmask 255.255.255.0  broadcast 192.168.85.255
        inet6 fe80::5054:83ff:fecc:dd21  prefixlen 64  scopeid 0x20<link>
        ether 52:54:83:cc:dd:21  txqueuelen 1000  (Ethernet)
        RX packets 6230639  bytes 324090678 (309.0 MiB)
        RX errors 0  dropped 13  overruns 0  frame 0
        TX packets 41  bytes 2798 (2.7 KiB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

lo: flags=73<UP,LOOPBACK,RUNNING>  mtu 65536
        inet 127.0.0.1  netmask 255.0.0.0
        inet6 ::1  prefixlen 128  scopeid 0x10<host>
        loop  txqueuelen 1  (Local Loopback)
        RX packets 37422659  bytes 9979561045 (9.2 GiB)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 37422659  bytes 9979561045 (9.2 GiB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

[root@jamza_vm_lp0_litepaas master]# docker network ls
NETWORK ID          NAME                DRIVER              SCOPE
eae88252b139        bridge              bridge              local
563243e33ad9        docker-br0          bridge              local
bd5b745fb87f        docker_gwbridge     bridge              local
a20b3d67c74d        host                host                local
d8bdffcb4f6e        none                null                local
~~~

创建容器，并使用自定义的网络，指定容器内的ip地址为172.172.1.10，查看容器内的网络环境，ip配置为设定值：

~~~shell
[root@jamza_vm_lp0_litepaas master]# docker run -it --net docker-br0 --ip 172.172.1.10 --name lp busybox/x86_64 /bin/sh
/ #
/ # ifconfig
eth0      Link encap:Ethernet  HWaddr 02:42:AC:AC:01:0A
          inet addr:172.172.1.10  Bcast:0.0.0.0  Mask:255.255.255.0
          inet6 addr: fe80::42:acff:feac:10a/64 Scope:Link
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
~~~

在容器内ping当前宿主机的网络地址，显示可以ping通：

~~~shell
/ #
/ # ping 192.168.83.85
PING 192.168.83.85 (192.168.83.85): 56 data bytes
64 bytes from 192.168.83.85: seq=0 ttl=64 time=0.046 ms
64 bytes from 192.168.83.85: seq=1 ttl=64 time=0.052 ms
64 bytes from 192.168.83.85: seq=2 ttl=64 time=0.051 ms
64 bytes from 192.168.83.85: seq=3 ttl=64 time=0.050 ms
64 bytes from 192.168.83.85: seq=4 ttl=64 time=0.050 ms
^C
--- 192.168.83.85 ping statistics ---
5 packets transmitted, 5 packets received, 0% packet loss
round-trip min/avg/max = 0.046/0.049/0.052 ms
/ #
/ #
/ # ping 192.168.84.85
PING 192.168.84.85 (192.168.84.85): 56 data bytes
64 bytes from 192.168.84.85: seq=0 ttl=64 time=0.044 ms
64 bytes from 192.168.84.85: seq=1 ttl=64 time=0.065 ms
64 bytes from 192.168.84.85: seq=2 ttl=64 time=0.050 ms
64 bytes from 192.168.84.85: seq=3 ttl=64 time=0.050 ms
64 bytes from 192.168.84.85: seq=4 ttl=64 time=0.048 ms
^C
--- 192.168.84.85 ping statistics ---
5 packets transmitted, 5 packets received, 0% packet loss
round-trip min/avg/max = 0.044/0.051/0.065 ms
/ #
/ #
/ # ping 192.168.85.85
PING 192.168.85.85 (192.168.85.85): 56 data bytes
64 bytes from 192.168.85.85: seq=0 ttl=64 time=0.089 ms
64 bytes from 192.168.85.85: seq=1 ttl=64 time=0.049 ms
64 bytes from 192.168.85.85: seq=2 ttl=64 time=0.049 ms
64 bytes from 192.168.85.85: seq=3 ttl=64 time=0.050 ms
64 bytes from 192.168.85.85: seq=4 ttl=64 time=0.050 ms
^C
--- 192.168.85.85 ping statistics ---
5 packets transmitted, 5 packets received, 0% packet loss
round-trip min/avg/max = 0.049/0.057/0.089 ms
/ #
/ #
~~~

## 跨主机容器互通

在主控宿主机上的容器与线卡宿主机上的容器之间互相不能ping通：

~~~shell
#主控上的容器ping线卡上的容器
/ #
/ # ping 172.172.1.10
PING 172.172.1.10 (172.172.1.10): 56 data bytes
^C
--- 172.172.1.10 ping statistics ---
3 packets transmitted, 0 packets received, 100% packet loss
/ #
/ #

#线卡上的容器ping主控上的容器
/ #
/ # ping 172.172.0.10
PING 172.172.0.10 (172.172.0.10): 56 data bytes
^C
--- 172.172.0.10 ping statistics ---
2 packets transmitted, 0 packets received, 100% packet loss
/ #
/ #
~~~

添加路由规则，在主控宿主机上，设定路由172.172.1.0/24网络，经过线卡（192.168.83.85）上的eth0物理网桥：

~~~shell
[root@jamza_vm_master_litepaas master]# ip route add 172.172.1.0/24 via 192.168.83.85 dev eth0
[root@jamza_vm_master_litepaas master]#
[root@jamza_vm_master_litepaas master]# ip route
default via 192.168.83.1 dev eth0
169.254.0.0/16 dev eth0 scope link metric 1002
169.254.0.0/16 dev eth20 scope link metric 1003
169.254.0.0/16 dev eth21 scope link metric 1004
172.17.6.0/24 dev docker0 proto kernel scope link src 172.17.6.252
172.18.0.0/20 dev docker_gwbridge proto kernel scope link src 172.18.0.1
172.172.0.0/24 dev br-760119ea5907 proto kernel scope link src 172.172.0.1
172.172.1.0/24 via 192.168.83.85 dev eth0
192.168.83.0/24 dev eth0 proto kernel scope link src 192.168.83.83
192.168.84.0/24 dev eth20 proto kernel scope link src 192.168.84.83
192.168.85.0/24 dev eth21 proto kernel scope link src 192.168.85.83
~~~

在线卡宿主机上，设定路由172.172.0.0/24网络，经过主控（192.168.83.83）上的eth0物理网桥：

~~~shell
[root@jamza_vm_lp0_litepaas master]# ip route add 172.172.0.0/24 via 192.168.83.83 dev eth0
[root@jamza_vm_lp0_litepaas master]#
[root@jamza_vm_lp0_litepaas master]#
[root@jamza_vm_lp0_litepaas master]#
[root@jamza_vm_lp0_litepaas master]# ip route
default via 192.168.83.1 dev eth0
169.254.0.0/16 dev eth0 scope link metric 1002
169.254.0.0/16 dev eth20 scope link metric 1003
169.254.0.0/16 dev eth21 scope link metric 1004
172.17.9.0/24 dev docker0 proto kernel scope link src 172.17.9.252
172.18.0.0/20 dev docker_gwbridge proto kernel scope link src 172.18.0.1
172.172.0.0/24 via 192.168.83.83 dev eth0
172.172.1.0/24 dev br-563243e33ad9 proto kernel scope link src 172.172.1.1
192.168.83.0/24 dev eth0 proto kernel scope link src 192.168.83.85
192.168.84.0/24 dev eth20 proto kernel scope link src 192.168.84.85
192.168.85.0/24 dev eth21 proto kernel scope link src 192.168.85.85
~~~

在主控宿主机上的容器与线卡宿主机上的容器之间互相能够ping通：

~~~shell
#主控上的容器ping线卡上的容器
/ #
/ # ping 172.172.1.10
PING 172.172.1.10 (172.172.1.10): 56 data bytes
64 bytes from 172.172.1.10: seq=0 ttl=62 time=0.492 ms
64 bytes from 172.172.1.10: seq=1 ttl=62 time=0.558 ms
64 bytes from 172.172.1.10: seq=2 ttl=62 time=0.449 ms
64 bytes from 172.172.1.10: seq=3 ttl=62 time=0.491 ms
64 bytes from 172.172.1.10: seq=4 ttl=62 time=0.570 ms
^C
--- 172.172.1.10 ping statistics ---
5 packets transmitted, 5 packets received, 0% packet loss
round-trip min/avg/max = 0.449/0.512/0.570 ms
/ #

#线卡上的容器ping主控上的容器
/ #
/ # ping 172.172.0.10
PING 172.172.0.10 (172.172.0.10): 56 data bytes
64 bytes from 172.172.0.10: seq=0 ttl=62 time=0.458 ms
64 bytes from 172.172.0.10: seq=1 ttl=62 time=0.490 ms
64 bytes from 172.172.0.10: seq=2 ttl=62 time=0.430 ms
64 bytes from 172.172.0.10: seq=3 ttl=62 time=0.466 ms
64 bytes from 172.172.0.10: seq=4 ttl=62 time=0.474 ms
64 bytes from 172.172.0.10: seq=5 ttl=62 time=0.609 ms
^C
--- 172.172.0.10 ping statistics ---
6 packets transmitted, 6 packets received, 0% packet loss
round-trip min/avg/max = 0.430/0.487/0.609 ms
/ #
~~~