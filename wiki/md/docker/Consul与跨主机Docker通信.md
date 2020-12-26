---
title: Consul与跨主机Docker通信
date: 2020-07-08 13:49:18
categories: docker
index_img: /img/articles/docker.jpg
tags:
	- docker
	- 跨主机通信
	- consul
---

# Consul

## 介绍

Consul包含多个组件,但是作为一个整体,为你的基础设施提供服务发现和服务配置的工具.他提供以下关键特性：

1. 服务发现。Consul的客户端可用提供一个服务，比如 api 或者mysql ，另外一些客户端可用使用Consul去发现一个指定服务的提供者。通过DNS或者HTTP应用程序可用很容易的找到他所依赖的服务。
2. 健康检查。Consul客户端可用提供任意数量的健康检查，指定一个服务(比如：webserver是否返回了200 OK 状态码)或者使用本地节点(比如：内存使用是否大于90%).。这个信息可由operator用来监视集群的健康。被服务发现组件用来避免将流量发送到不健康的主机。
3. Key/Value存储。应用程序可用根据自己的需要使用Consul的层级的Key/Value存储。比如动态配置，功能标记，协调，领袖选举等等，简单的HTTP API让他更易于使用。
4. 多数据中心。Consul支持开箱即用的多数据中心。这意味着用户不需要担心需要建立额外的抽象层让业务扩展到多个区域。

## 基础架构

Consul是一个分布式高可用的系统，这节将包含一些基础，我们忽略掉一些细节这样你可以快速了解Consul是如何工作的，如果要了解更多细节，请参考深入的架构描述。

每个提供服务给Consul的阶段都运行了一个Consul agent 。发现服务或者设置和获取 key/value存储的数据不是必须运行agent，这个agent是负责对节点自身和节点上的服务进行健康检查的。

Agent与一个和多个Consul Server 进行交互。Consul Server 用于存放和复制数据。server自行选举一个领袖。虽然Consul可以运行在一台server ，但是建议使用3到5台来避免失败情况下数据的丢失。每个数据中心建议配置一个server集群。

你的基础设施中需要发现其他服务的组件可以查询任何一个Consul 的server或者 agent，Agent会自动转发请求到server。

每个数据中运行了一个Consul server集群。当一个跨数据中心的服务发现和配置请求创建时，本地Consul Server转发请求到远程的数据中心并返回结果。

## 安装Consul

在官方网站找到适合你系统的安装包下载，Consul打包为一个zip文件。

官方下载网址：https://www.consul.io/downloads.html

下载后解开压缩包，拷贝Consul到你的PATH路径中。在Unix系统中`~/bin`和`/usr/local/bin`是通常的安装目录。根据你是想为单个用户安装还是给整个系统安装来选择。在Windows系统中有可以安装到`%PATH%`的路径中。

## 验证安装

完成安装后，通过打开一个新终端窗口检查`consul`安装是否成功。通过执行 `consul`你应该看到类似下面的输出：

~~~shell
[root@dhcp-10-201-102-248 ~]# consul
usage: consul [--version] [--help] <command> [<args>]
Available commands are:
    agent          Runs a Consul agent
    configtest     Validate config file
    event          Fire a new event
    exec           Executes a command on Consul nodes
    force-leave    Forces a member of the cluster to enter the "left" state
    info           Provides debugging information for operators
    join           Tell Consul agent to join cluster
    keygen         Generates a new encryption key
    keyring        Manages gossip layer encryption keys
    kv             Interact with the key-value store
    leave          Gracefully leaves the Consul cluster and shuts down
    lock           Execute a command holding a lock
    maint          Controls node or service maintenance mode
    members        Lists the members of a Consul cluster
    monitor        Stream logs from a Consul agent
    operator       Provides cluster-level tools for Consul operators
    reload         Triggers the agent to reload configuration files
    rtt            Estimates network round trip time between nodes
    snapshot       Saves, restores and inspects snapshots of Consul server state
    version        Prints the Consul version
    watch          Watch for changes in Consul
~~~

如果你得到一个`consul not be found`的错误，你的`PATH`可能没有正确设置，请返回检查你的`consul`的安装路径是否包含在`PATH`中。

## 运行Agent

