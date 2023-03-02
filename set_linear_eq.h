#pragma once 
#ifndef SET_LINAER_EQ_H
#define SET_LINEAR_EQ_H

#include "linear_equation.h"
#include <vector>
#include <variant>


class set_linear_eq {
public:
    using vec_lin_eq = std::vector<std::variant<linear_equation<EQUAL>, linear_equation<LESS_EQ>, linear_equation<MORE_EQ>>>;
    using iterator = vec_lin_eq::iterator;
    using const_iterator = vec_lin_eq::const_iterator;
protected:
    vec_lin_eq vec;
public:
    set_linear_eq() : vec() {}

    set_linear_eq(set_linear_eq&& set_lin) : vec(std::move(set_lin.vec)) {}

    set_linear_eq& operator=(set_linear_eq&& set_lin) noexcept {
        vec = std::move(set_lin.vec);
        return *this;
    }

    void reserve(size_t __new_cap) {
        vec.reserve(__new_cap);
    }

    size_t size() const noexcept { return vec.size(); }

    template<type_eq ratio> 
    void add_new_restriction(linear_equation<ratio> && lin_eq) {
        vec.emplace_back(std::move(lin_eq));
    }

    decltype(auto) operator[](size_t i) {
        return vec[i];
    }

    const_iterator begin() const noexcept {
        return vec.begin();
    }

    const_iterator end() const noexcept {
        return vec.end();
    }

    iterator begin() noexcept {
        return vec.begin();
    }

    iterator end() noexcept {
        return vec.end();
    }



    ~set_linear_eq() {}
};


std::ostream& operator<<(std::ostream& os, const set_linear_eq& lin_eq) {
    for(const auto& lin : lin_eq) {
        if (lin.index() == EQUAL) {
            os << std::get<EQUAL>(lin) << std::endl;
        } else if (lin.index() == LESS_EQ) {
            os << std::get<LESS_EQ>(lin) << std::endl;
        } else {
            os << std::get<MORE_EQ>(lin) << std::endl;
        }
        
    }

    return os; 
}




#endif