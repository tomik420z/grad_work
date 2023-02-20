#pragma once 
#ifndef LINEAR_EQUATION_H
#define LINEAR_EQUATION_H
#include <iostream>
#include <vector> 
#include "objective_function.h"
#include <algorithm>
#include <limits>
#include "fractional_num.h"

enum type_eq {
    EQUAL, 
    LESS_EQ,
    MORE_EQ
};

/// @brief класс, описывающий линейное уравнение или неравенство, хранит координаты вершин  
/// @tparam ratio - шаблонный параметр, отвечающий за знак неравенства или равенства  
template<type_eq ratio> 
class linear_equation
{
public:
    using index_variables = std::vector<std::pair<size_t, size_t>>;
    using iterator = index_variables::iterator;
    using const_iterator = index_variables::const_iterator;
protected:
    // уравнение оп координатам 
    index_variables lin_eq;
    // свободный член 
    fractional_num b;
    
public:
    linear_equation(std::initializer_list<std::pair<size_t, size_t>>&& init_list, fractional_num param) : lin_eq(init_list), b(param) {}
    
    template<typename Ty_var>
    linear_equation(Ty_var&& ind_var, fractional_num param) : lin_eq(std::forward<Ty_var>(ind_var)), b(param) {}

    linear_equation(linear_equation&& __move_param) noexcept : lin_eq(std::move(__move_param.lin_eq)), b(__move_param.b) {}

    fractional_num get_b() const noexcept { return b; }

    linear_equation& operator=(linear_equation &&__move_param) noexcept {
        lin_eq = std::move(__move_param.lin_eq);
        b = __move_param.b;
        return *this;
    }

    index_variables& get_line_eq() noexcept { return lin_eq; }

    const index_variables& get_line_eq() const noexcept { return lin_eq; }
    
    constexpr type_eq get_type() { return ratio; }

    ~linear_equation() {}

    iterator begin() noexcept {
        return lin_eq.begin();
    }

    const_iterator end() noexcept {
        return lin_eq.end();
    }   

    const_iterator begin() const noexcept { 
        return lin_eq.begin();
    }

    const_iterator end() const noexcept {
        return lin_eq.end();
    }

    bool empty() const noexcept {
        return lin_eq.empty();
    }
};

template<type_eq ratio>
std::ostream& operator<<(std::ostream& os, const linear_equation<ratio>& lin_eq) {
    os << "{";
    for(const auto&[__i, __j] : lin_eq) {
        os << "(" << __i << ", " << __j << ")";
    } 
    os << "}";
    if constexpr (ratio == EQUAL) {
        os << " = ";
    } else if (ratio == LESS_EQ) {
        os << " <= ";
    } else if (ratio == MORE_EQ) {
        os << " >= ";
    }

    os << lin_eq.get_b();

    return os;
}

#endif






