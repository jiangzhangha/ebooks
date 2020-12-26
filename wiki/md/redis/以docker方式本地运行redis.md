---
title: 以Docker方式本地运行redis
date: 2020-07-08 09:12:45
categories: redis
index_img: /img/articles/redis.jpg
tags:
	- redis
	- Docker
---

## 以Dokcer方式本地运行redis

体验学习redis，需要使用Linux环境或者Mac环境。

以Docker方式运行redis比较方便。具体操作如下：


```shell
[jamza@cloud-vm prjcfg]$ docker pull redis
Using default tag: latest
latest: Pulling from library/redis
8559a31e96f4: Pull complete
85a6a5c53ff0: Pull complete
b69876b7abed: Pull complete
a72d84b9df6a: Pull complete
5ce7b314b19c: Pull complete
04c4bfb0b023: Pull complete
Digest: sha256:800f2587bf3376cb01e6307afe599ddce9439deafbd4fb8562829da96085c9c5
Status: Downloaded newer image for redis:latest
docker.io/library/redis:latest
[jamza@cloud-vm prjcfg]$
[jamza@cloud-vm prjcfg]$
[jamza@cloud-vm prjcfg]$
[jamza@cloud-vm prjcfg]$ docker images
REPOSITORY          TAG                 IMAGE ID            CREATED             SIZE
redis               latest              235592615444        14 hours ago        104MB
[jamza@cloud-vm prjcfg]$
[jamza@cloud-vm prjcfg]$
[jamza@cloud-vm prjcfg]$ docker run --name myredis -d -p 6379:6379 redis:latest
44880060efdff5bb8d5d68b707ca23bc44f392e155308485ea62133a2456bcf0
[jamza@cloud-vm prjcfg]$
[jamza@cloud-vm prjcfg]$
[jamza@cloud-vm prjcfg]$ docker ps -a
CONTAINER ID        IMAGE               COMMAND                  CREATED             STATUS              PORTS                              NAMES
44880060efdf        redis:latest        "docker-entrypoint.s…"   6 seconds ago       Up 5 seconds        0.0.0.0:6379->6379          /tcp   myredis
[jamza@cloud-vm prjcfg]$
[jamza@cloud-vm prjcfg]$ docker exec -it myredis redis-cli
127.0.0.1:6379>
127.0.0.1:6379>
127.0.0.1:6379> help
redis-cli 6.0.5
To get help about Redis commands type:
      "help @<group>" to get a list of commands in <group>
      "help <command>" for help on <command>
      "help <tab>" to get a list of possible help topics
      "quit" to exit

To set redis-cli preferences:
      ":set hints" enable online hints
      ":set nohints" disable online hints
Set your preferences in ~/.redisclirc
127.0.0.1:6379>
```