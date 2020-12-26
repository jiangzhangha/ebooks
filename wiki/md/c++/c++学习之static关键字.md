---
title: c++学习之static关键字
date: 2020-10-14 14:20:00
categories: c++
index_img: /img/articles/c++.jpg
tags:
	- c++
	- static
---

本文总结c++中static关键的使用方式。

<!-- more -->

我们可以使用static关键字修饰以下的数据：

1. 函数中的变量
2. 类中的变量
3. 类对象
4. 类中的函数

## 修饰函数中的变量

当对函数中的变量使用static关键字修饰时，该变量的生命周期将贯穿整个程序，即使多次调用该函数，静态变量的空间也只分配一次，前一次调用中的变量值将通过下一次函数调用传递。举例如下：

~~~c++
#include <iostream>
#include <string>

using namespace std;

void demo()
{
    //count为函数内的静态变量，内存只分配一次
    static int count = 0;
    cout << count << " ";
    count++;
}

int main()
{
    for(int i = 0; i < 5; i++)
    {
        demo();
    }
    return 0;
}
//程序输出:
//0 1 2 3 4
~~~

## 修饰类中的变量

类中的变量若使用static关键字修饰，则此变量即为类的静态变量，静态变量是对象共享的，可以认为静态变量属于类，而不属于某一个对象。因此对于不同的对象，不能有相同的静态变量的多个副本。

类的静态变量不能使用构造函数初始化。

比如：

~~~c++
#include <iostream>
using namespace std;

class Apple
{
public:
    static int i;
    Apple() {};
};

int main()
{
    Apple obj1;
    Apple obj2;
    //error: i为类的静态变量，尝试创建多个副本将错误
    obj1.i = 2;
    obj2.i = 3;
    
    return 0;
}
~~~

在上面的程序中，尝试为多个对象创建静态变量的多个副本，这样会导致编译器报错，编译时的错误信息为：

~~~shell
[jamza@jamza-vm cpp]$ g11 7.cpp
/tmp/cctsD3A3.o: In function `main':
7.cpp:(.text+0x22): undefined reference to `Apple::i'
7.cpp:(.text+0x2c): undefined reference to `Apple::i'
collect2: error: ld returned 1 exit status
[jamza@jamza-vm cpp]$
~~~

对于类中的静态变量，正确的做法，是使用类名和范围解析运算符，显式地初始化。因此，以上的程序，应该修改为：

~~~c++
#include <iostream>
using namespace std;

class Apple
{
public:
    static int i;
    Apple() {};
};

int Apple::i = 1;

int main()
{
    Apple obj;
    cout << obj.i << endl;
    
    return 0;
}
~~~

## 修饰类对象

通过static关键字修饰类对象，使得类对象称为静态类对象，静态类对象的生命周期将贯穿整个程序。

考虑如下的代码：

~~~c++
#include<iostream> 
using namespace std; 

class Apple 
{ 
    int i; 
    public: 
        Apple() 
        { 
            i = 0; 
            cout << "Inside Constructor\n"; 
        } 
        ~Apple() 
        { 
            cout << "Inside Destructor\n"; 
        } 
}; 

int main() 
{ 
    int x = 0; 
    if (x==0) 
    { 
        Apple obj; 
    } 
    cout << "End of main\n"; 
} 
//程序输出:
//Inside Constructor
//Inside Destructor
//End of main
~~~

在上面的程序中，对象在if块内声明为非静态。因此，变量的范围仅在if块内。因此，当创建对象时，将调用构造函数，并且在if块的控制权越过析构函数的同时调用，因为对象的范围仅在声明它的if块内。 如果我们将对象声明为静态，现在让我们看看输出的变化：

~~~c++
#include<iostream> 
using namespace std; 

class Apple 
{ 
    int i; 
    public: 
        Apple() 
        { 
            i = 0; 
            cout << "Inside Constructor\n"; 
        } 
        ~Apple() 
        { 
            cout << "Inside Destructor\n"; 
        } 
}; 

int main() 
{ 
    int x = 0; 
    if (x==0) 
    { 
        static Apple obj; 
    } 
    cout << "End of main\n"; 
} 
//程序输出:
//Inside Constructor
//End of main
//Inside Destructor
~~~

可以清楚地看到输出的变化。现在，在main结束后调用析构函数。这是因为静态对象的范围是贯穿程序的生命周期。

## 修饰类中的函数

就像类中的静态数据成员或静态变量一样，静态成员函数也不依赖于类的对象。

我们被允许使用对象和'.'来调用静态成员函数。但建议使用类名和范围解析运算符调用静态成员。

允许静态成员函数仅访问静态数据成员或其他静态成员函数，它们无法访问类的非静态数据成员或成员函数。

例子：

~~~c++
#include<iostream> 
using namespace std; 

class Apple 
{ 
    public: 
        // static member function 
        static void printMsg() 
        {
            cout<<"Welcome to Apple!"; 
        }
}; 

// main function 
int main() 
{ 
    // invoking a static member function 
    Apple::printMsg(); 
} 
//程序输出:
//Welcome to Apple!
~~~