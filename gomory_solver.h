#pragma once
#ifndef GOMORY_SOLVER_H
#define GOMORY_SOLVER_H


#include <iostream>
#include <vector>
#include <numeric>
#include <limits>
#include <iomanip>
#include "adjacency_list.h"
#include "restrictions.h"
#include "set_linear_eq.h"
#include <boost/multiprecision/integer.hpp>
#include <boost/rational.hpp>
#include <unordered_set>
#include "parallel_algorithm.h"
#include "gauss_step.h"
using boost::rational;

rational<int64_t> get_frac_part(const rational<int64_t>& x) {
    return rational<int64_t>(x.numerator() % x.denominator(), x.denominator());
}

bool check_integer(const rational<int64_t>& x) {
    return (x.numerator() % x.denominator()) == 0;
}

rational<int64_t> floor(const rational<int64_t>& x) {
    if (x.numerator() % x.denominator() == 0) {
        return x;
    } else {
        return rational<int64_t>((x.numerator() / x.denominator()) - 1, 1);
    }
}

/// @brief решатель для задачи линейного целочисленного программирования
class gomory_solver {
public: 

    using coeff_table_t = restrictions; 
    using obj_func_coeff_t = std::vector<int>;
    using vec_name_basis = std::vector<int>;    
    using vec_linear_eq = std::vector<std::variant<linear_equation<EQUAL>, linear_equation<LESS_EQ>, linear_equation<MORE_EQ>>>;

    static constexpr size_t absence = std::numeric_limits<int>::max(); 
protected:
    size_t index_var; //индекс новой переменной 
    const size_t N;  // размер матрицы  
    const bool_variables& x; // множество переменных 
    const size_t count_variables; // кол-во главных переменных
    objective_function obj_f; // целевая функция 
    coeff_table_t table;     // таблица ограничений 
    obj_func_coeff_t obj_func_coeff;   // коэффициенты целевой функции   
    vec_name_basis vec_name; // имена базисных переменных 
    std::vector<rational<int64_t>> vec_delta; // массив дельт  

    /// @brief инициализатор для вектора коэффициентов целевой функции 
    /// @return вектор коэф. целевой функции
    obj_func_coeff_t create_obj_func_coeff() {
        obj_func_coeff_t res(table[0].size(), 0);
        auto it = res.begin();
        for(const auto& [c_it, coeff] : obj_f) {
            *it = coeff;
            ++it;
        } 
        return res;
    }
    
    
    /// @brief инициализатор вектора базисных переменных 
    /// @return вектор базисных переменных 
    vec_name_basis create_name() {
        vec_name_basis res;
        res.reserve(table.size());
        //size_t index = count_variables; 
        for(size_t i = 0; i < table.size(); ++i) {
            if (table.get_type(i) != EQUAL) {
                res.push_back(index_var++);
            } else {
                res.push_back(absence);
            }
        }
        return res;
    }
    
    /// @brief заполнить сотлбец базисных переменных
    void __fill_basis_coeff() {
        for(size_t i = 0; i < N; ++i) {
            vec_name[i] = i;
            gauss_step<coeff_table_t>::step(table, table[i], i, i);
        }  
    }

    size_t __find_max_abs_el(size_t i) {

        const auto& line_ref = table[i];
        auto __iter = std::find_if(std::cbegin(line_ref), std::cend(line_ref), [](const auto& el){
            return el < 0;
        });

        if (__iter == line_ref.end()) {
            throw "the system has no solutions";
        }
        
        // иначе ищем самый большой по модулую отрицательный элемент
        for(auto it = __iter; it != line_ref.end(); ++it) {
            if (*it < 0 && boost::abs(*it) > boost::abs(*__iter)) {
                __iter = it;
            }
        }
        return std::distance(line_ref.begin(), __iter);
    }

    void __calc_delta() {
        parallel_for_each(vec_delta, [](auto& el){
            el = 0;
        });
        
        const size_t count_of_column = table[0].size();
        const size_t count_of_line = table.size();
        #pragma omp parallel for 
        for(size_t i = 0; i < count_of_column; ++i) {
            for(size_t j = 0; j < count_of_line; ++j) {
                vec_delta[i] += (table[j][i] * obj_func_coeff[vec_name[j]]);  
            }
            vec_delta[i] -= obj_func_coeff[i];
        }

        const auto& line_free_members = table.get_free_members();
        #pragma omp parallel for 
        for(size_t i = 0; i < count_of_line; ++i) {
            vec_delta.back() += line_free_members[i] * obj_func_coeff[vec_name[i]];
        }
    }

