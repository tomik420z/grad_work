#ifndef LIST_MATRIX
#define LIST_MATRIX 

#include "file_include.hpp"

template<typename T, typename _Alloc>
class list_matrix;

template<typename T, typename _Alloc>
std::ifstream& operator>>(std::ifstream &buff, list_matrix<T, _Alloc> &mx) {
    auto sz = list_matrix<T, _Alloc>::__parsing_size(buff); 
    mx.size_matrix = sz;
    mx.__fill_list_names(); // vertex -> {0, ..., sz - 1}
    mx.__read_matrix(buff);

    return buff;
}


template<typename T, typename _Alloc = std::allocator<T>>
class list_matrix {
protected:
    struct Node {
        T value;
        Node* up;
        Node* right;
        Node* down;
        Node* left;
        //typename std::list<std::pair<int, Node*>>::iterator __it_row;
        //typename std::list<std::pair<int, Node*>>::iterator __it_col;
        Node* __it_row;
        Node* __it_col; 
    };

public:
    using value_type = T;
    using allocator_type = _Alloc;
    using size_type = size_t;
    using alloc_node = typename std::allocator_traits<allocator_type>::rebind_alloc<Node>;
    using alloc_tr = std::allocator_traits<alloc_node>;
    static constexpr int inf = std::numeric_limits<int>::max();
protected:
    //std::list<std::pair<int, Node*>> names_row;
    //std::list<std::pair<int, Node*>> names_col;
    alloc_node alloc;
    Node * __names_col;
    Node* __names_row;
    Node * __min_val_col_list;
    Node* __min_val_row_list;
    Node* head;
    Node* __null_ptr;
    size_type size_matrix;
    const size_t __sz;
    
    void __read_file(const char* file_name) {
        std::ifstream buff(file_name);
        if (!buff.is_open()) {
            throw "file_not_opened";
        }
        buff >> *this;
        buff.close();
    }

    static void __ignore_spaces_until_char(std::ifstream &, char);   
    static size_type __parsing_size(std::ifstream &);
    void __read_matrix(std::ifstream &);
    void __fill_first_line(std::ifstream &);
    static int __parsing_number(std::ifstream &);
    void __clear_line(Node *);
    void __clear_col(Node *);
    void __copy_first_line(const list_matrix & obj);
    void __bind_list_and_mx();
    void __bind_list_names_and_list_min_vals();
    void __erase_line(Node *);
    void __erase_col(Node *);
    void __fill_names_col();
    void __fill_names_row();
    void __fill_min_values_col();
    void __fill_min_values_row();
    void __fill_list_names();
    void __fill_list_min_vals();
    void __erase_col_element(Node*, Node*);
    void __erase_row_element(Node*, Node*);

    template<bool Is_const, bool Direction>
    class __common_iterator {
    protected:
        using ptr_type = std::conditional_t<Is_const, const Node*, Node*>;
        using value_type = std::conditional_t<Is_const, const T&, T&>;
        ptr_type ptr;   
    public:
        __common_iterator() : ptr(__null_ptr) {}

        __common_iterator(ptr_type p_ptr) : ptr(p_ptr) {}
        
        __common_iterator(const __common_iterator& it) : ptr(it.ptr) {}  

        value_type operator*() const { return ptr->value; }

        ptr_type operator->() const { return ptr; }
        

        __common_iterator& operator++() {
            if constexpr (Direction) {
                ptr = ptr->right;
            } else {
                ptr = ptr->down;
            }
            return *this; 
        }


        __common_iterator& operator--() {
            if constexpr (Direction) {
                ptr = ptr->left;
            } else {
                ptr = ptr->up;
            }
            return *this; 
        } 

        __common_iterator& operator=(__common_iterator rhs) {
            ptr = rhs.ptr;
            return *this;
        }
        
        bool operator==(__common_iterator rhs) const noexcept{
            return ptr == rhs.ptr;
        }

        bool operator!=(__common_iterator rhs) const noexcept{
            return !operator==(rhs);
        }

        ptr_type data() const noexcept {
            return ptr;
        }

        ~__common_iterator() {}

        friend class list_matrix;
    };

public:
    void print_list_row() {
        Node*tail = __names_row;
        while(tail != __null_ptr) {
            std::cout  << "("<< tail->value << " " << tail->left->value <<")";
            tail = tail->right;
        } 
        std::cout << '\n';
    }   

