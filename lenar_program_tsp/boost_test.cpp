
#include <boost/algorithm/is_palindrome.hpp>
#include <boost/algorithm/minmax_element.hpp>
#include <boost/multiprecision/integer.hpp>
#include <boost/rational.hpp>

#include <boost/ratio/detail/mpl/abs.hpp>
#include <vector>
#include <iostream>
using boost::rational;
using boost::multiprecision::int1024_t;

rational<int1024_t> get_frac_part(const rational<int1024_t>& x) {
    return rational<int1024_t>(x.numerator() % x.denominator(), x.denominator());
}

rational<int1024_t> floor(const rational<int1024_t>& x) {
    if (x.numerator() % x.denominator() == 0) {
        return x;
    } else {
        return rational<int1024_t>((x.numerator() / x.denominator()) - 1, 1);
    }
}


int main() {
    rational<int1024_t> r(-50, 33);
    
    std::cout << floor(r) << std::endl;
    return 0;
}