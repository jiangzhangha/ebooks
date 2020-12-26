---
title: Git使用方法梳理总结
date: 2020-07-08 21:54:47
categories: git
index_img: /img/articles/git.jpg
tags:
	- git
---

# Git流程图

![](流程图.png)

`Workspace`：工作区

`Index / Stage`：暂存区

`Repository`：仓库区（或本地仓库）

`Remote`：远程仓库

# 配置Git


```
# 配置全局用户
$ git config --global user.name "用户名"
$ git config --global user.email "git账号"

# 配置别名
$ git config --global alias.co checkout
$ git config --global alias.ss status
$ git config --global alias.cm commit
$ git config --global alias.br branch
$ git config --global alias.rg reflog

# 这里只是美化 log 的输出，实际使用时可以在 git lg 后面加命令参数，如：git lg -10 显示最近10条提交
$ git config --global alias.lg "log --color --graph --pretty=format:'%Cred%h%Creset -%C(yellow)%d%Creset %s %Cgreen(%cr) %C(bold blue)<%an>%Creset' --abbrev-commit"

# 删除全局配置
$ git config --global --unset alias.xxx
$ git config --global --unset user.xxx
```

# 查看Git信息


```
# 查看系统配置
$ git config --list

# 查看用户配置
$ cat ~/.gitconfig

# 查看当前项目的 git 配置
$ cat .git/config

# 查看暂存区的文件
$ git ls-files

# 查看本地 git 命令历史
$ git reflog

# 查看所有 git 命令
$ git --help -a

# 查看当前 HEAD 指向
$ cat .git/HEAD

# git 中 D 向下翻一行  F 向下翻页  B 向上翻页  Q 退出
# 查看提交历史
$ git log --oneline
          --grep="关键字"
          --graph
          --all
          --author "username"
          --reverse
          -num
          -p
          --before=  1  day/1  week/1  "2019-06-06"
          --after= "2019-06-06"
          --stat
          --abbrev-commit
          --pretty=format:"xxx"
          
# oneline -> 将日志记录一行一行的显示
# grep="关键字" -> 查找日志记录中(commit提交时的注释)与关键字有关的记录
# graph -> 记录图形化显示 ！！！
# all -> 将所有记录都详细的显示出来
# author "username" -> 查找这个作者提交的记录
# reverse -> commit 提交记录顺序翻转
# before -> 查找规定的时间(如:1天/1周)之前的记录
# num -> git log -10 显示最近10次提交 ！！！
# stat -> 显示每次更新的文件修改统计信息，会列出具体文件列表 ！！！
# abbrev-commit -> 仅显示 SHA-1 的前几个字符，而非所有的 40 个字符 ！！！
# pretty=format:"xxx" ->  可以定制要显示的记录格式 ！！！
# p -> 显示每次提交所引入的差异（按 补丁 的格式输出）！！！

```

## git reflog

显示的是一个` HEAD` 指向发生改变的时间列表。在你切换分支、用` git commit `进行提交、以及用 `git reset` 撤销 `commit` 时，`HEAD` 指向会改变，但当你进行 `git checkout -- <filename>` 撤销或者 `git stash` 存储文件等操作时，`HEAD` 并不会改变，这些修改从来没有被提交过，因此 `reflog `也无法帮助我们恢复它们。

## git log 点线图

`git` 中一条分支就是一个指针，新建一条分支就是基于当前指针新建一个指针
切换至某个分支 ，就是将 `HEAD` 指向某条分支（指针）
切换至某个` commit`，就是将` HEAD` 指向某个 `commit`

## 符号解释


```
*	表示一个 commit
|	表示分支前进
/	表示分叉
\	表示合入
|/	表示新分支
```

# Git常用命令


```
# 查看工作区和暂存区的状态
$ git status

# 将工作区的文件提交到暂存区
$ git add .

# 提交到本地仓库
$ git commit -m "本次提交说明"

# add和commit的合并，便捷写法（未追踪的文件无法直接提交到暂存区/本地仓库）
$ git commit -am "本次提交说明"

# 将本地分支和远程分支进行关联
$ git push -u origin branchName

# 将本地仓库的文件推送到远程分支
$ git push

# 拉取远程分支的代码
$ git pull origin branchName

# 合并分支
$ git merge branchName

# 查看本地拥有哪些分支
$ git branch

# 查看所有分支（包括远程分支和本地分支）
$ git branch -a

# 切换分支
$ git checkout branchName

# 临时将工作区文件的修改保存至堆栈中
$ git stash

# 将之前保存至堆栈中的文件取出来
$ git stash pop
```