完成Consul的安装后，必须运行agent。agent可以运行为`server`或`client`模式。每个数据中心至少必须拥有一台server 。建议在一个集群中有3或者5个server，部署单一的server，在出现失败时会不可避免的造成数据丢失。

其他的agent运行为client模式。一个client是一个非常轻量级的进程，用于注册服务，运行健康检查和转发对server的查询。agent必须在集群中的每个主机上运行。

## 启动Consul Server

启动Consul Server的典型命令如下：

~~~shell
consul agent -server -bootstrap-expect 3 -data-dir /tmp/consul -node=s1 -bind=10.201.102.198 -ui-dir ./consul_ui/ -rejoin -config-dir=/etc/consul.d/ -client 0.0.0.0
~~~

1. `-server` ： 定义agent运行在server模式；
2. `-bootstrap-expect` ：在一个datacenter中期望提供的server节点数目，当该值提供的时候，consul一直等到达到指定sever数目的时候才会引导整个集群，该标记不能和bootstrap共用；
3. `-bind`：该地址用来在集群内部的通讯，集群内的所有节点到地址都必须是可达的，默认是0.0.0.0；
4. `-node`：节点在集群中的名称，在一个集群中必须是唯一的，默认是该节点的主机名；
5. `-ui-dir`： 提供存放web ui资源的路径，该目录必须是可读的；
6. `-rejoin`：使consul忽略先前的离开，在再次启动后仍旧尝试加入集群中；
7. `-config-dir`：配置文件目录，里面所有以.json结尾的文件都会被加载；
8. `-client`：consul服务侦听地址，这个地址提供HTTP、DNS、RPC等服务，默认是127.0.0.1所以不对外提供服务，如果你要对外提供服务改成0.0.0.0；

~~~shell
[root@dhcp-10-201-102-198 consul]# consul agent -server -bootstrap-expect 1 -data-dir /tmp/consul -node=s1 -bind=10.201.102.198 -ui-dir ./consul_ui/ -rejoin -config-dir=/etc/consul.d/ -client 0.0.0.0
==> WARNING: Expect Mode enabled, expecting 3 servers
==> Starting Consul agent...
==> Starting Consul agent RPC...
==> Consul agent running!
           Version: 'v0.7.4'
           Node ID: '422ec677-74ef-8f29-2f22-01effeed6334'
         Node name: 's1'
        Datacenter: 'dc1'
            Server: true (bootstrap: false)
       Client Addr: 0.0.0.0 (HTTP: 8500, HTTPS: -1, DNS: 8600, RPC: 8400)
      Cluster Addr: 10.201.102.198 (LAN: 8301, WAN: 8302)
    Gossip encrypt: false, RPC-TLS: false, TLS-Incoming: false
             Atlas: <disabled>
==> Log data will now stream in as it occurs:
    2017/03/17 18:03:08 [INFO] raft: Restored from snapshot 139-352267-1489707086023
    2017/03/17 18:03:08 [INFO] raft: Initial configuration (index=6982): [{Suffrage:Voter ID:10.201.102.199:8300 Address:10.201.102.199:8300} {Suffrage:Voter ID:10.201.102.200:8300 Address:10.201.102.200:8300} {Suffrage:Voter ID:10.201.102.198:8300 Address:10.201.102.198:8300}]
    2017/03/17 18:03:08 [INFO] raft: Node at 10.201.102.198:8300 [Follower] entering Follower state (Leader: "")
    2017/03/17 18:03:08 [INFO] serf: EventMemberJoin: s1 10.201.102.198
    2017/03/17 18:03:08 [INFO] serf: Attempting re-join to previously known node: s2: 10.201.102.199:8301
    2017/03/17 18:03:08 [INFO] consul: Adding LAN server s1 (Addr: tcp/10.201.102.198:8300) (DC: dc1)
    2017/03/17 18:03:08 [INFO] consul: Raft data found, disabling bootstrap mode
    2017/03/17 18:03:08 [INFO] serf: EventMemberJoin: s2 10.201.102.199
    2017/03/17 18:03:08 [INFO] serf: EventMemberJoin: s3 10.201.102.200
    2017/03/17 18:03:08 [INFO] serf: Re-joined to previously known node: s2: 10.201.102.199:8301
    2017/03/17 18:03:08 [INFO] consul: Adding LAN server s2 (Addr: tcp/10.201.102.199:8300) (DC: dc1)
    2017/03/17 18:03:08 [INFO] consul: Adding LAN server s3 (Addr: tcp/10.201.102.200:8300) (DC: dc1)
    2017/03/17 18:03:08 [INFO] serf: EventMemberJoin: s1.dc1 10.201.102.198
    2017/03/17 18:03:08 [INFO] consul: Adding WAN server s1.dc1 (Addr: tcp/10.201.102.198:8300) (DC: dc1)
    2017/03/17 18:03:08 [WARN] serf: Failed to re-join any previously known node
    2017/03/17 18:03:14 [INFO] agent: Synced service 'consul'
    2017/03/17 18:03:14 [INFO] agent: Deregistered service 'consul01'
    2017/03/17 18:03:14 [INFO] agent: Deregistered service 'consul02'
    2017/03/17 18:03:14 [INFO] agent: Deregistered service 'consul03'
