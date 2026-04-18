#pragma once
#include "int128.hpp"
#include <string>
#include <map>
#include <memory>
#include <ostream>

class Expression {
public:
    virtual ~Expression() = default;
    virtual Int128 eval(const std::map<std::string, Int128>& vars) const = 0;
    virtual std::unique_ptr<Expression> clone() const = 0;
    virtual std::string to_string() const = 0;
};

class Const : public Expression {
private:
    Int128 m_value;

public:
    explicit Const(const Int128& val) : m_value(val) {}

    Int128 eval(const std::map<std::string, Int128>&) const override {
        return m_value;
    }

    std::unique_ptr<Expression> clone() const override {
        return std::make_unique<Const>(m_value);
    }

    std::string to_string() const override {
        return m_value.str();
    }
};

class Variable : public Expression {
private:
    std::string m_name;

public:
    explicit Variable(const std::string& name) : m_name(name) {}

    Int128 eval(const std::map<std::string, Int128>& vars) const override {
        return vars.at(m_name);
    }

    std::unique_ptr<Expression> clone() const override {
        return std::make_unique<Variable>(m_name);
    }

    std::string to_string() const override {
        return m_name;
    }
};

class Negate : public Expression {
private:
    std::unique_ptr<Expression> m_expr;

public:
    explicit Negate(std::unique_ptr<Expression> expr) : m_expr(std::move(expr)) {}

    Int128 eval(const std::map<std::string, Int128>& vars) const override {
        return -m_expr->eval(vars);
    }

    std::unique_ptr<Expression> clone() const override {
        return std::make_unique<Negate>(m_expr->clone());
    }

    std::string to_string() const override {
        return "(-" + m_expr->to_string() + ")";
    }
};

class Add : public Expression {
private:
    std::unique_ptr<Expression> m_left;
    std::unique_ptr<Expression> m_right;

public:
    Add(std::unique_ptr<Expression> left, std::unique_ptr<Expression> right)
        : m_left(std::move(left)), m_right(std::move(right)) {}

    Int128 eval(const std::map<std::string, Int128>& vars) const override {
        return m_left->eval(vars) + m_right->eval(vars);
    }

    std::unique_ptr<Expression> clone() const override {
        return std::make_unique<Add>(m_left->clone(), m_right->clone());
    }

    std::string to_string() const override {
        return "(" + m_left->to_string() + " + " + m_right->to_string() + ")";
    }
};

class Subtract : public Expression {
private:
    std::unique_ptr<Expression> m_left;
    std::unique_ptr<Expression> m_right;

public:
    Subtract(std::unique_ptr<Expression> left, std::unique_ptr<Expression> right)
        : m_left(std::move(left)), m_right(std::move(right)) {}

    Int128 eval(const std::map<std::string, Int128>& vars) const override {
        return m_left->eval(vars) - m_right->eval(vars);
    }

    std::unique_ptr<Expression> clone() const override {
        return std::make_unique<Subtract>(m_left->clone(), m_right->clone());
    }

    std::string to_string() const override {
        return "(" + m_left->to_string() + " - " + m_right->to_string() + ")";
    }
};

class Multiply : public Expression {
private:
    std::unique_ptr<Expression> m_left;
    std::unique_ptr<Expression> m_right;

public:
    Multiply(std::unique_ptr<Expression> left, std::unique_ptr<Expression> right)
        : m_left(std::move(left)), m_right(std::move(right)) {}

    Int128 eval(const std::map<std::string, Int128>& vars) const override {
        return m_left->eval(vars) * m_right->eval(vars);
    }

    std::unique_ptr<Expression> clone() const override {
        return std::make_unique<Multiply>(m_left->clone(), m_right->clone());
    }

    std::string to_string() const override {
        return "(" + m_left->to_string() + " * " + m_right->to_string() + ")";
    }
};

class Divide : public Expression {
private:
    std::unique_ptr<Expression> m_left;
    std::unique_ptr<Expression> m_right;

public:
    Divide(std::unique_ptr<Expression> left, std::unique_ptr<Expression> right)
        : m_left(std::move(left)), m_right(std::move(right)) {}

    Int128 eval(const std::map<std::string, Int128>& vars) const override {
        return m_left->eval(vars) / m_right->eval(vars);
    }

    std::unique_ptr<Expression> clone() const override {
        return std::make_unique<Divide>(m_left->clone(), m_right->clone());
    }

    std::string to_string() const override {
        return "(" + m_left->to_string() + " / " + m_right->to_string() + ")";
    }
};

inline std::unique_ptr<Expression> operator+(std::unique_ptr<Expression> left, std::unique_ptr<Expression> right) {
    return std::make_unique<Add>(std::move(left), std::move(right));
}

inline std::unique_ptr<Expression> operator-(std::unique_ptr<Expression> left, std::unique_ptr<Expression> right) {
    return std::make_unique<Subtract>(std::move(left), std::move(right));
}

inline std::unique_ptr<Expression> operator*(std::unique_ptr<Expression> left, std::unique_ptr<Expression> right) {
    return std::make_unique<Multiply>(std::move(left), std::move(right));
}

inline std::unique_ptr<Expression> operator/(std::unique_ptr<Expression> left, std::unique_ptr<Expression> right) {
    return std::make_unique<Divide>(std::move(left), std::move(right));
}

inline std::unique_ptr<Expression> operator-(std::unique_ptr<Expression> expr) {
    return std::make_unique<Negate>(std::move(expr));
}

inline std::ostream& operator<<(std::ostream& os, const Expression& expr) {
    return os << expr.to_string();
}
