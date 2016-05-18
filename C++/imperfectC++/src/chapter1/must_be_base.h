//
// Created by fenglin on 16/5/17.
//
#pragma once
template <typename B, typename D>
struct must_be_base{
    ~must_be_base(){
        void(*p)(B*, D*) = constraints;
    }
private:
    static void constraints(B* b, D* d){
        b = d;
    }
};

