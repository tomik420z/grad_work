#ifndef LINEAR_ALLOCATOR_H
#define LINEAR_ALLOCATOR_H

#pragma once
// [li_col, li_row, data_matrix]
template<typename T>
class linear_allocator {
public:
    using allocator_type = linear_allocator<T>;
    using value_type = T;
    using pointer = T *;
    using const_pointer = const pointer;
    using difference_type = std::ptrdiff_t;
protected:
    pointer __begin;
    pointer __curr;
    pointer __last;
    pointer __begin_col_list;
    pointer __begin_row_list;
    pointer __null_ptr;
public:
    linear_allocator(size_t _sz) {
        size_t N = _sz * _sz;
        __begin = reinterpret_cast<T*>(::operator new(N * sizeof(T)));
        __curr = __begin;
        __last = __begin + N;  
        __begin_col_list = __begin;
        __begin_row_list = __begin + (_sz - 1);
        __null_ptr = __begin + N - 1;
    }

    linear_allocator(const linear_allocator& alloc) {
     
        ptrdiff_t N = std::distance(alloc.__begin, alloc.__last);
        __begin = reinterpret_cast<T*>(::operator new(N * sizeof(T)));
        __last = __begin + N; 
        __curr = __begin + std::distance(alloc.__begin, alloc.__curr);
        __begin_col_list = __begin + std::distance(alloc.__begin, alloc.__begin_col_list);
        __begin_row_list = __begin + std::distance(alloc.__begin, alloc.__begin_row_list);
        __null_ptr = __begin + std::distance(alloc.__begin, alloc.__null_ptr);
    }

    linear_allocator(linear_allocator && alloc) noexcept : __begin(alloc.__begin), __curr(alloc.__curr), __last(alloc.__last),
        __begin_col_list(alloc.__begin_col_list), __begin_row_list(alloc.__begin_row_list), 
        __null_ptr(alloc.__null_ptr) {
        alloc.__begin = alloc.__curr = alloc.__last = alloc.__begin_row_list = alloc.__begin_col_list = alloc.__null_ptr = nullptr;
    }

    linear_allocator& operator=(const linear_allocator &) = delete; 

    linear_allocator& operator=(linear_allocator&& alloc) noexcept {
        __begin = alloc.__begin;
        __curr = alloc.__curr;
        __last = alloc.__last;
        __begin_col_list = alloc.__begin_col_list;
        __begin_row_list = alloc.__begin_row_list;
        __null_ptr = alloc.__null_ptr;
        alloc.__begin = alloc.__curr = alloc.__last = alloc.__begin_row_list = alloc.__begin_col_list = alloc.__null_ptr = nullptr;
        return *this;
    }

    T* allocate(size_t) {
        return __curr++;
    } 

    difference_type get_size() const noexcept{
        return __last - __begin;
    }
    
    void bind_list_col(pointer ptr) noexcept {
        __begin_col_list = ptr;
    }

    void bind_list_row(pointer ptr) noexcept {
        __begin_row_list = ptr;
    }

    void deallocate(pointer, size_t) noexcept {}

   ~linear_allocator() {
        //std::cout << "deallocate_memory" << std::endl;
        if (__begin) {
            ::operator delete(__begin);
        }
    }

    T& get_element(size_t i) noexcept {
        return *(__begin + i);
    } 

    const T& get_element(size_t i) const noexcept {
        return *(__begin + i);
    }
    /*
    pointer get_data() noexcept {
        return __begin;
    }*/
    pointer get_data_col_list() noexcept {
        return __begin_col_list;
    } 

    pointer get_data_row_list() noexcept {
        return __begin_row_list;
    } 

    const pointer data() const noexcept {
        return __begin;
    }
    const pointer curr() const noexcept {
        return __curr;
    }

    const pointer null_ptr_data() const noexcept {
        return __null_ptr;
    }
};

#endif

