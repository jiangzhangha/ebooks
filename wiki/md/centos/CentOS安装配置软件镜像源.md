---
title: CentOS安装配置软件镜像源
date: 2020-07-07 20:38:06
categories: Linux知识
index_img: /img/articles/centos.jpg
tags:
	- CentOS
	- 安装配置
---

## 阿里云Linux安装软件镜像源
阿里云是最近新出的一个镜像源。得益与阿里云的高速发展，这么大的需求，肯定会推出自己的镜像源。 阿里云Linux安装镜像源地址：http://mirrors.aliyun.com/

备份原镜像文件，出错后可以恢复

```
mv /etc/yum.repos.d/CentOS-Base.repo /etc/yum.repos.d/CentOS-Base.repo.backup
```

下载新的CentOS-Base.repo 到/etc/yum.repos.d/

```
CentOS 5
wget -O /etc/yum.repos.d/CentOS-Base.repo http://mirrors.aliyun.com/repo/Centos-5.repo

CentOS 6
wget -O /etc/yum.repos.d/CentOS-Base.repo http://mirrors.aliyun.com/repo/Centos-6.repo

CentOS 7
wget -O /etc/yum.repos.d/CentOS-Base.repo http://mirrors.aliyun.com/repo/Centos-7.repo
```

运行yum makecache生成缓存

```
yum clean all
yum makecache
```