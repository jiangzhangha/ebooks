---
title: 数据结构与算法之栈（C语言版）
date: 2020-07-19 14:46:15
categories: 数据结构与算法
index_img: /img/articles/algo3.jpg
tags:
	- 数据结构
	- 算法
	- 栈
	- c
---

# 数据结构与算法之栈（C语言版）

栈是按照后进先出（LIFO）的顺序存储和检索数据的高效的数据结构，其检索元素的顺序和存储元素的顺序是相反的。

## 接口定义

### stack_init

~~~c
void stack_init(Stack *stack, void (*destroy)(void *data));
/*
初始化由参数stack指定的栈
参数destroy是函数指针，指定释放动态分配的内存空间
*/
~~~

### stack_destroy

~~~c
void stack_destroy(Stack *stack);
/*
销毁由参数stack指定的栈
*/
~~~

### stack_push

~~~c
int stack_push(Stack *stack, const void *data);
/*
向参数stack指定的栈中压入一个元素
参数data为新元素所包含的数据域
如果压栈成功，返回0，否则返回-1
*/
~~~

### stack_pop

~~~c
int stack_pop(Stack *stack, void **data);
/*
从参数stack指定的栈中弹出一个元素
参数data指向已经弹出元素中存储的数据
如果弹栈成功，返回0，否则返回-1
*/
~~~

### stack_peek

~~~c
void *stack_peek(const Stack *stack);
/*
获取参数stack指定的栈的顶部元素的数据域
*/
~~~

### stack_size

~~~c
int stack_size(const Stack *stack);
/*
获取参数stack指定的栈中元素的个数
*/
~~~

## 实现

实现栈的方式很多，其中之一就是使用链表来实现。

栈的抽象数据类型的头文件：

~~~c
/* stack.h */

#ifndef STACK_H
#define STACK_H

#include <stdlib.h>
#include "list.h"

/* implement stacks as linked lists */
typedef List Stack;

/* public interface */
#define stack_init list_init
#define stack_destroy list_destroy

int stack_push(Stack *stack, const void *data);
int stack_pop(Stack *stack, void **data);

#define stack_peek(stack)	((stack)->head == NULL ? NULL : (stack)->head->data)
#define stack_size list_size

#endif
~~~

栈的抽象数据类型的实现文件：

~~~c
/* stack.c */

#include <stdlib.h>

#include "list.h"
#include "stack.h"

/* stack_push */
int stack_push(Stack *stack, const void *data)
{
    /* 将元素插入链表的头部位置 */
    return list_ins_next(stack, NULL, data);
}

/* stack_pop */
int stack_pop(Stack *stack, void **data)
{
    /* 删除链表头部的元素 */
    return list_rem_next(stack, NULL, data);
}
~~~

