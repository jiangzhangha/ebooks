---
title: redis数据库的C接口操作简介
date: 2020-07-08 09:21:45
categories: redis
index_img: /img/articles/redis.jpg
tags:
	- redis
	- c
	- 接口
---

## hiredis安装

`hiredis`是`redis`官方推荐的基于c接口的客户端组件，它提供接口，供c语言调用以操作数据库。我们需要将`hiredis`安装到我们的系统中，在`redis`的源码包的`deps/hiredis`下就有它的源码,也可以另行下载`hiredis`。

安装`hiredis`的方法，是进入 `deps/hiredis`目录下，执行编译，安装，动态库更新生效的操作：

~~~shell
[root@jamza-vm deps]# cd hiredis/
[root@jamza-vm hiredis]# pwd
/opt/redis-5.0.8/deps/hiredis
[root@jamza-vm hiredis]#
[root@jamza-vm hiredis]#
[root@jamza-vm hiredis]# make
cc -shared -Wl,-soname,libhiredis.so.0.13 -o libhiredis.so  net.o hiredis.o sds.o async.o read.o
cc -std=c99 -pedantic -c -O3 -fPIC  -Wall -W -Wstrict-prototypes -Wwrite-strings -g -ggdb  test.c
cc -O3 -fPIC  -Wall -W -Wstrict-prototypes -Wwrite-strings -g -ggdb  -o hiredis-test   test.o libhiredis.a
Generating hiredis.pc for pkgconfig...
[root@jamza-vm hiredis]#
[root@jamza-vm hiredis]#
[root@jamza-vm hiredis]# make install
mkdir -p /usr/local/include/hiredis /usr/local/lib
cp -a hiredis.h async.h read.h sds.h adapters /usr/local/include/hiredis
cp -a libhiredis.so /usr/local/lib/libhiredis.so.0.13
cd /usr/local/lib && ln -sf libhiredis.so.0.13 libhiredis.so
cp -a libhiredis.a /usr/local/lib
mkdir -p /usr/local/lib/pkgconfig
cp -a hiredis.pc /usr/local/lib/pkgconfig
[root@jamza-vm hiredis]#
[root@jamza-vm hiredis]#
[root@jamza-vm hiredis]# ldconfig
[root@jamza-vm hiredis]#

~~~



## 函数接口介绍

函数原型：

~~~c
redisContext *redisConnect(const char *ip, int port);
~~~

说明：该函数用来连接`redis`数据库，参数为数据库的`ip`地址和端口，通常默认端口为`6379`。该函数返回一个`redisContext`对象



函数原型：

~~~c
void *redisCommand(redisContext *c, const char *format, …);
~~~

说明：该函数执行`redis`命令，当然也包括由`lua`脚本组成的命令，返回`redisReply`对象



函数原型：

~~~c
void freeReplyObject(void *reply);
~~~

说明：释放`redisCommand`执行后返回的`redisReply`所占用的内存



函数原型：

~~~c
void redisFree(redisContext *c);
~~~

说明：释放`redisConnect`所产生的连接



## 重要结构体介绍

`redisReply`结构体定义的中英文注释：

~~~c
/* This is the reply object returned by redisCommand() */
 typedef struct redisReply {
     int type; 						/* 返回结果类型* */
     long long integer; 			/* 返回类型为整型的时候的返回值 */
     size_t len; 					/* 字符串长度 */
     char *str; 					/* 返回错误类型或者字符类型的字符串 */
     size_t elements; 				/* 返回数组类型时，元素的数量*/
     struct redisReply **element; 	/* 元素结果集合，redisReply对象 */
 } redisReply;


/* This is the reply object returned by redisCommand() */
typedef struct redisReply {
    int type; 						/* REDIS_REPLY_* */
    long long integer; 				/* The integer when type is REDIS_REPLY_INTEGER */
    size_t len; 					/* Length of string */
    char *str; 						/* Used for both REDIS_REPLY_ERROR and 													   REDIS_REPLY_STRING */
    size_t elements; 				/* number of elements, for REDIS_REPLY_ARRAY */
    struct redisReply **element; 	/* elements vector for REDIS_REPLY_ARRAY */
} redisReply;

~~~



返回值类型的定义为：

~~~c
#define REDIS_REPLY_STRING 		1	/* 字符串 */
#define REDIS_REPLY_ARRAY 		2   /* 数组，多个reply，通过element数组，elements数组大小访问 */
#define REDIS_REPLY_INTEGER 	3   /* 整型 */
#define REDIS_REPLY_NIL 		4   /* 空，无数据 */
#define REDIS_REPLY_STATUS 		5   /* 状态 */
#define REDIS_REPLY_ERROR 		6   /* 错误 */
~~~



`redisContext`结构体定义如下：

~~~c
enum redisConnectionType {
    REDIS_CONN_TCP,
    REDIS_CONN_UNIX
};

