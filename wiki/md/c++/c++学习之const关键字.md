---
title: c++学习之const关键字
date: 2020-10-14 09:29:00
categories: c++
index_img: /img/articles/c++.jpg
tags:
	- c++
	- const
---

本文对c++中的const关键字的使用方式进行总结。

<!-- more -->

## 含义

关键字const是常量类型的修饰符，常量类型的变量或者对象的值是不能被修改更新的。

## 作用

使用const关键字可以有以下的作用：

1. 可以定义常量；
2. 相比#define定义的数据类型只是替换，const类型数据可以借助编译器进行类型检查；
3. 防止变量或者对象被修改，增加程序的健壮性；
4. 可以节省内存空间，避免不必要的内存分配。

### const数据与宏定义的区别

由const定义的常量，与#define宏定义的常量，是有区别的。

由const定义的常量，编译器可以对其常量类型是否符合类型定义进行检查，而#define宏定义是没有数据类型的，在编译时，编译器只是执行简单的字符串替换，不能进行类型安全检查。

const定义的常量，从汇编角度看，只是给出了对应的内存地址，而不是像#define宏定义给出的是立即数。因此，const定义的常量在程序运行期间，只有一份拷贝，而#define宏定义定义的常量在内存中存在多份拷贝。

### const对象默认为文件局部变量

对于未被const修饰的变量，默认为extern，比如下例子：

文件1：

~~~c++
int ext;	//未被const修饰，默认为extern
~~~

文件2：

~~~c++
#include <iostream>
extern int ext;		//通过extern，其实引用的是文件1中的变量
int main()
{
    std::cout << ext << std::endl;
}
~~~

而被const修饰的变量，默认为文件局部变量，因此，要使得const变量能够被其他的cpp文件访问到，在定义const变量时，必须显式地指出该const变量为extern。如下例子：

文件1：

~~~c++
//const变量若需要被其他文件访问到，需显式声明extern
//const变量在定义后就不能修改，因此需定义时就赋予初始值
extern const int ext = 12;	
~~~

文件2：

~~~c++
#include <iostream>
extern const int ext;		//通过extern，其实引用的是文件1中的变量
int main()
{
    std::cout << ext << std::endl;
}
~~~

## 应用

const关键字可以应用的地方包括：

1. 定义常量
2. 定义指针或者指针指向的对象
3. 函数中使用const
4. 类中使用const

### 定义常量

使用const定义常量时，需注意：

1. 定义完常量值后，不可更改
2. 常量在定义后就不能修改，因此，必须在定义时就初始化

比如：

~~~c++
const int num = 10;
num = 12; //error: const常量在定义后，就不能被修改

const int i,j = 0; //error: const常量定义时就必须初始化，这里i未初始化
~~~

### 指针与const

与const相关的指针，包括两种大类：

1. 指向const对象的指针，即const修饰指针所指向的对象，指针本身不是常量，可修改；
2. 执行对象的const指针，即const修饰的是指针本身，指针本身是常量，不可修改。

对于如何区分const与指针的关系，总结为：

1. 如果const位于指针符号（*）的左侧，则const修饰的是指针所指的对象；
2. 如果const位于指针符号（*）的右侧，则const修饰的是指针本身，即指针为常量。

比如：

~~~c++
const char * a;  //const修饰的是指针指向的对象，对象为常量，指针非常量
char const * a;  //同上

char * const a;  //const修饰的是指针本身，即指针为常量

const char * const a;  //指向const对象的const指针，即指针与指向的对象均为常量
~~~

对于指向常量的指针，比如：

~~~c++
const int * ptr;
*ptr = 10;  //error: 指针指向的对象为常量，不可通过指针修改对象
~~~

ptr是一个指向int类型const对象的指针，const修饰的是int类型的对象，即ptr所指向的对象，而不是ptr本身。因此，ptr在定义时可以不初始化。不能通过ptr修改int类型对象，因为int类型对象被const修饰。

另外需注意的点：

1. 不能使用 void * 指针保存const对象的地址，必须使用 const void * 类型的指针保存const对象的地址；
2. 允许将非const对象的地址，赋给指向const对象的指针，但是不能通过指针修改对象的值，必须通过其他方式修改。

举例：

~~~c++
const int p = 10;
const void * vp = &p;
void * vp = &p;  //error: 必须使用const void * 类型的指针保存const对象的地址

const int *ptr;
int val = 3;
ptr = &val;  //ok: 允许将非const对象地址赋予指向const对象的指针
*ptr = 4;  //error: 不能通过ptr来修改val的值，即使val为非const对象
~~~

对于const修饰的指针，即常量指针，在定义时必须初始化，且后续常量指针不能被修改。比如：

~~~c++
int num = 0;
int * const ptr = &num;  //ok: ptr为const指针，定义时必须被初始化
int b = 2;
ptr = &b;  //error: ptr为const指针，不能被修改
~~~

### 函数中使用const

