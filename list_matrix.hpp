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
        std::list<int>::iterator __it_row;
        std::list<int>::iterator __it_col;
    };

public:
    using value_type = T;
    using allocator_type = _Alloc;
    using size_type = size_t;
    using alloc_t = typename std::allocator_traits<allocator_type>::rebind_alloc<Node>;
    using alloc_tr = std::allocator_traits<alloc_t>;
    static constexpr int inf = std::numeric_limits<int>::max();
protected:
    std::list<int> names_row;
    std::list<int> names_col;
    alloc_t alloc;
    Node* head;
    size_type size_matrix;
    
    void __read_file(const char* file_name) {
        std::ifstream buff(file_name);
        if (!buff.is_open()) {
            throw "file_note_opened";
        }
        buff >> *this;
        buff.close();
    }

    static void __ignore_spaces_until_char(std::ifstream &, char);   
    static size_type __parsing_size(std::ifstream &);
    void __fill_list_names();
    void __read_matrix(std::ifstream &);
    void __fill_first_line(std::ifstream &);
    static int __parsing_number(std::ifstream &);
    void __clear_line(Node* begin_row);
    void __copy_first_line(list_matrix & obj);

    template<bool Is_const, bool Direction>
    class __common_iterator {
    protected:
        using ptr_type = std::conditional_t<Is_const, const Node*, Node*>;
        using value_type = std::conditional_t<Is_const, const T&, T&>;
        ptr_type ptr;   
    public:
        __common_iterator() : ptr(nullptr) {}

        __common_iterator(ptr_type p_ptr) : ptr(p_ptr) {}
        
        __common_iterator(__common_iterator<Is_const, Direction>& it) : ptr(it.ptr) {}  

        value_type operator*() noexcept { return ptr->value; }

        ptr_type operator->() noexcept { return ptr; }
        

        __common_iterator& operator++() {
            if constexpr (Direction) {
                ptr = ptr->right;
            } else {
                ptr = ptr->down;
            }
            return *this; 
        }

        __common_iterator& operator++(int) {
            __common_iterator tmp = *this;
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

        
        bool operator==(const __common_iterator& rhs) {
            return ptr == rhs.ptr;
        }

        bool operator!=(const __common_iterator&rhs) {
            return !operator==(rhs);
        }

        ~__common_iterator() {}

        friend class list_matrix;
    };

public:
    using const_row_iterator = __common_iterator<true, false>;
    using row_iterator = __common_iterator<false, false>; 
    using const_col_iterator = __common_iterator<true, true>;
    using col_iterator = __common_iterator<false, true>;
     
    
    list_matrix(const char *file_name) {
        __read_file(file_name);
    }

    list_matrix(list_matrix & obj) : size_matrix(obj.size_matrix) {
        if (size_matrix <= 0) {
            return;
        }
        __copy_first_line(obj);
        auto row_obj = obj.head->down; 
        Node*head_row = head;  // указатели на начало списка (строки) 
        for(size_t i = 1; i < size_matrix; ++i) {
            Node * prev_ptr = nullptr;  // предыдущий указатель          
            Node* curr_ptr = head_row->down;  //текущий указатель 
            auto iter_col = names_col.begin(); 
            auto iter_row = ++names_row.begin(); // идём с первой строки 
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
    }

    void clear() {
        if (head == nullptr) {
            return;
        }
        auto prev = head;
        auto tail = head->down;
        while(tail != nullptr) {
            __clear_line(prev);
            prev = tail;
            tail = tail->down;
        }
        __clear_line(prev);
        head = nullptr;
    }

    ~list_matrix() {
        clear();
    }

    row_iterator row_begin(col_iterator & it_col) { return row_iterator(it_col.ptr); }
    row_iterator row_begin() noexcept { return row_iterator(head); }
    col_iterator col_begin() noexcept { return col_iterator(head); }
    col_iterator col_begin(row_iterator & it_row) noexcept { return col_iterator(it_row.ptr); }
    row_iterator row_end() noexcept { return row_iterator(nullptr); }
    col_iterator col_end() noexcept {return col_iterator(nullptr); }
    
    template<bool Is_const, bool Direction>
    friend class __common_iterator;

    friend std::ifstream &operator>><T, _Alloc>(std::ifstream &, list_matrix &);

};

template<typename T, typename _Alloc>
void list_matrix<T, _Alloc>::__copy_first_line(list_matrix<T, _Alloc> & obj) {
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
    for(size_t i = 1; i < size_matrix; ++i, ++iter_row) {
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
    }
}

template<typename T, typename _Alloc>
void list_matrix<T, _Alloc>::__clear_line(Node* row_begin) {
    Node* prev = row_begin;
    row_begin = row_begin->right;
    while(row_begin != nullptr) {
        std::cout << prev->value << " ";
        alloc_tr::destroy(alloc, &prev->value); 
        alloc_tr::deallocate(alloc, prev, 1);
        prev = row_begin;
        row_begin = row_begin->right;
    }
    std::cout << prev->value << std::endl;
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


template<typename T, typename _Alloc> 
void list_matrix<T, _Alloc>::__read_matrix(std::ifstream &ifs) {
    if (size_matrix <= 0) {
        return;
    }
    __ignore_spaces_until_char(ifs, '{'); 
    ifs.get(); // read '{'  

    __fill_first_line(ifs); // заполнить первую строку 

    Node*head_row = head;  // указатели на начало списка (строки) 
    for(size_t i = 1; i < size_matrix; ++i) {
        Node * prev_ptr = nullptr;  // предыдущий указатель          
        Node* curr_ptr = head_row->down;  //текущий указатель 
        auto iter_col = names_col.begin(); 
        auto iter_row = ++names_row.begin(); // идём с первой строки 
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
}

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

    for(size_t i = 1; i < size_matrix; ++i, ++iter_row) {
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

template<typename T, typename _Alloc>
void list_matrix<T, _Alloc>::__fill_list_names() {
    for(size_t i = 0; i < size_matrix; ++i) {
        names_row.push_back(static_cast<int>(i));
        names_col.push_back(static_cast<int>(i));
    }    
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

template<typename T, typename _Alloc>
std::ostream& operator<<(std::ostream &os, list_matrix<T, _Alloc> & mx) {
    for(auto it_row = mx.row_begin(); it_row != mx.row_end(); ++it_row) {
        for(auto it_col = mx.col_begin(it_row); it_col != mx.col_end(); ++it_col) {
            if (*it_col == list_matrix<T, _Alloc>::inf) {
                std::cout << "inf ";
            } else {
                std::cout << *it_col << " ";
            }
        }
        std::cout << std::endl;
    }
    return os;
}

/*
std::ifstream & operator>>(std::ifstream & ifs, matrix_dist & mx)  {
    mx.__parsing_size_mx(ifs, mx);
    auto & dist_ptr = mx.dist_matrix;
    auto & size_mx = mx.size_matrix;
    dist_ptr.resize(size_mx);
    std::for_each(std::begin(dist_ptr), std::end(dist_ptr), [&size_mx](auto & vec){
        vec.resize(size_mx);
    });
    matrix_dist::__ignore_spaces_until_char(ifs, '{');
    ifs.get(); // '{'

    // input matrix distance
    matrix_dist::size_type sz = dist_ptr.size();
    for(size_t i = 0; i < sz; ++i) {
        for(size_t j = 0; j < sz; ++j) {
            if (i == j) {
                matrix_dist::__ignore_spaces_until_char(ifs, 'i');
                std::string name_inf(3, '\0');
                name_inf[0] = ifs.get(); // 'i'
                name_inf[1] = ifs.get(); // 'n'
                name_inf[2] = ifs.get(); // 'f'
                if (name_inf != "inf") {
                    throw "bad_input";
                }
                dist_ptr[i][j] = std::numeric_limits<matrix_dist::value_type>::max();
            } else {
                ifs>> dist_ptr[i][j];
            }
        }
    }
    /*
    std::for_each(std::begin(dist_ptr), std::end(dist_ptr), [&ifs](auto & vec) {
        std::for_each(std::begin(vec), std::end(vec), [&ifs](auto & value){
            ifs >> value;
        }); 
    });
    */
  /*  
    matrix_dist::__ignore_spaces_until_char(ifs, '}');
    ifs.get();

    return ifs;
}
*/
/*
std::ostream & operator<<(std::ostream & os, const matrix_dist::line_t &l) {
    std::for_each(std::begin(l), std::end(l), [&os](auto num){
        if (num != std::numeric_limits<matrix_dist::value_type>::max()) {
            os << num << " ";
        } else {
            os << "inf "; 
        }
    });
    return os;
}
*/

/*
std::ostream & operator<<(std::ostream & os, const matrix_dist & mx) {
    const auto & mx_ref = mx.dist_matrix;
    std::for_each(std::begin(mx_ref), std::end(mx_ref) - 1, [&os](const auto & vec){
        os << vec << std::endl;
    });

    os << mx_ref.back();

    return os;
}
*/

#endif 