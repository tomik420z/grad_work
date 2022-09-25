#include <iostream>
#include <vector>
#include <fstream>
#include <random>

int parse_to_int(const std::string & num) {
    int reg = 0;
    for(auto dig : num) {
        reg = reg * 10 + (dig - '0');
    }
    return reg;
}

int main(int argc, char*argv[]) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(1, 1'000);
    std::ofstream buff(argv[1]);
    int sz = parse_to_int(argv[2]);
    buff << "{" << argv[2] << "};\n";
    buff << "{\n";
    for(size_t i = 0; i < sz; ++i) {
        for(size_t j = 0; j < sz; ++j) {
            if (i == j) {
                buff << "inf ";
            } else {
                buff << distrib(gen) << " "; 
            }
        }
        buff << '\n';
    }    
    buff << "};\n";
    buff.close();
}