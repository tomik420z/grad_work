#pragma once 
#ifndef FRACTIONAL_NUM_H
#define FRACTIONAL_NUM_H
#include <iostream>
#include <cmath>
#include <limits>
#include <numeric>

class fractional_num
{
private:
    using __integer = long long; 
    __integer numerator; // числитель
    size_t denominator; // знаменатель
public:

    /// @brief конструктор по умолчанию 
    fractional_num() noexcept : numerator(0), denominator(1) {}

    fractional_num(std::initializer_list<int> && init_li) = delete;
    /* {
        auto it = init_li.begin();
        numerator = *it;
        denominator = *(++it);
        ratsimp();
    }*/

    fractional_num(int val) noexcept : numerator(static_cast<__integer>(val)), denominator(1) {}


    fractional_num(__integer numerator, size_t denominator) noexcept : numerator(numerator), denominator(denominator) {
        
        if (denominator == 0) {
            std::cout << "init 0";
        }
        ratsimp(); 
    } 

    fractional_num(const fractional_num& num) noexcept : numerator(num.numerator), denominator(num.denominator) {}

    fractional_num(long long val) noexcept : numerator(val), denominator(1) {}

    //fractional_num(int val) noexcept : numerator(static_cast<__integer>(val)), denominator(1) {}

    fractional_num(size_t val) noexcept : numerator(static_cast<__integer>(val)), denominator(1) {}

    fractional_num(double x) = delete;

    /// @brief упростить дробь 
    void ratsimp() {
        if (denominator == 0) {
            std::cout << "ratsimp zero" << std::endl;
        }
        if (numerator == 0) { denominator = 1; return; }
        size_t num = std::gcd(numerator, denominator); // НОД
        numerator /= static_cast<__integer>(num);
        denominator /= num;
        //std::cout << numerator << "  " << denominator << std::endl;
    }

    /// @brief опрератор копирования 
    /// @param num 
    /// @return ссылку на сам класс 
    fractional_num& operator=(const fractional_num& num) noexcept {
        numerator = num.numerator;
        denominator = num.denominator;
        return *this;
    }
    
    fractional_num& operator=(int num) noexcept {
        numerator = static_cast<__integer>(num);
        denominator = 1;
        return *this;
    }

    bool check_less_zero() const noexcept {
        return numerator < 0;
    }

    bool check_zero() const noexcept {
        return numerator == 0;
    }

    bool check_more_zero() const noexcept {
        return numerator > 0;
    }

    

    fractional_num& operator=(double x) {
        numerator = static_cast<__integer>(x);
        denominator = 1;
        return *this;
    }
    // переписать !!!!!! неправильно 
    fractional_num operator+(const fractional_num& num) const noexcept {
        
        fractional_num res;
        size_t __gcd = std::gcd(denominator, num.denominator);
        size_t __b = denominator / __gcd;
        size_t __d = num.denominator / __gcd;
        res.numerator = numerator * static_cast<__integer>(__d) + num.numerator * static_cast<__integer>(__b);
        res.denominator = (denominator / __gcd) * num.denominator;
        res.ratsimp();
        return res;
    }

    fractional_num operator-(const fractional_num& num) const noexcept {
        //std::cout << "-";
        fractional_num res;
        size_t __gcd = std::gcd(denominator, num.denominator);
        size_t __b = denominator / __gcd;
        size_t __d = num.denominator / __gcd;
        res.numerator = numerator * static_cast<__integer>(__d) - num.numerator * static_cast<__integer>(__b);
        res.denominator = (denominator / __gcd) * num.denominator;
        res.ratsimp();
        return res;
    }


    fractional_num operator*(const fractional_num& num) const noexcept {
        fractional_num res;
        res.numerator = numerator * num.numerator;
        res.denominator = denominator * num.denominator;
        res.ratsimp();
        return res;
    }

    /// @brief делние 
    /// @param num делитель
    /// @return 
    fractional_num operator/(const fractional_num& num) const noexcept {
        fractional_num res;
        if (num.numerator < 0) {
            res.numerator = - numerator *  static_cast<__integer>(num.denominator);
            res.denominator = denominator * static_cast<size_t>(std::abs(num.numerator));
            res.ratsimp();
        } else {
            res.numerator = numerator * static_cast<__integer>(num.denominator);
            res.denominator = denominator * static_cast<size_t>(num.numerator);
            res.ratsimp();
        }
        return res;
    }

    
    fractional_num&operator-() {
        numerator = -numerator;
        return *this;
    }

