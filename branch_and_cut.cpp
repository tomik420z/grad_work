#include "gomory_solver.h"
 
class branch_and_cut {
protected:
    const matrix_dist& mx;

    linear_equation<MORE_EQ> get_new_restriction(const std::vector<std::list<size_t>> &__set, 
                                                    std::vector<std::list<size_t>>::const_iterator jt) {
    
        const auto& list_vertex = *jt;
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

public:
    branch_and_cut() = delete;

    branch_and_cut(const matrix_dist& mx) : mx(mx) {}    

    std::pair<size_t, std::vector<std::pair<size_t, size_t>>> solve() {
        std::pair<size_t, std::vector<std::pair<size_t, size_t>>> res;
        size_t N = mx.size();
        bool_variables x(N);
        set_linear_eq lin_eq = get_base_equals(N);

        size_t i = 0;
        gomory_solver simp_s(mx, x, lin_eq);
        
        simp_s.solve();
            
        const auto vec_coord = simp_s.get_path();
        
        size_t cost = simp_s.get_cost(mx, vec_coord);
        adjacency_list adj_li(vec_coord, N);
            
        const auto component = graph_component(adj_li);
        print_component(component);
        if (component.size() == 1) {
            std::cout << "succ\n";
            res.first = cost;
            res.second = std::move(vec_coord); 
            return res;
        } 
    
        res.first = cost;
        res.second = std::move(vec_coord); 


        auto vec_line_rest = get_additional_restrictions(component);
        simp_s.add_new_restrictions(vec_line_rest);
    
        print_path(res.second);
        //std::cout << lin_eq << std::endl;
        if (vec_coord.size() != N) {
            std::cout << "over\n";
            return res;
        }

        try {
        while(true) {
            simp_s.add_solve();
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

            auto vec_line_rest = get_additional_restrictions(component);
            simp_s.add_new_restrictions(vec_line_rest);
            
            print_path(res.second);
            //std::cout << lin_eq << std::endl;
            if (vec_coord.size() != N) {
                std::cout << "over\n";
                break;
            }
        }
        } catch(const char * e) {
            std::cout << e << std::endl;
        }
        print_path(res.second);
        std::cout << res.first << std::endl;
        
        return res;
    } 


    ~branch_and_cut() {}
};


int main(int argc, char* argv[]) {
    matrix_dist mx(argv[1]);
    branch_and_cut alg(mx);
    alg.solve();
    return 0;
}