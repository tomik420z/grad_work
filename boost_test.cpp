#include <iostream>
#include <algorithm>
#include <execution>
#include <vector>
#include <list>
#include <cstdlib>
#include <ctime>
#include <omp.h>


template<typename Iter, typename Unary_func>
void parallel_for_each(Iter __first, Iter __last, Unary_func __pred) {
    static_assert(std::is_base_of_v<std::random_access_iterator_tag, typename std::iterator_traits<Iter>::iterator_category>, "iterator must be random access");    

    #pragma omp parallel for 
    for(Iter __curr = __first; __curr < __last; ++__curr) {
        __pred(*__curr);
       
    }

}


template<typename Container_from, typename Unary_func>
Unary_func parallel_for_each(Container_from& container, Unary_func __pred) {

    const size_t sz = container.size();

    #pragma omp parallel for //num_threads()
    for(size_t i = 0; i < sz; ++i) {
        __pred(container[i]);
    }

    return __pred;
}

template<typename Iter_from, typename Iter_to, typename Unary_func>
Unary_func parallel_transform(Iter_from __first, Iter_from __last,  Iter_to __first_to, Unary_func __pred) {

    #pragma omp parallel for firstprivate(__first_to)//num_threads()
    for(Iter_from __curr = __first; __curr < __last; ++__curr) {
        *__first_to = __pred(*__curr); 
        ++__first_to;
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




int main()
{   
    std::vector<size_t> v;
    for(size_t i = 0; i < 10'000; ++i) {
        v.push_back(i);
    }
   // std::list li = {1,2,3,4,5,6};
    std::vector v1 = {1,2,3,4,5,6};
    std::cout << *parallel_max_element(v.begin(), v.end()) << std::endl;
    /*
    for(const auto& el : v1) {
        std::cout << el << std::endl;
    }*/
    /*
    parallel_for_each(v.begin(), v.end(), [](const auto& el){
        std::cout << el << " ";
    });*/
    
    return 0;
}