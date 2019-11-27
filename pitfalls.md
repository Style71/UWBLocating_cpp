# C++编程中遇到的问题及知识点总结

## 1. 类静态成员变量的初始化
```cpp
class SDLogWriter : public std::ofstream
{
private:
    static bool isInit;
    static list<SDLogWriter*> logArray;

    ...
}
```

```cpp
list<SDLogWriter*> SDLogWriter::logArray;
bool SDLogWriter::isInit = false;
```

## 2. 派生类构造函数调用基类构造函数的方式

## 3. endl的特性
```cpp
endl = putc('\n') + flush()
```

## 4. C++全局变量的声明和定义
### (1) 编译单元（模块）
在VC或VS上编写完代码，点击编译按钮准备生成exe文件时，编译器做了两步工作：
- 第一步，将每个.cpp(.c)和相应的.h文件编译成obj文件；
- 第二步，将工程中所有的obj文件进行LINK，生成最终.exe文件。

那么，错误可能在两个地方产生：
- 编译时的错误，这个主要是语法错误；
- 链接时的错误，主要是重复定义变量等。
    
编译单元指在编译阶段生成的每个obj文件。一个obj文件就是一个编译单元；一个.cpp(.c)和它相应的.h文件共同组成了一个编译单元；一个工程由很多编译单元组成，每个obj文件里包含了变量存储的相对地址等。

### (2) 声明与定义
函数或变量在`声明`时，并没有给它实际的物理内存空间，它有时候可保证你的程序编译通过；函数或变量在`定义`时，它就在内存中有了实际的物理空间。

如果你在编译单元中引用的外部变量没有在整个工程中任何一个地方定义的话，那么即使它在编译时可以通过，在连接时也会报错，因为程序在内存中找不到这个变量。

函数或变量可以`声明`多次，但`定义`只能有一次。

### (3) extern作用
- 作用一：当它与"C"一起连用时，如`extern "C" void fun(int a, int b);`，则编译器在编译`fun`这个函数名时按C的规则去翻译相应的函数名而不是C++的。
- 作用二：当它不与"C"在一起修饰变量或函数时，如在头文件中，`extern int g_nNum;`，它的作用就是`声明`函数或变量的作用范围的关键字，其声明的函数和变量可以在本编译单元或其他编译单元中使用。
 
即B编译单元要引用A编译单元中定义的全局变量或函数时，B编译单元只要包含A编译单元的头文件即可，在编译阶段，B编译单元虽然找不到该函数或变量，但它不会报错，它会在链接时从A编译单元生成的目标代码中找到此函数。
 
### (4) 全局变量(extern)
有两个类都需要使用共同的变量，我们将这些变量定义为全局变量。比如，res.h和res.cpp分别来声明和定义全局变量，类`ProducerThread`和`ConsumerThread`来使用全局变量。（以下是QT工程代码）
```cpp
/**********res.h声明全局变量************/
#pragma once

#include <QSemaphore>

const int g_nDataSize = 1000; // 生产者生产的总数据量
const int g_nBufferSize = 500; // 环形缓冲区的大小

extern char g_szBuffer[]; // 环形缓冲区
extern QSemaphore g_qsemFreeBytes; // 控制环形缓冲区的空闲区（指生产者还没填充数据的区域，或者消费者已经读取过的区域）
extern QSemaphore g_qsemUsedBytes; // 控制环形缓冲区中的使用区（指生产者已填充数据，但消费者没有读取的区域）
/**************************/
```
上述代码中`g_nDataSize`、`g_nBufferSize`为全局常量，其他为全局变量。
```cpp
/**********res.cpp定义全局变量************/
#pragma once
#include "res.h"

// 定义全局变量
char g_szBuffer[g_nBufferSize];
QSemaphore g_qsemFreeBytes(g_nBufferSize);
QSemaphore g_qsemUsedBytes;
/**************************/
```
在其他编译单元中使用全局变量时只要包含其所在头文件即可。
```cpp
/**********类ConsumerThread使用全局变量************/
#include "consumerthread.h"
#include "res.h"
#include <QDebug>

ConsumerThread::ConsumerThread(QObject* parent)
    : QThread(parent) {

}

ConsumerThread::ConsumerThread() {

}

ConsumerThread::~ConsumerThread() {

}

void ConsumerThread::run() {
     for (int i = 0; i < g_nDataSize; i++) {
          g_qsemUsedBytes.acquire();              
          qDebug()<<"Consumer "<<g_szBuffer[i % g_nBufferSize];
          g_szBuffer[i % g_nBufferSize] = ' ';
          g_qsemFreeBytes.release();
         
     }
     qDebug()<<"&&Consumer Over";
}
/**************************/
```
也可以把全局变量的声明和定义放在一起，这样可以防止忘记了定义，如上面的`extern char g_szBuffer[g_nBufferSize]`; 然后把引用它的文件中的`#include "res.h"`换成`extern char g_szBuffer[];`。

