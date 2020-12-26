---
title: 数据结构与算法之链表（C语言版）
date: 2020-07-18 19:46:45
categories: 数据结构与算法
index_img: /img/articles/algo3.jpg
tags:
	- 数据结构
	- 算法
	- 链表
	- c
---

# 数据结构与算法之链表（C语言版）

链表由一组元素，以一种特定的顺序组合或者链接在一起

相对数组，链表在执行插入、删除操作时具有更高的效率

链表需要动态的开辟存储空间，也就是存储空间在程序运行时是动态分配的

链表类型：单向链表、双向链表、循环链表

## 单向链表

### list_init

~~~c
void list_init(List *list, void (*destroy)(void *data));
/*
初始化由参数list指定的链表
参数destroy为函数指针，提供了一种释放动态分配数据的方法
*/
~~~

### list_destroy

~~~c
void list_destroy(List *list);
/*
销毁由参数list指定的列表
*/
~~~

### list_ins_next

~~~c
int list_ins_next(List *list, ListElmt *element, const void *data);
/*
在参数list指定的链表中element元素后面插入一个新的元素
如果element为NULL，则新元素插入链表的头部
新元素包含一个指向data的指针
插入元素成功，则返回0，否则返回-1
*/
~~~

### list_rem_next

~~~c
int list_rem_next(List *list, ListElmt *element, void **data);
/*
移除由参数list指定的链表中的element后面的元素
如果参数element为NULL，则移除链表的表头元素
调用返回后，data执行已经移除的元素中存储的数据
如果移除元素成功，则返回0，否则返回-1
*/
~~~

### list_size

~~~c
int list_size(const List *list);
/*
返回参数list指定的链表中的元素的个数
*/
~~~

### list_head

~~~c
ListElmt *list_head(const List *list);
/*
返回由参数list指定的链表中的头元素的指针
*/
~~~

### list_tail

~~~c
ListElmt *list_tail(const List *list);
/*
返回由参数list指定的链表中尾元素的指针
*/
~~~

### list_is_head

~~~c
int list_is_head(const ListElmt *element);
/*
判断由参数element指定的元素是否是链表的头元素
若是，返回1，否则返回-1
*/
~~~

### list_is_tail

~~~c
int list_is_tail(const ListElmt *element);
/*
判断由参数element指定的元素是否是链表的尾元素
若是，返回1，否则返回-1
*/
~~~

### list_data

~~~c
void *list_data(const ListElmt *element);
/*
返回由参数element所指定的链表元素中保存的数据
*/
~~~

### list_next

~~~c
ListElmt *list_next(const ListElmt *element);
/*
返回链表中由参数element所指定的结点的下一个结点元素
*/
~~~

### 实现

头文件的实现：

~~~c
/* list.h */

#ifndef LIST_H
#define LIST_H

#include <stdlib.h>

/* define a structure for linked list elements */
typedef struct ListElmt_
{
    void *data;
    struct ListElmt_ *next;
}ListElmt;

/* define a structure for linked lists */
typedef struct List_
{
    int size;
    int (*match)(const void *key1, const void *key2);
    void (*destroy)(void *data);
    ListElmt *head;
    ListElmt *tail;
}List;

/* public interface */
void list_init(List *list, void (*destroy)(void *data));
void list_destroy(List *list);
int list_ins_next(List *list, ListElmt *element, const void *data);
int list_rem_next(List *list, ListElmt *element, void **data);

#define list_size(list)		((list)->size)
#define list_head(list)		((list)->head)
#define list_tail(list)		((list)->tail)
#define list_is_head(list, element)		((element) == (list)->head ? 1 : 0)
#define list_is_tail(element)		    ((element)->next == NULL ? 1 : 0)
#define list_data(element)			    ((element)->data)
#define list_next(element)			    ((element)->next)

#endif
~~~

函数实现：

~~~c
/* list.c */
#include <stdlib.h>
#include <string.h>
#include "list.h"

/* list_init*/
void list_init(List *list, void (*destroy)(void *data))
{
    list->size = 0;
    list->destroy = destroy;
    list->head = NULL;
    list->tail = NULL;
    return;
}

/* list_destroy */
void list_destroy(List *list)
{
    void *data;
    
    while(list_size(list) > 0)
    {
        if((list_rem_next(list, NULL, (void **)&data) == 0) && (list->destroy != NULL))
        {
            list->destroy(data);
        }
    }
    memset(list, 0, sizeof(List));
    return;
}

