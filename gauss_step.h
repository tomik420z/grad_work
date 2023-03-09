#pragma once 
#ifndef GAUSS_STEP_H
#define GAUSS_STEP_H
#include <boost/rational.hpp>
#include <boost/ratio.hpp>
#include "parallel_algorithm.h"

template<typename type_table>
class gauss_step {
protected:
    static void convert_line(type_table& table, std::vector<rational<int64_t>>& line, size_t __i, size_t __j) {
        const rational<int64_t> frac = line[__j];
        parallel_for_each(std::begin(line), std::end(line), [&frac](auto&el) {
            el /= frac;
        });
        
        table.get_free_members()[__i] /= frac;
    }
    
    /// @brief подфункция метода гаусса  (вычитание)
    /// @param main_line вычитаемая строка 
    /// @param sub_line уменьшаемая строка  
    /// @param __iter итератор на соответтсвенный элемент свободного члена 
    /// @param __i индекс строки главного элемента 
    /// @param __j индекс столбца главного элемента 
    static void substract(type_table& table, const std::vector<rational<int64_t>>& main_line, std::vector<rational<int64_t>>& sub_line, typename type_table::member_iterator __iter, size_t __i, size_t __j) {
        const rational<int64_t> factor = sub_line[__j];
        parallel_accumulate(main_line, sub_line, [&factor](const auto& el){
            return - el * factor;
        });
        *__iter -= (table.get_free_members()[__i] * factor);
    }
public:
    // вычитаемая строка, индекс строки, индекс столбца  
    static void step(type_table& table, std::vector<rational<int64_t>>& main_line, size_t __i, size_t __j) {
        convert_line(table, main_line, __i, __j);
        auto it = table.free_begin();
        for(auto& line : table) {
            if (&main_line != &line && line[__j] != 0) {
                substract(table, main_line, line, it, __i, __j);    
            }
            ++it;
        }
    }

};

#endif