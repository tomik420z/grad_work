#include "input_file.hpp"
#include <random>
#include <unordered_set>
#include <algorithm>
#include <set>
#include <unordered_map>

class generic_alg {
public:
    using route_t = std::vector<int>;
    using matrix_ref = const std::vector<std::vector<int>> &;
    static constexpr size_t inf = std::numeric_limits<size_t>::max(); 
    using distance_t = size_t;
protected:
    using instance_t = route_t;
    using vec_population_t = std::vector<std::pair<instance_t, distance_t>>;
    matrix_ref mx_dist;
    size_t count_cities;
    route_t record_way;
    size_t record;
    size_t percent = 1;
    
    instance_t __create_instance() {
        instance_t instance;
        instance.reserve(count_cities + 1);
        for(size_t i = 0; i < count_cities; ++i) {
            instance.push_back(static_cast<int>(i));
        }
        std::random_shuffle(std::begin(instance), std::end(instance));
        return instance;
    }

    vec_population_t __create_population(size_t count_instance) {
        vec_population_t vec;
        vec.reserve(count_instance + 2);
        for(size_t i = 0; i < count_instance; ++i) {
            instance_t inst = __create_instance();
            distance_t dist = __calc_distance(inst);
            vec.emplace_back(std::move(inst), dist);
        }

        return vec;
    }

    distance_t __calc_distance(const instance_t & inst) {
        distance_t res = 0;
        for(instance_t::const_iterator c_prev = inst.begin(), c_curr = ++inst.cbegin(); 
                            c_curr != inst.cend(); ++c_prev, ++c_curr) {
            res+= mx_dist[*c_prev][*c_curr];
        }
        res+= mx_dist[inst.back()][inst.front()]; // return to begin city 
        return res;
    }

    size_t __get_random_index(size_t border) {
        return std::rand() % border;
    }

    std::pair<size_t, size_t> __choose_to_cross(size_t sz_population) {
        std::pair<size_t, size_t> res{0, 0};
        while(res.first == res.second) {
            res.first  = __get_random_index(sz_population);
            res.second  = __get_random_index(sz_population);
        }
        return res;
    }  

    std::unordered_set<size_t> __create_set_index(instance_t::const_iterator first, instance_t::const_iterator last) {
        std::unordered_set<size_t> set;
        for(; first != last; ++first) {
            set.insert(*first);
        }
        return set;
    }

    instance_t  __cross_two_instances(const instance_t& inst1, const instance_t& inst2) {
        instance_t child;
        child.reserve(count_cities);
        size_t break_point = __get_random_index(count_cities);
        std::unordered_set<size_t> set = __create_set_index(std::cbegin(inst1) + break_point, std::end(inst1));
        for(size_t i = 0; i < break_point; ++i) {
            child.push_back(inst1[i]);
        }

        for(size_t i = break_point; i < count_cities; ++i) {
            auto it = set.find(inst2[i]);
            if (it != set.end()) {
                child.push_back(*it);
                set.erase(it);
            }
        }

        for(auto el : set) {
            child.push_back(el);
        }

        return child;
    }

    void __try_mutation(instance_t & inst) {
        size_t param = __get_random_index(100);
        if (param < percent) {
            const auto& [i1, i2] = __choose_to_cross(count_cities);
            std::swap(inst[i1], inst[i2]);
        }        
    } 

    template<typename Pred>
    double __sum_fitness(const vec_population_t& population, const Pred& fitness_func, size_t max_len) {
        size_t sum = 0.0;
        for(const auto&[vec, lenght] : population) {
            sum += fitness_func(max_len, lenght);
        }
        return sum;
    }
    
