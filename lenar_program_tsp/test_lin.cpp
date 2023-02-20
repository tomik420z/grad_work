#include "set_linear_eq.h"
#include "restrictions.h"
int main(int argc, char* argv[]) {
    set_linear_eq set_eq;
    set_eq.reserve(10);
    set_eq.add_new_restriction(linear_equation<EQUAL>({std::pair{1,2}, std::pair{1,4}}, 2));
    set_eq.add_new_restriction(linear_equation<EQUAL>({std::pair{1,25}, std::pair{1,4}}, 2));
    set_eq.add_new_restriction(linear_equation<EQUAL>({std::pair{1,25}, std::pair{1,4}}, 2));
    for(const auto& line: set_eq) {
        std::cout << std::get<EQUAL>(line) << std::endl;
    }
    return 0;
}