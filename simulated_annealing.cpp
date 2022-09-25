#include "file_include.hpp"
#include "input_file.hpp"
#include <thread>
#include <mutex>
class simulated_annealing
{
public:
    using value_type = int;
    using size_type = size_t;
    using mx_distance_t = std::vector<std::vector<value_type>>;
    using way_t = std::vector<value_type>; 
    using list_temperture_t = std::list<double>;
    struct  __path_t {
        way_t way;
        value_type distance;
/*
        __path_t(__path_t&&rhs) {
            way = std::move(rhs.way);
        }
    
        __path_t(const __path_t &rhs) {
            way = rhs.way;
            distance = rhs.distance;
        }
*/
        bool operator<(const __path_t& rhs) const noexcept {
            return distance < rhs.distance;
        } 

        value_type &operator[](size_t i) {
            return way[i];
        }

        const value_type &operator[](size_t i) const {
            return way[i];
        }
/*
        __path_t & operator=(__path_t&&rhs) {
            way = std::move(rhs.way);
            return *this;
        }

        __path_t &operator=(const __path_t &rhs) {
            way = rhs.way;
            distance = rhs.distance;
        }
 */
        
    };

private:

    const mx_distance_t& mx;
    size_type size_matrix;
    way_t vec_names;
    value_type record;
    __path_t shortest_path;
    list_temperture_t list_temperature;
    static constexpr value_type inf = std::numeric_limits<value_type>::max();

    // заполнить массив имён [0, ..., size_matrix];
    way_t __fill_names(size_type);
    // вычислить расстояние 
    void __calc_distance(__path_t&);   
    // процедуры для нахождение y 
    __path_t __inverse(const __path_t &, size_type, size_type);
    __path_t __insert(const __path_t&, size_type, size_type);
    __path_t __swap(const __path_t&, size_type, size_type);
    __path_t __create_neighbour_solution(const __path_t&);
    double __delta_f(const __path_t&, const __path_t&); 

    __path_t __inverse_procedure(const __path_t &, size_type, size_type);
    __path_t __insert_procedure(const __path_t&, size_type, size_type);
    __path_t __swap_procedure(const __path_t&, size_type, size_type);

    double __get_r();

    double __fetch_max_temperature(list_temperture_t &);

    // создание списка температур 
    list_temperture_t __initial_temperature(size_type, double); 

    std::pair<size_type, size_type> __get_two_random_index();
public:

    simulated_annealing(const mx_distance_t & obj) : mx(obj), 
                                                size_matrix(obj.size()), 
                                                vec_names(__fill_names(size_matrix)), 
                                                record(inf) {}   

    template<typename Container>
    void print(const Container& container) {
        for(const auto & el : container){
            std::cout << el << " ";
        }
        std::cout << std::endl;
    }

    void operator()(double p0, std::mutex & mtx) {
        std::srand(time(NULL));
        __path_t x{vec_names, 0};
        //shortest_path.way = vec_names;
        std::random_shuffle(std::begin(x.way), std::end(x.way));
        __calc_distance(x);
        shortest_path = x;
        // оптимальное значение длины: 120 
        size_type size_list = size_matrix > 60 ? 120 : size_matrix;
        size_type number_of_iterations_outer_loop = 300;
        size_type number_of_iterations_inner_loop = 300;
        size_t count_move = 0;
       // double p0 = 0.0000000000000000001;        //p0: [0.1; 10^(-20)]
        auto li = __initial_temperature(size_list, p0);
        for(size_t i = 0; i < number_of_iterations_outer_loop; ++i) {
            double t = 0.0;
            auto it = std::max_element(std::begin(li), std::end(li));
            double t_max = *it;
            //std::cout <<std::fixed<< t_max << std::endl;
            size_type c = 0;
            for(size_t j = 0; j < number_of_iterations_inner_loop; ++j) {
                auto y = __create_neighbour_solution(x);
                if (y.distance < x.distance) {
                    x = std::move(y);
                    if (shortest_path.distance > x.distance) {
                        shortest_path = x;
                    }
                } else {
                    p0 = std::exp(-__delta_f(y, x) / t_max);
                    //std::cout << p0 << std::endl;
                    double r = __get_r();
                    if (r < p0) {
                        //std::cout << p0 << std::endl;
                        //t = 0.9 * t;
                        t = (t - __delta_f(y, x)) / std::log(r);
                        ++count_move;
                        ++c;
                        x = std::move(y);
                    }
                }
            }
            if (c != 0) {
                li.erase(it);
                li.push_back(t/static_cast<double>(c));
            }
        }
        mtx.lock();
        std::cout <<"count: " << count_move  << "  dist = " << shortest_path.distance << std::endl;
        mtx.unlock();
    }

    way_t & get_way() noexcept { return shortest_path.way; }

    size_type get_dist() const noexcept { return shortest_path.distance; } 

    ~simulated_annealing() {}
};

double simulated_annealing::__delta_f(const __path_t& y, const __path_t& x) {
    return static_cast<double>(
        y.distance - x.distance
    );
}

double simulated_annealing::__get_r() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> distrib(0.0, 1.0);
    return distrib(gen);
}

double simulated_annealing::__fetch_max_temperature(list_temperture_t & li) {
    auto it = std::max_element(std::begin(li), std::end(li));
    double num = *it;
    li.erase(it);
    return num; 
}

std::pair<simulated_annealing::size_type, simulated_annealing::size_type> simulated_annealing::__get_two_random_index() {
    auto res = std::pair{0, 0};
    while(res.first == res.second) {
        res.first  = std::rand() % size_matrix;
        res.second  = std::rand() % size_matrix;
    }

    return res;
}   