# Git常用命令详解

## add

将工作区的文件添加到暂存区


```
# 添加指定文件到暂存区（追踪新增的指定文件）
$ git add [file1] [file2] ...

# 添加指定目录到暂存区，包括子目录
$ git add [dir]

# 添加当前目录的所有文件到暂存区（追踪所有新增的文件）
$ git add .

# 删除工作区/暂存区的文件
$ git rm [file1] [file2] ...

# 停止追踪指定文件，但该文件会保留在工作区
$ git rm --cached [file]

# 改名工作区/暂存区的文件
$ git mv [file-original] [file-renamed]

# Git 2.0 以下版本
#只作用于文件的新增和修改
$ git add .

#只作用于文件的修改和删除
$ gti add -u

#作用于文件的增删改
$ git add -A

# Git 2.0 版本
$ git add . 等价于 $ git add -A

```

`git add .`：操作的对象是当前目录”所有文件变更。`.` 表示当前目录。会监控工作区的状态树，使用它会把工作区的所有变化提交到暂存区，包括文件内容修改（`modified`）以及新文件（`new`），但不包括被删除的文件

`git add -u`：操作的对象是整个工作区已经跟踪的文件变更，无论当前位于哪个目录下。仅监控已经被 add 的文件（即 `tracked file`），它会将被修改的文件（包括文件删除）提交到暂存区。`git add -u `不会提交新文件（`untracked file`）。（`git add --update` 的缩写）

`git add -A`：操作的对象是“整个工作区”所有文件的变更，无论当前位于哪个目录下。是上面两个功能的合集`git add --all `的缩写

## status


```
# 查看工作区和暂存区的状态
$ git status

```

## commit


```
# 将暂存区的文件提交到本地仓库并添加提交说明
$ git commit -m "本次提交的说明"

# add 和 commit 的合并，便捷写法
# 和 git add -u 命令一样，未跟踪的文件是无法提交上去的
$ git commit -am "本次提交的说明"

# 跳过验证继续提交
$ git commit --no-verify
$ git commit -n

# 编辑器会弹出上一次提交的信息，可以在这里修改提交信息
$ git commit --amend

# 修复提交，同时修改提交信息
$ git commit --amend -m "本次提交的说明"

# 加入 --no-edit 标记会修复提交但不修改提交信息，编辑器不会弹出上一次提交的信息
$ git commit --amend --no-edit

```

`git commit --amend `既可以修改上次提交的文件内容，也可以修改上次提交的说明。会用一个新的 `commit` 更新并替换最近一次提交的` commit `。如果暂存区有内容，这个新的` commit `会把任何修改内容和上一个` commit `的内容结合起来。如果暂存区没有内容，那么这个操作就只会把上次的` commit `消息重写一遍。永远不要修复一个已经推送到公共仓库中的提交，会拒绝推送到仓库

## push & pull

分支推送顺序的写法是 <来源地>:<目的地>


```
# 将本地仓库的文件推送到远程分支
# 如果远程仓库没有这个分支，会新建一个同名的远程分支
# 如果省略远程分支名，则表示两者同名
$ git push <远程主机名> <本地分支名>:<远程分支名>
$ git push origin branchname

# 如果省略本地分支名，则表示删除指定的远程分支
# 因为这等同于推送一个空的本地分支到远程分支。
$ git push origin :master
# 等同于
$ git push origin --delete master

# 建立当前分支和远程分支的追踪关系
$ git push -u origin master
# 如果当前分支与远程分支之间存在追踪关系
# 则可以省略分支和 -u
$ git push

# 不管是否存在对应的远程分支，将本地的所有分支都推送到远程主机
$ git push --all origin

# 拉取所有远程分支到本地镜像仓库中
$ git pull

# 拉取并合并项目其他人员的一个分支
$ git pull origin branchname
# 等同于 fetch + merge
$ git fetch origin branchName
$ git merge origin/branchName

# 如果远程主机的版本比本地版本更新，推送时 Git 会报错，要求先在本地做 git pull 合并差异，
# 然后再推送到远程主机。这时，如果你一定要推送，可以使用 –-force 选项
# （尽量避免使用）
$ git push --force origin | git push -f origin
```

