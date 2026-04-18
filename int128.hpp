#pragma once
#include <string>
#include <string_view>
#include <iostream>
#include <cstdint>
#include <algorithm>
#include <cstring>

class Int128 {
private:
    uint64_t m_low;
    uint64_t m_high;

    static constexpr uint64_t UINT64_MAX_VAL = 0xFFFFFFFFFFFFFFFFULL;
    static constexpr uint64_t SIGN_BIT = 0x8000000000000000ULL;

    bool is_negative() const {
        return (m_high & SIGN_BIT) != 0;
    }

    Int128 add_unsigned(const Int128& other) const {
        Int128 result;
        result.m_low = m_low + other.m_low;
        result.m_high = m_high + other.m_high;
        if (result.m_low < m_low) {
            ++result.m_high;
        }
        return result;
    }

    Int128 negate() const {
        Int128 result;
        result.m_low = ~m_low;
        result.m_high = ~m_high;
        ++result.m_low;
        if (result.m_low == 0) {
            ++result.m_high;
        }
        return result;
    }

    Int128 multiply_unsigned(const Int128& other) const {
        Int128 result(0);
        Int128 shifted = *this;
        Int128 multiplier = other;

        for (int i = 0; i < 128 && (multiplier.m_low != 0 || multiplier.m_high != 0); ++i) {
            if (multiplier.m_low & 1) {
                result = result.add_unsigned(shifted);
            }
            shifted = shifted.add_unsigned(shifted);
            multiplier.m_low = (multiplier.m_low >> 1) | (multiplier.m_high << 63);
            multiplier.m_high >>= 1;
        }
        return result;
    }

    std::pair<Int128, Int128> divide_unsigned(const Int128& divisor) const {
        if (divisor.m_low == 0 && divisor.m_high == 0) {
            throw std::runtime_error("");
        }

        Int128 quotient(0);
        Int128 remainder(0);

        for (int i = 127; i >= 0; --i) {
            uint64_t carry = remainder.m_high >> 63;
            remainder.m_high = (remainder.m_high << 1) | (remainder.m_low >> 63);
            remainder.m_low = (remainder.m_low << 1) | carry;

            if (i >= 64) {
                if ((m_high >> (i - 64)) & 1) {
                    remainder.m_low |= 1;
                }
            } else {
                if ((m_low >> i) & 1) {
                    remainder.m_low |= 1;
                }
            }

            if (remainder.m_high > divisor.m_high || 
                (remainder.m_high == divisor.m_high && remainder.m_low >= divisor.m_low)) {
                
                uint64_t prev_low = remainder.m_low;
                remainder.m_low -= divisor.m_low;
                remainder.m_high -= divisor.m_high;
                if (remainder.m_low > prev_low) {
                    --remainder.m_high;
                }

                if (i >= 64) {
                    quotient.m_high |= (1ULL << (i - 64));
                } else {
                    quotient.m_low |= (1ULL << i);
                }
            }
        }
        return {quotient, remainder};
    }

    static Int128 from_string_positive(std::string_view str) {
        Int128 result(0);
        for (char ch : str) {
            uint32_t digit = ch - '0';
            Int128 temp = result;
            for (int j = 0; j < 3; ++j) {
                result = result.add_unsigned(temp);
            }
            temp = result;
            result = result.add_unsigned(temp);
            result = result.add_unsigned(Int128(digit));
        }
        return result;
    }

public:
    Int128() : m_low(0), m_high(0) {}

    Int128(int64_t value) {
        if (value >= 0) {
            m_low = static_cast<uint64_t>(value);
            m_high = 0;
        } else {
            m_low = static_cast<uint64_t>(value);
            m_high = UINT64_MAX_VAL;
        }
    }

    explicit Int128(std::string_view str) {
        if (str.empty()) {
            m_low = m_high = 0;
            return;
        }

        bool minus = (str[0] == '-');
        std::string_view digits = minus ? str.substr(1) : str;

        *this = from_string_positive(digits);
        if (minus) {
            *this = this->negate();
        }
    }