typename simulated_annealing::__path_t simulated_annealing::__inverse(const __path_t&way, size_type i, size_type j) {
    if (i > j) {
        std::swap(i, j);
    }
    __path_t way_res = way;
    while(i < j) {
        std::swap(way_res[i], way_res[j]);
        ++i; 
        --j;
    }
    __calc_distance(way_res);
    return way_res;
}

typename simulated_annealing::__path_t simulated_annealing::__insert(const __path_t&way, size_type i, size_type j) {
    __path_t way_res = way;
    if (i < j) {
        way_res[i] = way[j];
        for(size_t it = i + 1; it <= j; ++it) {
            way_res[it] = way[it - 1];
        } 
    } else {
        for(size_t it = j; it < i; ++it) {
            way_res[it] = way[it + 1];
        }
        way_res[i] = way[j];
    }
    __calc_distance(way_res);
    return way_res;
}

typename simulated_annealing::__path_t simulated_annealing::__swap(const __path_t &way, size_type i, size_type j) {
    __path_t way_res = way;
    std::swap(way_res[i], way_res[j]);
    __calc_distance(way_res);
    return way_res;
}


typename simulated_annealing::__path_t simulated_annealing::__inverse_procedure(const __path_t &path, size_type i, size_type j) {    
    __path_t res_path = __path_t{{}, simulated_annealing::inf};
    res_path = std::move(std::min(res_path, __inverse(path, i, j)));
    return res_path;
}


typename simulated_annealing::__path_t simulated_annealing::__insert_procedure(const __path_t&path, size_type i, size_type j) {
    __path_t res_path{{}, simulated_annealing::inf};
    res_path = std::move(std::min(res_path, __insert(path, i, j)));
    return res_path;
}
 
typename simulated_annealing::__path_t simulated_annealing::__swap_procedure(const __path_t&path, size_type i, size_type j) {
        
    __path_t res_path{{}, simulated_annealing::inf};
    res_path = std::move(std::min(res_path, __swap(path, i, j)));   
    return res_path;
}


typename simulated_annealing::__path_t simulated_annealing::__create_neighbour_solution(const __path_t&common_way) {
    //__path_t res = {{}, simulated_annealing::inf};
    auto [i, j] = __get_two_random_index(); 
       
            return std::move(std::min({__inverse_procedure(common_way, i, j),
                __insert_procedure(common_way, i, j), 
                __swap_procedure(common_way, i, j)}));

}

simulated_annealing::list_temperture_t simulated_annealing::__initial_temperature(size_type sz, double p0) {
    list_temperture_t li;
    for(size_t i = 0; i < sz; ++i) {
        auto y = __create_neighbour_solution(shortest_path);
        shortest_path = std::move(std::min(shortest_path, y)); 
        double t = -(static_cast<double>
                        (std::abs(y.distance - shortest_path.distance))
                                                                ) / log(p0);
        li.push_back(t);
    }

    return li;
}


typename simulated_annealing::way_t simulated_annealing::__fill_names(size_type sz) {
    way_t vec(sz);
    for(size_t i = 0; i < sz; ++i) {
        vec[i] = static_cast<value_type>(i);
    }
    return vec;
}

void simulated_annealing::__calc_distance(__path_t & p) {
    value_type sum = 0;
    auto beg = p.way.begin();
    auto end = p.way.end();
    for(auto prev = beg, curr = ++beg; curr != end; ++curr, ++prev) {
        sum += mx[*prev][*curr];
    }
    sum += mx[p.way.back()][p.way.front()];
    p.distance = sum;
}

class thread_solution {
protected:
    simulated_annealing obj_alg;
    std::mutex mtx;
    std::vector<std::thread> vec_thread;
    int max_dist;
    std::vector<int> solution;

public:
    thread_solution(size_t N, double p0, const std::vector<std::vector<int>> & mx_dist) : obj_alg(mx_dist),
                                                                                        mtx() {
        vec_thread.reserve(N);
        for(size_t i = 0; i < N; ++i) {
            vec_thread.emplace_back(std::thread(obj_alg, p0, std::ref(mtx)));
        }
        for(size_t i = 0; i < N; ++i) {
            vec_thread[i].join();
        }
    }
    ~thread_solution() {}
};


int main(int argc, const char *argv[]) {
    if (argc != 2) {
        std::cerr << "incorrect format" << std::endl;
        return -1;
    }
    matrix_dist obj(argv[1]);

    thread_solution(50, 0.01, obj.get_matrix_dist());
    //simulated_annealing rand_alg(obj.get_matrix_dist());
    
/*
    for(size_t i = 0; i < 1'000; ++i) {
        std::cout << "p0: "<< std::fixed << p0 << std::endl;
        buff << "p0: "<< p0 << std::endl;
        std::mutex mtx;
        for(size_t i = 0; i < 10; ++i) {
            v.emplace_back(std::thread(rand_alg, p0, std::ref(mtx), std::ref(buff)));
        }
        /*
        for(size_t j = 0; j < 10; ++j) {
            rand_alg(p0);
            std::cout << rand_alg.get_dist() << std::endl;
        }*/
    /*    std::for_each(std::begin(v), std::end(v), [](auto & th){
            th.join();
        });
        v.clear();
        std::cout << "---------------------------\n";
        p0 += 0.1;
    }
    */
    //auto&ref = rand_alg.get_way();
    //size_t sz = rand_alg.get_dist();
    //for(auto num : ref) {
      //  std::cout << num << " ";
   // }
    //std::cout << '\n';
    //std::cout << sz << std::endl;
}