    fractional_num& operator+=(const fractional_num& num) noexcept {
        size_t __gcd = std::gcd(denominator, num.denominator);
        size_t __b = denominator / __gcd;
        size_t __d = num.denominator / __gcd;
        numerator = numerator * static_cast<__integer>(__d) + num.numerator * static_cast<__integer>(__b);
        denominator = (denominator / __gcd) * num.denominator;
        ratsimp();
        return *this;
    }

    fractional_num& operator-=(const fractional_num& num) noexcept {
        size_t __gcd = std::gcd(denominator, num.denominator);
        size_t __b = denominator / __gcd;
        size_t __d = num.denominator / __gcd;
        numerator = numerator * static_cast<__integer>(__d) - num.numerator * static_cast<__integer>(__b);
        denominator = (denominator / __gcd) * num.denominator;
        ratsimp();
        return *this;
    }

    fractional_num& operator*=(const fractional_num& num) noexcept {
        numerator *= num.numerator;
        denominator *= num.denominator;
        ratsimp();
        return *this;
    }

    fractional_num& operator/=(const fractional_num& num) {
        if(num.numerator < 0) {
            numerator *=  -static_cast<__integer>(num.denominator);
            denominator *= static_cast<size_t>(std::abs(num.numerator));
        } else {
            numerator *= static_cast<__integer>(num.denominator);
            denominator *= static_cast<size_t>(num.numerator);
        }

        if (denominator == 0) {
            throw "/zero";
        }

        ratsimp();
        return *this;
    }

    bool check_integer() const noexcept {
        return numerator % static_cast<__integer>(denominator) == 0;
    }

    fractional_num get_frac_part() const noexcept {
        // выделить целую часть 
        __integer int_part = numerator / static_cast<__integer>(denominator);
        return fractional_num(numerator - int_part * static_cast<__integer>(denominator), denominator);
    }

    long long get_integer_part() const noexcept {
        return numerator / static_cast<__integer>(denominator);
    }
    
    ~fractional_num() {}

    friend std::ostream& operator<<(std::ostream&, const fractional_num&);

    bool operator==(const fractional_num& rhs) const noexcept {
        return numerator == rhs.numerator && denominator == rhs.denominator;
    }

    bool operator!=(const fractional_num& rhs) const noexcept {
        return !operator==(rhs);
    }

    bool operator<(const fractional_num& rhs) const noexcept {
        return static_cast<double>(numerator) / static_cast<double>(denominator) < static_cast<double>(rhs.numerator) / static_cast<double>(rhs.denominator); 
    }

    bool operator>(const fractional_num& rhs) const noexcept {
       return static_cast<double>(numerator) / static_cast<double>(denominator) > static_cast<double>(rhs.numerator) / static_cast<double>(rhs.denominator);  
    }

    
    bool operator==(long long val) const noexcept {
        return numerator == val && denominator == 1;
    }

    bool operator!=(long long val) const noexcept {
        return !operator==(val);
    }

    bool operator<(long long val) const noexcept {
        return static_cast<double>(numerator) / static_cast<double>(denominator) < static_cast<double>(val); 
    }

    bool operator>(long long val) const noexcept {
        return static_cast<double>(numerator) / static_cast<double>(denominator) > static_cast<double>(val); 
    }

    fractional_num& operator=(long long lhs) {
        numerator = lhs;
        denominator = 1;
        return *this;
    }

    static fractional_num absf(const fractional_num& num) {
        return fractional_num(std::abs(num.numerator), num.denominator);
    }

    static fractional_num floor(const fractional_num& num) {
        return fractional_num(static_cast<__integer>(std::floor(static_cast<double>(num.numerator) / static_cast<double>(num.denominator))));
    }

    void round() {
        if ((std::abs(numerator) > INT32_MAX / 10) || (denominator > UINT32_MAX / 10)) {
            double div = static_cast<double>(numerator) / static_cast<double>(denominator);
            double integer_part;
            double frac_part = std::modf(div, &integer_part);
            frac_part *= 10'000;
            denominator = 10'000;
            numerator = static_cast<__integer>(integer_part) * static_cast<__integer>(denominator) + static_cast<__integer>(frac_part);
        } 
        ratsimp();
    }
    
};


std::ostream& operator<<(std::ostream& os, const fractional_num& num) {
    if (num.numerator == 0) {
        os << 0;
    }
    else if (num.denominator == 1) {
        os << num.numerator;
    } else { 
        os << num.numerator << '/' << num.denominator;
    }
    return os;
}

#endif