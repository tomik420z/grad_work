#ifndef LITTLE_ALG
#define LITTLE_ALG

#include "input_file.hpp"
#include "list_matrix.hpp"

class little_alg {
public:
    using size_type = size_t;
    using value_type = int;
    using line_t = std::vector<value_type>;
    using ptr_line_t = std::unique_ptr<line_t>;
    using matrix_t = list_matrix<value_type>;
    using ptr_matrix_t = std::unique_ptr<matrix_t>;
    using edge_t = std::pair<value_type, value_type>;
    using way_t = std::vector<edge_t>;
    using ptr_way_t = std::unique_ptr<way_t>;
    using random_accses_mx_t = std::vector<std::vector<value_type>>;  
protected:
    static constexpr value_type infinity_1 = std::numeric_limits<value_type>::max(); // inf1
    static constexpr value_type infinity_2 = infinity_1 - 1;                         // inf2
    static constexpr value_type inf = infinity_1;                                    // inf

    random_accses_mx_t mx_dist; // матрица достижимости 
    value_type record; // длина самого короткого пути 
    way_t min_way;     // самый короткий путь 
    ptr_matrix_t ptr_mx; 
public:
    random_accses_mx_t copy_matrix_list(matrix_t & mx) {
        random_accses_mx_t matrix(mx.size(), 
                                std::vector<value_type>(mx.size()));
        auto row_end_iter = mx.row_end();
        auto col_end_iter = mx.col_end();
        size_t i = 0;
        for(auto row_it = mx.row_begin(); row_it != row_end_iter; ++row_it) {
            size_t j = 0;
            for(auto col_it = mx.get_col_iter(row_it); col_it != col_end_iter; ++col_it) {
                matrix[i][j] = *col_it;
                ++j;
            }
            ++i;
        }
        return matrix;
    }

    little_alg(matrix_t & mx) : mx_dist(copy_matrix_list(mx)), 
                                record(inf), min_way(), 
                                ptr_mx(std::make_unique<matrix_t>(mx)) {       
        min_way.reserve(mx.size());
        std::cout << "copy:\n"; 
        for(auto vec : mx_dist) {
            for(auto num : vec) {
                std::cout << num << " ";
            }
            std::cout << '\n';
        }
    }   

    // поиск минимального элемента в строке
    value_type get_min_element_line(matrix_t::col_iterator it_beg, matrix_t::col_iterator it_end) {
        value_type t = infinity_1;
        for(;it_beg != it_end; ++it_beg) {
            t = std::min(t, *it_beg);
        }
        return t;
    }

    // вычитание минимального элемента из строки 
    void sub_min_line(matrix_t::col_iterator it_beg, matrix_t::col_iterator it_end, value_type val) {
        for(; it_beg != it_end; ++it_beg) {
            if (*it_beg < infinity_2) {
                *it_beg -= val;
            }
        }
    }

    // преобразование строки 
    void line_conversion(matrix_t & mx, value_type& border_cur) {
        auto mx_end_col = mx.col_end(); 
        auto mx_end_row = mx.row_end(); 
        for(auto it_row = mx.row_begin(); it_row != mx_end_row; ++it_row) {
            value_type min_val = get_min_element_line(mx.get_col_iter(it_row), mx_end_col);
            border_cur += min_val;
            sub_min_line(mx.get_col_iter(it_row), mx_end_col, min_val);
        }
        /*
        size_type sz = mx.size();
        for(size_t i = 0; i < sz; ++i) {
            value_type min_val = get_min_element_line(mx, i);
            border_cur += min_val;
            // прибавляется к нижней границе min_val
            sub_min_line(mx, min_val, i);
        } */
    }
    
    // поиск минимального элемента в столбце 
    value_type get_min_element_col(matrix_t::row_iterator beg, matrix_t::row_iterator end) {
        value_type t = inf;
        for(; beg != end; ++beg) {
            t = std::min(t, *beg);
        }
        return t;
    }

    // вычесть минимальный элемент по столбцу 
    void sub_min_row(matrix_t::row_iterator beg, matrix_t::row_iterator end, value_type val) {
        for(; beg != end; ++beg) {
            if (*beg < infinity_2) {
                *beg -= val;
            }
        }
    }