## branch


```
# 查看本地分支
$ git branch | git branch -l

# 查看远程分支
$ git branch -r

# 查看所有分支（本地分支+远程分支）
$ git branch -a

# 查看所有分支并带上最新的提交信息
$ git branch -av

# 查看本地分支对应的远程分支
$ git branch -vv

# 新建分支
# 在别的分支下新建一个分支，新分支会复制当前分支的内容
# 注意：如果当前分支有修改，但是没有提交到仓库，此时修改的内容是不会被复制到新分支的
$ git branch branchname

# 切换分支(切换分支时，本地工作区，仓库都会相应切换到对应分支的内容)
$ git checkout branchname

# 创建一个 aaa 分支，并切换到该分支 （新建分支和切换分支的简写）
$ git checkout -b aaa

# 可以看做是基于 master 分支创建一个 aaa 分支，并切换到该分支
$ git checkout -b aaa master

# 新建一条空分支（详情请看问题列表）
$ git checkout --orphan emptyBranchName
$ git rm -rf .

# 删除本地分支,会阻止删除包含未合并更改的分支
$ git brnach -d branchname

# 强制删除一个本地分支，即使包含未合并更改的分支
$ git branch -D branchname

# 删除远程分支
# 推送一个空分支到远程分支，其实就相当于删除远程分支
$ git push origin  :远程分支名
# 或者
$ git push origin --delete 远程分支名

# 修改当前分支名
$ git branch -m branchname
```

## merge


```
# 默认 fast-forward ，HEAD 指针直接指向被合并的分支
$ git merge

# 禁止快进式合并
$ git merge --no-ff

$ git merge --squash
```

![](merge.png)

`fast-forward`：会在当前分支的提交历史中添加进被合并分支的提交历史（得先理解什么时候会发生快速合并，并不是每次 merge 都会发生快速合并）

`--no-ff`：会生成一个新的提交，让当前分支的提交历史不会那么乱

`--squash`：不会生成新的提交，会将被合并分支多次提交的内容直接存到工作区和暂存区，由开发者手动去提交，这样当前分支最终只会多出一条提交记录，不会掺杂被合并分支的提交历史

## stash

能够将所有未提交的修改保存至堆栈中，用于后续恢复当前工作区内容

如果文件没有提交到暂存区（使用 git add . 追踪新的文件），使用该命令会提示` No local changes to save `，无法将修改保存到堆栈中

使用场景： 当你接到一个修复紧急 bug 的任务时候，一般都是先创建一个新的 bug 分支来修复它，然后合并，最后删除。但是，如果当前你正在开发功能中，短时间还无法完成，无法直接提交到仓库，这时候可以先把当前工作区的内容` git stash` 一下，然后去修复 bug，修复后，再 `git stash pop`，恢复之前的工作内容


```
# 将所有未提交的修改（提交到暂存区）保存至堆栈中
$ git stash

# 给本次存储加个备注，以防时间久了忘了
$ git stash save "存储"

# 存储未追踪的文件
$ git stash -u

# 查看存储记录
$ git stash list

在 Windows 上和 PowerShell 中，需要加双引号
# 恢复后，stash 记录并不删除
$ git stash apply "stash@{index}"

# 恢复的同时把 stash 记录也删了
$ git stash pop "stash@{index}"

# 删除 stash 记录
$ git stash drop "stash@{index}"

# 删除所有存储的进度
$ git stash clear

# 查看当前记录中修改了哪些文件
$ git stash show "stash@{index}"

# 查看当前记录中修改了哪些文件的内容
$ git stash show -p "stash@{index}"
```

## diff


```
# 查看工作区和暂存区单个文件的对比
$ git diff filename

# 查看工作区和暂存区所有文件的对比
$ git diff

# 查看工作区和暂存区所有文件的对比，并显示出所有有差异的文件列表
$ git diff --stat

# 注意：
# 1.你修改了某个文件，但是没有提交到暂存区，这时候会有对比的内容
# 一旦提交到暂存区，就不会有对比的内容(因为暂存区已经更新)
# 2.如果你新建了一个文件，但是没有提交到暂存区，这时候 diff 是没有结果的	

# 查看暂存区与上次提交到本地仓库的快照（即最新提交到本地仓库的快照）的对比
$ git diff --cached/--staged

# 查看工作区与上次提交到本地仓库的快照（即最新提交到本地仓库的快照）的对比
$ git diff branchname

# 查看工作区与 HEAD 指向（默认当前分支最新的提交）的对比
$ git diff HEAD

# 查看两个本地分支中某一个文件的对比
$ git diff branchname..branchname filename

# 查看两个本地分支所有的对比
$ git diff branchname..branchname

# 查看远程分支和本地分支的对比
$ git diff origin/branchname..branchname

# 查看远程分支和远程分支的对比
$ git diff origin/branchname..origin/branchname

# 查看两个 commit 的对比
$ git diff commit1..commit2
```

