#pragma once

#ifndef OBJECTIVE_FUNCTION_H
#define OBJECTIVE_FUNCTION_H

#include <iostream>
#include <vector>
#include <algorithm>
#include "variables.h"

#include "C:\univer\gradwork\input_file.hpp"
/*
struct __elem {
    bool x;
    size_t c;   
};
*/



class objective_function {
    using value_type = std::pair<const bool*, size_t>;
    // контейнер, который хранит ссылку  на переменную и коэфф 
    using container = std::vector<value_type>;
    using const_container = const container;
    using iterator = container::iterator;
    using const_iterator = container::const_iterator;

protected:
    const_container __seq;
    size_t __n;

    container __create_objective_function(const matrix_dist& mx_dist, const bool_variables& x) {
        container res;
        const size_t N = mx_dist.size();
        auto curr_it = x.cbegin();
        for(size_t i = 0; i < N; ++i) {
            for(size_t j = i + 1; j < N; ++j) {
                res.push_back(value_type{&curr_it->value, mx_dist[i][j]});
            }
        }
        return res;
    }

public:

    objective_function(const matrix_dist& mx_dist, const bool_variables& x) : __seq(__create_objective_function(mx_dist, x)), 
                                                                                __n(mx_dist.size() - 1) {}

    ~objective_function() {}   

    size_t calc() const {
        size_t result = 0;
        for(const auto& [c_it, coeff] : __seq) {
            result += *c_it * coeff;
        }
        return result;    
    }

    const value_type& operator[](size_t i) const {
        return __seq[i];
    }

    const value_type& item(size_t i, size_t j) const noexcept {
        if (i > 0) {
            size_t a = __n - i + 1;
            size_t b = __n;
            return __seq[((b - a + 1) * (b + a)) / 2 + j - (i + 1)];
        } else {
            return __seq[j - (i + 1)];
        }
    }


    size_t size() const noexcept {
        return __seq.size();
    }   

    const_iterator begin() const noexcept {
        return __seq.begin();
    }

    const_iterator end() const noexcept {
        return __seq.end();
    }

};


std::ostream& operator<<(std::ostream& os, const objective_function& mx) {
    os << "MINIMALIZE" << std::endl;

    const auto&[c_it, coeff] =  *mx.begin();
    std::cout << "F = " << coeff << "*" << *c_it;
    std::for_each(++mx.begin(), mx.end(), [&os](const auto&el){
        const auto& [c_it, coeff] = el;
        os << " + " << coeff << "*" << *c_it;
    });
    os << std::endl;
    os<< "result = " << mx.calc();
    return os;
}

#endif
