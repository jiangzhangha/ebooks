---
title: 数据结构与算法之集合（C语言版）
date: 2020-07-19 15:17:15
categories: 数据结构与算法
index_img: /img/articles/algo3.jpg
math: true
tags:
	- 数据结构
	- 算法
	- 集合
	- c


---

# 数据结构与算法之集合（C语言版）

集合是不同的成员对象的无序聚集，由于成员元素之间彼此互相关联，因此可以将集合理解为归集在一起的成员组合。

集合的两个重要的特点为：

1. 成员是无序的
2. 每个成员都只在集合中出现一次

## 简介

集合是相关联的成员的无序组合，每个成员在集合中仅出现一次。在维基百科中，对集合的介绍为：

> 在计算机科学中，**集合**是一组可变量量的数据项（也可能是0个）的组合，这些数据项可能共享某些特征，需要以某种操作方式一起进行操作。一般来讲，这些数据项的类型是相同的，或基类相同（若使用的语言支持继承）。列表（或数组）通常不被认为是集合，因为其大小固定，但事实上它常常在实现中作为某些形式的集合使用。
>
> 集合的种类包括列表，集，多重集，树和图。枚举类型可以是列表或集。

如果集合S包含成员1、2与3，则可以写作：
$$
S={1,2,3}
$$
如果成员m在集合S中，则成员关系可以写作：
$$
m \in S
$$
如果成员m不在集合S中，则成员关系可以写作：
$$
m \notin S
$$

## 定义

没有包含任何成员的集合称为**空集**

集合的所有的可能的成员称为**全域**
$$
S=U表示全域
$$

$$
S= \emptyset 表示空集
$$

如何两个集合中所包含的成员完全一样，则称这两个集合相等。
$$
S_1=S_2 表示集合相等
$$

$$
S_1 \not= S_2 表示集合不相等
$$

如果集合S2包含另一个集合S1的所有成员，则S1是S2的子集
$$
S_1 \subset S_2 表示S_1是S_2的子集
$$

$$
S_1 \not\subset S_2 表示S_1不是S_2的子集
$$

## 基本操作

两个集合S1与S2的并集也是一个集合，其包含了S1与S2中的所有成员。写法为：
$$
S_u=S_1 \cup S_2
$$
两个集合S1与S2的交集也是一个集合，其只包含同时存在于S1和S2中的成员，写法为：
$$
S_i=S_1 \cap S_2
$$
两个集合S1与S2的差集也是一个集合，其只包含在S1中出现过，且不属于S2的成员。写法为：
$$
S_d=S_1-S_2
$$

## 性质

某个集合与空集的交集一定是空集

某个集合与空集的并集还是原来的集合本身
$$
S \cap \emptyset=\emptyset
$$

$$
S \cup \emptyset=S
$$

与集合本身求交集，结果还是集合本身

与集合本身求并集，结果还是集合本身
$$
S \cap S = S
$$

$$
S \cup S = S
$$

集合S1与S2的交集，其结果等同于集合S2与集合S2的交集，同样道理适用于并集，即集合的交换律：
$$
S_1 \cap S_2 = S_2 \cap S_1
$$

$$
S_1 \cup S_2 = S_2 \cup S_1
$$

一组集合的交集，可以按照任意的顺序来求解，同样道理适用于并集，即集合的结合律：
$$
S_1 \cap (S_2 \cap S_3)=(S_1 \cap S_2) \cap S_3
$$

$$
S_1 \cup (S_2 \cup S_3)=(S_1 \cup S_2) \cup S_3
$$

某集合和其他两个集合的并集相交的结果，可以按照一种分配的方式来解决，同样的道理也适用于求解某个集合与其他两个集合的交集相合并的结果，即集合的分配律：
$$
S_1 \cap (S_2 \cup S_3)=(S_1 \cap S_2)\cup(S_1 \cap S_3)
$$

$$
S_1 \cup (S_2 \cap S_3)=(S_1 \cup S_2)\cap(S_1 \cup S_3)
$$

某集合与该集合和另一个集合的并集相交的结果等于该集合本身，同样的道理也适用于求解某集合与该集合和另一个集合的交集相结合的结果，即集合的合并律：
$$
S_1 \cap (S_1 \cup S_2) = S_1
$$

$$
S_1 \cup (S_1 \cap S_2) = S_1
$$

## 接口定义

### set_init

~~~c
void set_init(Set *set, int (*match)(const void *key1, const void *key2), void (*destroy)(void *data));
/*
初始化由参数set所指定的集合
参数match是函数指针，用来判断集合中两个成员是否相互匹配
参数destroy是函数指针，提供了释放动态分配空间的方法
*/
~~~

### set_destroy

~~~c
void set_destroy(Set *set);
/*
销毁由参数set所指定的集合
*/
~~~

### set_insert

~~~c
int set_insert(Set *set, const void *data);
/*
在由参数set所指定的集合中插入一个成员
参数data指向该成员的数据域
如果插入成功，则返回0，如果插入的成员在集合中已经存在，则返回1，否则返回-1
*/
~~~

### set_remove

~~~c
int set_remove(Set *set, void **data);
/*
在由参数set所指定的集合中移除数据域同data相吻合的成员
函数返回后，data指向移除的成员的数据域
如果移除成功则返回0，否则返回-1
*/
~~~

### set_union

~~~c
int set_union(Set *setu, const Set *set1, const Set *set2);
/*
建立一个集合，其结果为set1与set2所指定的集合的并集，setu返回这个并集
如果并集成功则返回0，否则返回-1
*/
~~~

### set_intersection

