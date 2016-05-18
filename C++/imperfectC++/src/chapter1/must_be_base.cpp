//
// Created by fenglin on 16/5/18.
//
#include "must_be_base.h"
#include <iostream>

using namespace std;

class Base{
public:
    Base(){
        cout << "base" << endl;
    }
};

class Derive : public Base {
public:
    Derive(){
        cout << "derive";
    }
};

void testBase(){
    must_be_base<Base, Derive> test;
}