    void print_list_col() {
        Node*tail = __names_col;
        while(tail != __null_ptr) {
            std::cout  << "("<< tail->value << " " << tail->left->value <<")";
            tail = tail->right;
        } 
        std::cout << '\n';
    }   

    using const_row_iterator = __common_iterator<true, false>;
    using row_iterator = __common_iterator<false, false>; 
    using const_col_iterator = __common_iterator<true, true>;
    using col_iterator = __common_iterator<false, true>;
    /*
    void () {
        for(Node* tail = head; tail != nullptr; tail = tail->down) {
            for(Node* it = tail; it != nullptr; it = it->right) {
                std::cout << "(" << it->__it_row->second->value << "," << it->__it_col->second->value << ")";
            }
            std::cout << std::endl;
        }
        for(Node* tail = head; tail != nullptr; tail = tail->down) {
            for(Node* it = tail; it != nullptr; it = it->right) {
                std::cout << "(" << it->__it_row->first << "," << it->__it_col->first << ")";
            }
            std::cout << std::endl;
        }
        
    }*/

    void __copy_first_line(const std::vector<int>& obj) {
        Node *tail = __null_ptr;
        Node *prev = __null_ptr;
        Node* iter_col = __names_col;
        Node* iter_row = __names_row;
        
        auto tail_obj = obj.begin();  
        head = alloc_tr::allocate(alloc, 1);
        alloc_tr::construct(alloc, head, Node{
            *tail_obj,
            __null_ptr, 
            __null_ptr,
            __null_ptr,
            __null_ptr,
            iter_row,
            iter_col
        });
        tail = head;
        prev = tail;
        tail = tail->right;
        //tail_obj = tail_obj->right;
        ++tail_obj;
        //++iter_col;
        iter_col = iter_col->right;
        for(size_t i = 1; i < size_matrix; ++i) {
            tail = alloc_tr::allocate(alloc, 1);
            alloc_tr::construct(alloc, tail, Node{
                *tail_obj, 
                __null_ptr,
                __null_ptr,
                __null_ptr,
                prev,
                iter_row, 
                iter_col
            });
            prev->right = tail;
            prev = tail;
            tail = tail->right;
            //tail_obj = tail_obj->right;
            ++tail_obj;
            iter_col = iter_col->right;
        }
    }

    list_matrix(list_matrix&& __mx) noexcept : alloc(std::move(__mx.alloc)),
                                    __min_val_col_list(__mx.__min_val_col_list),
                                    __min_val_row_list(__mx.__min_val_row_list), head(__mx.head), 
                                    __names_row(__mx.__names_row), __names_col(__mx.__names_col), 
                                    size_matrix(__mx.size_matrix), __sz(__mx.__sz), __null_ptr(__mx.__null_ptr)  {
        __mx.head = nullptr;
        __mx.size_matrix = 0;
        __mx.__names_col = nullptr;
        __mx.__names_row = nullptr;
        __mx.__null_ptr = nullptr;
        __mx.__min_val_col_list = nullptr;
        __mx.__min_val_row_list = nullptr;
    }
    // создание матрицы из массива 
    list_matrix(const std::vector<std::vector<int>>&__mx_dist, alloc_node && __alloc = alloc_node()) : 
                                                    size_matrix(__mx_dist.size()), alloc(std::move(__alloc)), __sz(__mx_dist.size()),
                                                    __null_ptr(alloc.null_ptr_data()) {
        
        __fill_list_names(); // заполнить список номеров вершин 
        __fill_list_min_vals(); // создать список минимальных коэффициентов
        __bind_list_names_and_list_min_vals(); // связать списки имён со списками мин коэф. 
        // скопировать первую строку из таблицы 
        __copy_first_line(__mx_dist[0]); // скопировать первую строку из таблицы 
        auto row_obj = ++__mx_dist.begin(); //obj.head->down; 
        Node*head_row = head;  // указатели на начало списка (строки) 
        //auto iter_row = ++names_row.begin(); // идём с первой строки 
        Node* iter_row = __names_row->right; // строка односвязный список 

        for(size_t i = 1; i < size_matrix; ++i) {
            Node * prev_ptr = __null_ptr;  // предыдущий указатель          
            Node* curr_ptr = head_row->down;  //текущий указатель 
            Node* iter_col = __names_col;
            //auto iter_col = names_col.begin(); 
            auto col_obj = row_obj->begin();
            // предыдущая строка  
            for(Node*tail_col = head_row; tail_col != __null_ptr; tail_col = tail_col->right) {
                curr_ptr = alloc_tr::allocate(alloc, 1);
                alloc_tr::construct(alloc, curr_ptr, Node{
                    *col_obj,   
                    tail_col,
                    __null_ptr,
                    __null_ptr,
                    prev_ptr, 
                    iter_row,
                    iter_col
                });
                if (prev_ptr != __null_ptr) {
                    prev_ptr->right = curr_ptr;
                }
                tail_col->down = curr_ptr;
                prev_ptr = curr_ptr;
                curr_ptr = curr_ptr->right;
                iter_col = iter_col->right;
                //col_obj = col_obj->right;
                ++col_obj;
            }
            ++row_obj;
            //row_obj = row_obj->down;
            //++iter_row;
            iter_row = iter_row->right;
            head_row = head_row->down;
        }
        __bind_list_and_mx();  // cвязать списки и матрицу
    }

