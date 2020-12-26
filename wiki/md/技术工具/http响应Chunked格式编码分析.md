---
title: HTTP响应Chunked格式编码分析
date: 2020-07-08 13:55:45
categories: HTTP
index_img: /img/articles/http.jpg
tags:
	- HTTP
	- Chunked
---

## HTTP响应Chunked格式编码分析

在某些时候，服务端生成的`HTTP`应答数据是无法确定信息的大小，这个时候，使用`Content-Length`无法在事前写入长度，当需要实时生成消息长度，服务端一般采用`Chunked`格式编码。

在进行`Chunked`格式编码时，回复的消息头部具有`transfer-coding`字段，并且该字段被定义为`Chunked`，表示将采用`Chunked`格式编码传输数据内容。

`Chunked`编码，使用若干个`Chunk`串组合而成，且由一个标明长度为0的`Chunk`块标识结束。

每个`Chunk`块分为头部与正文两个部分。

头部内容指定正文的字符总数，用十六进制数字表示，头部内容还指定数量单位，但是一般省略不写。

正文部分就是指定长度的实际数据内容。

在正文与头部之间，使用回车换行符（`CRLF`）隔开。

在最后一个`Chunk`块，其头部定义长度为0，该`Chunk`块中的内容被称为`footer`，是一些附加的`header`信息，通常可以省略。

一般数据结构可模拟表示为：

~~~
[chunk块长度][CRLF][chunk块数据][CRLF][chunk块长度][CRLF][chunk块数据][CRLF]......[chunk块长度][CRLF][chunk块数据][CRLF][长度0][CRLF][footer内容][CRLF]
~~~

注意，`chunk`块长度是以十六进制的ASCII码表示，比如86AE，表示长度为34478。一般`chunk`块长度占7个字节，不满7个字节以空格补齐，空格的ASCII码为`0x20`。



对`chunked`编码格式的数据进行解码，就是将分块的`chunk-data`数据整合恢复成一块报文体，同时记录此数据体的长度。

解码流程的伪代码为：

~~~
length=0											//长度计数器置为0
read chunk-size, chunk-extension(if any) and CRLF	//读取chunk-size与chunk-extension
while(chunk-size > 0)								//表明不是last-chunk
{
	read chunk-data and CRLF						//读chunk-size大小的chunk-data
	append chunk-data to entity-body				//追加chunk-data数据至整体数据块
	length = length + chunk-size					//计算数据长度
	read chunk-size and CRLF						//读取新的chunk-size
}
read entity-header									//entity-header的格式为name:value
													//如果为空。则只有CRLF
while(entity-header not empty)						//非空行
{
	append entity-header to existing header fields
	read entity-header
}
Content-Length = length								//获取整个报文的长度值
remove "chunked" from Transfer-Encoding				//去除chunked标志
~~~



范例：

~~~
Encoded response
HTTP/1.1 200 OK
Content-Type: text/plain
Transfer-Encoding: chunked

25
This is the data in the first chunk

1A
and this is the second one
0

same as above, raw bytes in hex
0000-000F   48 54 54 50 2f 31 2e 31 20 32 30 30 20 4f 4b 0d   HTTP/1.1 200 OK.
0010-001F   0a 43 6f 6e 74 65 6e 74 2d 54 79 70 65 3a 20 74   .Content-Type: t
0020-002F   65 78 74 2f 70 6c 61 69 6e 0d 0a 54 72 61 6e 73   ext/plain..Trans
0030-003F   66 65 72 2d 45 6e 63 6f 64 69 6e 67 3a 20 63 68   fer-Encoding: ch
0040-004F   75 6e 6b 65 64 0d 0a 0d 0a 32 35 0d 0a 54 68 69   unked....25..Thi
0050-005F   73 20 69 73 20 74 68 65 20 64 61 74 61 20 69 6e   s is the data in
0060-006F   20 74 68 65 20 66 69 72 73 74 20 63 68 75 6e 6b    the first chunk
0070-007F   0d 0a 0d 0a 31 41 0d 0a 61 6e 64 20 74 68 69 73   ....1A..and this
0080-008F   20 69 73 20 74 68 65 20 73 65 63 6f 6e 64 20 6f    is the second o
0090-009F   6e 65 0d 0a 30 0d 0a 0d 0a                        ne..0....
~~~