## remote


```
# 查看所有远程主机
$ git remote

# 查看关联的远程仓库的详细信息
$ git remote -v

# 删除远程仓库的 “关联”
$ git remote rm projectname

# 设置远程仓库的 “关联”
$ git remote set-url origin <newurl>
```

## tag

常用于发布版本


```
# 默认在 HEAD 上创建一个标签
$ git tag v1.0
# 指定一个 commit id 创建一个标签
$ git tag v0.9 f52c633
# 创建带有说明的标签，用 -a 指定标签名，-m 指定说明文字
$ git tag -a v0.1 -m "version 0.1 released"

# 查看所有标签
# 注意：标签不是按时间顺序列出，而是按字母排序的。
$ git tag

# 查看单个标签具体信息
$ git show <tagname>

# 推送一个本地标签
$ git push origin <tagname>
# 推送全部未推送过的本地标签
$ git push origin --tags

# 删除本地标签
# 因为创建的标签都只存储在本地，不会自动推送到远程。
# 所以，打错的标签可以在本地安全删除。
$ git tag -d v0.1
# 删除一个远程标签（先删除本地 tag ，然后再删除远程 tag）
$ git push origin :refs/tags/<tagname>
```

## 删除文件


```
# 删除暂存区和工作区的文件
$ git rm filename

# 只删除暂存区的文件，不会删除工作区的文件
$ git rm --cached filename
```

如果在配置` .gitignore `文件之前就把某个文件上传到远程仓库了，这时候想把远程仓库中的该文件删除，此时你配置` .gitignore `文件也没有用，因为该文件已经被追踪了，但又不想在本地删除该文件后再重新提交到远程仓库，这时候可以使用` git rm --cached filename `命令取消该文件的追踪，这样下次提交的时候，`git `就不会再提交这个文件，从而远程仓库的该文件也会被删除

## 版本切换 & 重设 & 撤销

`checkout `可以撤销工作区的文件，`reset `可以撤销工作区/暂存区的文件

`reset `和` checkout` 可以作用于 `commit`或者文件，`revert `只能作用于` commit`

## checkout


```
# 恢复暂存区的指定文件到工作区
$ git checkout <filename>

# 恢复暂存区的所有文件到工作区
$ git checkout .

# 回滚到最近的一次提交
# 如果修改某些文件后，没有提交到暂存区，此时的回滚是回滚到上一次提交
# 如果是已经将修改的文件提交到仓库了，这时再用这个命令回滚无效
# 因为回滚到的是之前自己修改后提交的版本
$ git checkout HEAD
$ git checkout HEAD -- filename

# 回滚到最近一次提交的上一个版本
$ git checkout HEAD^

# 回滚到最近一次提交的上2个版本
$ git checkout HEAD^^

# 切换分支，在这里也可以看做是回到项目「当前」状态的方式
$ git checkout <当前你正在使用的分支>

# 切换到某个指定的 commit 版本
$ git checkout <commit_id>

# 切换指定 tag
$ git checkout <tag>
```

在开发的正常阶段，`HEAD` 一般指向` master `或是其他的本地分支，但当你使用 `git checkout <commit id> `切换到指定的某一次提交的时候，`HEAD` 就不再指向一个分支了——它直接指向一个提交，`HEAD `就会处于 `detached `状态（游离状态）

切换到某一次提交后，你可以查看文件，编译项目，运行测试，甚至编辑文件而不需要考虑是否会影响项目的当前状态，你所做的一切都不会被保存到主栈的仓库中。当你想要回到主线继续开发时，使用 `git checkout branchName `回到项目初始的状态（这时候会提示你是否需要新建一条分支用于保留刚才的修改）