    // преобразование столбцов 
    void row_conversion(matrix_t& mx, value_type & border_cur) {
        auto col_end = mx.col_end();
        auto row_end = mx.row_end();
        for(auto it_col = mx.col_begin(); it_col != col_end; ++it_col) {
            value_type min_val = get_min_element_col(mx.get_row_iter(it_col), row_end);
            border_cur+= min_val;
            // прибавление к нижней границе min_val
            sub_min_row(mx.get_row_iter(it_col), row_end, min_val);
        }
    }
    /*
    value_type get_min_line_coeff(matrix_t& mx, size_type i_ind, size_type j_ind) {
        value_type min = infinity_1;
        size_type sz = mx.size();
        for(size_t j = 0; j < sz; ++j) {
            if (j != j_ind) {
                min = std::min(mx[i_ind][j], min);
            }
        }
        return min;
    }
    */
    value_type get_min_line_coeff(matrix_t::col_iterator beg, 
                                    matrix_t::col_iterator end, 
                                    matrix_t::col_iterator it_avoid) {
        value_type min = infinity_1;
        for(auto it = beg; it != end; ++it) {
            if (it != it_avoid) {
                min = std::min(min, *it);
            }
        }
        return min;
    }
    /*
    value_type get_min_col_coeff(matrix_t& mx, size_type i_ind, size_type j_ind) {
        value_type min = infinity_1;
        size_type sz = mx.size();
        for(size_t i = 0; i < sz; ++i) {
            if (i != i_ind) { 
                min = std::min(mx[i][j_ind], min);
            }
        }
        return min;
    }
    */
    value_type get_min_col_coeff(matrix_t::row_iterator beg,
                                matrix_t::row_iterator end,
                                matrix_t::row_iterator it_avoid) {
        value_type min = infinity_1;
        for(auto it = beg; it != end; ++it) {
            if (it != it_avoid) {
                min = std::min(min, *it);
            }
        }
        return min;                
    }

    value_type get_min_coeff(matrix_t& mx, matrix_t::col_iterator it) {
        //std::cout << "row_begin "<< *mx.row_begin(it) << std::endl; // по столбцу 
        return get_min_line_coeff(mx.col_begin(it), mx.col_end(), it) + 
                                    get_min_col_coeff(mx.row_begin(it), mx.row_end(), mx.get_row_iter(it));   
    }

    matrix_t::col_iterator find_min_coord(matrix_t &mx) {
        auto res_it = mx.col_begin();
        value_type max_val = std::numeric_limits<int>::min();
        auto it_col_end = mx.col_end();
        auto it_row_end = mx.row_end();
        for(auto curr_row = mx.row_begin(); curr_row != it_row_end; ++curr_row) {
            for(auto curr_col = mx.get_col_iter(curr_row); curr_col != it_col_end; ++curr_col) {
                if (*curr_col == 0) {
                    value_type t = get_min_coeff(mx, curr_col);
                    if (t > max_val) {
                        //std::cout<< "max_val " << t << std::endl;
                        res_it = curr_col;
                        max_val = t;
                    }
                }
            }
        } 
        return res_it;
    }

    /*
    // найти (i, j) которую будем вычёркивать 
    coord_t find_min_coord(matrix_t& mx) {
        size_type sz = mx.size();
        coord_t res = {0, 0};
        value_type max_val = std::numeric_limits<value_type>::min();
        for(size_t i = 0; i < sz; ++i) {    
            for(size_t j = 0; j < sz; ++j) {
                if (mx[i][j] == 0) {
                    value_type t = get_min_coeff(mx, i, j);
                    if (t > max_val) {
                        res = {i, j};
                        max_val = t;
                    } 
                }
            }
        }
        return res;
    }
    */
/*
    void erase_i_j(matrix_t &mx, size_type i_ind, size_type j_ind){
        size_type sz = mx.size();  
        mx.erase(std::begin(mx) + i_ind); // удаление i-ой строки 
        for(size_t i = 0; i < sz - 1; ++i) {
            mx[i].erase(mx[i].begin() + j_ind);
        }
    }
*/
    bool line_without_inf(matrix_t::col_iterator beg, matrix_t::col_iterator end) {
        for(auto it = beg; it != end; ++it) {
            if(*it == infinity_1) {
                return false;
            }
        }
        return true;
        /*size_type sz = mx.size();
        for(size_t i = 0; i < sz; ++i) {
            if (mx[i_ind][i] == infinity_1) {
                return false;
            }
        }
        return true;
        */
    }

    bool row_without_inf(matrix_t & mx,matrix_t::row_iterator r) {
        auto row_end = mx.row_end();
        for(auto it = r; it != row_end; ++it) {
            if(*it == infinity_1) {
                return false;
            }
        }
        for(auto it = r; it != row_end; --it) {
            if (*it == infinity_1) {
                return false;
            }
        }
        return true;
        /*
        size_type sz = mx.size();
        for(size_t i = 0; i < sz; ++i) {
            if (mx[i][j_ind] == infinity_1) {
                return false;
            }
        }
        return true;
        */
    }