~~~

新开一个终端窗口运行`consul members`，你可以看到Consul集群的成员：

~~~shell
[root@dhcp-10-201-102-198 ~]# consul members
Node  Address              Status  Type    Build  Protocol  DC
s1    10.201.102.198:8301  alive   server  0.7.4  2         dc1
s2    10.201.102.199:8301  alive   server  0.7.4  2         dc1
s3    10.201.102.200:8301  alive   server  0.7.4  2         dc1
~~~

## 启动Consul Client

启动Consul Client的典型命令如下：

~~~shell
consul agent -data-dir /tmp/consul -node=c1 -bind=10.201.102.248 -config-dir=/etc/consul.d/ -join 10.201.102.198
~~~

运行cosnul agent以client模式，`-join` 加入到已有的集群中去。

~~~shell
[root@dhcp-10-201-102-248 ~]# consul agent -data-dir /tmp/consul -node=c1 -bind=10.201.102.248 -config-dir=/etc/consul.d/ -join 10.201.102.198
==> Starting Consul agent...
==> Starting Consul agent RPC...
==> Joining cluster...
    Join completed. Synced with 1 initial agents
==> Consul agent running!
           Version: 'v0.7.4'
           Node ID: '564dc0c7-7f4f-7402-a301-cebe7f024294'
         Node name: 'c1'
        Datacenter: 'dc1'
            Server: false (bootstrap: false)
       Client Addr: 127.0.0.1 (HTTP: 8500, HTTPS: -1, DNS: 8600, RPC: 8400)
      Cluster Addr: 10.201.102.248 (LAN: 8301, WAN: 8302)
    Gossip encrypt: false, RPC-TLS: false, TLS-Incoming: false
             Atlas: <disabled>
==> Log data will now stream in as it occurs:
    2017/03/17 15:35:16 [INFO] serf: EventMemberJoin: c1 10.201.102.248
    2017/03/17 15:35:16 [INFO] agent: (LAN) joining: [10.201.102.198]
    2017/03/17 15:35:16 [INFO] serf: EventMemberJoin: s2 10.201.102.199
    2017/03/17 15:35:16 [INFO] serf: EventMemberJoin: s3 10.201.102.200
    2017/03/17 15:35:16 [INFO] serf: EventMemberJoin: s1 10.201.102.198
    2017/03/17 15:35:16 [INFO] agent: (LAN) joined: 1 Err: <nil>
    2017/03/17 15:35:16 [INFO] consul: adding server s2 (Addr: tcp/10.201.102.199:8300) (DC: dc1)
    2017/03/17 15:35:16 [INFO] consul: adding server s3 (Addr: tcp/10.201.102.200:8300) (DC: dc1)
    2017/03/17 15:35:16 [INFO] consul: adding server s1 (Addr: tcp/10.201.102.198:8300) (DC: dc1)
    2017/03/17 15:35:16 [INFO] agent: Synced node info
~~~

# 跨主机Docker间通信

## 准备环境

准备物理机或者虚拟机dev-11，IP地址为162.105.75.113，在主机上运行docker容器host1。

准备物理机或者虚拟机dev-12，IP地址为162.105.75.220，在主机上运行docker容器host2。

## 安装配置consul

直接从官网下载，并解压，将二进制文件拷贝到 /usr/local/bin 目录下即安装完成，同时创建新文件夹 /opt/consul 用于存放consul运行时产生的文件。