但是这样做很不好，因为你无法使用`#include "res.h"`（使用它，若达到两次及以上，就出现重定义错误；注：即使在`res.h`中加`#pragma once`，或`#ifndef`也会出现重复定义，因为每个编译单元是单独的，都会对它各自进行定义），那么`res.h`声明的其他函数或变量，你也就无法使用了，除非也都用`extern`修饰，这样太麻烦，所以还是推荐使用`.h`中声明，`.cpp`中定义的做法。

### (5) 静态全局变量(static)
注意使用static修饰变量，就不能使用extern来修饰，即static和extern不可同时出现。

static修饰的全局变量的声明与定义同时进行，即当你在头文件中使用static声明了全局变量，同时它也被定义了。

static修饰的全局变量的作用域只能是本身的编译单元。在其他编译单元使用它时，只是简单的把其值复制给了其他编译单元，其他编译单元会另外开个内存保存它，在其他编译单元对它的修改并不影响本身在定义时的值。即在其他编译单元A使用它时，它所在的物理地址，和其他编译单元B使用它时，它所在的物理地址不一样，A和B对它所做的修改都不能传递给对方。

多个地方引用静态全局变量所在的头文件，不会出现重定义错误，因为在每个编译单元都对它开辟了额外的空间进行存储。

以下是Windows控制台应用程序代码示例：
```cpp
/***********res.h**********/
static char g_szBuffer[6] = "12345";
void fun();
/************************/

/***********res.cpp**********/
#include "res.h"
#include <iostream>
using namespace std;

void fun() {
     for (int i = 0; i < 6; i++) {
          g_szBuffer[i] = 'A' + i;
     }
     cout<<g_szBuffer<<endl;
}
/************************/

/***********test1.h**********/
void fun1();
/************************/

/***********test1.cpp**********/
#include "test1.h"
#include "res.h"
#include <iostream>
using namespace std;

void fun1() {
    fun();

     for (int i = 0; i < 6; i++) {
          g_szBuffer[i] = 'a' + i;
     }
     cout<<g_szBuffer<<endl;
}
/************************/

/***********test2.h**********/
void fun2();
/************************/

/***********test2.cpp**********/
#include "test2.h"
#include "res.h"
#include <iostream>
using namespace std;

void fun2() {
     cout<<g_szBuffer<<endl;
}
/************************/

/***********main.cpp**********/
#include "test1.h"
#include "test2.h"

int main() {
     fun1();
     fun2();

     system("PAUSE");
     return 0;
}
/************************/
```
运行结果如下：

按我们的直观印象，认为`fun1()`和`fun2()`输出的结果都为abcdef，可实际上`fun2()`输出的确是初始值。然后我们再跟踪调试，发现`res`、`test1`、`test2`中`g_szBuffer`的地址都不一样，分别为0x0041a020、0x0041a084、0x0041a040，这就解释了为什么不一样。

  > 注：一般定义 static 全局变量时，都把它放在.cpp文件中而不是.h文件中，这样就不会给其他编译单元造成不必要的信息污染。
 
### (6) 全局常量(const)
`const`单独使用时，其特性与`static`一样（每个编译单元中地址都不一样，不过因为是常量，也不能修改，所以就没有多大关系）。`const`与`extern`一起使用时，其特性与`extern`一样。
```cpp
extern const char g_szBuffer[];      //写入 .h中
const char g_szBuffer[] = "123456"; // 写入.cpp中
```

## 5. C++中`enum`的特性

## 6. C++中函数默认实参的声明

## 7. C/C++结构体字节对齐与`<cstddef>`中`offsetof`的使用

## 7. 成员函数中的静态变量
类的**成员函数中的静态变量**与类的**静态成员变量**具有同样的特性，当同一个类的两个对象调用这个包含静态变量的成员函数时，不同对象访问的是同一个静态变量。示例代码如下：
```cpp
#include <iostream>
using std::cout;

class test
{
private:
    int num;
public:
    test(/* args */);
    ~test();

    int increase();
    int getNum();
};

test::test(/* args */)
{
    num=0;
}

test::~test()
{
}

int test::increase(/* args */)
{
    static int invokeTimes=0;

    num++;
    invokeTimes++;

    return invokeTimes;
}

int test::getNum(/* args */)
{
    return num;
}

int main(void)
{
    test a,b;
    int invoketimes;
    a.increase();
    b.increase();
    invoketimes=b.increase();
    cout<<"a = "<<a.getNum()<<", b = "<<b.getNum()<<", function increase() is invoked for "<<invoketimes<<" times.\n";

    return 0;
}
```
程序输出：
```
a = 1, b = 2, function increase() is invoked for 3 times.
```