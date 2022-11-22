#pragma once

#ifndef LINEAR_ALLOCATOR_H
#define LINEAR_ALLOCATOR_H

// [li_col, li_row, min_coef_col, min_coef_row, data_matrix]
template<typename T>
class linear_allocator {
public:
    using allocator_type = linear_allocator<T>;
    using value_type = T;
    using pointer = T *;
    using const_pointer = const pointer;
    using difference_type = std::ptrdiff_t;
protected:
    pointer __begin; // начало 
    pointer __curr; 
    pointer __last; // конец
    pointer __col_name_ptr;
    pointer __row_name_ptr;
    pointer __min_val_col_ptr;
    pointer __min_val_row_ptr;
    pointer __null_ptr;
public:
    linear_allocator(size_t _sz) { // _sz - рзмер матрицы 
        size_t N = _sz * _sz + 4 * _sz + 2;
        __begin = reinterpret_cast<T*>(::operator new(N * sizeof(T)));
        __curr = __begin;
        __last = __begin + N;   
        __col_name_ptr = __begin; 
        __row_name_ptr = __begin + _sz;
        __min_val_col_ptr = __row_name_ptr + _sz;
        __min_val_row_ptr = __min_val_col_ptr + _sz;
        __null_ptr = __begin + N - 1; 
    }

    linear_allocator(const linear_allocator& alloc) : 
            __begin(reinterpret_cast<T*>(::operator new(std::distance(alloc.__begin, alloc.__last) * sizeof(T)))),
            __last(__begin + std::distance(alloc.__begin, alloc.__last)),  
            __curr(__begin + std::distance(alloc.__begin, alloc.__curr)),
            __col_name_ptr(__begin + std::distance(alloc.__begin, alloc.__col_name_ptr)),
            __row_name_ptr(__begin + std::distance(alloc.__begin, alloc.__row_name_ptr)),
            __min_val_col_ptr(__begin + std::distance(alloc.__begin, alloc.__min_val_col_ptr)),
            __min_val_row_ptr(__begin + std::distance(alloc.__begin, alloc.__min_val_row_ptr)),
            __null_ptr(__begin + std::distance(alloc.__begin, alloc.__null_ptr)) {}

    linear_allocator(linear_allocator && alloc) noexcept : 
        __begin(alloc.__begin), 
        __curr(alloc.__curr), 
        __last(alloc.__last),
        __col_name_ptr(alloc.__col_name_ptr), 
        __row_name_ptr(alloc.__row_name_ptr),
        __min_val_col_ptr(alloc.__min_val_col_ptr),
        __min_val_row_ptr(alloc.__min_val_row_ptr), 
        __null_ptr(alloc.__null_ptr)  {
        alloc.__begin = alloc.__curr = alloc.__last =
            alloc.__min_val_col_ptr = alloc.__min_val_row_ptr =  
                alloc.__row_name_ptr = alloc.__col_name_ptr = alloc.__null_ptr = nullptr;
    }

    linear_allocator& operator=(const linear_allocator &) = delete; 

    linear_allocator& operator=(linear_allocator&& alloc) noexcept {
        __begin = alloc.__begin;
        __curr = alloc.__curr;
        __last = alloc.__last;
        __col_name_ptr = alloc.__col_name_ptr;
        __row_name_ptr = alloc.__row_name_ptr;
        __min_val_col_ptr = alloc.__min_val_col_ptr;
        __min_val_row_ptr = alloc.__min_val_row_ptr;
        __null_ptr = alloc.__null_ptr;
        alloc.__begin = alloc.__curr = alloc.__last = 
        alloc.__min_val_col_ptr = alloc.__min_val_row_ptr = 
        alloc.__row_name_ptr = alloc.__col_name_ptr = alloc.__null_ptr = nullptr;
        return *this;
    }

    T* allocate(size_t) {
        return __curr++;
    } 

    difference_type get_size() const noexcept{
        return __last - __begin;
    }
    
    void bind_list_col(pointer ptr) noexcept {
        __col_name_ptr = ptr;
    }

    void bind_list_row(pointer ptr) noexcept {
        __row_name_ptr = ptr;
    }

    void deallocate(pointer, size_t) noexcept {}

   ~linear_allocator() {
        //std::cout << "deallocate_memory" << std::endl;
        ::operator delete(__begin);
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
    pointer get_data_min_val_col() noexcept {
        return __min_val_col_ptr;
    }

    pointer get_data_min_val_row() noexcept {
        return __min_val_row_ptr;
    }

    pointer get_data_col_list() noexcept {
        return __col_name_ptr;
    } 

    pointer get_data_row_list() noexcept {
        return __row_name_ptr;
    } 

    pointer data() const noexcept {
        return __begin;
    }
    pointer curr() const noexcept {
        return __curr;
    }

    pointer null_ptr_data() const noexcept {
        return __null_ptr;
    }

};

#endif