    list_matrix(const char *file_name, alloc_node &&__alloc) : alloc(__alloc) {
        __read_file(file_name);
    }

    void mem_copy(const Node* __beg,const Node* __end, Node *__out) {
            ptrdiff_t i;
            ptrdiff_t n = __end - __beg;
            for(i = 0; i < n; ++i) {
                //std::cout << i << std::endl;
                __out->value = __beg->value;
                //std::cout << (__beg->up - __beg) << " " << (__beg->right - __beg) << " " << (__beg->left - __beg)  << " "<< (__beg->down - __beg) << std::endl;
                __out->up = __out + (__beg->up - __beg);
                __out->right =  __out + (__beg->right - __beg);
                __out->left =  __out +  (__beg->left - __beg);
                __out->down =  __out + (__beg->down - __beg);
                __out->__it_col =  __out + (__beg->__it_col - __beg);
                __out->__it_row = __out + (__beg->__it_row - __beg);
                ++__beg;
                ++__out;
            }
    }

    list_matrix(const list_matrix & obj) : size_matrix(obj.size_matrix), alloc(obj.alloc), __sz(obj.__sz) {
        __names_col = alloc.get_data_col_list();
        mem_copy(obj.__names_col, obj.__names_row, __names_col);
        __names_row = alloc.get_data_row_list();
        mem_copy(obj.__names_row, obj.__min_val_col_list, __names_row);
        __min_val_col_list = alloc.get_data_min_val_col();
        mem_copy(obj.__min_val_col_list, obj.__min_val_row_list,__min_val_col_list);
        __min_val_row_list = alloc.get_data_min_val_row();
        mem_copy(obj.__min_val_row_list, obj.head, __min_val_row_list);
        head = alloc.data() + std::distance(obj.alloc.data(), obj.head);    
        mem_copy(obj.head, obj.alloc.curr(), head);   

        __null_ptr = alloc.null_ptr_data();
        /*Node* ptr_col = alloc.get_data_col_list();
        __names_col = ptr_col;
        Node* ptr_row = alloc.get_data_row_list();
        __names_row = ptr_row;
        head = ptr_row->up;
        */
        // скопировать порядок вершин 
        /*
        const auto &ref_names_col = obj.names_col;
        for(auto [v, ptr] : ref_names_col) {
            names_col.push_back(std::pair{v, nullptr});
        }
        const auto &ref_names_row = obj.names_row;
        for(auto [v, ptr] : ref_names_row) {
            names_row.push_back(std::pair{v, nullptr});
        }
        */
       /* Node* tail = head;
        auto it_cols = names_col.begin();
        for(Node* tail = head; tail!=nullptr; tail = tail->down) {
            auto it_rows = names_row.begin(); 
            for(Node* tail_row = tail; tail_row != nullptr; tail_row = tail_row->right) {
                tail_row->__it_col = it_cols;
                tail_row->__it_row = it_rows;
                ++it_rows;
            }
            ++it_cols;
        }
        __bind_list_and_mx(); 
        std::cout << *this << std::endl;
        */
        /*
        if (size_matrix <= 0) {
            return;
        }
        // скопировать порядок вершин 
        const auto &ref_names_col = obj.names_col;
        for(auto [v, ptr] : ref_names_col) {
            names_col.push_back(std::pair{v, nullptr});
        }
        const auto &ref_names_row = obj.names_row;
        for(auto [v, ptr] : ref_names_row) {
            names_row.push_back(std::pair{v, nullptr});
        }

        __copy_first_line(obj); // заполнить первую строку 
        auto row_obj = obj.head->down; 
        Node*head_row = head;  // указатели на начало списка (строки) 
        auto iter_row = ++names_row.begin(); // идём с первой строки 
        for(size_t i = 1; i < size_matrix; ++i) {
            Node * prev_ptr = nullptr;  // предыдущий указатель          
            Node* curr_ptr = head_row->down;  //текущий указатель 
            auto iter_col = names_col.begin(); 
            auto col_obj = row_obj;
            // предыдущая строка  
            for(Node*tail_col = head_row; tail_col != nullptr; tail_col = tail_col->right) {
                curr_ptr = alloc_tr::allocate(alloc, 1);
                alloc_tr::construct(alloc, curr_ptr, Node{
                    col_obj->value,   
                    tail_col,
                    nullptr,
                    nullptr,
                    prev_ptr, 
                    iter_row,
                    iter_col
                });
                if (prev_ptr != nullptr) {
                    prev_ptr->right = curr_ptr;
                }
                tail_col->down = curr_ptr;
                prev_ptr = curr_ptr;
                curr_ptr = curr_ptr->right;
                ++iter_col;
                col_obj = col_obj->right;
            }
            row_obj = row_obj->down;
            ++iter_row;
            head_row = head_row->down;
        }
        __bind_list_and_mx();  // звязать списки и матрицу
        */
    }

