#include <iostream>
#include <cmath>
#include <iomanip>
#include "parallel_algorithm.h"
#include <random>
#include <ctime>
#include <vector>
#include <algorithm>


int main() {
    std::vector<int> v;
    std::srand(time(NULL));
    v.reserve(100'000'000);
    for(size_t i = 0; i < 1'000'000'000; ++i) {
        v.push_back(std::rand());
    }
    std::clock_t t1 = clock();
    auto it = std::max_element(std::begin(v), std::end(v));
    std::clock_t t2 = clock();
    auto jt = parallel_max_element(std::begin(v), std::end(v));
    std::clock_t t3 = clock();
    std::cout  <<(*it == *jt) << " "<< t2 - t1 << " " << t3 - t2 << std::endl;
    return 0;
}