#ifndef LITTLE_ALG
#define LITTLE_ALG

#include "input_file.hpp"

class little_alg {
public:
    using size_type = size_t;
    using value_type = int;
    using line_t = std::vector<value_type>;
    using ptr_line_t = std::unique_ptr<line_t>;
    using matrix_t = std::vector<line_t>;
    using ptr_matrix_t = std::unique_ptr<matrix_t>;
    using coord_t = std::pair<size_type, size_type>;
    using way_t = std::vector<coord_t>;
    using ptr_way_t = std::unique_ptr<way_t>;
protected:
    static constexpr value_type infinity_1 = std::numeric_limits<value_type>::max(); // inf1
    static constexpr value_type infinity_2 = infinity_1 - 1;  
    static constexpr value_type inf = infinity_1;

    matrix_t& mx_dist; // матрица достижимости 
    value_type record; // длина самого короткого пути 
    way_t min_way;     // самый короткий путь  
public:
    little_alg(matrix_t & mx) : mx_dist(mx), 
                                record(inf), min_way() {        
    }   

    // поиск минимального элемента в строке
    value_type get_min_element_line(matrix_t& mx, size_type index) {
        value_type min = inf;
        size_type sz = mx.size();
        for(size_t i = 0; i < sz; ++i) {
            min = std::min(min, mx[index][i]);
        }
        return min;
    }

    // вычитание минимального элемента из строки 
    void sub_min_line(matrix_t & mx, value_type val, size_type index) {
        size_type sz = mx.size();
        for(size_t i = 0; i < sz; ++i) {
            if (mx[index][i] < infinity_2) {
                mx[index][i] -= val;
            }
        }
    }   

    // преобразование строки 
    void line_conversion(matrix_t & mx, value_type& border_cur) {
        size_type sz = mx.size();
        for(size_t i = 0; i < sz; ++i) {
            value_type min_val = get_min_element_line(mx, i);
            border_cur += min_val;
            // прибавляется к нижней границе min_val
            sub_min_line(mx, min_val, i);
        } 
    }

    // поиск минимального элемента в строке
    value_type get_min_element_row(matrix_t& mx, size_type index) {
        value_type min = inf;
        size_type sz = mx.size();
        for(size_t i = 0; i < sz; ++i) {
            min = std::min(min, mx[i][index]);
        }
        return min;
    }

    // вычесть минимальный элемент по столбцу 
    void sub_min_row(matrix_t& mx, value_type val, size_type index) {
        size_type sz = mx.size();
        for(size_t i = 0; i < sz; ++i) {
            if (mx[i][index] < infinity_2) {
                mx[i][index] -= val;
            }
        }
    }

    // преобразование столбцов 
    void row_conversion(matrix_t& mx, value_type & border_cur) {
        size_type sz = mx.size();
        for(size_t i = 0; i < sz; ++i) {
            value_type min_val = get_min_element_row(mx, i);
            border_cur+= min_val;
            // прибавление к нижней границе min_val
            sub_min_row(mx, min_val, i);
        }
    }
    
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

    
    value_type get_min_row_coeff(matrix_t& mx, size_type i_ind, size_type j_ind) {
        value_type min = infinity_1;
        size_type sz = mx.size();
        for(size_t i = 0; i < sz; ++i) {
            if (i != i_ind) { 
                min = std::min(mx[i][j_ind], min);
            }
        }
        return min;
    }

    value_type get_min_coeff(matrix_t& mx, size_type i, size_type j) {
        return get_min_line_coeff(mx, i, j) + get_min_row_coeff(mx, i, j);   
    }

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

    void erase_i_j(matrix_t &mx, size_type i_ind, size_type j_ind){
        size_type sz = mx.size();  
        mx.erase(std::begin(mx) + i_ind); // удаление i-ой строки 
        for(size_t i = 0; i < sz - 1; ++i) {
            mx[i].erase(mx[i].begin() + j_ind);
        }
    }

    bool line_without_inf(matrix_t &mx, size_type i_ind) {
        size_type sz = mx.size();
        for(size_t i = 0; i < sz; ++i) {
            if (mx[i_ind][i] == infinity_1) {
                return false;
            }
        }
        return true;
    }

    bool row_without_inf(matrix_t &mx, size_type j_ind) {
        size_type sz = mx.size();
        for(size_t i = 0; i < sz; ++i) {
            if (mx[i][j_ind] == infinity_1) {
                return false;
            }
        }
        return true;
    }

    coord_t get_coord_without_inf(matrix_t &mx) {
        coord_t p = {0, 0};
        size_type sz = mx.size();
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

        return p;
    }

