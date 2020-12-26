---
title: redis命令简介 HyperLogLog
date: 2020-07-08 13:48:45
categories: redis
index_img: /img/articles/redis.jpg
tags:
	- redis
	- HyperLogLog
---

# HyperLogLog

首先需要理解一个概念：`基数统计`。基数统计指的是，一个集合（可以包含重复元素的聚合）中不重复的元素的个数。例如集合`{1,2,3,1,2}`，集合具有5个元素，但是由3个不重复的元素，因此其基数为3

基数统计（`cardinality counting`）通常用来统计一个集合中不重复的元素的个数，例如统计某个网站的访问量，或者用户搜索网站的关键词的数量等。数据分析、网络监控、数据库优化等领域，都会涉及基数统计的需求

要实现基数统计，最简单的做法是记录集合中所有不重复的元素集合`S`，当新来一个元素`x`，若`S`中不包含元素`x`，则将`x`加入到`S`中。基数值就是`S`中元素的数量。但是这样统计基数，存在的问题为：

1. 当统计的数据量变大时，相应的存储内存也线性增长
2. 当集合`S`变大，则判断其是否包含新的元素`x`的计算成本也将变大

一般常见的基数计数方法包括：

1. `B树`：优势是插入与查找的效率很高，但是没有节省存储内存

2. `bitmap`，通过bit数组来存储特定数据，统计一亿个数据，大约需要内存12M，虽然可以节约内存，但是对于大数据量的情况，内存节省的还不够

3. `概率算法`：目前还没有在大数据场景下准确计算基数的高效算法，因此在不追求绝对准确的情况下，使用概率算法是很好的解决方案，概率算法不直接存储数据集合本身，而是通过一定的概率统计方法预估基数值，同时保证误差控制在一定的范围内，可以大大节省内存。目前用于基数计数的概率算法包括：

   a）`Linear Counting`：早期的基数估计算法

   b）`LogLog Counting`：相比`Linear Counting`更加节省内存

   c）`HyperLogLog Counting`：是基于`LogLog Counting`的优化和改进，具有更小的误差

**`redis`正是基于`HyperLogLog Counting`算法实现的`HyperLogLog`数据结构，用于统计一组数据集合中的不重复的数据个数。**

在`redis` 中，每个`HyperLogLog`键占用的内存都是`12K`，理论存储接近`2^64`个值。误差大概是`0.81%`



## pfadd

**作用**：使用`HyperLogLog`对给定的一个或者多个集合元素进行计数

**格式**：`pfadd hyperloglog element [element ...]`

**扩展**：

如果给定的所有元素都已经进行过计数，则命令将返回`0`，表示`HyperLogLog`计算出的近似基数没有发生变化

如果给定的元素中出现至少一个元素是之前没有进行过计数的，将导致`HyperLogLog`计算出的近似基数发生了变化，则命令将返回`1`



## pfcount

**作用**：获取`HyperLogLog`为集合计算出的近似基数值

**格式**：`pfcount hyperloglog [hyperloglog ...]`

**扩展**：

当给定的`HyperLogLog`不存在时，命令将返回`0`作为结果

当向命令传入多个`HyperLogLog`时，命令将对所有的`HyperLogLog`执行并集计算，然后返回并集`HyperLogLog`计算出的近似基数



## pfmerge

作用：对多个指定的`HyperLogLog`执行并集计算，然后将计算得出的并集`HyperLogLog`保存到指定的键中

格式：`pfmerge destination hyperloglog [hyperloglog ...]`

扩展：

命令在成功执行并集计算后，将返回`OK`作为结果

如果指定的键已经存在，则命令将覆盖已经存在的键

`pfcount`命令在计算多个`HyperLogLog`的近似基数时，会执行以下的操作：

1. 在内部调用`pfmerge`命令，计算所有给定的`HyperLogLog`的并集，并将这个并集存储到一个临时的`HyperLogLog`中
2. 对临时`HyperLogLog`执行`pfcount`命令，得到其近似的基数值
3. 删除临时`HyperLogLog`
4. 向用户返回得到的近似的基数值