/* list_ins_next */
int list_ins_next(List *list, ListElmt *element, const void *data)
{
    ListElmt *new_element;
    if((new_element = (ListElmt *)malloc(sizeof(ListElmt))) == NULL)
    {
        return -1;
    }
    
    new_element->data = (void *)data;
    if(element = NULL)
    {
        /* handle insertion at the head of the list */
        if(list_size(list) == 0)
        {
            list->tail = new_element;
        }
        new_element->next = list->head;
        list->head = new_element;
    }
    else
    {
        /* handle insertion somewhere other than at the head */
        if(element->next == NULL)
        {
            list->tail = new_element;
        }
        new_element->next = element->next;
        element->next = new_element;
    }
    
    list->size++;
    return 0;
}

/* list_rem_next */
int list_rem_next(List *list, ListElmt *element, void **data)
{
    ListElmt *old_element;
    if(list_size(list) == 0)
    {
        return -1;
    }
    
    if(element == NULL)
    {
        /* handle removal from the head of the list */
        *data = list->head->data;
        old_element = list->head;
        list->head = list->head->next;
        
        if(list_size(list) == 1)
        {
            list->tail = NULL;
        }
    }
    else
    {
        /* handle removal from somewhere other than the head */
        if(element->next = NULL)
        {
            return -1;
        }
        *data = element->next->data;
        old_element = element->next;
        element->next = element->next->next;
        
        if(element->next = NULL)
        {
            list->tail = element;
        }
    }
    
    free(old_element);
    list->size--;
    return 0;
}

~~~

## 双向链表

### dlist_init

~~~c
void dlist_init(DList *list, void (*destroy)(void *data));
/*
初始化由参数list所指定的双向链表
参数destroy为函数指针，提供了一种释放动态分配空间的方法
*/
~~~

### dlist_destroy

~~~c
void dlist_destroy(DList *list);
/*
销毁由参数list所指定的双向链表
函数将移除双向链表中所有的元素
*/
~~~

### dlist_ins_next

~~~c
int dlist_ins_next(DList *list, DListElmt *element, const void *data);
/*
将元素插入到由参数list指定的双向链表中element元素之后
参数data为新元素中的数据
若插入成功，返回0，否则返回-1
*/
~~~

### dlist_ins_prev

~~~c
int dlist_ins_prev(DList *list, DListElmt *element, const void *data);
/*
将元素插入由参数list指定的双向链表中element元素之前
参数data为新元素中的数据
若插入成功，返回0，否则返回-1
*/
~~~

### dlist_remove

~~~c
int dlist_remove(DList *list, DListElmt *element, void **data);
/*
从由参数list指定的双向链表中移除由element所指定的元素
函数返回后，参数data指向已移除元素中存储的数据
若移除成功，返回0，否则返回-1
*/
~~~

### dlist_size

~~~c
int dlist_size(const DList *list);
/*
计算由参数list所指定的双向链表中的元素个数
*/
~~~

### dlist_head

~~~c
DListElmt *dlist_head(const DList *list);
/*
返回由参数list所指定的双向链表中的头元素
*/
~~~

### dlist_tail

~~~c
DListElmt *dlist_tail(const DList *list);
/*
返回由参数list所指定的双向链表中的尾元素
*/
~~~

### dlist_is_head

~~~c
int dlist_is_head(const DListElmt *element);
/*
判断由参数element所指定的元素是否为双向链表的头元素
若是，返回0，否则返回-1
*/
~~~

### dlist_is_tail

~~~c
int dlist_is_tail(const DListElmt *element);
/*
判断由参数element所指定的元素是否为双向链表的尾元素
若是，返回0，否则返回-1
*/
~~~

### dlist_data

~~~c
void *dlist_data(const DListElmt *element);
/*
返回由参数element所指定的双向链表元素中的数据域
*/
~~~

### dlist_next

~~~c
DListElmt *dlist_next(const DListElmt *element);
/*
返回由参数element所指定的双向链表中元素的后一个元素
*/    
~~~

### dlist_prev

~~~c
DListElmt *dlist_prev(const DListElmt *element);
/*
返回由参数element所指定的双向链表中元素的前一个元素
*/    
~~~

### 实现

双向链表的抽象数据类型的头文件：

~~~c
/* dlist.h */

#ifndef DLIST_H
#define DLIST_H

#include <stdlib.h>

/* define a structure for doubly-linked list elements */
typedef struct DListElmt_
{
    void *data;
    struct DListElmt *prev;
    struct DListElmt *prev;
}DListElmt;

