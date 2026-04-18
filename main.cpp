#include "expression.hpp"
#include <iostream>

int main() {
    auto expr = std::make_unique<Const>(Int128(2)) * 
                std::make_unique<Variable>("x") + 
                std::make_unique<Const>(Int128(1));

    std::map<std::string, Int128> vars;
    vars["x"] = Int128(100);
    vars["y"] = Int128(42);

    Int128 result = expr->eval(vars);
    std::cout << *expr << " = " << result << std::endl;

    auto expr2 = expr->clone();

    Int128 a = Int128(-123);
    Int128 b = Int128("456");
    Int128 c = a + b;
    std::cout << a << " + " << b << " = " << c << std::endl;

    return 0;
}
