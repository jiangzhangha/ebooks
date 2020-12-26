---
title: CentOS安装配置Docker服务
date: 2020-07-07 21:43:45
categories: Linux知识
index_img: /img/articles/docker.jpg
tags:
	- CentOS
	- 安装配置
	- Docker
---

## 查看内核版本
通过uname -r查看当前内核版本，docker要求centos系统的内核版本高于3.10


```
uname -r
```

## 确保yum包更新到最新

```
sudo yum update
```

## 如果已安装过docker，则卸载旧版本

```
sudo yum remove docker  docker-common docker-selinux docker-engine
```

## 安装需要的软件包

```
sudo yum install -y yum-utils device-mapper-persistent-data lvm2
```

## 设置yum源

```
//官方源
sudo yum-config-manager --add-repo https://download.docker.com/linux/centos/docker-ce.repo

//阿里云源
sudo yum-config-manager --add-repo http://mirrors.aliyun.com/docker-ce/linux/centos/docker-ce.repo
```

## 查看所有仓库中所有docker版本，并选择特定版本安装

```
yum list docker-ce --showduplicates | sort -r
```

## 安装docker

```
sudo yum install docker-ce  
#由于repo中默认只开启stable仓库，故这里安装的是最新稳定版

sudo yum install <FQPN>      
#例如：sudo yum install docker-ce-17.12.0.ce
```

## 启动并加入开机启动

```
sudo systemctl start docker
sudo systemctl enable docker
```

## 验证安装是否成功

```
docker version
```

有client和service两部分表示docker安装启动都成功了。


```
[jamza@Jamza ~]$ docker version
Client:
 Version:           18.09.0
 API version:       1.39
 Go version:        go1.10.4
 Git commit:        4d60db4
 Built:             Wed Nov  7 00:48:22 2018
 OS/Arch:           linux/amd64
 Experimental:      false

Server: Docker Engine - Community
 Engine:
  Version:          18.09.0
  API version:      1.39 (minimum version 1.12)
  Go version:       go1.10.4
  Git commit:       4d60db4
  Built:            Wed Nov  7 00:19:08 2018
  OS/Arch:          linux/amd64
  Experimental:     false
```

## 权限问题
安装完docker后，执行docker命令出现：connect: permission denied

docker进程使用Unix Socket而不是TCP端口。而默认情况下，Unix socket属于root用户，需要root权限才能访问。

解决方法1：使用sudo执行docker命令。

解决办法2：将用户账号添加到docker组中。

```
sudo groupadd docker                 #添加docker用户组
sudo gpasswd -a $USER docker         #将登陆用户加入到docker用户组中
newgrp docker                         #更新用户组
docker ps                            #测试docker命令是否可以使用sudo正常使用
```

## 删除标签为none的docker镜像
使用脚本删除，脚本内容为：


```
#!/bin/bash
docker ps -a | grep "Exited" | awk '{print $1 }'|xargs docker stop
docker ps -a | grep "Exited" | awk '{print $1 }'|xargs docker rm
docker images|grep none|awk '{print $3 }'|xargs docker rmi
```

## 镜像加速器
国内从Docker Hub拉取镜像速度比较慢，Docker官方和国内很多云服务商提供了国内加速器服务。

比如Docker官方提供中国仓库镜像：https://registry.docker-cn.com 或者七牛云加速器：https://reg-mirror.qiniu.com/

对于Centos7系统，在 /etc/docker/daemon.json中写入以下内容：


```
{
"registry-mirrors": ["https://registry.docker-cn.com"]
}
```

然后重启Docker服务：


```
sudo systemctl daemon-reload
sudo systemctl restart docker
```

使用DaoCloud的镜像源加速器，网址 https://www.daocloud.io/mirror, 执行脚本：


```
curl -sSL https://get.daocloud.io/daotools/set_mirror.sh | sh -s http://f1361db2.m.daocloud.io
```

该脚本可以将 --registry-mirror 加入到你的 Docker 配置文件 /etc/docker/daemon.json 中。适用于 Ubuntu14.04、Debian、CentOS6 、CentOS7、Fedora、Arch Linux、openSUSE Leap 42.1