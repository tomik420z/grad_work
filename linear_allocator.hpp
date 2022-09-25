#ifndef LINEAR_ALLOCATOR_H
#define LINEAR_ALLOCATOR_H

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
public:
    linear_allocator(size_t N) {
        __begin = reinterpret_cast<T*>(::operator new(N * sizeof(T)));
        __curr = __begin;
        __last = __begin + N;     
    }

    linear_allocator(const linear_allocator& alloc) {
        size_t N = alloc.__last - alloc.__begin;
        __begin = reinterpret_cast<T*>(::operator new(N * sizeof(T)));
        __curr = __begin;
        __last = __begin + N;      
    } 

    linear_allocator(linear_allocator && alloc) noexcept : __begin(alloc.__begin), __curr(alloc.__curr), __last(alloc.__last) {
        alloc.__begin = alloc.__curr = alloc.__last = nullptr;
    }

    linear_allocator& operator=(const linear_allocator &) = delete; 
    linear_allocator& operator=(linear_allocator&& alloc) noexcept {
        __begin = alloc.__begin;
        __curr = alloc.__curr;
        __last = alloc.__last;
        alloc.__begin = alloc.__curr = alloc.__last = nullptr;
        return *this;
    }

    T* allocate(size_t) {
        if (__curr != __last) {
            return __curr++;
        } else {
            throw "bad_alloc__";
        }
    } 

    difference_type get_size() const noexcept{
        return __last - __begin;
    }

    void deallocate(T* __ptr, size_t) {}

   ~linear_allocator() {
        //std::cout << "deallocate_memory" << std::endl;
        ::operator delete(__begin);
   } 
};

#endif

