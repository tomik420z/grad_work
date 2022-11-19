#include "little_alg.hpp"
#include "list_matrix.hpp"
#include "linear_allocator.hpp"
#include "input_file.hpp"
#include "class_time.hpp"

list_matrix<int, linear_allocator<int>> create_matrix_list(const std::string & f_name) {
    matrix_dist dist_mx(f_name);
    size_t n = dist_mx.get_matrix_dist().size();    
    
    using alloc_ty = list_matrix<int, linear_allocator<int>>::alloc_node;
    alloc_ty alloc((n + 1));
    list_matrix<int, linear_allocator<int>> mx_li(dist_mx.get_matrix_dist(), std::move(alloc));
    return mx_li;
}

int main(int argc, char*argv[]) {
    if (argc > 2) {
        std::cout << "incorrect format\n";
        return -1;
    }
    try {
        auto li = create_matrix_list(argv[1]);
        execution_time tm;
        
    
        little_alg alg(li);
        tm.start();
        alg();
        tm.finish();
        std::cout << "time: "<< tm.get_str_time_in_second() << std::endl; 
        auto& v = alg.get_way();
        for(auto [v1, v2] : v) {
            std::cout << "(" << v1 << "," << v2 << ")";
        } 
        std::cout<< "   cost = " << alg.get_min_len_way() << std::endl;
            
    } catch(const char * e) {
        std::cerr << e << std::endl;
    }
    
    return 0;
}

/*
    формат ввода: исполнительный_файл.exe входной_файл 
    входной файл содержит матрицу смежности следующего формата:

    {size_matrix}; 
    {
        val_11 ... val_1n
        val_21 ... val_2n
            ...
        val_n1 ... val_nn
    };

    где size_matrix - положительное целое число, val_ij - целое положительное 


    матрица хранится в связном двумерном списке, который имеет шесть указателей: 
    вверх, вправо, вниз, влево, указаетль на столбец имён, указатель на строку имён.

    я решил сделать матрицу, основанную на списках в целях эффективного удаления строки и столбца.  
    основной алгоритм реализован в классе little_alg и вызывается с помощью перегруженного оператора круглые скобки.
    для удобства я релизовал два итератора в классе list_matrix: один из них проходит по строке, другой по столбцу.

    при корректных данных программа выводит шаги решения алгоритма. 
*/