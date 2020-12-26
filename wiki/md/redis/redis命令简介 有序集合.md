---
title: redis命令简介 有序集合
date: 2020-07-08 13:43:45
categories: redis
index_img: /img/articles/redis.jpg
tags:
	- redis
	- 有序集合
	- sorted_set
---

# 有序集合

`redis`的有序集合（`sorted set`）同时具有“有序”和“集合”的两种性质。

有序集合中的每个元素，都由一个`成员`和一个与成员相关联的`分值`组成。成员以字符串的方式存储，分值以64位双精度浮点数格式存储。

与集合类似，有序集合中的每个成员都是独一无二的，同一个有序集合中不会出现重复的成员。

有序集合中的成员按照其各自的分值大小进行排序。

有序集合中的分值，除了可以是数字以外，还可以是字符串`+inf`或者`-inf`，表示无穷大和无穷小。

**需要注意：虽然同一个有序集合中不能存储相同的成员，但是不同的成员的分值却是可以相同的。**

当两个或者多个成员拥有相同的分值时，`redis`将按照这些成员在字典序列中的大小进行排列。



## zadd

**作用**：添加或者更新成员

**格式**：`zadd sorted_set [XX|NX|CH] score member [score member ...]`

**扩展**：

命令可以向有序集合添加一个或者多个成员，还可以对有序集合中已经存在的成员的分值进行更新。

命令具有三个选项，具体作用为：

1. `XX`：命令只会对给定成员中已经存在于有序集合的成员进行更新，而那些不存在于有序集合中的给定成员将会被忽略。即命令只会对有序集合中已有成员进行更新，不会向有序集合添加任何新成员
2. `NX`：命令只会把给定成员中不存在于有序集合的成员添加到有序集合中，而那些已经存在于有序集合中的给定成员则会被忽略。即命令只会向有序集合添加新成员，而不会对已有的成员进行任何更新
3. `CH`：默认情况下，命令返回新添加的成员的数量。若设置`CH`选项，则命令返回被修改的成员数量



## zrem

**作用**：移除指定的成员，以及与这些成员相关联的分值

**格式**：`zrem sorted_set member [member ...]`

**扩展**：

命令返回被移除的成员的数量

如果给定的某个成员并不存在于有序集合中，则命令将自动忽略该成员



## zscore

**作用**：获取成员的分值

**格式**：`zscore sorted_set member`

**扩展**：

如果给定的有序集合并不存在，或者有序集合中并未包含给定的成员，则命令将返回空值



## zincrby

**作用**：对成员的分值执行自增或者自减操作

**格式**：`zincrby sorted_set increment member`

**扩展**：

命令在执行完成后，将返回给定成员的当前的分值

`redis`只提供了对分值执行自增操作的命令，没有提供相对应的自减操作的命令，因此，可以将一个负数增量传递给`zincrby`命令，从而达到对分值执行自减操作的目的

如果给定的成员并不存在于有序集合中，或者给定的有序集合并不存在，则命令将直接把给定的成员添加到有序集合中，并把给定的增量设置为该成员的分值，效果相当于执行`zadd`命令



## zcard

**作用**：获取有序集合的大小

**格式**：`zcard sorted_set`

**扩展**：

如果给定的有序集合不存在，则命令将返回`0`



## zrank & zrevrank

**作用**：获取成员在有序集合中的排名

**格式**：

`zrank sorted_set member`

`zrevrank sorted_set member`

**扩展**：

命令`zrank`返回的是成员的升序排列的排名，即成员在按照分值从小到大进行排列时的排名

命令`zrevrank`返回的是成员的降序排列的排名，即成员在按照分值从大到小进行排列时的排名

如果给定的有序集合不存在，或者给定的成员并不存在于有序集合中，则命令将返回一个空值



## zrange & zrevrange

**作用**：以升序排列或者降序排列的方式，从有序集合中获取指定索引范围内的成员

**格式**：

`zrange sorted_set start end [WITHSCORES]`

`zrevrange sorted_set start end [WITHSCORES]`

**扩展**：

命令`zrange`用于获取按照分值大小实施升序排列的成员

命令`zrevrange`用于获取按照分值大小实施降序排列的成员

参数`start`与`end` 是闭区间的索引范围，位于这两个索引上的成员也会包含在命令的返回结果中

命令也可以接受负数索引

可选的参数`WITHSCORES`可以使得命令在返回指定索引范围内的成员同时，也返回与之关联的分值

如果给定的有序集合不存在，命令将返回一个空列表



## zrangebyscore & zrevrangebyscore

**作用**：以升序排列或者降序排列的方式，获取有序集合中的分值介于指定范围内的成员

**格式**：

`zrangebyscore sorted_set min max [WITHSCORES] [LIMIT offset count]`

`zrevrangebyscore sorted_set max min [WITHSCORES] [LIMIT offset count]`

**扩展**：

**注意：两个命令中，`max`与`min`的位置不同**

可选的参数`WITHSCORES`可以使得命令在返回指定索引范围内的成员同时，也返回与之关联的分值

默认情况下，命令直接返回给定分值范围内的所有成员，但是若范围内的成员数量比较多，则可以使用可选参数`LIMIT offset count`来限制命令返回的成员数量