    size_t find_permission_line(size_t j) {
        
        size_t index = 0;
        const auto& ref_free_member = table.get_free_members();
        rational<int64_t> val = std::numeric_limits<int64_t>::max();
        const size_t count_elem = table.size();
        


        for(size_t i = 0; i < count_elem; ++i) {
            rational<int64_t> ratio = table[i][j] > 0 ? ref_free_member[i] / table[i][j] : std::numeric_limits<int64_t>::max();   
            if (ratio < val) {
                val = ratio;
                index = i;
            }
        }
        return index;
    }
    
    /// @brief проверить решение на целочисленность 
    /// @return булев ответ true - решение целочисленное : false - иначе 
    bool check_for_integer() const noexcept {
        const auto& line_free = table.get_free_members();
        size_t __count_column = x.size();
        const size_t line_free_size = line_free.size();
        
        for(size_t i = 0; i < line_free_size; ++i) {
            //std::cout << std::fixed << std::setprecision(20) << std::modf(line_free[i], NULL) << " ";
            if(static_cast<size_t>(vec_name[i]) < __count_column) {
                if (!check_integer(line_free[i])) { // решение не целочисленное 
                    return false;
                }
            } 
        } 

        return true;
    }
    
    /// @brief найти число, имеющее наибольшую дробную часть
    /// @return индекс строки 
    size_t find_max_fractional() const noexcept {
        const auto& line_free = table.get_free_members();
        size_t max_index = table.size();
        rational<int64_t> max_val = 0;
        for(size_t i = 0; i < line_free.size(); ++i) {
            // проверяем дробную часть
            if (!check_integer(line_free[i])) {
                if (rational<int64_t> frac = get_frac_part(line_free[i]); 
                                    max_val < frac && static_cast<size_t>(vec_name[i]) < count_variables) {
                    max_val = frac;
                    max_index = i;
                }
            }
        }
        return max_index;
    }

    // ГОМОРИ метод 
    // ищем минимальное отношение 
    size_t find_premmision_column(size_t j) {
        auto& last_line = table[j];
        rational<int64_t> min_val = std::numeric_limits<int64_t>::max();
        size_t index_res = 0;
        for(size_t i = 0; i < last_line.size() - 1; ++i) {
            if (last_line[i] < 0) {
                if (rational<int64_t> del = boost::abs(vec_delta[i] / last_line[i]); del < min_val) {
                    min_val = del;
                    index_res = i;
                }
            } 
        }
        //std::cout << '\n';
        return index_res;
    }


    bool check_test() {
        const auto& vec_line = table.get_free_members();
        for(size_t i = 0; i < vec_line.size(); ++i) {
            if(static_cast<size_t>(vec_name[i]) < count_variables) {
                if (vec_line[i] < 0) {
                    return false;
                }
            }
        }
        return true;
    }

    void converting_a_table_to_canonical_form() {
        for(size_t i = table.find_max_b();  i < table.size(); i = table.find_max_b()) {
            size_t j = __find_max_abs_el(i);

            vec_name[i] = j;
            gauss_step<coeff_table_t>::step(table, table[i], i, j);
            //__gauss_step(table[i], i, j);
            //round_vec_free_members();
        }
    }

    size_t f_max() const noexcept {
        return std::distance(std::begin(vec_delta), parallel_max_element(std::begin(vec_delta), std::end(vec_delta) - 1));
    }

    void solve_simplex() {
        for (auto j = f_max(); true; j = f_max()) {
            //std::cout << *iter<< std::endl;
            //std::cout << *iter << std::endl;
            if (vec_delta[j] <= 0) {
                break;
            }
            
            size_t i = find_permission_line(j);
            vec_name[i] = j;
            gauss_step<coeff_table_t>::step(table, table[i], i, j);
            //__gauss_step(table[i], i, j);
            __calc_delta();
    
        } 
    }


public:
    // initialization 
    gomory_solver(const matrix_dist& mx, const bool_variables&x, const set_linear_eq& lin_eq) : N(mx.size()), x(x), index_var(x.size()), count_variables(x.size()), obj_f(mx, x), 
                                            table(lin_eq, count_variables, mx.size() - 1),
                                            obj_func_coeff(create_obj_func_coeff()),
                                            vec_name(create_name()),
                                            vec_delta(table[0].size() + 1, 0)
                                            {}


    

    void print_vec_name() {
        std::cout << "vec name: "; 
        for(const auto el : vec_name) {
            if (el == absence) {
                std::cout << "?" << " ";
            } else {
                std::cout << el << " ";
            }
        }
        std::cout << '\n';
    }

    void print_vec_coeff() {
        std::cout << "C: ";
        for(const auto el : obj_func_coeff) {            
            std::cout << el << " ";
        }
        
        std::cout << '\n';
    }

    void print_delta() {
        std::cout << "delta: ";
        for(const auto el : vec_delta) {            
            std::cout << el << " ";
        }
        std::cout << '\n';
    }
    