/* define a structure for doubly-linked lists */
typedef struct DList_
{
    int size;
    int (*match)(const void *key1, const void *key2);
    void (*destroy)(void *data);
    DListElmt *head;
    DListElmt *tail;
}DList;

/* public interface */
void dlist_init(DList *list, void (*destroy)(void *data));
void dlist_destroy(DList *list);
int dlist_ins_next(DList *list, DListElmt *element, const void *data);
int dlist_ins_prev(DList *list, DListElmt *element, const void *data);
int dlist_remove(DList *list, DListElmt *element, void **data);

#define dlist_size(list)	((list)->size)
#define dlist_head(list)	((list)->head)
#define dlist_tail(list)	((list)->tail)
#define dlist_is_head(element)		((element)->prev == NULL ? 1 : 0)
#define dlist_is_tail(element)		((element)->next == NULL ? 1 : 0)
#define dlist_data(element)			((element)->data)
#define dlist_next(element)			((element)->next)
#define dlist_prev(element)			((element)->prev)

#endif
~~~

双向链表的抽象数据类型的实现文件：

~~~c
/* dlist.c */

#include <stdlib.h>
#include <string.h>

#include "dlist.h"

/* dlist_init */
void dlist_init(DList *list, void (*destroy)(void *data))
{
    list->size = 0;
    list->size = destroy;
    list->head = NULL;
    list->tail = NULL;
    return;
}

/* dlist_destroy */
void dlist_destroy(DList *list)
{
    void *data;
    while(dlist_size(list) > 0)
    {
        if((dlist_remove(list, dlist_tail(list), (void **)&data) == 0) && 
           (list->destroy != NULL))
        {
            list->destroy(data);
        }
    }
    memset(list, 0, sizeof(DList));
    return;
}

/* dlist_ins_next */
int dlist_ins_next(DList *list, DListElmt *element, const void *data)
{
    DListElmt *new_element;
    
    /* do not allow a NULL element unless the list is empty */
    if((element == NULL) && (dlist_size(list) != 0))
    {
        return -1;
    }
    
    if((new_element = (DListElmt *)malloc(sizeof(DListElmt))) == NULL)
    {
        return -1;
    }
    
    new_element->data = (void *)data;
    if(dlist_size(list) == 0)
    {
        list->head = new_element;
        list->head->prev = NULL;
        list->head->next = NULL;
        list->tail = new_element;
    }
    else
    {
        new_element->next = element->next;
        new_element->prev = element;
        if(element->next = NULL)
        {
            list->tail = new_element;
        }
        else
        {
            element->next->prev = new_element;
        }
        element->next = new_element;
    }
    list->size++;
    return 0;
}

/* dlist_ins_prev */
int dlist_ins_prev(DList *list, DListElmt *element, const void *data)
{
    DListElmt *new_element;
    
    /* do not allow a NULL element unless the list is empty */
    if((element == NULL) && (dlist_size(list) != 0))
    {
        return -1;
    }
    
    if((new_element = (DListElmt *)malloc(sizeof(DListElmt))) == NULL)
    {
        return -1;
    }
    
    new_element->data = (void *)data;
    if(dlist_size(list) == 0)
    {
        list->head = new_element;
        list->head->prev = NULL;
        list->head->next = NULL;
        list->tail = new_element;
    }
    else
    {
        new_element->next = element;
        new_element->prev = element->prev;
        if(element->prev = NULL)
        {
            list->head = new_element;
        }
        else
        {
            element->prev->next = new_element;
        }
        element->prev = new_element;
    }
    list->size++;
    return 0;
}

/* dlist_remove */
int dlist_remove(DList *list, DListElmt *element, void **data)
{
    /* do not allow a NULL element or removal from an empty list */
    if((element == NULL) || (dlist_size(list) == 0))
    {
        return -1;
    }
    
    *data = element->data;
    if(element == list->head)
    {
        list->head = element->next;
        if(list->head == NULL)
        {
            list->tail = NULL;
        }
        else
        {
            element->next->prev = NULL;
        }
    }
    else
    {
        element->prev->next = element->next;
        if(element->next == NULL)
        {
            list->tail = element->prev;
        }
        else
        {
            element->next->prev = element->prev;
        }
    }
    free(element);
    list->size--;
    return 0;
}
~~~

## 循环链表

循环链表是另一种形式的链表，循环链表可以是单向的，也可以是双向的。区分一个链表是不是循环链表，主要看其有没有尾部元素。

