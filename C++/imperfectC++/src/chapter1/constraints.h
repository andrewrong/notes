//
// Created by fenglin on 16/5/18.
//
#pragma once
#include "pod.h"

#define CONSTRAINT_MUST_BE_POD(T) \
    static_assert(sizeof(must_be_pod<T>::constraints()) != 0))