    void erase(col_iterator it);

    size_type size() const noexcept { return size_matrix; }

    void clear() {
        if (head == __null_ptr) {
            return;
        }
       /* names_col.clear();
        names_row.clear();*/
        


        auto prev = head;
        auto tail = head->down;
        while(tail != __null_ptr) {
            __clear_line(prev);
            prev = tail;
            tail = tail->down;
        }
        __clear_line(prev);
        head = nullptr;
    }


    ~list_matrix() {
        //clear();
    }

    void this_not_clear() {
        alloc.dont_clear();
    }


    row_iterator get_row_iter(col_iterator it_col) const noexcept { return row_iterator(it_col.ptr); }
    col_iterator get_col_iter(row_iterator it_row) const noexcept { return col_iterator(it_row.ptr); }

    row_iterator head_col() const noexcept { return row_iterator(head); }
    col_iterator col_begin() const noexcept { return col_iterator(head); }

    row_iterator head_col(col_iterator it_col) const noexcept { 
        return row_iterator(it_col.ptr->__it_col->up); 
    }
    col_iterator col_begin(col_iterator it_col) const noexcept {
        return col_iterator(it_col.ptr->__it_row->up);
    }
    row_iterator head_col(row_iterator & it_row) const noexcept { 
        return row_iterator(it_row.ptr->__it_col->up); 
    }
    col_iterator col_begin(row_iterator & it_row) const noexcept { 
        return col_iterator(it_row.ptr->__it_row->up); 
    }

    row_iterator row_end() const noexcept { return row_iterator(__null_ptr); }
    col_iterator col_end() const noexcept {return col_iterator(__null_ptr); }
    /*
    const std::list<std::pair<int, Node*>>& get_list_col()  { return names_col; }
    const std::list<std::pair<int, Node*>>& get_list_row() { return names_row; }
    */
    template<bool Is_const, bool Direction>
    friend class __common_iterator;

    void print(col_iterator it) {
        std::cout << "("<< it.ptr->__it_row->value << "," << it.ptr->__it_col->value << "," << *it << ")\n"; 
    }

/*
    void set_element(size_t i, size_t j, value_type _val) {
        Node* tail = head;
        for(; tail->__it_col->first != i; tail = tail->down) ;
        for(; tail->__it_row->first != j; tail = tail->right) ;
        //Node& el = alloc.get_element(__sz * i + j);   
        std::cout << tail->__it_row->first << "  " << tail->__it_col->first << std::endl;
        tail->value = _val;
    }   
*/
    std::pair<int, int> get_edge(col_iterator it) const noexcept {
        return { it.ptr->__it_row->value, it.ptr->__it_col->value };
    }  

    list_matrix&operator=(list_matrix && __mx) noexcept {
        alloc = std::move(__mx.alloc);
        __min_val_col_list = __mx.__min_val_col_list;
        __min_val_row_list = __mx.__min_val_row_list;
        head = __mx.head;
        __names_row = __mx.__names_row; 
        __names_col = __mx.__names_col;
        size_matrix = __mx.size_matrix;
        __null_ptr = __mx.__null_ptr;
        __mx.__null_ptr = __mx.__names_row = __mx.__names_col = __mx.head = __mx.__min_val_col_list = __mx.__min_val_row_list = nullptr;
        __mx.size_matrix = 0;
        return *this;
    }

