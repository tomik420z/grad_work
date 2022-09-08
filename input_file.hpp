#ifndef INPUT_FILE
#define INPUT_FILE

#include "file_include.hpp"

class matrix_dist {
public:
    using size_type = size_t;
    using value_type = int;
    using line_t = std::vector<value_type>;
    using matix_t = std::vector<line_t>;
protected:
    matix_t dist_matrix;
    size_type size_matrix;

    static void __ignore_spaces_until_char(std::istream &, char);
    void __parsing_size_mx(std::ifstream &, matrix_dist &);
    void __print_line(std::ostream&, const line_t &) const;
public:
    matrix_dist(const std::string & file_name) {
        std::ifstream ifs(file_name.c_str());
        if (!ifs.is_open()) {
            throw "bad_open_file";
        } 
        
        ifs >> *this;
        ifs.close();
    }

    matix_t& get_matrix_dist() noexcept { return dist_matrix; }

    line_t& operator[](size_t i) {
        return dist_matrix[i];
    }

    const line_t& operator[](size_t i) const {
        return dist_matrix[i];
    }

    ~matrix_dist() {}

    friend std::ifstream & operator>>(std::ifstream &, matrix_dist &);
    friend std::ostream &operator<<(std::ostream &, const matrix_dist&);
    friend std::ostream & operator<<(std::ostream & os, const line_t &l);
};

void matrix_dist::__ignore_spaces_until_char(std::istream & ifs, char ch) {
    while(ifs.peek() != ch) { // ignore space 
        char s = ifs.get();
        if (s != ' ' && s != '\n') {
            throw "bad_input";
        }
    }
}

void matrix_dist::__parsing_size_mx(std::ifstream & ifs, matrix_dist & mx) {
    __ignore_spaces_until_char(ifs, '{');
    
    ifs.get(); // read '{'

    size_t reg_num = 0;
    while('0' <= ifs.peek() && ifs.peek() <= '9') {
        reg_num = reg_num * 10 + (ifs.get() - '0');
    }
    
    __ignore_spaces_until_char(ifs, '}');

    ifs.get();

    mx.size_matrix = reg_num;
}

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
    
    matrix_dist::__ignore_spaces_until_char(ifs, '}');
    ifs.get();

    return ifs;
}

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

std::ostream & operator<<(std::ostream & os, const matrix_dist & mx) {
    const auto & mx_ref = mx.dist_matrix;
    std::for_each(std::begin(mx_ref), std::end(mx_ref) - 1, [&os](const auto & vec){
        os << vec << std::endl;
    });

    os << mx_ref.back();

    return os;
}

#endif