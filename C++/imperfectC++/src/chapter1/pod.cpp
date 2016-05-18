//
// Created by fenglin on 16/5/18.
//
#include "pod.h"
#include <iostream>
#include <type_traits>

using namespace std;

/**
 * 1. 标量类型(数值类型, 指针类型, 枚举类型)
 * 2. pod 结构体
 * 3. pod 联合体
 */

struct podStruct {
    int mem1;
    double mem2;
};

//没有纯粹的析构函数
struct noTrivialDestructor {
public:
    ~noTrivialDestructor(){
        cout << "noTrivialDestructor" << endl;
    }
};

//没有纯粹的构造函数
struct noTrivialConstructor {
    noTrivialConstructor() {
        cout << "noTrivialConstructor" << endl;
    }
};

//没有纯粹的 copy 构造函数
struct noTrivialCopyConstructor {
    noTrivialCopyConstructor(const noTrivialCopyConstructor& object){
        cout << "copy constructor" << endl;
    }
};

//没有纯粹的赋值构造函数
struct noTrivialAssignmentConstructor {
    noTrivialAssignmentConstructor& operator = (const noTrivialAssignmentConstructor& object){
        cout << "noTrivialAssignmentConstructor";
        return *this;
    }
};

//带有私有成员的类
struct privateStruct {
private:
    int mem1;
};

struct staticPrivateStruct {
private:
    static int mem;
};

int staticPrivateStruct::mem = 0;

void testPod(){
    // int is pod
    must_be_pod<int> intTest;

    // double is pod
    must_be_pod<double> doubleTest;

    // podstruct
    must_be_pod<podStruct> podStructTest;

//    must_be_pod<noTrivialConstructor> n1;
//    must_be_pod<noTrivialCopyConstructor> n2;
    must_be_pod<noTrivialAssignmentConstructor> n3;
//    must_be_pod<noTrivialDestructor> n4;
    must_be_pod<privateStruct> n5;
    must_be_pod<staticPrivateStruct> n6;

    //使用 is_pod 也做判断

    if(std::is_pod<int>::value){
        cout << "int is pod" << endl;
    }else{
        cout << "int is not pod" << endl;
    }

    if(std::is_pod<double>::value){
        cout << "double is pod" << endl;
    }else{
        cout << "double is not pod" << endl;
    }

    if(std::is_pod<podStruct>::value){
        cout << "podStruct is pod" << endl;
    }else{
        cout << "podStruct is not pod" << endl;
    }

    if(std::is_pod<noTrivialConstructor>::value){
        cout << "noTrivialConstructor is pod" << endl;
    }else{
        cout << "noTrivialConstructor is not pod" << endl;
    }

    if(std::is_pod<noTrivialAssignmentConstructor>::value){
        cout << "noTrivialAssignmentConstructor is pod" << endl;
    }else{
        cout << "noTrivialAssignmentConstructor is not pod" << endl;
    }

    if(std::is_pod<privateStruct>::value){
        cout << "privateStruct is pod" << endl;
    }else{
        cout << "privateStruct is not pod" << endl;
    }

    if(std::is_pod<staticPrivateStruct>::value){
        cout << "staticPrivateStruct is pod" << endl;
    }else{
        cout << "staticPrivateStruct is not pod" << endl;
    }
}
