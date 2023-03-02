#pragma once 
#ifndef VARIABLES_H
#define VARIABLES_H
#include <iostream>
#include <vector>
#include "C:\univer\gradwork\input_file.hpp"


// базовые переменные 
class bool_variables {
public:
    struct node_variable {
        bool value;
        size_t __i;
        size_t __j;
    };  

    

    using value_type = struct node_variable;
    using container = std::vector<node_variable>;
    using iterator = container::iterator;
    using const_iterator = container::const_iterator;
protected:
    size_t sz;
    container vec_var; 

    container __create() const {
        container vec_res;
        vec_res.reserve((sz * (sz - 1)) / 2);

        for(size_t i = 0; i < sz; ++i) {
            for(size_t j = i + 1; j < sz; ++j) {
                vec_res.push_back(node_variable{false, i, j});
            }
        }    
        
        return vec_res;
        
    }

public:
    bool_variables(size_t sz) : sz(sz), vec_var(__create()) {}
    
    ~bool_variables() {}
    
    const bool& operator[](size_t i) const {
        return vec_var[i].value;
    }
    
   
    bool& operator[](size_t i) {
        return vec_var[i].value;
    }
    
    size_t size() const noexcept {
        return vec_var.size();
    }
    
    iterator begin() noexcept {
        return vec_var.begin();
    }

    iterator end() noexcept {
        return vec_var.end();
    }

    const_iterator begin() const noexcept {
        return vec_var.begin();
    }

    const_iterator end() const  noexcept {
        return vec_var.end();
    }

    const_iterator cbegin() const noexcept {
        return vec_var.cbegin();
    }

    const_iterator cend() const noexcept {
        return vec_var.cend();
    }

    std::pair<size_t, size_t> get_coord(size_t i) const {
        const auto& [v, __i, __j] = vec_var[i];
        return std::pair<size_t, size_t>{__i, __j};
    } 
    
};


std::ostream& operator<<(std::ostream& os, const bool_variables& vars) {
    for(const auto& [b, __i, __j] : vars) {
        os << "(" << b << ", " << __i << ", " << __j << ") ";
    }
    return os;
}

#endif