在循环链表中，最后一个元素的next指针又指向头元素，而不是设置为NULL。

在双向的循环链表中，头元素的prev指针则指向最后一个元素，这样使得循环链表中每个元素都既可以看成是头元素，也可以看成是尾元素。

### clist_init

~~~c
void clist_init(CList *list, void (*destroy)(void *data));
/*
初始化由参数list指定的循环链表
参数destroy提供了一种释放动态分配空间的方法
*/
~~~

### clist_destroy

~~~c
void clist_destroy(CList *list);
/*
销毁由参数list所指定的循环链表
*/
~~~

### clist_ins_next

~~~c
int clist_ins_next(CList *list, CListElmt *element, const void *data);
/*
将元素插入到由参数list指定的循环链表中element的后面
参数data指向新元素所包含的数据域
插入成功则返回0，否则返回-1
*/
~~~

### clist_rem_next

~~~c
int clist_rem_next(CList *list, CListElmt *element, void **data);
/*
移除由参数list所指定的循环链表中element后面的元素
参数data将指向已经移除的元素中存储的数据域
移除成功则返回0，否则返回-1
*/
~~~

### clist_size

~~~c
int clist_size(const CList *list);
/*
计算参数list所指定的循环链表中的元素个数
*/
~~~

### clist_head

~~~c
CListElmt *clist_head(const CList *list);
/*
返回参数list所指定的循环链表中的头元素
*/
~~~

### clist_data

~~~c
void *clist_data(const CListElmt *element);
/*
返回循环链表中参数element所指定的元素中存储的数据域
*/
~~~

### clist_next

~~~c
CListElmt *clist_next(const CListElmt *element);
/*
返回循环链表中由参数element所指定的元素的后继元素
*/
~~~

### 实现

循环链表的抽象数据类型的头文件：

~~~c
/* clist.h */

#ifndef CLIST_H
#define CLIST_H

#include <stdlib.h>

/* define a structure for circular list elements */
typedef struct CListElmt_
{
    void *data;
    struct CListElmt_ *next;
}CListElmt;

/* define a structure for circular lists */
typedef struct CList_
{
    int size;
    int (*match)(const void *key1, const void *key2);
    void (*destroy)(void *data);
    CListElmt *head;
}CList;

/* public interface */
void clist_init(CList *list, void (*destroy)(void *data));
void clist_destroy(CList *list);
int clist_ins_next(CList *list, CListElmt *element, const void *data);
int clist_rem_next(CList *list, CListElmt *element, void **data);

#define clist_size(list)	((list)->size)
#define clist_head(list)	((list)->head)
#define clist_data(element)		((element)->data)
#define clist_next(element)		((element)->next)

#endif
~~~

循环链表的抽象数据类型的实现文件：

~~~c
/* clist.c */

#include <stdlib.h>
#include <string.h>

#include "clist.h"

/* clist_init */
void clist_init(CList *list, void (*destroy)(void *data))
{
    list->size = 0;
    list->destroy = destroy;
    list->head = NULL;
    return;
}

/* clist_destroy */
void clist_destroy(CList *list)
{
    void *data;
    
    while(clist_size(list) > 0)
    {
        if((clist_rem_next(list, list->head, (void **)&data) == 0) &&
           (list->destroy != NULL))
        {
            list->destroy(data);
        }
    }
    memset(list, 0, sizeof(CList));
    return;
}


/* clist_ins_next */
int clist_ins_next(CList *list, CListElmt *element, const void *data)
{
    CListElmt *new_element;
    
    if((new_element = (CListElmt *)malloc(sizeof(CListElmt))) == NULL)
    {
        return -1;
    }
    
    new_element->data = (void *)data;
    if(clist_size(list) == 0)
    {
        new_element->next = new_element;
        list->head = new_element;
    }
    else
    {
        new_element->next = element->next;
        element->next = new_element;
    }
    list->size++;
    return 0;
}

/* clist_rem_next */
int clist_rem_next(CList *list, CListElmt *element, void **data)
{
    CListElmt *old_element;
    
    if(clist_size(list) == 0)
    {
        return -1;
    }
    
    *data = element->next->data;
    if(element->next = element)
    {
        old_element = element->next;
        list->head = NULL;
    }
    else
    {
        old_element = element->next;
        element->next = element->next->next;
        if(old_element == clist_head(list))
        {
            list->head = old_element->next;
        }
    }
    free(old_element);
    list->size--;
    return 0;
}
~~~