    explicit operator int64_t() const {
        if (m_high == 0 && m_low <= 0x7FFFFFFFFFFFFFFFULL) {
            return static_cast<int64_t>(m_low);
        }
        if (m_high == UINT64_MAX_VAL && m_low >= 0x8000000000000000ULL) {
            return static_cast<int64_t>(m_low);
        }
        throw std::overflow_error("");
    }

    explicit operator double() const {
        bool minus = is_negative();
        Int128 temp = minus ? negate() : *this;
        
        double result = 0.0;
        double multiplier = 1.0;
        
        while (!(temp.m_low == 0 && temp.m_high == 0)) {
            auto [q, r] = temp.divide_unsigned(Int128(10));
            result += static_cast<double>(r.m_low) * multiplier;
            multiplier *= 10.0;
            temp = q;
        }
        
        return minus ? -result : result;
    }

    std::string str() const {
        if (m_low == 0 && m_high == 0) {
            return "0";
        }

        bool minus = is_negative();
        Int128 temp = minus ? negate() : *this;
        
        char buffer[40];
        char* ptr = buffer + sizeof(buffer) - 1;
        *ptr = '\0';

        while (!(temp.m_low == 0 && temp.m_high == 0)) {
            auto [q, r] = temp.divide_unsigned(Int128(10));
            *--ptr = '0' + static_cast<char>(r.m_low);
            temp = q;
        }

        if (minus) {
            *--ptr = '-';
        }

        return std::string(ptr);
    }

    Int128 operator+() const {
        return *this;
    }

    Int128 operator-() const {
        return negate();
    }

    Int128 operator+(const Int128& other) const {
        return add_unsigned(other);
    }

    Int128 operator-(const Int128& other) const {
        return add_unsigned(other.negate());
    }

    Int128 operator*(const Int128& other) const {
        bool sign_a = is_negative();
        bool sign_b = other.is_negative();
        
        Int128 a = sign_a ? negate() : *this;
        Int128 b = sign_b ? other.negate() : other;
        
        Int128 result = a.multiply_unsigned(b);
        return (sign_a != sign_b) ? result.negate() : result;
    }

    Int128 operator/(const Int128& other) const {
        bool sign_a = is_negative();
        bool sign_b = other.is_negative();
        
        Int128 a = sign_a ? negate() : *this;
        Int128 b = sign_b ? other.negate() : other;
        
        auto [q, r] = a.divide_unsigned(b);
        return (sign_a != sign_b) ? q.negate() : q;
    }

    Int128& operator+=(const Int128& other) {
        *this = *this + other;
        return *this;
    }

    Int128& operator-=(const Int128& other) {
        *this = *this - other;
        return *this;
    }

    Int128& operator*=(const Int128& other) {
        *this = *this * other;
        return *this;
    }

    Int128& operator/=(const Int128& other) {
        *this = *this / other;
        return *this;
    }

    bool operator==(const Int128& other) const {
        return m_low == other.m_low && m_high == other.m_high;
    }

    bool operator!=(const Int128& other) const {
        return !(*this == other);
    }

    bool operator<(const Int128& other) const {
        bool a_neg = is_negative();
        bool b_neg = other.is_negative();

        if (a_neg != b_neg) {
            return a_neg;
        }
        if (m_high != other.m_high) {
            return a_neg ? m_high > other.m_high : m_high < other.m_high;
        }
        return a_neg ? m_low > other.m_low : m_low < other.m_low;
    }

    bool operator>(const Int128& other) const {
        return other < *this;
    }

    bool operator<=(const Int128& other) const {
        return !(other < *this);
    }

    bool operator>=(const Int128& other) const {
        return !(*this < other);
    }
};

std::ostream& operator<<(std::ostream& os, const Int128& num) {
    return os << num.str();
}