~~~c
int set_intersection(Set *seti, const Set *set1, const Set *set2);
/*
建立一个集合，其结果为set1与set2所指定的集合的交集，seti返回这个交集
如果交集成功则返回0，否则返回-1
*/
~~~

### set_difference

~~~c
int set_difference(Set *setd, const Set *set1, const Set *set2);
/*
建立一个集合，其结果为set1与set2所指定的集合的差集，setd返回这个差集
如果差集成功则返回0，否则返回-1
*/
~~~

### set_is_member

~~~c
int set_is_member(const Set *set, const void *data);
/*
判断由data所指定的成员是否存在于set所指定的集合中
*/
~~~

### set_is_subset

~~~c
int set_is_subset(const Set *set1, const Set *set2);
/*
判断由参数set1所指定的集合是否为参数set2所指定的集合的子集
*/
~~~

### set_is_equal

~~~c
int set_is_equal(const Set *set1, const Set *set2);
/*
判断由参数set1所指定的集合是否等于参数set2所指定的集合
*/
~~~

### set_size

~~~c
int set_size(const Set *set);
/*
返回由参数set所指定的集合中元素的个数
*/
~~~

## 实现

实现集合的一种方式是采用链表。

集合的抽象数据类型的头文件：

~~~c
/* set.h */

#ifndef SET_H
#define SET_H

#include <stdlib.h>
#include "list.h"

/* implement sets as linked lists */
typedef List Set;

/* public interface */
void set_init(Set *set, int (*match)(const void *key1, const void *key2), void (*destroy)(void *data));
int set_insert(Set *set, const void *data);
int set_remove(Set *set, void **data);
int set_union(Set *setu, const Set *set1, const Set *set2);
int set_intersection(Set *seti, const Set *set1, const Set *set2);
int set_difference(Set *setd, const Set *set1, const Set *set2);
int set_is_member(const Set *set, const void *data);
int set_is_subset(const Set *set1, const Set *set2);
int set_is_equal(const Set *set1, const Set *set2);

#define set_destroy list_destroy
#define set_size(set)	((set)->size)

#endif
~~~

集合的抽象数据类型的实现文件：

~~~c
/* set.c */

#include <stdlib.h>
#include <string.h>
#include "list.h"
#include "set.h"

/* set_init */
void set_init(Set *set, int (*match)(const void *key1, const void *key2), void (*destroy)(void *data))
{
    list_init(set, destroy);
    set->match = match;
    return;
}

/* set_insert */
int set_insert(Set *set, const void *data)
{
    if(set_is_member(set, data))
    {
        return 1;
    }
    return list_ins_next(set, list_tail(set), data);
}

/* set_remove */
int set_remove(Set *set, void **data)
{
    ListElmt *member;
    ListElmt *prev = NULL;
    
    for(member = list_head(set); member != NULL; member = list_next(member))
    {
        if(set->match(*data, list_data(member)))
        {
            break;
        }
        prev = member
    }
    if(member == NULL)
    {
        return -1;
    }
    return list_rem_next(set, prev, data);
}

/* set_union */
int set_union(Set *setu, const Set *set1, const Set *set2)
{
    ListElmt *member;
    void *data;
    
    set_init(setu, set1->match, NULL);
    for(member = list_head(set1); member != NULL; member = list_next(member))
    {
        data = list_data(member);
        if(list_ins_next(setu, list_tail(setu), data) != 0)
        {
            set_destroy(setu);
            return -1;
        }
    }
    
    for(member = list_head(set2); member != NULL; member = list_next(member))
    {
        if(set_is_member(set1, list_data(member)))
        {
            continue;
        }
        else
        {
            data = list_data(member);
            if(list_ins_next(setu, list_tail(setu), data) != 0)
            {
                set_destroy(setu);
                return -1;
            }
        }
    }
    return 0;
}

/* set_intersection */
int set_intersection(Set *seti, const Set *set1, const Set *set2)
{
    ListElmt *member;
    void *data;
    
    set_init(seti, set1->match, NULL);
    for(member = list_head(set1); member != NULL; member = list_next(member))
    {
        if(set_is_member(set2, list_data(member)))
        {
            data = list_data(member);
            if(list_ins_next(seti, list_tail(seti), data) != 0)
            {
                set_destroy(seti);
                return -1;
            }
        }
    }
    return 0;
}

/* set_difference */
int set_difference(Set *setd, const Set *set1, const Set *set2)
{
    ListElmt *member;
    void *data;
    
    set_init(setd, set1->match, NULL);
    for(member = list_head(set1); member != NULL; member = list_next(member))
    {
        if(!set_is_member(set2, list_data(member)))
        {
            data = list_data(member);
            if(list_ins_next(setd, list_tail(setd), data) != 0)
            {
                set_destroy(setd);
                return -1;
            }
        }
    }
    return 0;
}

/* set_is_member */
int set_is_member(const Set *set, const void *data)
{
    ListElmt *member;
    
    for(member = list_head(set); member != NULL; member = list_next(member))
    {
        if(set->match(data, list_data(member)))
        {
            return 1;
        }
    }
    return 0;
}

/* set_is_subset */
int set_is_subset(const Set *set1, const Set *set2)
{
    ListElmt *member;
    
    if(set_size(set1) > set_size(set2))
    {
        return 0;
    }
    
    for(member = list_head(set1); member != NULL; member = list_next(member))
    {
        if(!set_is_member(set2, list_data(member)))
        {
            return 0;
        }
    }
    return 1;
}

/* set_is_equal */
int set_is_equal(const Set *set1, const Set *set2)
{
    if(set_size(set1) != set_size(set2))
    {
        return 0;
    }
    /* sets of the same size are equal if they are subsets */
    return set_is_subset(set1, set2);
}
~~~







