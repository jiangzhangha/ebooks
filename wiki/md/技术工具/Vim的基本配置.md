---
title: Vim的基本配置
date: 2020-07-08 14:00:00
categories: vim
index_img: /img/articles/vim.jpg
tags:
	- vim
	- 配置
---

## Vim的基本配置

Vim的默认配置不适合生产环境中的正常使用需求，因此需对`.vimrc`的配置文件进行配置设置。以下总结了一些Vim常用的配置方式。


```
'设置编码'
set fileencodings=utf-8,ucs-bom,gb18030,gbk,gb2312,cp936
set termencoding=utf-8
set encoding=utf-8

'显示行号'
set nu
set number

'突出显示当前行'
set cursorline

'启用鼠标'
set mouse=a
set selection=exclusive
set selectmode=mouse,key

'显示括号匹配'
set showmatch

'设置Tab长度为4空格'
set tabstop=4

'设置自动缩进长度为4空格'
set shiftwidth=4

'继承前一行的缩进方式，适用于多行注释'
set autoindent

'设置粘贴模式'
set paste

'显示空格和tab键'
set listchars=tab:>-,trail:-

'总是显示状态栏'
set laststatus=2

'显示光标当前位置'
set ruler

'不要使用vi的键盘模式，而是vim自己的'
set nocompatible

'侦测文件类型'
filetype on

'语法高亮'
syntax enable
syntax on

'使回格键（backspace）正常处理'
set backspace=2

'设置制表符'
set tabstop=4

```