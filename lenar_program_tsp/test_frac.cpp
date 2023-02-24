#include <iostream>
#include <cmath>
#include <iomanip>
//#include "fractional_num.h"
//#include "restrictions.h"
static constexpr double eps = 0.0001;
int main() {
    
    double x = 2.99999;

    x = std::round(x / 0.0025) * 0.0025;
    //bool z = std::modf(x, &p) < eps;
    std::cout << std::setprecision(20) << x << std::endl;
    return 0;
}