`LIMIT`选项接受`offset`与`count`两个参数作为输入，`offset`指定命令在返回结果之前需要跳过的成员数量，`count`指示命令最多可以返回多少个成员

默认情况下，命令接受的分值范围都是闭区间分值范围，即分值等于给定的`min`或者`max`的成员也会被包含在结果中

如果想定义的是开区间，则可以在给定分值范围时，在分值参数之前加上一个单括号`“（”`，这样，具有给定分值的成员将不会出现在命令的返回结果中

比如`zrangebyscore salary (3500 (5000`，命令只返回分值大于`3500`，并且小于`5000`的成员，不会返回分值等于`3500`或者`5000`的成员

参数`min`与`max`还可以是特殊的值`+inf`或者`-inf`，分别表示无穷大和无穷小



## zcount

**作用**：统计有序集合中分值介于指定范围内的成员的数量

**格式**：`zcount sorted_set min max`

**扩展**：

命令接受的分值范围参数`min`与`max`，可以使用`+inf`表示无穷大，使用`-inf`表示无穷小，使用单括号`(`定义开区间的分值范围



## zremrangebyrank

**作用**：从升序排列的有序集合中移除位于指定排名范围内的成员

**格式**：`zremrangebyrank sorted_set start end`

**扩展**：

命令将返回被移除的成员的数量

命令接受的也是一个闭区间的范围，即排名为`start`与`end` 的成员也将被移除

排名参数也可以是负数



## zremrangebyscore

**作用**：从有序集合中移除位于指定的分值范围内的成员

**格式**：`zremrangebyscore sorted_set min max`

**扩展**：

命令将返回被移除的成员的数量

命令接受的分值范围参数`min`与`max`，可以使用`+inf`表示无穷大，使用`-inf`表示无穷小，使用单括号`(`定义开区间的分值范围



## zunionstore & zinterstore

**作用**：有序集合的并集运算与交集运算

**格式**：

`zunionstore destination numbers sorted_set [sorted_set ...]`

`zinterstore destination numbers sorted_set [sorted_set ...]`

**扩展**：

参数`number`指定参与计算的有序集合的数量

参数`sorted_set`指定参与计算的多个有序集合的键

计算得出的结果会存储到`destination`参数指定的键中

命令将返回计算结果包含的成员数量



另外，命令还提供了可选的参数`AGGREGATE` 选项，决定使用哪个聚合函数来计算结果的有序集合成员的分值，格式为：

`zunionstore destination numbers sorted_set [sorted_set ...] [AGGREGATE SUM|MIN|MAX]`

`zinterstore destination numbers sorted_set [sorted_set ...] [AGGREGATE SUM|MIN|MAX]`

聚合函数为`SUM`,则将给定的有序集合中所有相同成员的分值相加，它们的和就是该成员在结果有序集合中的分值

聚合函数为`MIN`，则从给定的有序集合中所有相同成员的分值中选出最小的分值，将其作为该成员在结果有序集合中的分值

聚合函数为`MAX`，则从给定的有序集合中所有相同成员的分值中选出最大的分值，将其作为该成员在结果有序集合中的分值

如果没有显式地使用`AGGREGATE`选项指定聚合函数，则命令默认使用`SUM`作为聚合函数



在默认情况下，命令将直接使用给定有序集合的成员分值去计算结果有序集合的成员分值。但是在某些情况下，也可以通过可选参数`WEIGHTS`为各个给定的有序集合的成员分值设置权重。

`zunionstore destination numbers sorted_set [sorted_set ...] [WEIGHTS weight [weight ...]]`

`zinterstore destination numbers sorted_set [sorted_set ...] [WEIGHTS weight [weight ...]]`

用户需要为每个给定的有序集合分别设置一个权重，命令会将这个权重与成员的分值相乘，得出成员的新的分值，然后执行聚合函数计算。与此相反，如何不想改变某个给定的有序集合的分值，则只需要将那个有序集合的权重设置为`1`即可。



## zpopmax & zpopmin

**作用**：移除并返回有序集合中分值最大的和最小的`N`个元素

**格式**：

`zpopmax sorted_set [count]`

`zpopmin sorted_set [count]`

**扩展**：

被移除的元素的数量可以是通过可选的参数`count`来指定，如没有指定，则命令默认只会移除一个元素



## bzpopmax & bzpopmin

**作用**：组赛式的最大或者最小元素的弹出操作

**格式**：

`bzpopmax sorted_set [sorted_set ...] timeout`

`bzpopmin sorted_set [sorted_set ...] timeout`

**扩展**：

命令依次检查用户给定的有序集合，并从其遇到的第一个非空有序集合中弹出指定的元素

如果命令在检查了所有的有序集合之后，都没有发现可弹出的元素，则命令将阻塞执行命令的客户端，并在给定的时限之内等待可弹出的元素出现，直到等待时间超出给定的时限为止

可以设置等待时限为`0`，让命令一直阻塞，直到可弹出的元素出现为止

命令在成功弹出元素时，将返回一个包含`3`项的列表，分别是被弹出元素所在的有序集合、被弹出元素的成员、被弹出元素的分值