#ifndef LITTLE_ALG
#define LITTLE_ALG

//#pragma comment(linker, "/STACK:76777216")

#include "input_file.hpp"
#include "list_matrix.hpp"
#include "path.hpp"

template<typename _Alloc = std::allocator<int>>  
class little_alg {
public:
    using allocator_type = _Alloc;
    using size_type = size_t;
    using value_type = int;
    using line_t = std::vector<value_type>;
    using ptr_line_t = std::unique_ptr<line_t>;
    using matrix_t = list_matrix<value_type, _Alloc>;
    using ptr_matrix_t = std::unique_ptr<matrix_t>;
    using edge_t = std::pair<value_type, value_type>;
    using way_t = std::vector<edge_t>;
    using ptr_way_t = std::unique_ptr<way_t>;
    using random_accses_mx_t = std::vector<std::vector<value_type>>;  
    using ptr_path_t = std::unique_ptr<path_t>;
protected:
    using stack_call_t = std::stack<std::tuple<matrix_t, way_t, value_type>>;
    using vec_thread = std::vector<pthread_t>;
    static constexpr value_type infinity_1 = std::numeric_limits<value_type>::max(); // inf1
    static constexpr value_type infinity_2 = infinity_1 - 1;                         // inf2
    static constexpr value_type inf = infinity_1;                                    // inf
    omp_lock_t mtx;
    random_accses_mx_t mx_dist; // матрица достижимости 
    value_type record; // длина самого короткого пути 
    path_t min_way;     // самый короткий путь 
    ptr_matrix_t ptr_mx; // указатель на матрицу списков 
    std::stack<std::tuple<ptr_matrix_t, ptr_path_t, value_type>> data_stack;
    /*
    typedef struct arguments_function_thread_tag {
        sem_t *semaphore;
        pthread_mutex_t *mtx;
        //queue *q;
    } arg_function_thread;

    void create_threads(std::vector<pthread_t>& ths, arg_function_thread *args, int count_of_threads)    {
        for (int i = 0; i < count_of_threads; ++i) {
            pthread_create(&ths[i], NULL, work_threads, args);
        }
    }
    */
public:
    random_accses_mx_t copy_matrix_list(matrix_t & mx) {
        random_accses_mx_t matrix(mx.size(), 
                                std::vector<value_type>(mx.size()));
        auto row_end_iter = mx.row_end();
        auto col_end_iter = mx.col_end();
        size_t i = 0;
        for(auto row_it = mx.head_col(); row_it != row_end_iter; ++row_it) {
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
                                record(inf), min_way(mx.size()), 
                                ptr_mx(std::make_unique<matrix_t>(mx)) {
                                    omp_init_lock(&mtx);
                                }   

    // поиск минимального элемента
    template<typename Iter>
    value_type get_min_element(Iter it_beg, Iter it_end) {
        value_type t = infinity_1;
        for(;it_beg != it_end; ++it_beg) {
            t = std::min(t, *it_beg);
        }
        return t;
    }

    // вычитание минимального элемента
    template<typename Iter>
    void sub_min_element(Iter it_beg, Iter it_end, value_type val) {
        for(; it_beg != it_end; ++it_beg) {
            if (*it_beg < infinity_2) {
                *it_beg -= val;
            }
        }
    }

    void line_conversion(matrix_t & mx, value_type& border_cur) {
        auto mx_end_col = mx.col_end(); 
        auto mx_end_row = mx.row_end(); 
        for(auto it_row = mx.head_col(); it_row != mx_end_row; ++it_row) {
            value_type min_val = get_min_element(mx.get_col_iter(it_row), mx_end_col);
            border_cur += min_val;
            sub_min_element(mx.get_col_iter(it_row), mx_end_col, min_val);
        }
    }
    
    // преобразование столбцов 
    void row_conversion(matrix_t& mx, value_type & border_cur) {
        auto col_end = mx.col_end();
        auto row_end = mx.row_end();
        for(auto it_col = mx.col_begin(); it_col != col_end; ++it_col) {
            value_type min_val = get_min_element(mx.get_row_iter(it_col), row_end);
            border_cur+= min_val;
            // прибавление к нижней границе min_val
            sub_min_element(mx.get_row_iter(it_col), row_end, min_val);
        }
    }

    template<typename Iter>
    value_type get_min_coeff(Iter beg, 
                            Iter end, 
                            Iter it_avoid) {
        value_type min = infinity_1;
        for(auto it = beg; it != end; ++it) {
            if (it != it_avoid) {
                min = std::min(min, *it);
            }
        }
        return min;
    }
    
    value_type get_sum_min_coeff(matrix_t& mx, typename matrix_t::col_iterator it) {
        return get_min_coeff(mx.col_begin(it), mx.col_end(), it) + 
                                    get_min_coeff(mx.head_col(it), mx.row_end(), mx.get_row_iter(it));   
    }

    typename matrix_t::col_iterator find_min_coord(matrix_t &mx) {
        auto res_it = mx.col_begin();
        value_type max_val = std::numeric_limits<int>::min();
        auto it_col_end = mx.col_end();
        auto it_row_end = mx.row_end();
        for(auto curr_row = mx.head_col(); curr_row != it_row_end; ++curr_row) {
            for(auto curr_col = mx.get_col_iter(curr_row); curr_col != it_col_end; ++curr_col) {
                if (*curr_col == 0) {
                    value_type t = get_sum_min_coeff(mx, curr_col);
                    if (t > max_val) {
                        res_it = curr_col;
                        max_val = t;
                    }
                }
            }
        } 
        return res_it;
    }

    bool line_without_inf(typename matrix_t::col_iterator beg, typename matrix_t::col_iterator end) {
        for(auto it = beg; it != end; ++it) {
            if(*it == infinity_1) {
                return false;
            }
        }
        return true;
    }

    bool row_without_inf(const matrix_t & mx, typename matrix_t::row_iterator r) {
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
    }

    typename matrix_t::col_iterator get_rib_without_inf(const matrix_t &mx) {
        auto p = mx.head_col();
        auto row_end = mx.row_end();
        auto col_end = mx.col_end();
        for(auto it = mx.head_col(); it != row_end; ++it) {
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

        return res_iter;
    }

    typename matrix_t::col_iterator set_inf_l_k(matrix_t& mx, typename matrix_t::col_iterator erase_coord) {
        mx.erase(erase_coord); // вычеркнуть столбец и строку 
        return get_rib_without_inf(mx); 
    }

    long long calc_distance(const way_t& way) {
        long long sum = 0;
        for(auto[i, j] : way) {
                sum += mx_dist[i][j];
        }
        return sum;
    }

    void add_the_last_two_edges(path_t & way, const matrix_t & M, bool &bad_dist) {
        auto curr_row = M.head_col();
        auto curr_col = M.get_col_iter(curr_row);
        auto pt00 = curr_col;
        auto pt01 = ++curr_col;
        ++curr_row;
        curr_col = M.get_col_iter(curr_row);
        auto pt10 = curr_col;
        auto pt11 = ++curr_col;
        
        if (*pt00 >= infinity_2 && 
            *pt01 >= infinity_2 && 
            *pt10 >= infinity_2 &&
            *pt11 >= infinity_2) {
                bad_dist = false;
                return;
            }
    
        if (*pt00 < infinity_2) {
            way.add_edge(mx_dist, M.get_edge(pt00));
            way.add_edge(mx_dist, M.get_edge(pt11));
        } else {
            way.add_edge(mx_dist, M.get_edge(pt01));   
            way.add_edge(mx_dist, M.get_edge(pt10)); 
        }
    }

    void initial_conversion(matrix_t & M, value_type & border_lim) {
        line_conversion(M, border_lim);
        row_conversion(M, border_lim);
    }

    void print_way(way_t & way, value_type curr_dist) {
        std::cout << "way:"; 
        for(auto [v1, v2] : way) {
            std::cout << "(" << v1 << "," << v2 << ")";
        }
        std::cout << "   len_way = " << curr_dist << std::endl;
        std::cout << '\n';
    }

    std::ofstream buff; 
/*
    void __procedure(ptr_matrix_t&& M1, ptr_path_t&& way, value_type border_lim) {
        value_type border_cur = 0; 
        initial_conversion(*M1, border_cur);
        border_lim += border_cur;

        // если превышена граница, то выход
        if (border_lim >= record) {
            //std::cout <<"limit: " << border_lim << "  record: " << record << std::endl;
            return;
        }

        // поиск минимального элемента, имеющего наименьщий коэффициент 
        auto it = find_min_coord(*M1);
        M1->print(it);
        //std::cout << "edge: ";
        //M1->print(it);
        auto copy_way = *way;
        way->add_edge(mx_dist, M1->get_edge(it));
        value_type temp = *it;
        (*it) = infinity_2;
        auto M2 = *M1; 
        (*it) = temp;
        auto it_kl = set_inf_l_k(*M1, it);
        M1->print(it_kl);
        (*it_kl) = infinity_1;


        if (M1->size() > 2) {
            // stack.push(&little_alg<_Alloc>::recoursive_procedure, 
                                            // std::move(M1), std::move(way), border_lim);
            //std::thread th1(&little_alg<_Alloc>::recoursive_procedure, this, std::move(M1), std::move(way), border_lim);
            //vec_call.emplace_back(&little_alg<_Alloc>::recoursive_procedure, this, std::move(M1), std::move(way), border_lim);
            data_stack.emplace(std::move(M1), std::move(way), border_lim);
            //recoursive_procedure(std::move(M1), std::move(way), border_lim);
            //th1.join();
        } else {
            bool fl = true; 
            add_the_last_two_edges(*way, *M1, fl);
            if (!fl){
                return;
            }
            const auto sum = way->get_cost();
            //std::cout << sum << std::endl;
            if (sum < record) {
                min_way = *way;
                record = sum;
                buff << "record = "<< record << std::endl;
                //buff << *M1 << std::endl;
                buff << "----------------------------------\n";
            }
        }
        /*
        recoursive_procedure(std::move(std::make_unique<matrix_t>(M2)),  
            std::move(std::make_unique<path_t>(copy_way)),
            border_lim);
        */ 
       /*
       data_stack.emplace(std::make_unique<matrix_t>(M2),  
            std::make_unique<path_t>(copy_way),
            border_lim);
    }
*/
/*
    void execute() {        
        buff.open("48.txt");
        //mx_temp.print();
        if (ptr_mx->size() == 1) {
            min_way.add_edge(mx_dist, ptr_mx->get_edge(ptr_mx->col_begin()));   
            record = 0;
            return;
        } else if (ptr_mx->size() == 2) {
            bool fl = true; 
            add_the_last_two_edges(min_way, *ptr_mx, fl);
            record = min_way.get_cost();
            //record = calc_distance(min_way);
            return;
        }
        path_t w(ptr_mx->size());        

        __procedure(std::move(ptr_mx),
                std::move(std::make_unique<path_t>(w)), 0);
        while(!data_stack.empty()) {
            auto [M, w, dist] = std::move(data_stack.top());
            std::cout << dist << std::endl; 
            /*auto && M = std::move(std::get<0>(data_stack.top()));
            auto && w = std::move(std::get<1>(data_stack.top()));
            auto dist = std::get<2>(data_stack.top());*/
    /*        data_stack.pop();
            __procedure(std::move(M), std::move(w), dist);
        }

    }
    */
    void recoursive_procedure(matrix_t M1, path_t way, value_type border_lim) {

#pragma omp parallel 
{
    #pragma omp single nowait
    {
    
//std::cout << omp_get_thread_num() << std::endl;

        value_type border_cur = 0; 
        initial_conversion(M1, border_lim);

        // печать преобразованной матрицы 
       // std::cout << "matrix_after_substracting:\n";
       // std::cout << *M1 << std::endl;
       // std::cout << "-------------------------------\n"; 

        // если превышена граница, то выход
        if (border_lim < record) {
                //std::cout <<"limit: " << border_lim << "  record: " << record << std::endl;

            // поиск минимального элемента, имеющего наименьщий коэффициент 
            auto it = find_min_coord(M1);
            //std::cout << "edge: ";
            //M1->print(it);
            auto copy_way = way;
            way.add_edge(mx_dist, M1.get_edge(it));
            value_type temp = *it;
            (*it) = infinity_2;
            auto M2 = M1; 
            (*it) = temp;
            auto it_kl = set_inf_l_k(M1, it);
            (*it_kl) = infinity_1;
            if (M1.size() > 2) {

#pragma omp task firstprivate(M1, way, border_lim)
                {
                    //auto M = M1;
                    //auto w = way;
                    //auto border_l = border_lim;  
                    recoursive_procedure(std::move(M1), std::move(way), border_lim);
                }
            } else {
                bool fl = true; 
                add_the_last_two_edges(way, M1, fl);
                //if (!fl){
                  //  return;
                //}
                long long sum = way.get_cost();
                //std::cout << sum << std::endl;
                omp_set_lock(&mtx);
                if (sum < record) {
                    min_way = way;
                    record = sum;
                }
                omp_unset_lock(&mtx);
                //std::cout << "record = "<< record << std::endl;
                //buff << *M1 << std::endl;
                //std::cout << "----------------------------------\n";
                //print_way(*way, sum);
            }
#pragma omp task firstprivate(M2, copy_way, border_lim)
            {
                recoursive_procedure(std::move(M2),  
                    std::move(copy_way),
                    border_lim);
                //print_way(copy_way, sum);
            }
        } 
//#pragma omp taskwait
    }
}

    } 


    void operator()() {

        buff.open("48.txt");
        //mx_temp.print();
        if (ptr_mx->size() == 1) {
            min_way.add_edge(mx_dist, ptr_mx->get_edge(ptr_mx->col_begin()));   
            record = 0;
            return;
        } else if (ptr_mx->size() == 2) {
            bool fl = true; 
            add_the_last_two_edges(min_way, *ptr_mx, fl);
            record = min_way.get_cost();
            //record = calc_distance(min_way);
            return;
        }
        path_t w(ptr_mx->size());        
        omp_set_num_threads(8);
        omp_set_dynamic(0);
        recoursive_procedure(std::move(*ptr_mx),
                std::move(w), 0);
    
    }

    const way_t& get_way() const noexcept {
        return min_way.get_way();
    } 

    value_type get_min_len_way() const noexcept {
        return min_way.get_cost();
    }

    ~little_alg() {} 
};

#endif