    double __get_random_number(double __first, double __last) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> distrib(__first, __last);
        return distrib(gen);
    }

    template<typename Pred>
    instance_t __roulette_procedure(vec_population_t& population, const Pred& fitness_func, size_t max_len) {
        instance_t parrent;
        size_t sum_fitness = __sum_fitness(population, fitness_func, max_len);
        if (sum_fitness > 0) {
            size_t rand_border = __get_random_index(sum_fitness);

            size_t sum = 0;
            auto it_parrent1 = std::find_if(std::begin(population), std::end(population), 
                [&fitness_func, &sum, &rand_border, &max_len](const auto& el){
                    sum += fitness_func(max_len, el.second);
                    return sum >= rand_border;
            });

            parrent = std::move(it_parrent1->first);
            population.erase(it_parrent1);
        } else {
            parrent = population.back().first;
            population.pop_back();
        }
        return parrent;
    }

    instance_t __coding(const instance_t& inst) {
        instance_t cod_dist;
        cod_dist.reserve(inst.size());
        std::set<size_t> set;
        for(auto num : inst) {
            set.insert(num);
        }
        for(auto num : inst) {
            auto it = set.find(num);
            cod_dist.push_back(std::distance(set.begin(), it));
            set.erase(it);
        }
        return cod_dist;
    }
    
    void __decoding(instance_t & inst) {
        std::set<size_t> set;
        for(size_t i = 0; i < inst.size(); ++i) {
            set.insert(i);
        }

        for(auto & num : inst) {
            auto it = set.begin();
            std::advance(it, num);
            num = *it;
            set.erase(it);
        }
    }


    instance_t __base_cross(const instance_t& inst1, const instance_t& inst2) {
        instance_t code_inst1 = __coding(inst1);
        instance_t code_inst2 = __coding(inst2);

        size_t N = inst1.size();
        size_t rand_param = __get_random_index(N);
        instance_t child;
        child.reserve(N);
        for(size_t i = 0; i < rand_param; ++i) {
            child.push_back(code_inst1[i]);
        }
        for(size_t i = rand_param; i < N; ++i) {
            child.push_back(code_inst2[i]);
        }
        __decoding(child);
        return child;
    }

    instance_t __PMX(const instance_t& inst1, const instance_t& inst2) {
        size_t N = inst1.size();
        size_t l_border = __get_random_index(N);
        size_t r_border = __get_random_index(N);
        if (l_border > r_border) {
            std::swap(l_border, r_border);
        }

        instance_t child = inst2;
        // множество дубликатов
        std::unordered_set<size_t> set_duplicates;
        std::unordered_set<size_t> set__missing;
        for(size_t i = l_border; i < r_border; ++i) {
            set_duplicates.insert(inst1[i]);
            set__missing.insert(inst2[i]);
        }


        for(size_t i = l_border; i < r_border; ++i) {
            child[i] = inst1[i];   
            set_duplicates.erase(inst2[i]);
            set__missing.erase(inst1[i]);
        }
        
    
        std::unordered_map<size_t, size_t> set_permutation;
        for(size_t i = 0; i < N; ++i) {
            set_permutation.emplace(inst2[i], inst1[i]);
        }

        // permutation 
        for(auto num : set_duplicates) {
            auto second = set_permutation[num];
           // std::cout << num << "->" << second;
            while(set__missing.find(second) == set_duplicates.end()) {
                second = set_permutation[second];
             //   std::cout <<"->"<< second; 
            }
            //std::cout << '\n';
            size_t i = 0;
            size_t val = second;
            while(inst2[i] != num) {
                ++i;
            }
            child[i] = val;
        } 

        return child;
    }


    instance_t __cycle_cross(const instance_t& inst1, const instance_t& inst2) {
        size_t N = inst1.size();
        instance_t child = inst2;   

        struct index_val {
            size_t index; // индекс эл-та inst1 
            size_t val;  // val inst2
        };

        std::unordered_map<size_t, index_val> set_permutation;
        
        for(size_t i = 0; i < inst1.size(); ++i) {
            size_t val = inst2[i];
            set_permutation.emplace(inst1[i], index_val{i, val});
        }

        size_t rand_ind = __get_random_index(N);
        size_t val = inst1[rand_ind];
        size_t i = rand_ind;
        size_t curr_value = val;
        child[i] = inst2[i];
        //for(index_val)
        //std::cout << curr_value;
        do {
            index_val iv = set_permutation[curr_value];
            i = iv.index;
            curr_value = iv.val;
            //std::cout << "->" << curr_value;
            child[i] = inst1[i];
        } while(curr_value != val);
        //std::cout << '\n';
        return child;
    }


    instance_t __davis_crossover(const instance_t &inst1, const instance_t& inst2) {
        size_t N = inst1.size();
        size_t border = __get_random_index(N);

        instance_t child = inst2;
        // множество дубликатов
        std::unordered_set<size_t> set_duplicates;
        std::unordered_set<size_t> set__missing;
        for(size_t i = 0; i < border; ++i) {
            set_duplicates.insert(inst1[i]);
            set__missing.insert(inst2[i]);
        }


        for(size_t i = 0; i < border; ++i) {
            child[i] = inst1[i];   
            set_duplicates.erase(inst2[i]);
            set__missing.erase(inst1[i]);
        }


        std::unordered_map<size_t, size_t> set_permutation;
        for(size_t i = 0; i < N; ++i) {
            set_permutation.emplace(inst2[i], inst1[i]);
        }

        // permutation 
        for(auto num : set_duplicates) {
            auto second = set_permutation[num];
           // std::cout << num << "->" << second;
            while(set__missing.find(second) == set_duplicates.end()) {
                second = set_permutation[second];
             //   std::cout <<"->"<< second; 
            }
            //std::cout << '\n';
            size_t i = 0;
            size_t val = second;
            while(inst2[i] != num) {
                ++i;
            }
            child[i] = val;
        } 
        return child;
    }

    instance_t __ordered_crossover(const instance_t& inst1, const instance_t& inst2) {
        size_t N = inst1.size();
        instance_t child(N, 0);
        size_t l_border = __get_random_index(N);
        size_t r_border = __get_random_index(N);
        if (l_border > r_border) {
            std::swap(l_border, r_border);
        }
        std::vector<size_t> vec_seq;
        vec_seq.reserve(N);
        for(size_t i = r_border; i < N; ++i) {
            vec_seq.push_back(inst2[i]);
        }

        for(size_t i = 0; i < r_border; ++i) {
            vec_seq.push_back(inst2[i]);
        }

        std::unordered_set<size_t> set;

        for(size_t i = l_border; i < r_border; ++i) {
            set.insert(inst1[i]);
        }
        // fill 
        for(size_t i = l_border; i < r_border; ++i) {
            child[i] = inst1[i];
        }

        auto it = std::begin(vec_seq);
        for(size_t i = r_border; i < N; ++i) {
            while(set.find(*it) != set.end()) {
                ++it;
            }
            child[i] = *it;
            ++it;
        }

        for(size_t i = 0; i < l_border; ++i) {
            while(set.find(*it) != set.end()) {
                ++it;
            }
            child[i] = *it;
            ++it;
        }
    
        return child;
    }

