#ifndef CLASS_TIME_H
#define CLASS_TIME_H
#pragma once 

#include "file_include.hpp"
class execution_time {
protected:
    clock_t __start;
    clock_t __finish;
public:
    execution_time() : __start(), __finish()  {}
    
    void start() noexcept {
        __start = clock();
    }
    
    void finish() noexcept {
        __finish = clock();
    }

    clock_t get_time() const noexcept {
        return __finish - __start;
    }

    ~execution_time() {}
};

#endif