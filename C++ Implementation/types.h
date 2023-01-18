#ifndef TYPES_H_
#define TYPES_H_

#include <stdint.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <string>

#define count_t UnsignedBigInt_t
#define limit_t unsigned long long

#define digit_t int8_t

class UnsignedBigInt_t{
public:
    std::vector<digit_t> digits;

    UnsignedBigInt_t (const UnsignedBigInt_t & ubi) {
        digits = ubi.digits;
    }
    
    UnsignedBigInt_t (unsigned int num) {
        if (num==0) return;
        while (num>0) {
            digits.push_back(num%10);
            num/=10;
        }
    }

    UnsignedBigInt_t (std::string num_str) {
        if (num_str.size()==0) return;
        for(char c: num_str) {
            digits.push_back(c-'0');
        }
        std::reverse(digits.begin(), digits.end());
    }

    size_t num_digits() const {
        return digits.size();
    }

    digit_t operator[](size_t i) const {
        if (i>=digits.size()) return 0;
        return digits[i];
    }

    UnsignedBigInt_t & operator+=(const UnsignedBigInt_t & other) {
        digit_t carry=0;
        size_t i=0;
        for(; i<this->num_digits() && i<other.num_digits(); i++) {
            digit_t digit = digits[i]+other.digits[i]+carry;
            if (digit > 9) {
                carry=1;
                digit-=10;
            }else{
                carry=0;
            }
            digits[i]=digit;
        }
        for(; i<this->num_digits() || i<other.num_digits(); i++) {
            digit_t digit = (*this)[i]+other[i]+carry;
            if (digit > 9) {
                carry=1;
                digit-=10;
            }else{
                carry=0;
            }
            if (i>=digits.size()){
                digits.push_back(digit);
            }else{
                digits[i]=digit;
            }
        }
        if(carry==1) digits.push_back(1);
        return *this;
    }

    UnsignedBigInt_t & operator+=(unsigned int other){
        UnsignedBigInt_t ubi_other(other);
        this->operator+=(ubi_other);
        return *this;
    }

    UnsignedBigInt_t & operator=(const UnsignedBigInt_t & other) {
        digits = other.digits;
        return *this;
    }

    UnsignedBigInt_t & operator=(unsigned int other) {
        digits.clear();
        this->operator+=(other);
        return *this;
    }

    UnsignedBigInt_t operator+(const UnsignedBigInt_t & other) const {
        UnsignedBigInt_t new_num (other);
        new_num += *this;
        return new_num;
    }

    UnsignedBigInt_t operator+(unsigned int other) const {
        UnsignedBigInt_t new_num (other);
        new_num += *this;
        return new_num;
    }

    std::string to_string() {
        std::string str;
        for(int i=digits.size()-1; i>=0; i--) {
            str.push_back(digits[i]+'0');
        }
        if (str.empty()) str="0";
        return str;
    }
};

std::ostream & operator<< (std::ostream & o, UnsignedBigInt_t ubi);

#endif