    friend std::ifstream &operator>><T, _Alloc>(std::ifstream &, list_matrix &);
};


template<typename T, typename _Alloc> 
void list_matrix<T, _Alloc>::__clear_col(Node*head_col) {
    Node* prev = head_col;
    head_col = head_col->down;
    while(head_col != __null_ptr) {
        alloc_tr::destroy(alloc, &prev->value); 
        alloc_tr::deallocate(alloc, prev, 1);
        prev = head_col;
        head_col = head_col->down;
    }
    alloc_tr::destroy(alloc, &prev->value); 
    alloc_tr::deallocate(alloc, prev, 1);
}

template<typename T, typename _Alloc>
void list_matrix<T, _Alloc>::__erase_line(Node* ptr_row_head) {
    auto col_head = ptr_row_head;
    if (col_head == head) {
        head = head->down;
        auto tail = head;
        // перевязать список
        Node * tail_col = __names_col;
        for(; tail_col != __null_ptr; tail_col = tail_col->right) {
            tail_col->up = tail; // up используется как привязка к основной матрице 
            tail = tail->right;
        }
        /*
        for(auto &[names, ptr]: names_col) {
            ptr = tail;
            tail = tail->right;
        }*/
    }
    for(; col_head != __null_ptr; col_head = col_head->right) {
        Node*up_iter = col_head->up;
        Node*down_iter = col_head->down;
        if (up_iter != __null_ptr) {
            up_iter->down = down_iter;
        } 
        if (down_iter != __null_ptr) {
            down_iter->up = up_iter;
        }
    }
}

template<typename T, typename _Alloc>
void list_matrix<T, _Alloc>::__erase_col(Node* ptr_col_head) {
    auto row_head = ptr_col_head;
    if (row_head == head) {
        head = head->right;
        auto tail = head;
        Node *tail_row = __names_row;
        for(; tail_row != __null_ptr; tail_row = tail_row->right) {
            tail_row->up = tail;
            tail = tail->down;
        }
        /*
        for(auto &[name, ptr] : names_row) {
            ptr = tail;
            tail = tail->down;
        }*/
    }
    for(; row_head != __null_ptr; row_head = row_head->down) {
        Node*left_iter = row_head->left;
        Node*right_iter = row_head->right;
        if (left_iter != __null_ptr) {
            left_iter->right = right_iter;
        } 
        if (right_iter != __null_ptr) {
            right_iter->left = left_iter;
        }
    }
}
template<typename T, typename _Alloc>
void list_matrix<T, _Alloc>::__erase_col_element(Node* prev, Node *curr) {
    prev->right = curr->right;
    alloc_tr::deallocate(alloc, curr, 1);
    alloc_tr::destroy(alloc, curr);
}

template<typename T, typename _Alloc>
void list_matrix<T, _Alloc>::__erase_row_element(Node* prev, Node*curr) {
    prev->right = curr->right;
    alloc_tr::deallocate(alloc, curr, 1);
    alloc_tr::destroy(alloc, curr);
}

template<typename T, typename _Alloc>
void list_matrix<T, _Alloc>::erase(col_iterator it) {
    // удаление строки 
    Node * ptr = it.ptr;
    //удалить строку
    __erase_line(ptr->__it_row->up);

    // переписать
    if (ptr->up != __null_ptr) {
        __erase_row_element(ptr->up->__it_row, ptr->__it_row);
    } else {
        Node * next = ptr->__it_row->right;
        __names_row = next;
        alloc.bind_list_row(__names_row);
        alloc_tr::deallocate(alloc, ptr->__it_row, 1);
        alloc_tr::destroy(alloc, ptr->__it_row);
    }
    //names_row.erase(ptr->up->__it_row, ptr->__it_row);
    __erase_col(ptr->__it_col->up);
    // переписать 
    if (ptr->left != __null_ptr) {
        __erase_col_element(ptr->left->__it_col, ptr->__it_col);
    } else {
        Node * next = ptr->__it_col->right;
        __names_col = next;
        alloc.bind_list_col(__names_col);
        alloc_tr::deallocate(alloc, ptr->__it_col, 1);
        alloc_tr::destroy(alloc, ptr->__it_col);
    }
   /* std::cout << "list_col = {\n"; 
    print(__names_col);
    std::cout << "\n}\n";
    std::cout << "list_row = {\n";
    print(__names_row);
    std::cout << "\n}\n"; 
    */
    //names_col.erase(it.ptr->__it_col);
    //__erase_line(it.ptr->__it_row->second);
    //__erase_col(it.ptr->__it_col->second);
    --size_matrix;
}


