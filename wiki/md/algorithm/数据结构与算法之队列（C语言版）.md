---
title: 数据结构与算法之队列（C语言版）
date: 2020-07-19 14:48:15
categories: 数据结构与算法
index_img: /img/articles/algo3.jpg
tags:
	- 数据结构
	- 算法
	- 队列
	- c

---

# 数据结构与算法之队列（C语言版）

队列的显著特征是先进先出（FIFO）的方式存储与检索元素。即首先存入队列的元素将首先被删除。

将一个元素放置到队尾，称为入队，将一个元素从队列头删除，称为出队。

## 接口定义

### queue_init

~~~c
void queue_init(Queue *queue, void (*destroy)(void *data));
/*
初始化由参数queue指定的队列
参数destroy是函数指针，用来释放动态分配的内存空间
*/
~~~

### queue_destroy

~~~c
void queue_destroy(Queue *queue);
/*
销毁由参数queue指定的队列
*/
~~~

### queue_enqueue

~~~c
int queue_enqueue(Queue *queue, const void *data);
/*
向参数queue指定的队列尾部插入一个元素
参数data指向新元素所包含的数据域
如果入队成功，则返回0，否则返回-1
*/
~~~

### queue_dequeue

~~~c
int queue_dequeue(Queue *queue, void **data);
/*
从参数queue指定的队列头部删除一个元素
参数data指向已经删除的元素的数据域
如果出队成功，则返回0，否则返回-1
*/
~~~

### queue_peek

~~~c
void *queue_peek(const Queue *queue);
/*
获取由参数queue指定的队列的头部元素中的数据域
*/
~~~

### queue_size

~~~c
int queue_size(const Queue *queue);
/*
获取由参数queue指定的队列中的元素个数
*/
~~~

## 实现

队列同样也可以以链表的底层方式来实现。

队列的抽象数据类型的头文件：

~~~c
/* queue.h */

#ifndef QUEUE_H
#define QUEUE_H

#include <stdlib.h>
#include "list.h"

/* implement queue as linked lists */
typedef List Queue;

/* public interface */
#define queue_init list_init
#define queue_destroy list_destroy

int queue_enqueue(Queue *queue, const void *data);
int queue_dequeue(Queue *queue, void **data);

#define queue_peek(queue)	((queue)->head == NULL ? NULL : (queue)->head->data)
#define queue_size list_size

#endif
~~~

队列的抽象数据类型的实现文件：

~~~c
/* queue.c */

#include <stdlib.h>
#include "list.h"
#include "queue.h"

/* queue_enqueue */
int queue_enqueue(Queue *queue, const void *data)
{
    /* 向链表的尾部添加元素 */
    return list_ins_next(queue, list_tail(queue), data);
}

/* queue_dequeue */
int queue_dequeue(Queue *queue, void **data)
{
    /* 删除链表头部的元素 */
    return list_rem_next(queue, NULL, data);
}
~~~