哪怕你切换到了某一版本的提交，并且对它做了修改后，不小心提交到了暂存区，只要你切换回分支的时候，依然会回到项目的初始状态。（注意：你所做的修改，如果 `commit `了，会被保存到那个版本中。切换完分支后，会提示你是否要新建一个分支来保存刚才修改的内容。如果你刚才解决了一个 bug ，这时候可以新建一个临时分支，然后你本地自己的开发主分支去合并它，合并完后删除临时分支）

## reset

`git reset [--hard|soft|mixed|merge|keep] [<commit>或HEAD]`：将当前的分支重设(`reset`)到指定的` <commit> `或者` HEAD` （默认，如果不显示指定` <commit>`，默认是` HEAD `，即最新的一次提交），并且根据` [mode] `有可能更新索引和工作目录。`mode` 的取值可以是 `hard、soft、mixed、merged、keep`


```
# 从暂存区撤销特定文件，但不改变工作区。它会取消这个文件的暂存，而不覆盖任何更改
$ git reset <fileName>

# 重置暂存区最近的一次提交，但工作区的文件不变
$ git reset
# 等价于
$ git reset HEAD （默认）

# 重置暂存区与工作区，回退到最近一次提交的版本内容
$ git reset --hard

# 重置暂存区与工作区，回退到最近一次提交的上一个版本
$ git reset --hard HEAD^

# 将当前分支的指针指向为指定 commit（该提交之后的提交都会被移除），同时重置暂存区，但工作区不变
$ git reset <commit>
# 等价于
$ git reset --mixed  <commit>

# 将当前分支的指针指向为指定 commit（该提交之后的提交都会被移除），但保持暂存区和工作区不变
$ git reset --soft  <commit>

# 将当前分支的指针指向为指定 commit（该提交之后的提交都会被移除），同时重置暂存区、工作区
$ git reset --hard  <commit>
```

`git reset `有很多种用法。它可以被用来移除提交快照，尽管它通常被用来撤销暂存区和工作区的修改。不管是哪种情况，它应该只被用于本地修改——你永远不应该重设和其他开发者共享的快照

当你用` reset `回滚到了某个版本后，那么在下一次` git `提交时，之前该版本后面的版本会被作为垃圾删掉

当我们回退到一个旧版本后，此时再用` git log` 查看提交记录，会发现之前的新版本记录没有了。如果第二天，你又想恢复到新版本怎么办？找不到新版本的 `commit_id `怎么办？我们可以用 `git reflog `查看历史命令，这样就可以看到之前新版本的 `commit_id` ，然后` git reset --hard commit_id `就可以回到之前的新版本代码

虽然可以用 `git reflog `查看本地历史，然后回复到之前的新版本代码，但是在别的电脑上是无法获取你的历史命令的，所以这种方法不安全。万一你的电脑突然坏了，这时候就无法回到未来的版本

## revert


```
# 生成一个撤销最近的一次提交的新提交
$ git revert HEAD

# 生成一个撤销最近一次提交的上一次提交的新提交
$ git revert HEAD^

# 生成一个撤销最近一次提交的上两次提交的新提交
$ git revert HEAD^^

# 生成一个撤销最近一次提交的上n次提交的新提交
$ git revert HEAD~num

# 生成一个撤销指定提交版本的新提交
$ git revert <commit_id>

# 生成一个撤销指定提交版本的新提交，执行时不打开默认编辑器，直接使用 Git 自动生成的提交信息
$ git revert <commit_id> --no-edit
```

`git revert`命令用来撤销某个已经提交的快照（和 reset 重置到某个指定版本不一样）。它是在提交记录最后面加上一个撤销了更改的新提交，而不是从项目历史中移除这个提交，这避免了 Git 丢失项目历史

撤销（revert）应该用在你想要在项目历史中移除某个提交的时候。比如说，你在追踪一个 bug，然后你发现它是由一个提交造成的，这时候撤销就很有用

撤销（revert）被设计为撤销公共提交的安全方式，重设（reset）被设计为重设本地更改

因为两个命令的目的不同，它们的实现也不一样：重设完全地移除了一堆更改，而撤销保留了原来的更改，用一个新的提交来实现撤销。千万不要用` git reset `回退已经被推送到公共仓库上的 提交，它只适用于回退本地修改（从未提交到公共仓库中）。如果你需要修复一个公共提交，最好使用 `git revert`