template <typename T, typename _Alloc>
void list_matrix<T, _Alloc>::__bind_list_names_and_list_min_vals() {
    Node*tail = __min_val_col_list;
    for(Node * p = __names_col; p != __null_ptr; p = p->right) {
        p->left = tail;
        tail = tail->right;
    }
    
    tail = __min_val_row_list;
    for(Node * p = __names_row; p != __null_ptr; p = p->right) {
        p->left = tail;
        tail = tail->right;
    }
}


template <typename T, typename _Alloc>
void list_matrix<T, _Alloc>::__bind_list_and_mx() {
    Node*tail = head;
    for(Node * p = __names_col; p != __null_ptr; p = p->right) {
        p->up = tail;
        tail = tail->right;
    }
    
    tail = head;
    for(Node * p = __names_row; p != __null_ptr; p = p->right) {
        p->up = tail;
        tail = tail->down;
    }
}
/*
template<typename T, typename _Alloc>
void list_matrix<T, _Alloc>::__copy_first_line(const list_matrix<T, _Alloc> & obj) {
    if (size_matrix <= 0) {
        throw "err1";
    }
    Node *tail = nullptr;
    Node *prev = nullptr;
    auto iter_col = names_col.begin();
    auto iter_row = names_row.begin();
    auto tail_obj = obj.head;  
    head = alloc_tr::allocate(alloc, 1);
    alloc_tr::construct(alloc, head, Node{
        tail_obj->value,
        nullptr, 
        nullptr,
        nullptr,
        nullptr,
        iter_row,
        iter_col
    });
    tail = head;
    prev = tail;
    tail = tail->right;
    tail_obj = tail_obj->right;
    ++iter_col;
    for(size_t i = 1; i < size_matrix; ++i) {
        tail = alloc_tr::allocate(alloc, 1);
        alloc_tr::construct(alloc, tail, Node{
            tail_obj->value, 
            nullptr,
            nullptr,
            nullptr,
            prev,
            iter_row, 
            iter_col
        });
        prev->right = tail;
        prev = tail;
        tail = tail->right;
        tail_obj = tail_obj->right;
        ++iter_col;
    }
}
*/
template<typename T, typename _Alloc>
void list_matrix<T, _Alloc>::__clear_line(Node* row_begin) {
    Node* prev = row_begin;
    row_begin = row_begin->right;
    while(row_begin != __null_ptr) {
        alloc_tr::destroy(alloc, &prev->value); 
        alloc_tr::deallocate(alloc, prev, 1);
        prev = row_begin;
        row_begin = row_begin->right;
    }
    alloc_tr::destroy(alloc, &prev->value); 
    alloc_tr::deallocate(alloc, prev, 1);
}

template<typename T, typename _Alloc>
int list_matrix<T, _Alloc>::__parsing_number(std::ifstream &ifs) {
    while(ifs.peek() == ' ' || ifs.peek() == '\n') {
        ifs.get();   
    }
    int reg_number = 0;
    std::string reg(3, '\0');
    if (ifs.peek() == 'i') {
        reg[0] = ifs.get();
        reg[1] = ifs.get();
        reg[2] = ifs.get();
        if (reg != "inf") {
            throw "error2";
        } 
        return inf;
    } else {
        while ('0' <=  ifs.peek() && ifs.peek() <= '9') {
            reg_number  = reg_number * 10 + (ifs.get() - '0');
        
        }
        return reg_number;
    }
}