/* Context for a connection to Redis */
typedef struct redisContext {
    int err; /* Error flags, 0 when there is no error */
    char errstr[128]; /* String representation of error when applicable */
    int fd;
    int flags;
    char *obuf; /* Write buffer */
    redisReader *reader; /* Protocol reader */

    enum redisConnectionType connection_type;
    struct timeval *timeout;

    struct {
        char *host;
        char *source_addr;
        int port;
    } tcp;

    struct {
        char *path;
    } unix_sock;

} redisContext;
~~~



## 范例

实例通过redis数据库的hash表存储以下学生信息：

| 字段名称 | 含义     |
| -------- | -------- |
| sid      | 学号     |
| name     | 学生姓名 |
| gender   | 学生性别 |
| major    | 专业     |

对此信息的c语言结构体描述如下：

~~~c
#define SID_MAX_LENGHT 16
#define NAME_MAX_LENGHT 16
#define MAJOR_MAX_LENGHT 64
typedef struct Stu_Info_Struct
{
	char sid[SID_MAX_LENGHT];
	char name[NAME_MAX_LENGHT];
	int gender;			//0 male,1 female
	char major[MAJOR_MAX_LENGHT];
}Stu_Info_Struct;
~~~

范例代码如下：

~~~c

/***************************************************************
*   Copyright (C) 2017 All rights reserved.
*
*   文件名称：stu_manager.c
*   创 建 者：hyb
*   创建日期：2017年10月07日
*   描    述：
*
***************************************************************/
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<hiredis/hiredis.h>

/*宏定义*/
#define SID_MAX_LENGHT 16
#define NAME_MAX_LENGHT 16
#define MAJOR_MAX_LENGHT 64
#define CMD_MAX_LENGHT  256
#define REDIS_SERVER_IP "127.0.0.1"
#define REDIS_SERVER_PORT 6379
/*结构体定义*/
typedef struct Stu_Info_Struct
{
	char sid[SID_MAX_LENGHT];
	char name[NAME_MAX_LENGHT];
	int gender;//0 male,1 female
	char major[MAJOR_MAX_LENGHT];
}Stu_Info_Struct;

typedef enum STU_RESULT
{
    SUCCESS=0,
    FAILURE=1
}STU_RESULT;