    coord_t set_inf_l_k(matrix_t& mx, const coord_t &erase_coord) {
        erase_i_j(mx, erase_coord.first, erase_coord.second);
        return get_coord_without_inf(mx);
    }

    void recoursive_procedure(ptr_matrix_t M1, ptr_line_t line, ptr_line_t row, ptr_way_t way, value_type border_lim) {
        std::cout << "matrix: \n";
        for(const auto& vec : *M1) {
            for(const auto & el : vec) {
                if (el == infinity_1) {
                    std::cout << "inf1 ";
                } else if(el == infinity_2) {
                    std::cout << "inf2 ";
                } else {
                    std::cout << el << " ";
                }

            }
            std::cout << '\n';
        }
        std::cout << "-----------------------------\n";
        value_type border_cur = 0;
        line_conversion(*M1, border_cur); // (1)
        row_conversion(*M1, border_cur); // (2)
        border_lim += border_cur;
        if (border_lim > record) {
            std::cout <<"limit: " << border_lim << "  record: " << record << std::endl;
            return;
        }
        //std::cout << "2\n";
        coord_t min_r = find_min_coord(*M1); // (3-4)
        auto copy_way = *way;
        auto copy_line = *line;
        auto copy_row = *row;
        way->push_back(coord_t{(*row)[min_r.first], (*line)[min_r.second]}); 
        line->erase(line->begin() + min_r.second);
        /*std::cout << "way: ";
        for(auto num : line) {
            std::cout << num << "  ";
        }
        std::cout << '\n';
        */
        row->erase(row->begin() + min_r.first);
        //std::cout << "3-4\n";
        auto M2 = *M1;
        //std::cout << min_r.first <<"   "<< min_r.second;
        auto [k, l] = set_inf_l_k(*M1, min_r); // (5)
        //std::cout << "5\n";
        (*M1)[k][l] = infinity_1;
        M2[min_r.first][min_r.second] = infinity_2;
        std::cout << "matrix_after_substruction:\n";
        for(const auto& vec : *M1) {
            for(const auto & el : vec) {
                if (el == infinity_1) {
                    std::cout << "inf1 ";
                } else if(el == infinity_2) {
                    std::cout << "inf2 ";
                } else {
                    std::cout << el << " ";
                }

            }
            std::cout << '\n';
        }
        std::cout << "----------------------------------------\n";
        
 
        if (M1->size() > 2) {
            recoursive_procedure(std::move(M1), std::move(line), std::move(row), std::move(way), border_lim);
        } else {
            if ((*M1)[0][0] < infinity_2) {
                way->push_back(coord_t{(*row)[0], (*line)[0]});
                way->push_back(coord_t{(*row)[1], (*line)[1]});
            } else {
                way->push_back(coord_t{(*row)[0], (*line)[1]});   
                way->push_back(coord_t{(*row)[1], (*line)[0]}); 
            }
            value_type sum = 0;
            for(auto[i, j] : *way) {
                sum += mx_dist[i][j];
            }
            if (sum < record) {
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
                std::move(std::make_unique<line_t>(copy_line)),
                std::move(std::make_unique<line_t>(copy_row)), 
                std::move(std::make_unique<way_t>(copy_way)),
                border_lim);
        } else {
            if (M2[0][0] < infinity_2) {
                copy_way.push_back(coord_t{(*row)[0], (*line)[0]});
                copy_way.push_back(coord_t{(*row)[1], (*line)[1]});
            } else {
                copy_way.push_back(coord_t{(*row)[0], (*line)[1]});   
                copy_way.push_back(coord_t{(*row)[1], (*line)[0]}); 
            }
            value_type sum = 0;
            for(auto[i, j] : copy_way) {
                sum += mx_dist[i][j];
            }
            if (sum < record) {
                record = sum;
            }

            std::cout << "copy_way:"; 
            for(auto [v1, v2] : copy_way) {
                std::cout << "(" << v1 << "," << v2 << ")";
            }
            std::cout << "   len_way = " << sum << std::endl;
            std::cout << '\n';

        }   

        std::cout << "end\n";
        //recoursive_procedure(M2, line, row);

    } 

    void operator()() {
        line_t vertex_line(mx_dist.size());
        line_t vertex_row(mx_dist.size());
        for(size_t i = 0; i < mx_dist.size(); ++i) {
            vertex_line[i] = i;
            vertex_row[i] = i;
        }
        auto mx_temp = mx_dist;
        way_t w;
        w.reserve(mx_temp.size());
        recoursive_procedure(std::move(std::make_unique<matrix_t>(mx_temp)),
                std::move(std::make_unique<line_t>(vertex_line)), 
                std::move(std::make_unique<line_t>(vertex_row)), 
                std::move(std::make_unique<way_t>(w)), 0);
    }

    ~little_alg() {} 
};

#endif