在函数中使用const关键字，主要在两处：

1. const修饰函数的返回值
2. const修饰函数的参数

对于const修饰函数的返回值，与const修饰普通变量与指针的含义相同，比如：

~~~c++
const int func();  //合法，但是无意义
const int * func();  //返回的指针指向的对象为常量
int * const func();  //返回的指针为常量
~~~

对于const修饰函数的参数，一些例子为：

~~~c++
//表示函数的形参var为常量，但是无意义，因为var为形参，在函数内不会改变
void func(const int var);

//表示函数的入参指针ptr为常量，但是无意义
void func(int * const ptr);

//表示函数的入参src指向的字符串不可变
//因此函数体内若试图修改src的内容，则编译器将报错
void func(char * dst, const char * src);

//效率较低，函数体内将产生A类型的临时对象复制参数a
//临时对象的构造，复制，析构等过程将消耗资源
void func(A a);

//为了提高效率，引用传递，不会产生临时对象
//但是函数体内可通过引用改变a的值
void func(A & a);

//增加const修饰，此时依然是引用传递，但是函数体内无法修改a的值
void func(const A &a);
~~~

### 类中使用const

在设计类时，一个原则是，对于不改变数据成员的成员函数，都要在函数后面加上const，而对于改变数据成员的成员函数则不加const。

类的成员函数后面加const，表明这个函数不会对这个类对象的数据成员做任何的改变操作。所以，有const修饰的成员函数只能读取数据成员，不能改变数据成员，而没有const修饰的成员函数对数据成员是可读可写的。

在一个类中，任何不会修改数据成员的函数，都应该声明为const类型。

如果在编写const成员函数时，不慎修改了数据成员，或者调用了其他非const成员函数，编译器将报错。这样可以及时发现程序的错误，提高程序的健壮性。

准确地说，成员函数中的const，修饰的是指向对象的this指针。例如：

~~~c++
class A
{
public:
    func(int);
};
~~~

注意，这里的func函数其实具有两个参数，一个是int类型的入参，另一个是隐含的 `A * const this`。

如果不想让函数func修改参数的值，可以将函数原型修改为`func(const int)`。但是如果不运行函数修改this指针指向的对象呢？

因为this指针是隐含的参数，无法使用const直接修饰this，因此可以将const加在函数的后面，即`func(int) const`，这样，this指令就被修饰为`const A * const this`。通过这样的修饰，函数func就无法修改类对象的成员数据了，如果试图修改，则编译器将会报错。

下面是一个例子：

~~~c++
#include <iostream>
#include <string>

using namespace std;

class Student
{
public:
    Student(string str=NULL, double sco=0.0);
    void set_student(string str, double sco);
    string get_name() const;
    double get_score() const;
    void display();
    void display() const;
private:
    string name;
    double score;
};

//构造函数
Student::Student(string str, doubel sco)
{
    name = str;
    score = sco;
}

//修改成员数据，必须为非const
void Student::set_student(string str, doubel sco)
{
    name = str;
    score = sco;
}

//不修改成员数据，可使用const修饰
string Student::get_name() const
{
    return name;
}

//不修改成员数据，可使用const修饰
double Student::get_score() const
{
    return score;
}

void Student::display()
{
    cout << "调用非const: " << endl;
    cout << "name: " << name << ", score: " << score << endl;
}

//const成员函数可以重载非const成员函数
void Student::display() const
{
    cout << "调用const: " << endl;
    cout << "name: " << name << ", score: " << score << endl;
}

//外部函数
//如果对象非cons型，则不管get_name()与get_socre()函数是否为const成员函数，都可以调用
//如果对象为const型，则get_name()与get_socre()函数必须为const成员函数 
void display(const Student &stu)                     
{
	cout<<"外部函数:"<<endl;
	cout<<"name:"<<stu.get_name()<<",score:"<<stu.get_score()<<endl;
}

int main()
{
	Student stu1("lanzhihui",89.1);
    //如果存在非const型的成员函数，则调用非const型的成员函数
    //如果只存在const型的成员函数，则调用const型的成员函数
	stu1.display();
 
	stu1.set_student("wangdan",19.9);
	stu1.display();
 
	const Student stu2("wangqian",19.3);
    //必须调用const型的成员函数
	stu2.display();
 
	display(stu1);
	display(stu2);
 
	system("pause");
	return 0;
}
~~~

总结：

1. const对象只能访问const成员函数，而非const对象可以访问任意的成员函数，包括const成员函数；
2. const对象的成员是不可修改的，但是const对象通过指针维护的成员却是可以修改的；
3. const成员函数不可以修改对象的成员数据，不管对象是否为const；
4. 作为一个良好的编程风格，在声明成员函数时，若该成员函数并不对成员数据进行修改，则应该尽可能地将该成员函数使用const修饰；
5. 使用mutable修饰的成员数据，在任何情况下都可以被修改，此时const成员函数是可以修改mutable修饰的成员数据。