#pragma one 
#ifndef RESTRICTIONS_H
#define RESTRICTIONS_H

#include <iostream>
#include <vector>
#include <variant>
#include "objective_function.h"
#include "linear_equation.h"
#include "set_linear_eq.h"
//#include "fractional_num.h"
#include <boost/ratio/detail/mpl/abs.hpp>

/// @brief класс, который хранит ограничения(таблицей) для сиплекс метода  
class restrictions {
public: 
    using vec_index_variables = std::vector<std::variant<linear_equation<EQUAL>, linear_equation<LESS_EQ>, linear_equation<MORE_EQ>>>; // массив индексов 
    using container = std::vector<std::vector<rational<int64_t>>>; // множество ограничений
    using set_free_members = std::vector<rational<int64_t>>; // строка свободных членов 
    using iterator = container::iterator;  
    using const_iterator = container::const_iterator; 
    using member_iterator = set_free_members::iterator;
    using const_member_iterator = set_free_members::const_iterator;
    using vec_type = std::vector<type_eq>;
protected:
    size_t __count_column; // кол-во столбцов
    size_t __count_line;  // кол-во строк
    container linear_system; // коэффициенты     
    set_free_members vec_free_mem; //свободные члены 
    const size_t __n; // вспомогательная переменная, посволяющая получить плоский индекс (для матрицы)      
    size_t index_addtional_variables; // индекс, с которого начинаются дополнительные переменные 
    std::vector<type_eq> vec_ratio;

    // получить плоский индекс 
    size_t __to_flat_index(size_t i, size_t j) const noexcept {
        if (i > 0) {
            size_t a = __n - i + 1;
            size_t b = __n;
            return ((b - a + 1) * (b + a)) / 2 + j - (i + 1);
        } else {
            return j - (i + 1);
        }
    }

    // посчитать количество дополнительных параметров 
    size_t __calc_count_free_var(const set_linear_eq& lin_sys) const {
        size_t sum = 0;
        for(const auto& line : lin_sys) { 
            if (line.index() != static_cast<size_t>(EQUAL)) {
                ++sum;
            }
        }
        return sum;
    }

public:    
    /// @brief инициализация 
    /// @param lin_sys система линейных уравнений(и неравенств) 
    /// @param count_variables количество базовых переменных 
    /// @param __n вспомогательный индекс для получения плоского индекса 
    restrictions(const set_linear_eq& lin_sys, size_t count_variables, size_t __n):
                        __count_column(count_variables + __calc_count_free_var(lin_sys)), 
                        __count_line(lin_sys.size()), 
                        linear_system(__count_line, std::vector<rational<int64_t>>(__count_column, 0)), 
                        __n(__n), vec_free_mem(), index_addtional_variables(count_variables), vec_ratio() { 
        vec_free_mem.reserve(__count_line);
        vec_ratio.reserve(__count_line);

        // заполняется вектор свободных членов 
        for(const auto& line : lin_sys) {
            vec_ratio.push_back(static_cast<type_eq>(line.index()));
            if (line.index() == static_cast<size_t>(EQUAL)) {      
                vec_free_mem.push_back(std::get<EQUAL>(line).get_b());
            } else if (line.index() == static_cast<size_t>(LESS_EQ)) {
                vec_free_mem.push_back(std::get<LESS_EQ>(line).get_b());
            } else {
                vec_free_mem.push_back(-std::get<MORE_EQ>(line).get_b());
            }
        }

        // заполняется матрица 
        auto it = linear_system.begin();
        for(const auto& line : lin_sys) { 
            if (line.index() == static_cast<size_t>(EQUAL)) {
                for(const auto& [__i, __j] : std::get<EQUAL>(line)) {
                    (*it)[__to_flat_index(__i, __j)] = 1;
                }
            } else if (line.index() == static_cast<size_t>(LESS_EQ)) {
                for(const auto& [__i, __j] : std::get<LESS_EQ>(line)) {
                    (*it)[__to_flat_index(__i, __j)] = 1; 
                }
                (*it)[index_addtional_variables++] = 1; // доп переменная для равенства 
            } else {
                for(const auto& [__i, __j] : std::get<MORE_EQ>(line)) {
                    (*it)[__to_flat_index(__i, __j)] = -1;
                }
                (*it)[index_addtional_variables++] = 1;
            }
            ++it;
        }
    }
    /*
    explicit restrictions(const restrictions& obj_restr) : __count_column(obj_restr.__count_column), __count_line(obj_restr.__count_line), 
                            linear_system(obj_restr.linear_system), vec_free_mem(obj_restr.vec_free_mem), __n(obj_restr.__n),
                            index_addtional_variables(obj_restr.index_addtional_variables)  {}
    */
    /// @brief найти максимальный свободный член, которы меньше нуля (для симплекс метода: задача минимум)
    /// @return расстояние 
    ptrdiff_t find_max_b() {
        // найти первый отрицательный элемент 
        auto __iter = std::find_if(std::begin(vec_free_mem), std::end(vec_free_mem), [](const auto el){
            return el < 0;
        });
        // если такой отсутвует, то выход
        if (__iter == vec_free_mem.end()) {
            return std::numeric_limits<ptrdiff_t>::max();
        }
        // иначе ищем самый большой по модулую отрицательный элемент
        for(auto it = __iter + 1; it != vec_free_mem.end(); ++it) {
            if (*it < 0 && boost::abs(*it) > boost::abs(*__iter)) {
                __iter = it;
            }
        }
        return std::distance(vec_free_mem.begin(), __iter);
    }
    
