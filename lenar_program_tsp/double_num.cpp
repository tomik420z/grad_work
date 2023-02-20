#include <iostream>
#include <cmath>
class double_num {
protected:
    double val;
    static constexpr double eps = 0.00001;
    void __round() {
        double frac = std::modf(val, NULL);
        if (std::abs(frac) < eps) {
            val -= frac;
        }
    }
public:
    double_num(int int_num) noexcept : val(static_cast<double>(int_num)) {}
    double_num() noexcept : val(0.0) {}
    double_num(double x) noexcept : val(x) {}

    double_num operator+(const double_num & d) {
        double_num res(d.val + val);
        //res.__round;
        return res;
    } 

    void print() const noexcept {
        std::cout << val << std::endl;
    }

    ~double_num() {}
};

int main() {
    double_num x = 0.000000001;
    double_num y = 0.1;
    auto z = x + y; 
    z.print();
    


    return 0;
}