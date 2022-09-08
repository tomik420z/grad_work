#include "little_alg.hpp"



int main() {
    try {
    matrix_dist mx("inp.txt");
   // std::cout << mx << std::endl;
    little_alg alg(mx.get_matrix_dist());
    alg();
    } catch(const char * e) {
        std::cerr << e << std::endl;
    }
    return 0;
}