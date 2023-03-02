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
using boost::rational;
//using boost::multiprecision::int64_t;

template<typename Container_from, typename Unary_func>
Unary_func parallel_for_each(Container_from& container, Unary_func __pred) {

    const size_t sz = container.size();

    #pragma omp parallel for //num_threads()
    for(size_t i = 0; i < sz; ++i) {
        __pred(container[i]);
    }

    return __pred;
}

template<typename Iter, typename Unary_func>
Unary_func parallel_for_each(Iter __first, Iter __last, Unary_func __pred) {
    static_assert(std::is_base_of_v<std::random_access_iterator_tag, typename std::iterator_traits<Iter>::iterator_category>, "iterator must be random access");    

    #pragma omp parallel for 
    for(Iter __curr = __first; __curr < __last; ++__curr) {
        __pred(*__curr);
       
    }
    return __pred;
}

template<typename Container_from, typename Container_to, typename Unary_func>
Unary_func parallel_accumulate(const Container_from& container_from,  Container_to& container_to, Unary_func __pred) {

    const size_t sz = container_from.size();

    #pragma omp parallel for //num_threads()
    for(size_t i = 0; i < sz; ++i) {
        container_to[i] += __pred(container_from[i]);
    }

    return __pred;
}

template<typename Container_from, typename Container_to, typename Unary_func>
Unary_func parallel_transform(const Container_from& container_from,  Container_to& container_to, Unary_func __pred) {

    const size_t sz = container_from.size();

    #pragma omp parallel for //num_threads()
    for(size_t i = 0; i < sz; ++i) {
        container_to[i] = __pred(container_from[i]);
    }

    return __pred;
}

template<class Iter>
Iter parallel_max_element(Iter first, Iter last)
{
    static_assert(std::is_base_of_v<std::random_access_iterator_tag, typename std::iterator_traits<Iter>::iterator_category>, "iterator must be random access");    
    
    Iter largest = first;
    ++first;
    #pragma omp declare reduction(parallel_max_element: Iter: omp_out = *omp_out > *omp_in ? omp_out : omp_in) initializer (omp_priv=(omp_orig))

    #pragma omp parallel for reduction(parallel_max_element: largest)
    for (Iter curr = first; curr < last; ++curr) {
        largest = (*largest < *curr) ? curr : largest;
    }
    return largest;
}




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

/// @brief решатель
class simplex_solver {
public: 
    
    using coeff_table_t = restrictions; 
    using obj_func_coeff_t = std::vector<int>;
    using vec_name_basis = std::vector<int>;    
    using vec_linear_eq = std::vector<std::variant<linear_equation<EQUAL>, linear_equation<LESS_EQ>, linear_equation<MORE_EQ>>>;

    static constexpr size_t absence = std::numeric_limits<int>::max(); 
    //static constexpr double eps = 0.000001;
protected:
    size_t index_var;
    const size_t N;  // размер матрицы  
    const bool_variables& x; // множество переменных 
    const size_t count_variables; // кол-во главных переменных
    objective_function obj_f; // целевая функция 
    coeff_table_t table;     // таблица ограничений 
    obj_func_coeff_t obj_func_coeff;   // коэффициенты целевой функции   
    vec_name_basis vec_name; // имена базисных переменных 
    std::vector<rational<int64_t>> vec_delta; // массив дельт  

    obj_func_coeff_t create_obj_func_coeff() {
        obj_func_coeff_t res(table[0].size(), 0);
        auto it = res.begin();
        for(const auto& [c_it, coeff] : obj_f) {
            *it = coeff;
            ++it;
        } 
        return res;
    }
    
    void convert_line(std::vector<rational<int64_t>>& line, size_t __i, size_t __j) {
        const rational<int64_t> frac = line[__j];
        parallel_for_each(std::begin(line), std::end(line), [&frac](auto&el) {
            el /= frac;
        });
        /*for(auto & el : line) {
            el /= frac;
        }*/
        table.get_free_members()[__i] /= frac;
        //std::cout << "table after conversuion:\n" << table << std::endl;
    }
    
    /// @brief подфункция метода гаусса  (вычитание)
    /// @param main_line вычитаемая строка 
    /// @param sub_line уменьшаемая строка  
    /// @param __iter итератор на соответтсвенный элемент свободного члена 
    /// @param __i индекс строки главного элемента 
    /// @param __j индекс столбца главного элемента 
    void substract(const std::vector<rational<int64_t>>& main_line, std::vector<rational<int64_t>>& sub_line, coeff_table_t::member_iterator __iter, size_t __i, size_t __j) {
        const rational<int64_t> factor = sub_line[__j];
        parallel_accumulate(main_line, sub_line, [&factor](const auto& el){
            return - el * factor;
        });
        /*
        auto it = sub_line.begin();
        for(const auto& el : main_line) {
            //std::cout<< *it << "  "<< el << "  "<< factor << std::endl;
            *it -= (el * factor);
            ++it;
        }*/
        *__iter -= (table.get_free_members()[__i] * factor);
    }