/*
template<typename T, typename _Alloc> 
void list_matrix<T, _Alloc>::__read_matrix(std::ifstream &ifs) {
    if (size_matrix <= 0) {
        return;
    }
    __ignore_spaces_until_char(ifs, '{'); 
    ifs.get(); // read '{' 

    __fill_first_line(ifs); // заполнить первую строку
    auto iter_row = ++names_row.begin(); // идём с первой строки 
    Node*head_row = head;  // указатели на начало списка (строки) 
    for(size_t i = 1; i < size_matrix; ++i) {
        Node * prev_ptr = nullptr;  // предыдущий указатель          
        Node* curr_ptr = head_row->down;  //текущий указатель 
        auto iter_col = names_col.begin(); 
        // предыдущая строка  
        for(Node*tail_col = head_row; tail_col != nullptr; tail_col = tail_col->right) {
            int temp_var = __parsing_number(ifs); 
            curr_ptr = alloc_tr::allocate(alloc, 1);
            alloc_tr::construct(alloc, curr_ptr, Node{
                temp_var,   
                tail_col,
                nullptr,
                nullptr,
                prev_ptr, 
                iter_row,
                iter_col
            });
            if (prev_ptr != nullptr) {
                prev_ptr->right = curr_ptr;
            }
            tail_col->down = curr_ptr;
            prev_ptr = curr_ptr;
            curr_ptr = curr_ptr->right;
            ++iter_col;
        }
        ++iter_row;
        head_row = head_row->down;
    }

    __ignore_spaces_until_char(ifs,'}');
    ifs.get();
    __ignore_spaces_until_char(ifs, ';');
    ifs.get();   
    __bind_list_and_mx();   // связать матрицу и списки 
}
*/
/*
template<typename T, typename _Alloc>
void list_matrix<T, _Alloc>::__fill_first_line(std::ifstream &ifs) {
    if (size_matrix <= 0) {
        throw "err1";
    }
    Node *tail = nullptr;
    Node *prev = nullptr;
    auto iter_col = names_col.begin();
    auto iter_row = names_row.begin();
    int t = __parsing_number(ifs);
    head = alloc_tr::allocate(alloc, 1);
    alloc_tr::construct(alloc, head, Node{
        t,
        nullptr, 
        nullptr,
        nullptr,
        nullptr,
        iter_row,
        iter_col
    });
    tail = head;
    prev = tail;
    tail = tail->right;
    ++iter_col;

    for(size_t i = 1; i < size_matrix; ++i, ++iter_col) {
        int temp_var = __parsing_number(ifs);
        
        tail = alloc_tr::allocate(alloc, 1);
        alloc_tr::construct(alloc, tail, Node{
            temp_var, 
            nullptr,
            nullptr,
            nullptr,
            prev,
            iter_row, 
            iter_col
        });
        prev->right = tail;
        prev = tail;
        tail = tail->right;
    }
}
*/
template<typename T, typename _Alloc>
void list_matrix<T, _Alloc>::__fill_names_col() {
    Node* element = alloc_tr::allocate(alloc, 1);
    alloc_tr::construct(alloc, element, Node{
        0, // value 
        __null_ptr,  // up 
        __null_ptr,  // right
        __null_ptr,  // down 
        __null_ptr,  // left
        __null_ptr, // iterator 
        __null_ptr // iterator
    });
    __names_col = element;
    Node*curr = __names_col;
    for(size_t i = 1; i < size_matrix; ++i) {
        Node*element = alloc_tr::allocate(alloc, 1);
        alloc_tr::construct(alloc, element, Node{
            static_cast<int>(i), __null_ptr, __null_ptr, __null_ptr, __null_ptr, __null_ptr, __null_ptr
        });
        curr->right = element;
        curr = curr->right;
    }
}

template<typename T, typename _Alloc>
void list_matrix<T, _Alloc>::__fill_min_values_col() {
    Node* element = alloc_tr::allocate(alloc, 1);
    alloc_tr::construct(alloc, element, Node{
        0, // value 
        __null_ptr,  // up 
        __null_ptr,  // right
        __null_ptr,  // down 
        __null_ptr,  // left
        __null_ptr, // iterator 
        __null_ptr // iterator
    });
    __min_val_col_list = element;
    Node*curr = __min_val_col_list;
    for(size_t i = 1; i < size_matrix; ++i) {
        Node*element = alloc_tr::allocate(alloc, 1);
        alloc_tr::construct(alloc, element, Node{
            static_cast<int>(i), __null_ptr, __null_ptr, __null_ptr, __null_ptr, __null_ptr, __null_ptr
        });
        curr->right = element;
        curr = curr->right;
    }
}


template <typename T, typename _Alloc>
void list_matrix<T, _Alloc>::__fill_names_row() {
    Node* element = alloc_tr::allocate(alloc, 1);
    alloc_tr::construct(alloc, element, Node{
        0, // value 
        __null_ptr,  // up 
        __null_ptr,  // right
        __null_ptr,  // down 
        __null_ptr,  // left
        __null_ptr, // iterator 
        __null_ptr // iterator
    });
    __names_row = element;
    Node*curr = __names_row;
    for(size_t i = 1; i < size_matrix; ++i) {
        Node*element = alloc_tr::allocate(alloc, 1);
        alloc_tr::construct(alloc, element, Node{
            static_cast<int>(i), __null_ptr, __null_ptr, __null_ptr, __null_ptr, __null_ptr, __null_ptr
        });
        curr->right = element;
        curr = curr->right;
    }
}


