#pragma once
#ifndef PARALLEL_ALGORITHM_H
#define PARALLEL_ALGORITHM_H

#include <omp.h>

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
    #pragma omp declare reduction(parallel_max_element: Iter: omp_out = *omp_out < *omp_in ? omp_in : omp_out) initializer (omp_priv=(omp_orig))

    #pragma omp parallel for reduction(parallel_max_element: largest)
    for (Iter curr = first; curr < last; ++curr) {
        largest = (*largest < *curr) ? curr : largest;
    }
    return largest;
}

#endif