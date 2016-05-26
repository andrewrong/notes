#include <type_traits>
#include <iostream>

template <typename T>
struct must_be_pod{
    ~must_be_pod(){
        void(*p)() = constraints;
    }

    static void constraints(){
        union{
            T T_is_not_Pod_Type;
        };
    }
};