    // вычитаемая строка, индекс строки, индекс столбца  
    void gauss_step(std::vector<rational<int64_t>>& main_line, size_t __i, size_t __j) {
        convert_line(main_line, __i, __j);
        //std::cout << "+\n";
        auto it = table.free_begin();
        for(auto& line : table) {
            if (&main_line != &line && line[__j] != 0) {
                substract(main_line, line, it, __i, __j);    
            }
            ++it;
        }
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
    
    void __fill_basis_coeff() {
        for(size_t i = 0; i < N; ++i) {
            vec_name[i] = i;
            gauss_step(table[i], i, i);
        }
      /*  
        for(size_t i = N; i < table.size(); ++i) {
            if (vec_name[i] == absence) {
                auto jt = std::find_if(std::begin(table[i]), std::end(table[i]), [](const auto & el){
                    return std::abs(el) > eps;
                });
                if (jt == table[i].end()) {
                    throw "system error";
                }
                gauss_step(table[i], i, std::distance(table[i].begin(), jt));
            }
        }
        */
    }

    size_t __find_max_abs_el(size_t i) {

        const auto& line_ref = table[i];
        
        auto __iter = std::find_if(std::begin(line_ref), std::end(line_ref), [](const auto& el){
            return el < 0;
        });
        //std::cout << std::endl;

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
        //std::fill(std::begin(vec_delta), std::end(vec_delta), 0);
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
        
        //#pragma omp parallel for private(ratio)
        for(size_t i = 0; i < count_elem; ++i) {
            rational<int64_t> ratio = table[i][j] > 0 ? ref_free_member[i] / table[i][j] : std::numeric_limits<int64_t>::max(); 
           // #pragma omp critical
               
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
    /*
    void round_vec_free_members() {
        auto& line = table.get_free_members();
        std::for_each(std::begin(line), std::end(line), [](auto& el) {
            if (double frac = std::modf(el, NULL); std::abs(frac) > 0.99) {
                el = std::round(el);
            } else if (std::abs(el) < eps) {
                el = 0.0;
            }
        });
    }
    */
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



public:
    // initialization 
    simplex_solver(const matrix_dist& mx, const bool_variables&x, const set_linear_eq& lin_eq) : N(mx.size()), x(x), index_var(x.size()), count_variables(x.size()), obj_f(mx, x), 
                                            table(lin_eq, count_variables, mx.size() - 1),
                                            obj_func_coeff(create_obj_func_coeff()),
                                            vec_name(create_name()),
                                            vec_delta(table[0].size() + 1, 0)
                                            {}


    decltype(auto) f_max() {
        //auto back = vec_delta.end();
        //std::advance(back, -1);
        //return std::distance(std::begin(vec_delta), parallel_max_element(std::begin(vec_delta), back));
        rational<int64_t> max_val = vec_delta.front();
        size_t index = 0; 
        const size_t sz = vec_delta.size() - 1;
        for(size_t i = 0; i < sz; ++i) {
            
            if (max_val < vec_delta[i]) {
                max_val = vec_delta[i];
                index = i;
            }
        }
        return index;
        
        //return std::max_element(std::begin(vec_delta), std::end(vec_delta) - 1);
    }

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

        // добавить новое ограничение не меняя матрицу 
        for(size_t i = table.find_max_b();  i < table.size(); i = table.find_max_b()) {
            size_t j = __find_max_abs_el(i);
            if (j >= table[i].size()) {
                break;
            }

            vec_name[i] = j;
            gauss_step(table[i], i, j);
            //round_vec_free_members();
        }

        __calc_delta();

        while (true) {
            for (auto j = f_max(); true; j = f_max()) {
                //std::cout << *iter<< std::endl;
                //std::cout << *iter << std::endl;
                if (vec_delta[j] <= 0) {
                    break;
                }
                
                size_t i = find_permission_line(j);
                vec_name[i] = j;
                gauss_step(table[i], i, j);
                __calc_delta();
             
            } 


                //std::cout << std::boolalpha << check_test() << std::endl;
            
                    
                    //std::cout << "test = "<< check_test() << std::endl;
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
                        gauss_step(table[i], i, j);
                        __calc_delta();
                    }
                }
        }
    

       
    
    void add_solve(restrictions & table_add) {
        __fill_basis_coeff();

        while(true) {
            for(size_t i = table_add.find_max_b();  i < table_add.size(); i = table_add.find_max_b()) {
                size_t j = __find_max_abs_el(i);
                if (j >= table_add[i].size()) {
                    break;
                }

                vec_name[i] = j;
                gauss_step(table_add[i], i, j);
                //round_vec_free_members();
            }

            __calc_delta();


            for (auto j = f_max(); true; j = f_max()) {
                //std::cout << *iter<< std::endl;
                //std::cout << *iter << std::endl;
                if (vec_delta[j] <= 0) {
                    break;
                }
                
                //size_t j = std::distance(vec_delta.begin(), iter);
            
                // найти разрешающую строку
                size_t i = find_permission_line(j);
                //std::cout << i << " " << j << std::endl;
                vec_name[i] = j;
            // std::cout << i << " " << j << std::endl; 
                gauss_step(table_add[i], i, j);
                __calc_delta();
                //round_vec_free_members();
            } 

            
                if (check_for_integer()) {
                    std::cout << "solution is integer!!!\n";
                    return; //решение целочисленное     
                }
                
                
            
                std::cout << "solution is not int\n";
                const size_t index_max_frac = find_max_fractional();

                std::vector<rational<int64_t>> integer_restriction; //ограничение для целочисленности
                integer_restriction.resize(table_add[0].size());

                // составление и добавление нового ограничения 
                parallel_transform(table_add[index_max_frac], integer_restriction, [](const auto& val){
                    if (!check_integer(val)) { // если число дробное  
                        if(val < 0) {
                            return val - floor(val);
                        } else {
                            return -get_frac_part(val);
                        }
                    }
                    return rational<int64_t>{0};
                });

                rational<int64_t> b = -get_frac_part(table_add.get_free_members()[index_max_frac]);
                
                table_add.add_new_restriction_less(std::move(integer_restriction), b);
                vec_name.push_back(index_var++);
                vec_delta.push_back(0);
                //std::swap(vec_delta.back(), vec_delta[vec_delta.size() - 2]);
                obj_func_coeff.push_back(0);
            }
                //__calc_delta();
                /*
                std::for_each(std::begin(vec_delta), std::end(vec_delta), [](auto &el){
                    el = -el;
                });
            
                for(size_t i = table.find_max_b();  i < table.size(); i = table.find_max_b()) {
                    size_t j = find_premmision_column(i);
                    std::cout << i << " " << j << std::endl;
                    if (j >= table[i].size()) {
                        throw "error";
                    }

                    vec_name[i] = j;
                    gauss_step(table[i], i, j);
                    //round_vec_free_members();
                }

                __calc_delta();
                std::for_each(std::begin(vec_delta), std::end(vec_delta), [](auto &el){
                    el = -el;
                });

/*
                for (auto iter = f_max(); true; iter = f_max()) {
                    std::cout << *iter << std::endl;
                    if (*iter <= 0) {
                        break;
                    }
                    size_t j = std::distance(vec_delta.begin(), iter);
                    // найти разрешающую строку
                    size_t i = find_permission_line(j);
                    //std::cout << i << " " << j << std::endl;
                    vec_name[i] = j;
                // std::cout << i << " " << j << std::endl; 
                    gauss_step(table[i], i, j);
                    __calc_delta();
                    //round_vec_free_members();
               } 
*/
                //size_t j = find_premmision_column();
                //vec_name[index_max_frac] = j;
                //gauss_step(table.back(), table.size() - 1, j);
                
          //  } while(!check_for_integer());

            /*
            __calc_delta();
            
            std::for_each(std::begin(vec_delta), std::end(vec_delta), [](auto& el){
                if (el.check_more_zero()) {
                    el = -el;
                }
            });

            for(size_t i = table.find_max_b();  i < table.size(); i = table.find_max_b()) {
                size_t j = find_premmision_column(i);
                
                if (j >= table[i].size()) {
                    break;
                }

                vec_name[i] = j;
                gauss_step(table[i], i, j);
                break;
            }
            print_delta();
            */
           //std::cout << std::boolalpha << check_for_integer() << std::endl;
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

    ~simplex_solver() {}

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
};


linear_equation<MORE_EQ> get_new_restriction(const std::vector<std::list<size_t>> &__set, std::vector<std::list<size_t>>::const_iterator jt) {
    
    const auto& list_vertex = *jt;//__set[index];
    linear_equation<MORE_EQ>::index_variables ind_var;
    for(const auto& el_set : list_vertex) {
        for(auto it = __set.begin(); it != __set.end(); ++it) {
            if (it == jt) {
                continue;
            } 

            for(const auto& el : *it) {
                if (el_set < el) {
                    ind_var.emplace_back(el_set, el);
                } else {
                    ind_var.emplace_back(el, el_set);
                }
            }
        }
    }
    return linear_equation<MORE_EQ>(std::move(ind_var), 2);
}

std::vector<linear_equation<MORE_EQ>> get_additional_restrictions(const std::vector<std::list<size_t>> &__set) {
    std::vector<linear_equation<MORE_EQ>> vec_lin;
    vec_lin.reserve(__set.size());  
    for(auto it = __set.cbegin(); it != __set.cend(); ++it) {
        vec_lin.emplace_back(get_new_restriction(__set, it));
    }
    return vec_lin;
}




/// @brief задать начальные ограничения (инициализатор для объекта ограничения)
/// @return объект ограничения 
set_linear_eq get_base_equals(size_t N) {
    set_linear_eq set;
   // set.reserve(2 * N);
    for(size_t i = 0; i < N; ++i) {
        std::vector<std::pair<size_t, size_t>> __vec_index;
        __vec_index.reserve(N - 1);
        for(size_t j = 0; j < N; ++j) {
            if (i != j) {
                size_t p = (i < j) ? i * N + j : j * N + i;
                __vec_index.emplace_back(p / N, p % N);
            }
        }
        // == 2 
        set.add_new_restriction(linear_equation<EQUAL>(std::move(__vec_index), 2));
    }  
    
    for(size_t i = 0; i < N; ++i) {
        for(size_t j = i + 1; j < N; ++j) {
            set.add_new_restriction(linear_equation<LESS_EQ>({std::pair{i, j}}, 1));
        }
    }

    return set;
}

void print_path(const std::vector<std::pair<size_t, size_t>>& vec_res) {
    std::cout << "path = ";
    for(const auto & [__i, __j] : vec_res) {
        std::cout << '(' << __i << ", " << __j << ')';
    }
    std::cout << std::endl;
}

void print_component(const std::vector<std::list<size_t>>& component) {
    for(const auto& li : component) {
        std::cout << "{ ";
        for(const auto& vertex : li) {
            std::cout << vertex << " ";
        }
        std::cout << "}";
    }
    std::cout << std::endl;
}

std::pair<size_t, std::vector<std::pair<size_t, size_t>>> lin_alg(const matrix_dist& mx) {
    std::pair<size_t, std::vector<std::pair<size_t, size_t>>> res;
    size_t N = mx.size();
    bool_variables x(N);
    set_linear_eq lin_eq = get_base_equals(N);

    size_t i = 0;
    try {
    do {
        simplex_solver simp_s(mx, x, lin_eq);
        simp_s.solve();

        
        //simplex_solver simp_s(mx, x, lin_eq);
        //simp_s.solve();
        const auto vec_coord = simp_s.get_path();
        
        size_t cost = simp_s.get_cost(mx, vec_coord);
        adjacency_list adj_li(vec_coord, N);
        
        
        const auto component = graph_component(adj_li);
        print_component(component);
        if (component.size() == 1) {
            std::cout << "succ\n";
            res.first = cost;
            res.second = std::move(vec_coord); 
            break;
        } 
         
        res.first = cost;
        res.second = std::move(vec_coord); 


        //auto vec_line_rest = get_additional_restrictions(component);
        /*if (component.size() == 2) {
            auto line = get_new_restriction(component, component.begin());
            if (!line.empty()) { 
                    lin_eq.add_new_restriction(std::move(line));
            }*/
        //} else {
            auto vec_line_rest = get_additional_restrictions(component);
        
            for(auto& line : vec_line_rest) {
                if (!line.empty()) { 
                    lin_eq.add_new_restriction(std::move(line));
                }
            }
        //} 
        
        print_path(res.second);
        //std::cout << lin_eq << std::endl;
        if (vec_coord.size() == N - 1) {
            std::cout << "over\n";
            break;
        }
        
        ++i;
    
    } while(true); 
    } catch(const char * e) {
        std::cout << e << std::endl;
    }
    print_path(res.second);
    std::cout << res.first << std::endl;
    
    return res;
} 


int main(int argc, char* argv[]) {
    matrix_dist mx(argv[1]);
    lin_alg(mx);
    /*std::vector<std::pair<size_t, size_t>> vec = {
        std::pair{2,5}, std::pair{3,4}, std::pair{1,4}, std::pair{0,5}, std::pair{1,3}, std::pair{0,2}
    };
    adjacency_list adj(vec, 6);
    adj.print();
    auto vec_component = graph_component(adj);

    for(const auto& li : vec_component) {
        std::cout << "{ ";
        for(const auto& v : li) {
            std::cout << v << " ";
        }
        std::cout << " }" << std::endl;
    }
    std::cout << get_new_restriction(vec_component);
    */
   /*
    simplex_solver ss(mx);
    ss.solve();
    std::cout << ss.get_cost() << std::endl;
    auto vec = ss.get_path();
    for(const auto& [i, j] : vec) {
        std::cout << "("<< i << "," << j << ")" << " "; 
    }
    std::cout << std::endl;
    //ss.print();
    */

    return 0;
}