template <typename T, typename _Alloc>
void list_matrix<T, _Alloc>::__fill_min_values_row() {
    Node* element = alloc_tr::allocate(alloc, 1);
    alloc_tr::construct(alloc, element, Node{
        0, // value 
        __null_ptr,  // up 
        __null_ptr,  // right
        __null_ptr,  // down 
        __null_ptr,  // left
        __null_ptr, // iterator 
        __null_ptr // iterator
    });
    __min_val_row_list = element;
    Node*curr = __min_val_row_list;
    for(size_t i = 1; i < size_matrix; ++i) {
        Node*element = alloc_tr::allocate(alloc, 1);
        alloc_tr::construct(alloc, element, Node{
            0, __null_ptr, __null_ptr, __null_ptr, __null_ptr, __null_ptr, __null_ptr
        });
        curr->right = element;
        curr = curr->right;
    }
}

template<typename T, typename _Alloc>
void list_matrix<T, _Alloc>::__fill_list_min_vals() { 
    __fill_min_values_col(); // заполнить список столбцов
    __fill_min_values_row(); // заполнить список строк     
}


template<typename T, typename _Alloc>
void list_matrix<T, _Alloc>::__fill_list_names() {
    __fill_names_col(); // заполнить список столбцов 
    __fill_names_row(); // заполнить список строк 
    /* 
    Node* element = alloc_tr::allocate(alloc, 1);
    alloc_tr::construct(alloc, element, Node{
        1, // value 
        nullptr,  // up 
        nullptr,  // right
        nullptr,  // down 
        nullptr,  // left
        nullptr, // iterator 
        nullptr // iterator
    });
    __names_row = element;
    Node*curr = __names_row;
    for(size i = 1; i < size_matrix; ++i) {
        Node*element = alloc_tr::allocate(alloc, 1);
        alloc_tr::construct(alloc, element, Node{
            i, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
        });
        curr->right = element;
        curr = curr->right;
    }

    */
    
    /*for(size_t i = 0; i < size_matrix; ++i) {
        names_row.push_back(std::pair{static_cast<int>(i), nullptr});
        names_col.push_back(std::pair{static_cast<int>(i), nullptr});
    } */   
}

template<typename T, typename _Alloc>
void list_matrix<T, _Alloc>::__ignore_spaces_until_char(std::ifstream & ifs, char ch) {
    while(ifs.peek() != ch) { // ignore space 
        char s = ifs.get();
        if (s != ' ' && s != '\n') {
            throw "bad_input";
        }
    }
}
/*
template<typename T, typename _Alloc>
typename list_matrix<T, _Alloc>::size_type  list_matrix<T, _Alloc>::__parsing_size(std::ifstream & ifs) {
    __ignore_spaces_until_char(ifs, '{');
    
    ifs.get(); // read '{'

    size_t reg_num = 0;
    while('0' <= ifs.peek() && ifs.peek() <= '9') {
        reg_num = reg_num * 10 + (ifs.get() - '0');
    }
    
    __ignore_spaces_until_char(ifs, '}');

    ifs.get(); // read '}'

    __ignore_spaces_until_char(ifs, ';');

    ifs.get(); // read ';'

    return reg_num;
}
*/
template<typename T, typename _Alloc>
std::ostream& operator<<(std::ostream &os, list_matrix<T, _Alloc> & mx) {
    /*const auto& ref_col_li = mx.get_list_col();
    for(const auto & [names, ptr] :  ref_col_li) {
        std::cout.width(7);
        std::cout << names << " ";
    } 
    std::cout << std::endl;
    */
    //const auto&ref_row_li = mx.get_list_row();
    //auto it_li = ref_row_li.cbegin();

    for(auto it_row = mx.head_col(); it_row != mx.row_end(); ++it_row) {
        std::cout.width(2);
        //std::cout << it_li->first;  
        for(auto it_col = mx.get_col_iter(it_row); it_col != mx.col_end(); ++it_col) {
            std::cout.width(7);
            if (*it_col == list_matrix<T, _Alloc>::inf) {
                std::cout << "(" << "inf" << ")";
            } else if (*it_col == list_matrix<T, _Alloc>::inf - 1) {
                std::cout << "(" << "inf2" << ")";
            } else {
                std::cout << "(" << *it_col << ")";
            }
            
        }

        
        //++it_li;
        std::cout << std::endl;
    }
    return os;
}

#endif 