/*函数*/
STU_RESULT addStu(Stu_Info_Struct *stu);/*添加信息*/
/*执行命令*/
STU_RESULT exeRedisIntCmd(char *cmd);
STU_RESULT quryStuBySid(char *sid);
STU_RESULT exeRedisStrCmd(char *cmd);
/**************************************
函数名:addStu
函数功能:添加学生信息
输入参数:stu 学生信息结构指针
输出参数:
返回值:STU_RESULT  成功或失败
************************************/
STU_RESULT addStu(Stu_Info_Struct *stu)
{
    char cmd[CMD_MAX_LENGHT] = {0};
    /*检查入参*/
    if(NULL == stu)
    {
        printf("NULL pointer");
        return FAILURE;
    }
    /*组装redis命令*/
    snprintf(cmd,CMD_MAX_LENGHT,"hset stu:%s name %s gender %d major %s",
            stu->sid,stu->name,stu->gender,stu->major);

    /*执行redis命令*/
    if(FAILURE == exeRedisIntCmd(cmd))
    {
        printf("add student %s,%s,%d,%s failure\n",stu->sid,stu->name,stu->gender,stu->major);
        return FAILURE;
    }
    printf("add student %s,%s,%d,%s success\n",stu->sid,stu->name,stu->gender,stu->major);
    return SUCCESS;
}
/**************************************
函数名:exeRedisIntCmd
函数功能:执行redis 返回值为int类型命令
输入参数:cmd  redis命令
输出参数:redis 返回结构
返回值:STU_RESULT
*************************************/
STU_RESULT exeRedisIntCmd(char *cmd)
{
    /*检查入参*/
    if(NULL == cmd)
    {
        printf("NULL pointer");
        return FAILURE;
    }
    /*连接redis*/
    redisContext *context = redisConnect(REDIS_SERVER_IP,REDIS_SERVER_PORT);
    if(context->err)
    {
        redisFree(context);
        printf("%d connect redis server failure:%s\n",__LINE__, context->errstr);
        return FAILURE;
    }
    printf("connect redis server success\n");

    /*执行redis命令*/
    redisReply *reply = (redisReply *)redisCommand(context, cmd);
    if(NULL == reply)
    {
        printf("%d execute command:%s failure\n",__LINE__,cmd);
        redisFree(context);
        return FAILURE;
     }
    //返回执行结果为整型的命令,只有状态为REDIS_REPLY_INTEGER,并且INTEGER是大于0时,才表示这种类型的命令执行成功
    if(!(reply->type == REDIS_REPLY_INTEGER && reply->integer > 0))
    {
        printf("%d execute command:%s failure\n",__LINE__, cmd);
        freeReplyObject(reply);
        redisFree(context);
        return FAILURE;
    }

    freeReplyObject(reply);
    redisFree(context);
    printf("%d execute command:%s success\n",__LINE__,cmd);
    return SUCCESS;

}
/**************************************
函数名:quryStuBySid
函数功能:通过sid查询学生信息
输入参数:cmd  redis命令
输出参数:redis 返回结构
返回值:STU_RESULT
*************************************/
STU_RESULT queryStuBySid(char *sid)
{
    char cmd[CMD_MAX_LENGHT] = {0};
    /*入参检查*/
    if(NULL == sid)
    {
        printf("%d NULL pointer\n",__LINE__);
        return FAILURE;
    }
    /*组装执行命令*/
    snprintf(cmd,CMD_MAX_LENGHT,"HGETALL stu:%s",sid);
    if(FAILURE == exeRedisStrCmd(cmd))
    {
        printf("%d query stu failue",__LINE__);
        return FAILURE;
    }
    return SUCCESS;
}
STU_RESULT exeRedisStrCmd(char *cmd)
{

    /*检查入参*/
    if(NULL == cmd)
    {
        printf("NULL pointer");
        return FAILURE;
    }

    /*连接redis*/
    redisContext *context = redisConnect(REDIS_SERVER_IP,REDIS_SERVER_PORT);
    if(context->err)
    {
        redisFree(context);
        printf("%d connect redis server failure:%s\n",__LINE__, context->errstr);
        return FAILURE;
    }
    printf("connect redis server success\n");

    /*执行redis命令*/
    redisReply *reply = (redisReply *)redisCommand(context, cmd);
    if(NULL == reply)
    {
        printf("%d execute command:%s failure\n",__LINE__,cmd);
        redisFree(context);
        return FAILURE;
     }
    //返回执行结果为整型的命令,只有状态为REDIS_REPLY_INTEGER,并且INTEGER是大于0时,才表示这种类型的命令执行成功
    if(!(reply->type == REDIS_REPLY_ARRAY && reply->elements > 0))
    {
        printf("%d execute command:%s failure\n",__LINE__, cmd);
        freeReplyObject(reply);
        redisFree(context);
        return FAILURE;

    }

    printf("%d,%lu\n",reply->type,reply->elements);
    int i = 0;
    for(i=0;i < reply->elements;i++)
    {
        if(i%2 ==0)
        {
            printf("%s:",reply->element[i]->str);
        }
        else
        {

            printf("%s\n",reply->element[i]->str);
        }
    }
    freeReplyObject(reply);
    redisFree(context);
    return SUCCESS;
}
int main(int argc,char *argv[])
{
    Stu_Info_Struct stu =
    {
     "01",
     "hu",
    1,
    "CS"
    };

    addStu(&stu);
    queryStuBySid("01");
    return 0;
}
~~~



### 编译

编译代码，如下：

~~~
gcc -g stu_manager.c -o stu -lhiredis
~~~

执行程序时，会遇到以下的问题：

~~~shell
[jamza@jamza-vm redis]$ ./stu
./stu: error while loading shared libraries: libhiredis.so.0.13: cannot open shared object file: No such file or directory
[jamza@jamza-vm redis]$
~~~

系统无法找到库文件，在`redis`数据库安装目录下查找库文件：

~~~shell
[root@jamza-vm redis-5.0.8]# pwd
/opt/redis-5.0.8
[root@jamza-vm redis-5.0.8]#
[root@jamza-vm redis-5.0.8]#
[root@jamza-vm redis-5.0.8]# find -name libhiredis*
./deps/hiredis/libhiredis.so
./deps/hiredis/libhiredis.a
~~~

将库文件拷贝到`/usr/lib/hiredis`目录下，并设置`/etc/ld.so.conf`文件：

~~~shell
[root@jamza-vm redis-5.0.8]# mkdir /usr/lib/hiredis/
[root@jamza-vm redis-5.0.8]#
[root@jamza-vm redis-5.0.8]#
[root@jamza-vm redis-5.0.8]# cp ./deps/hiredis/libhiredis.so /usr/lib/hiredis/
[root@jamza-vm redis-5.0.8]#
[root@jamza-vm redis-5.0.8]# ll /usr/lib/hiredis/
total 268
-rwxr-xr-x 1 root root 270992 Jun 23 17:04 libhiredis.so
[root@jamza-vm redis-5.0.8]#
[root@jamza-vm redis-5.0.8]# mkdir /usr/include/hiredis
[root@jamza-vm redis-5.0.8]#
[root@jamza-vm redis-5.0.8]# cp deps/hiredis/hiredis.h /usr/include/hiredis/
[root@jamza-vm redis-5.0.8]# echo '/usr/local/lib' >> /etc/ld.so.conf
[root@jamza-vm redis-5.0.8]# ldconfig
[root@jamza-vm redis-5.0.8]#
~~~