发布一个提交之后，你必须假设其他开发者会依赖于它。移除一个其他团队成员在上面继续开发的提交在协作时会引发严重的问题。当他们试着和你的仓库同步时，他们会发现项目历史的一部分突然消失了。一旦你在重设之后又增加了新的提交，Git 会认为你的本地历史已经和 `origin/master` 分叉了，同步你的仓库时的合并提交(`merge commit`)会使你的同事困惑

## cherry-pick

将指定的提交` commit `应用于当前分支（可以用于恢复不小心撤销（`revert/reset`）的提交）


```
$ git cherry-pick <commit_id>
$ git cherry-pick <commit_id> <commit_id>
$ git cherry-pick <commit_id>^..<commit_id>

```

## git submodule 子模块

有种情况我们经常会遇到：某个工作中的项目需要包含并使用另一个项目。也许是第三方库，或者你独立开发的，用于多个父项目的库。现在问题来了：你想要把它们当做两个独立的项目，同时又想在一个项目中使用另一个。如果将另外一个项目中的代码复制到自己的项目中，那么你做的任何自定义修改都会使合并上游的改动变得困难。Git 通过子模块来解决这个问题，允许你将一个 Git 仓库作为另一个 Git 仓库的子目录。它能让你将另一个仓库克隆到自己的项目中，同时还保持提交的独立


```
# 在主项目中添加子项目，URL 为子模块的路径，path 为该子模块存储的目录路径
git submodule add [URL] [Path]

# 克隆含有子项目的主项目
git clone [URL]

# 当你在克隆这样的项目时，默认会包含该子项目的目录，但该目录中还没有任何文件
# 初始化本地配置文件
git submodule init

# 从当前项目中抓取所有数据并检出父项目中列出的合适的提交
git submodule update

# 等价于 git submodule init && git submodule update
git submodule update --init

# 自动初始化并更新仓库中的每一个子模块， 包括可能存在的嵌套子模块
git clone --recurse-submodules [URL]

```

# 新建一个 Git 项目的两种方式

## 本地新建好 Git 项目，然后关联远程仓库


```
# 初始化一个Git仓库
$ git init

# 关联远程仓库
$ git remote add <name> <git-repo-url>

# 例如
$ git remote add origin https://github.com/xxxxxx
```

## clone 远程仓库


```
# 新建好远程仓库，然后 clone 到本地
$ git clone <git-repo-url>

# 将远程仓库下载到（当前 git bash 启动位置下面的）指定文件中，如果没有会自动生成
$ git clone <git-repo-url> <project-name>
```

# Git 分支管理规范

实际开发的时候，一人一条分支（个人见解：除非是大项目，参与的开发人员很多时，可以采用` feature `分支，否则一般的项目中，一个开发者一条分支够用了）。除此之外还要有一条` develop `开发分支，一条` test` 测试分支，一条` release `预发布分支。

* 「develop」：「开发分支」，开发人员每天都需要拉取/提交最新代码的分支；
* 「test」：「测试分支」，开发人员开发完并自测通过后，发布到测试环境的分支；
* 「release」：「预发布分支」，测试环境测试通过后，将测试分支的代码发布到预发环境的分支（「这个得看公司支不支持预发环境，没有的话就可以不采用这条分支」）；
* 「master」：「线上分支」，预发环境测试通过后，运营/测试会将此分支代码发布到线上环境；

大致流程：

* 开发人员每天都需要拉取/提交最新的代码到 「develop 分支」；
* 开发人员开发完毕，开始 「集成测试」，测试无误后提交到 「test 分支」并发布到测试环境，交由测试人员测试；
* 测试环境通过后，发布到 「release 分支」 上，进行预发环境测试；
* 预发环境通过后，发布到 「master 分支」上并打上标签（`tag`）；
* 如果线上分支出了 bug ，这时候相关开发者应该基于预发布分支（「没有预发环境，就使用 master 分支」），新建一个 「bug 分支」用来临时解决 bug ，处理完后申请合并到 预发布 分支。这样做的好处就是：不会影响正在开发中的功能。


「预发布环境的作用：」 预发布环境是正式发布前最后一次测试。因为在少数情况下即使预发布通过了，都不能保证正式生产环境可以100%不出问题；预发布环境的配置，数据库等都是跟线上一样；有些公司的预发布环境数据库是连接线上环境，有些公司预发布环境是单独的数据库；如果不设预发布环境，如果开发合并代码有问题，会直接将问题发布到线上，增加维护的成本。