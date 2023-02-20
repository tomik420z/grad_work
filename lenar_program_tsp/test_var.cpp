#include <iostream>
#include <limits>
#include <numeric>
#include "linear_equation.h"
constexpr int n = 5;

int main() {
    linear_equation<EQUAL> lin({std::pair{0, 1}, std::pair{0, 3}, std::pair{3,4}}, 2);
    
    std::cout << lin << std::endl;
    


    return 0;
}