在dev-11机器上执行以下命令，将dev-11作为server节点：

~~~shell
consul agent -server -bootstrap -data-dir /opt/consul -bind=162.105.75.113
~~~

在dev-12机器上执行以下命令，将dev-11作为client节点，并加入集群：

~~~shell
consul agent -data-dir /opt/consul -bind=162.105.75.220 -join 162.105.75.113
~~~

分别在dev-11和dev-12上执行consul members，查看集群中是否有两个主机：

~~~shell
[root@dev-12 skc]# consul members
Node    Address              Status  Type    Build  Protocol  DC
dev-11  162.105.75.113:8301  alive   server  0.7.5  2         dc1
dev-12  162.105.75.220:8301  alive   client  0.7.5  2         dc1
~~~

如果在搭建过程中出现500错误，无法建成集群，可查看防火墙是否已经关闭。

## 配置Docker启动参数

需配置docker的启动参数：

~~~shell
#修改配置文件 /lib/systemd/system/docker.service
[root@jamza_vm_master_litepaas registry_test]# cat /lib/systemd/system/docker.service
[Unit]
Description=Docker Application Container Engine
Documentation=https://docs.docker.com
After=network.target

[Service]
Type=notify
# the default is not to use systemd for cgroups because the delegate issues still
# exists and systemd currently does not support the cgroup feature set required
# for containers run by docker

# cluster-store 的主机指定为localhost即可
# cluster-advertise的ip可以指定为本机的网卡名
ExecStart=/usr/bin/dockerd -H tcp://0.0.0.0:2376 -H unix:///var/run/docker.sock --insecure-registry=172.18.0.3:5000 --cluster-store=consul://127.0.0.1:8500 --cluster-advertise=eth0:2376
ExecReload=/bin/kill -s HUP $MAINPID
# Having non-zero Limit*s causes performance problems due to accounting overhead
# in the kernel. We recommend using cgroups to do container-local accounting.
LimitNOFILE=infinity
LimitNPROC=infinity
LimitCORE=infinity
# Uncomment TasksMax if your systemd version supports it.
# Only systemd 226 and above support this version.
#TasksMax=infinity
TimeoutStartSec=0
# set delegate yes so that systemd does not reset the cgroups of docker containers
Delegate=yes
# kill only the docker process, not all processes in the cgroup
KillMode=process

[Install]
WantedBy=multi-user.target
[root@jamza_vm_master_litepaas registry_test]#

#重启Docker服务
[root@jamza_vm_master_litepaas registry_test]# systemctl daemon-reload
[root@jamza_vm_master_litepaas registry_test]# service docker restart
~~~

## 创建overlay网络

在dev-11上执行`docker network create -d overlay multihost` 创建overlay类型网络multihost，然后查看创建结果：

~~~shell
[root@dev-11 ~]# docker network create -d overlay multihost
[root@dev-11 ~]# docker network ls
NETWORK ID          NAME                DRIVER              SCOPE
914e62484c33        bridge              bridge              local
018d41df39c5        docker_gwbridge     bridge              local
0edff5347b33        host                host                local
e7b16dd58248        multihost           overlay             global
1d25e019c111        none                null                local
~~~

此时，在dev-12机器上：

~~~shell
[root@dev-12 skc]# docker network ls
NETWORK ID          NAME                DRIVER              SCOPE
7af47cbb82c8        bridge              bridge              local
30911dfed7f2        docker_gwbridge     bridge              local
6e6deb4077c4        host                host                local
e7b16dd58248        multihost           overlay             global
dc7f861e601a        none                null                local
~~~

说明overlay网络被同步过去，在dev-12上可以看到在dev-11上创建的multihost网络。

## 创建容器并测试

在dev-11上创建容器：

~~~shell
[root@dev-11 skc]# docker run -it --name=host1 --net=multihost debugman007/ubt14-ssh:v1 bash
~~~

在dev-12上创建容器：

~~~shell
[root@dev-12 skc]# docker run -it --name=host2 --net=multihost debugman007/ubt14-ssh:v1 bash
~~~

在host1中：

