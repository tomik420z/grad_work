#include <iostream>
#include "fractional_num.h"
#include "restrictions.h"
int main() {
    
    fractional_num x(INT32_MAX - 1, INT32_MAX);
    x.round();
    std::cout << x << std::endl;
    return 0;
}