    void solve() {

        __fill_basis_coeff();

        // привести таблицу к каноническому виду(для решения симплекс методом)
        converting_a_table_to_canonical_form();

        __calc_delta();
        size_t count_deletes = 0;
        while (true) {
            // вычислить симплекс-таблицу 
            solve_simplex();

            if (check_for_integer()) {            
                std::cout << "solution is integer!!!\n";
                return; //решение целочисленное     
            }

            ++count_deletes;
                    
            std::cout << "solution is not int\n";
            const size_t index_max_frac = find_max_fractional();

            std::vector<rational<int64_t>> integer_restriction; //ограничение для целочисленности
            integer_restriction.resize(table[0].size());

            // составление и добавление нового ограничения 
            parallel_transform(table[index_max_frac], integer_restriction, [](const auto& val){
                if (!check_integer(val)) { // если число дробное  
                    if(val < 0) {
                        return -(val - floor(val));
                    } else {
                        return -get_frac_part(val);
                    }
                } else {
                    return rational<int64_t>{0};
                }
            });
                    
            rational<int64_t> b = -get_frac_part(table.get_free_members()[index_max_frac]);
                
            table.add_new_restriction_less(std::move(integer_restriction), b);
            vec_name.push_back(index_var++);
            vec_delta.push_back(0);
                
            obj_func_coeff.push_back(0);

            __calc_delta();
                
            for(size_t i = table.find_max_b();  i < table.size(); i = table.find_max_b()) {
                size_t j = find_premmision_column(i);
                
                vec_name[i] = j;
                gauss_step<coeff_table_t>::step(table, table[i], i, j);
                __calc_delta();
            }
        }
    }
    
    void add_solve() {

        converting_a_table_to_canonical_form();

        __calc_delta();
        
        while (true) {
            // вычислить симплекс-таблицу 
            solve_simplex();

            if (check_for_integer()) {
                std::cout << "solution is integer!!!\n";
                return; //решение целочисленное     
            }
                    
            std::cout << "solution is not int\n";
            const size_t index_max_frac = find_max_fractional();

            std::vector<rational<int64_t>> integer_restriction; //ограничение для целочисленности
            integer_restriction.resize(table[0].size());

                // составление и добавление нового ограничения 
            parallel_transform(table[index_max_frac], integer_restriction, [](const auto& val){
                if (!check_integer(val)) { // если число дробное  
                    if(val < 0) {
                        return -(val - floor(val));
                    } else {
                        return -get_frac_part(val);
                    }
                } else {
                    return rational<int64_t>{0};
                }
            });
                    
            rational<int64_t> b = -get_frac_part(table.get_free_members()[index_max_frac]);
                
            table.add_new_restriction_less(std::move(integer_restriction), b);
            vec_name.push_back(index_var++);
            vec_delta.push_back(0);
                
            obj_func_coeff.push_back(0);

            __calc_delta();
                
            for(size_t i = table.find_max_b();  i < table.size(); i = table.find_max_b()) {
                size_t j = find_premmision_column(i);
                
                vec_name[i] = j;
                gauss_step<coeff_table_t>::step(table, table[i], i, j);
                __calc_delta();
            }
        }
    }

    void print() const noexcept {   
        auto it = table.free_begin();
        for(const auto& line : table) {
            for(const auto& el : line) {
                std::cout.width(3);
                std::cout << el << " ";
            }
            std::cout << "|";
            std::cout.width(3);
            std::cout<< *it << std::endl;
            ++it;
        }
    }

    restrictions& get_table() noexcept { return table; }

    ~gomory_solver() {}

    std::vector<std::pair<size_t, size_t>> get_path() const {
        std::vector<std::pair<size_t, size_t>> path;
        path.reserve(N); 
        const auto& free_line = table.get_free_members();
        for(size_t i = 0; i < vec_name.size(); ++i) {
            if (free_line[i] == 1 && (vec_name[i] < static_cast<int>(count_variables))) {
                path.emplace_back(x.get_coord(vec_name[i]));
            }
        }
        return path;
    }

    size_t get_cost(const matrix_dist& mx, const std::vector<std::pair<size_t, size_t>>& path) const noexcept {
        size_t res = 0;
        for(const auto& [__i, __j] : path) {
            res += mx[__i][__j];
        }
        return res;
    }


    void add_new_restrictions(const std::vector<linear_equation<MORE_EQ>>& vec_add_restr) {
        for(const auto& line : vec_add_restr) {
            table.add_new_restricton(line);
        }
        vec_name.reserve(vec_add_restr.size());
        for(; vec_name.size() < table.size(); ) {
            vec_name.push_back(index_var++);
        }

        for(; obj_func_coeff.size() < table[0].size();) {
            obj_func_coeff.push_back(0);
            vec_delta.push_back(0);
        }
        
    }
};






#endif