    void add_new_restriction_less(std::vector<rational<int64_t>>&& vec_value, rational<int64_t> b) {
        ++__count_column;
        ++__count_line;
        for(auto & line : linear_system) { // ресайзим по столбцам (+1 доп столбец) 
            line.push_back(0);
        }

        vec_free_mem.push_back(b);

        vec_value.push_back(0);
        vec_value[index_addtional_variables++] = 1;
        linear_system.emplace_back(std::move(vec_value));        
    }

    void add_new_restricton(const linear_equation<MORE_EQ>& lin_restr) {
        ++__count_column;
        ++__count_line;
        for(auto & line : linear_system) { // ресайзим по столбцам (+1 доп столбец) 
            line.push_back(0);
        }
        // добавляем новую строку для нашего ограничения 
        linear_system.emplace_back(__count_column, 0); 
        auto& back_line = linear_system.back(); 
        vec_free_mem.push_back(-lin_restr.get_b());
        for(const auto& [__i, __j] : lin_restr) {
            back_line[__to_flat_index(__i, __j)] = -1; 
        }
        back_line[index_addtional_variables++] = 1;
    }

    void add_new_restricton(const linear_equation<LESS_EQ>& lin_restr) {
        ++__count_column;
        ++__count_line;
        for(auto & line : linear_system) { // ресайзим по столбцам (+1 доп столбец) 
            line.push_back(0);
        }
        // добавляем новую строку для нашего ограничения 
        linear_system.emplace_back(__count_column, 0); 
        auto& back_line = linear_system.back(); 
        vec_free_mem.emplace_back(lin_restr.get_b());
        for(const auto& [__i, __j] : lin_restr) {
            back_line[__to_flat_index(__i, __j)] = 1; 
        }
        back_line[index_addtional_variables++] = 1;
    } 

    void add_new_restricton(const linear_equation<EQUAL>& lin_restr) {
        ++__count_column;
        ++__count_line;
        for(auto & line : linear_system) { // ресайзим по столбцам (+1 доп столбец) 
            line.push_back(0);
        }
        // добавляем новую строку для нашего ограничения 
        linear_system.emplace_back(__count_column, 0); 
        auto& back_line = linear_system.back(); 
        vec_free_mem.emplace_back(lin_restr.get_b());
        for(const auto& [__i, __j] : lin_restr) {
            back_line[__to_flat_index(__i, __j)] = 1; 
        }
    }

    type_eq get_type(size_t i) const {
        return vec_ratio[i];
    }

    size_t get_index_free_var() const noexcept {
        return index_addtional_variables;
    }

    /// @brief получить ссылку на массив свободных членов 
    /// @return 
    set_free_members& get_members() noexcept {
        return vec_free_mem;
    }

    std::vector<rational<int64_t>>& back() noexcept {
        return linear_system.back();
    }


    std::vector<rational<int64_t>>& operator[](size_t i) {
        return linear_system[i];
    }

    const std::vector<rational<int64_t>>& operator[](size_t i) const {
        return linear_system[i];
    }

    iterator begin() noexcept {
        return linear_system.begin();
    }

    const_iterator begin() const noexcept {
        return linear_system.begin();
    }

    const_iterator end() const noexcept {
        return linear_system.end();
    }

    const_iterator cbegin() const noexcept {
        return linear_system.cbegin();
    }

    iterator end() noexcept {
        return linear_system.end();
    }

    const_iterator cend() const noexcept {
        return linear_system.cend();
    }

    size_t size() const noexcept {
        return linear_system.size();
    }

    const_member_iterator free_begin() const noexcept {
        return vec_free_mem.begin();
    }
    
    const_member_iterator free_end() const noexcept {
        return vec_free_mem.end();
    }

    member_iterator free_begin() noexcept {
        return vec_free_mem.begin();
    }
    
    member_iterator free_end() noexcept {
        return vec_free_mem.end();
    }

    container& get_coeff() noexcept { return linear_system; }

    set_free_members& get_free_members() noexcept { return vec_free_mem; }
    
    const set_free_members& get_free_members() const noexcept { return vec_free_mem; }

    ~restrictions() {}
};


std::ostream& operator<<(std::ostream&os, const restrictions& restr) {
    auto it = restr.free_begin();
    for(const auto& line : restr) {
        for(const auto el : line) {
            os.width(1);
            os << el << " ";
        }
        os.width(1);
        os << *it << std::endl;
        ++it;
    }
    return os;
}


#endif