~~~shell
root@d19636118ead:/# ifconfig
eth0      Link encap:Ethernet  HWaddr 02:42:0a:00:00:02  
          inet addr:10.0.0.2  Bcast:0.0.0.0  Mask:255.255.255.0
          inet6 addr: fe80::42:aff:fe00:2/64 Scope:Link
          UP BROADCAST RUNNING MULTICAST  MTU:1450  Metric:1
          RX packets:24 errors:0 dropped:0 overruns:0 frame:0
          TX packets:25 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:0 
          RX bytes:1904 (1.9 KB)  TX bytes:2122 (2.1 KB)
 
eth1      Link encap:Ethernet  HWaddr 02:42:ac:11:00:02  
          inet addr:172.17.0.2  Bcast:0.0.0.0  Mask:255.255.0.0
          inet6 addr: fe80::42:acff:fe11:2/64 Scope:Link
          UP BROADCAST RUNNING MULTICAST  MTU:1500  Metric:1
          RX packets:12 errors:0 dropped:0 overruns:0 frame:0
          TX packets:12 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:0 
          RX bytes:1018 (1.0 KB)  TX bytes:868 (868.0 B)
 
lo        Link encap:Local Loopback  
          inet addr:127.0.0.1  Mask:255.0.0.0
          inet6 addr: ::1/128 Scope:Host
          UP LOOPBACK RUNNING  MTU:65536  Metric:1
          RX packets:12 errors:0 dropped:0 overruns:0 frame:0
          TX packets:12 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:0 
          RX bytes:970 (970.0 B)  TX bytes:970 (970.0 B)
~~~

在host2中：

~~~shell
root@7bd8ff1ab133:/# ifconfig
eth0      Link encap:Ethernet  HWaddr 02:42:0a:00:00:03  
          inet addr:10.0.0.3  Bcast:0.0.0.0  Mask:255.255.255.0
          inet6 addr: fe80::42:aff:fe00:3/64 Scope:Link
          UP BROADCAST RUNNING MULTICAST  MTU:1450  Metric:1
          RX packets:25 errors:0 dropped:0 overruns:0 frame:0
          TX packets:23 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:0 
          RX bytes:1966 (1.9 KB)  TX bytes:1850 (1.8 KB)
 
eth1      Link encap:Ethernet  HWaddr 02:42:ac:11:00:02  
          inet addr:172.17.0.2  Bcast:0.0.0.0  Mask:255.255.0.0
          inet6 addr: fe80::42:acff:fe11:2/64 Scope:Link
          UP BROADCAST RUNNING MULTICAST  MTU:1500  Metric:1
          RX packets:22 errors:0 dropped:0 overruns:0 frame:0
          TX packets:8 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:0 
          RX bytes:2412 (2.4 KB)  TX bytes:648 (648.0 B)
 
lo        Link encap:Local Loopback  
          inet addr:127.0.0.1  Mask:255.0.0.0
          inet6 addr: ::1/128 Scope:Host
          UP LOOPBACK RUNNING  MTU:65536  Metric:1
          RX packets:12 errors:0 dropped:0 overruns:0 frame:0
          TX packets:12 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:0 
          RX bytes:934 (934.0 B)  TX bytes:934 (934.0 B)
~~~

此时，在host1中ping host2：

~~~shell
root@d19636118ead:/# ping host1
PING host1 (10.0.0.2) 56(84) bytes of data.
64 bytes from d19636118ead (10.0.0.2): icmp_seq=1 ttl=64 time=0.047 ms
64 bytes from d19636118ead (10.0.0.2): icmp_seq=2 ttl=64 time=0.057 ms
^C
--- host1 ping statistics ---
2 packets transmitted, 2 received, 0% packet loss, time 999ms
rtt min/avg/max/mdev = 0.047/0.052/0.057/0.005 ms
root@d19636118ead:/# ping host2
PING host2 (10.0.0.3) 56(84) bytes of data.
64 bytes from host2.multihost (10.0.0.3): icmp_seq=1 ttl=64 time=0.917 ms
64 bytes from host2.multihost (10.0.0.3): icmp_seq=2 ttl=64 time=0.975 ms
64 bytes from host2.multihost (10.0.0.3): icmp_seq=3 ttl=64 time=0.935 ms
^C
--- host2 ping statistics ---
3 packets transmitted, 3 received, 0% packet loss, time 2002ms
rtt min/avg/max/mdev = 0.917/0.942/0.975/0.034 ms
~~~

能够ping通，说明跨主机的容器搭建完成。