    matrix_t::col_iterator get_rib_without_inf(matrix_t &mx) {
        auto p = mx.row_begin();
        auto row_end = mx.row_end();
        auto col_end = mx.col_end();
        for(auto it = mx.row_begin(); it != row_end; ++it) {
            if(line_without_inf(mx.get_col_iter(it), col_end)) {
                p = it;
            }
        }

        auto res_iter = mx.col_begin();
        for(auto it = mx.get_col_iter(p); it != col_end; ++it) {
            if (row_without_inf(mx, mx.get_row_iter(it))) {
                res_iter = it;
                break;
            }
        }

        /*size_type sz = mx.size();
        for(size_t i = 0; i < sz; ++i) {
            if (line_without_inf(mx, i)) {
                p.first = i;
            }
        }
        for(size_t j = 0; j < sz; ++j) {
            if (row_without_inf(mx, j)) {
                p.second = j;
            }
        }
*/
        return res_iter;
    }

    matrix_t::col_iterator set_inf_l_k(matrix_t& mx, matrix_t::col_iterator erase_coord) {
        mx.erase(erase_coord);
        //erase_i_j(mx, erase_coord.first, erase_coord.second);
        return get_rib_without_inf(mx);
    }

    value_type calc_distance(const way_t& way) {
        value_type sum = 0;
        for(auto[i, j] : way) {
                sum += mx_dist[i][j];
        }
        return sum;
    }

    void add_the_last_two_edges(way_t & way, matrix_t & M) {
        auto curr_row = M.row_begin();
        auto curr_col = M.get_col_iter(curr_row);
        auto pt00 = curr_col;
        auto pt01 = ++curr_col;
        ++curr_row;
        curr_col = M.get_col_iter(curr_row);
        auto pt10 = curr_col;
        auto pt11 = ++curr_col;
        if (*pt00 < infinity_2) {
            way.push_back(M.get_rib(pt00));
            way.push_back(M.get_rib(pt11));
        } else {
            way.push_back(M.get_rib(pt01));   
            way.push_back(M.get_rib(pt10)); 
        }
    }

    void initial_conversion(matrix_t & M, value_type & border_lim) {
        line_conversion(M, border_lim);
        row_conversion(M, border_lim);
    }

    void recoursive_procedure(ptr_matrix_t M1, ptr_way_t way, value_type border_lim) {
        // печать исходной матрицы 
        std::cout << "matrix:\n" << *M1 << std::endl; 
        std::cout << "-----------------------------\n";
        
        value_type border_cur = 0; 
        initial_conversion(*M1, border_lim);

        // печать преобразованной матрицы 
        std::cout << "matrix_after_substracting:\n";
        std::cout << *M1 << std::endl;
        std::cout << "-------------------------------\n"; 

        // если превышена граница, то выход
        border_lim += border_cur;
        if (border_lim > record) {
            std::cout <<"limit: " << border_lim << "  record: " << record << std::endl;
            return;
        }

        auto it = find_min_coord(*M1);
        std::cout << "edge: ";
        M1->print(it);

        auto copy_way = *way;
        way->push_back(M1->get_rib(it));
        value_type temp = *it;
        (*it) = infinity_2;
   
        auto M2 = *M1; 
        (*it) = temp;
        auto it_kl = set_inf_l_k(*M1, it);
        (*it_kl) = infinity_1; 
        if (M1->size() > 2) {
            recoursive_procedure(std::move(M1), std::move(way), border_lim);
        } else {
            add_the_last_two_edges(*way, *M1);
            value_type sum = calc_distance(*way);
            if (sum < record) {
                min_way = *way;
                record = sum;
            }
            std::cout << "way:"; 
            for(auto [v1, v2] : *way) {
                std::cout << "(" << v1 << "," << v2 << ")";
            }
            std::cout << "   len_way = " << sum << std::endl;
            std::cout << '\n';
        }

        
        if (M2.size() > 2) {
            recoursive_procedure(std::move(std::make_unique<matrix_t>(M2)),  
                std::move(std::make_unique<way_t>(copy_way)),
                border_lim);
        } else {
            add_the_last_two_edges(copy_way, M2);
            value_type sum = calc_distance(copy_way);
            if (sum < record) {
                min_way = copy_way;
                record = sum;
            }

            std::cout << "copy_way:"; 
            for(auto [v1, v2] : copy_way) {
                std::cout << "(" << v1 << "," << v2 << ")";
            }
            std::cout << "   len_way = " << sum << std::endl;
            std::cout << '\n';

        } 
    } 

    void operator()() {
        //mx_temp.print();
        way_t w;
        w.reserve(ptr_mx->size());
        recoursive_procedure(std::move(ptr_mx),
                std::move(std::make_unique<way_t>(w)), 0);
    }

    const way_t& get_way() const noexcept {
        return min_way;
    } 

    value_type get_min_len_way() const noexcept {
        return record;
    }

    ~little_alg() {} 
};

#endif