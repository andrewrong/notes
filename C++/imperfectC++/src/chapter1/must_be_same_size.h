//
// Created by fenglin on 16/5/18.
//
#pragma once
template <typename T>
struct size_of {
    enum{
        value = sizeof(T)
    };
};

template <>
struct size_of<void>{
    enum{
        value = 0
    };
};

template <typename T1, typename T2>
struct must_be_same_size {
    ~must_be_same_size(){
        void(*p)() = constraints;
    }

private:
    static void constraints(){
        const int T1_must_be_same_size_T2 = size_of<T1>::value == size_of<T2>::value ? 1 : -1;
        int i[T1_must_be_same_size_T2];
    }
};