public:
    generic_alg(const matrix_dist & mx) : mx_dist(mx.get_matrix_dist()), count_cities(mx.size()), record_way(mx.size()), record(inf) {
        std::srand(time(NULL));
    }  
    
    ~generic_alg() {}



    void operator()() {
        size_t size_population  = 30;
        std::srand(time(NULL));
        vec_population_t population = __create_population(size_population);
        std::sort(std::begin(population), std::end(population),[](const auto& lhs, const auto& rhs) {
                return lhs.second < rhs.second;
        });
        size_t N = 1000;
        for(size_t i = 0; i < N; ++i) {
            const auto &[i1, i2] = __choose_to_cross(population.size());
            instance_t child1 = __cross_two_instances(population[i1].first, population[i2].first);
            instance_t child2 = __cross_two_instances(population[i2].first, population[i1].first);
            size_t dist1 = __calc_distance(child1);
            size_t dist2 = __calc_distance(child2);

            population.emplace_back(std::move(child1), dist1);
            population.emplace_back(std::move(child2), dist2);


            std::sort(std::begin(population), std::end(population),[](const auto& lhs, const auto& rhs) {
                return lhs.second < rhs.second;
            });

            population.pop_back();
            population.pop_back();
        }

        std::cout << population[0].second << std::endl;
    }

    


    void basic_alg() {
        auto fitness_func = [](size_t max_leght, size_t lenght) {
            return max_leght - lenght; 
        };
        size_t count_iter = 600; //кол-во поколений  
        size_t size_population = 30; // размер популяции 
        vec_population_t population = __create_population(size_population); 
        
        for(size_t i = 0; i < count_iter; ++i) {
            auto min_val = std::min_element(std::begin(population), std::end(population), [](const auto &lhs, const auto& rhs){
                return lhs.second < rhs.second;
            })->second;
            record = std::min(min_val, record);

            // отбор рулеткой
            size_t max_len = std::max_element(std::begin(population), std::end(population), [](const auto &lhs, const auto&rhs){
                    return lhs.second < rhs.second;
            })->second;
            vec_population_t population_child;
            population_child.reserve(size_population);
            for(size_t i = 0; i < size_population; i += 2) {
                instance_t mother = __roulette_procedure(population, fitness_func, max_len);
                instance_t dad = __roulette_procedure(population, fitness_func, max_len);
                vec_population_t vec = {std::pair{__base_cross(mother, dad), 0},
                                        std::pair{__base_cross(dad, mother),0},
                                        //std::pair{__cycle_cross(mother, dad), 0},
                                        //std::pair{__cycle_cross(dad, mother), 0},
                                        std::pair{__PMX(dad, mother), 0},
                                        std::pair{__PMX(mother, dad), 0},
                                        std::pair{__ordered_crossover(mother,dad), 0},
                                        std::pair{__ordered_crossover(dad, mother), 0}
                                        };
        
                for(size_t i = 0; i < vec.size(); ++i) {
                    __try_mutation(vec[i].first);
                }
                for(size_t i = 0; i < vec.size(); ++i) {
                    vec[i].second = __calc_distance(vec[i].first);
                }
                std::sort(std::begin(vec), std::end(vec), [](const auto &lhs, const auto & rhs){
                    return lhs.second < rhs.second;
                });
                population_child.emplace_back(std::move(vec[0].first), vec[0].second);
                population_child.emplace_back(std::move(vec[1].first), vec[1].second);
                /*
                distance_t dist_1 = __calc_distance(child1);
                distance_t dist_2 = __calc_distance(child2);
    
                population_child.emplace_back(std::move(child1), dist_1);
                population_child.emplace_back(std::move(child2), dist_2);
                */
            }
            population = std::move(population_child);
            
        }   

        std::cout << record << std::endl;
    }

    void test() {
        instance_t child = __ordered_crossover({1, 2, 5, 6, 4, 3, 8, 7}, {1,4, 2,3,6,5,7,8});
        for(auto num : child) {
            std::cout << num << " ";
        }
        std::cout << '\n';
        
    }
}; 

int main(int argc, char* argv[]) {
    matrix_dist mx_dist(argv[1]);
    //std::cout << mx_dist << std::endl;
    generic_alg alg(mx_dist);
    alg.basic_alg();
    //alg.test();


    return 0;
}