#pragma once

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <istream>
#include <memory>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

struct Measures {
    double md = 0;
    double mnd = 0;
};

enum class Operation { Fact, Not, And, Or };

struct Expression {
    Operation operation = Operation::Fact;
    int rule = 0;
    Measures value;
    std::unique_ptr<Expression> left;
    std::unique_ptr<Expression> right;
};

struct Assumption {
    std::string expression;
    std::unique_ptr<Expression> root;
    Measures result;
};

class ParseError : public std::runtime_error {
public:
    explicit ParseError(const std::string& message) : std::runtime_error(message) {}
};

inline const std::vector<std::string>& getRules() {
    static const std::vector<std::string> rules = {
        "X lives in Y",
        "X is a member of party Z",
        "X is T years old",
        "X is a sole proprietor",
        "X will vote for candidate B"
    };
    return rules;
}

inline std::string trim(const std::string& text) {
    size_t first = text.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = text.find_last_not_of(" \t\r\n");
    return text.substr(first, last - first + 1);
}

inline std::string formatNumber(double value) {
    std::ostringstream out;
    out << std::fixed << std::setprecision(4) << value;
    return out.str();
}

class ExpressionParser {
private:
    const std::string& text_;
    size_t pos_ = 0;

    void skipSpaces() {
        while (pos_ < text_.size() && std::isspace(static_cast<unsigned char>(text_[pos_]))) pos_++;
    }

    bool consume(char symbol) {
        skipSpaces();
        if (pos_ >= text_.size() || text_[pos_] != symbol) return false;
        pos_++;
        return true;
    }

    bool consumeWord(const std::string& word) {
        skipSpaces();
        if (pos_ + word.size() > text_.size()) return false;
        for (size_t i = 0; i < word.size(); i++) {
            if (std::tolower(static_cast<unsigned char>(text_[pos_ + i])) != word[i]) return false;
        }
        size_t end = pos_ + word.size();
        if (end < text_.size() && (std::isalnum(static_cast<unsigned char>(text_[end])) || text_[end] == '_')) return false;
        pos_ = end;
        return true;
    }

    double parseNumber() {
        skipSpaces();
        const char* begin = text_.c_str() + pos_;
        char* end = nullptr;
        double value = std::strtod(begin, &end);
        if (end == begin || !std::isfinite(value) || value < 0 || value > 1) throw ParseError("MD and MND must be numbers from 0 to 1");
        pos_ += static_cast<size_t>(end - begin);
        return value;
    }

    std::unique_ptr<Expression> parseFact() {
        skipSpaces();
        size_t start = pos_;
        while (pos_ < text_.size() && std::isdigit(static_cast<unsigned char>(text_[pos_]))) pos_++;
        if (start == pos_) throw ParseError("expected a rule number or '(' ");
        std::string number = text_.substr(start, pos_ - start);
        if (number.size() > 1 || number[0] < '1' || number[0] > '4') throw ParseError("rule must be numbered from 1 to 4 (rule 5 is the hypothesis)");
        if (!consume('(')) throw ParseError("expected '(' after the rule number");
        double md = parseNumber();
        if (!consume(',')) throw ParseError("expected ',' between MD and MND");
        double mnd = parseNumber();
        if (!consume(')')) throw ParseError("expected ')' after MND");
        auto node = std::make_unique<Expression>();
        node->operation = Operation::Fact;
        node->rule = number[0] - '0';
        node->value = {md, mnd};
        return node;
    }

    std::unique_ptr<Expression> parsePrimary() {
        if (!consume('(')) return parseFact();
        auto node = parseOr();
        if (!consume(')')) throw ParseError("expected ')' to close the group");
        return node;
    }

    std::unique_ptr<Expression> parseUnary() {
        if (!consumeWord("not")) return parsePrimary();
        auto node = std::make_unique<Expression>();
        node->operation = Operation::Not;
        node->left = parseUnary();
        return node;
    }

    std::unique_ptr<Expression> parseAnd() {
        auto left = parseUnary();
        while (consumeWord("and")) {
            auto node = std::make_unique<Expression>();
            node->operation = Operation::And;
            node->left = std::move(left);
            node->right = parseUnary();
            left = std::move(node);
        }
        return left;
    }

    std::unique_ptr<Expression> parseOr() {
        auto left = parseAnd();
        while (consumeWord("or")) {
            auto node = std::make_unique<Expression>();
            node->operation = Operation::Or;
            node->left = std::move(left);
            node->right = parseAnd();
            left = std::move(node);
        }
        return left;
    }

public:
    explicit ExpressionParser(const std::string& text) : text_(text) {}

    std::unique_ptr<Expression> parse() {
        auto node = parseOr();
        skipSpaces();
        if (pos_ != text_.size()) throw ParseError("unexpected text at position " + std::to_string(pos_ + 1));
        return node;
    }
};

inline Measures evaluate(const Expression& node) {
    if (node.operation == Operation::Fact) return node.value;
    Measures left = evaluate(*node.left);
    if (node.operation == Operation::Not) return {1 - left.md, 1 - left.mnd};
    Measures right = evaluate(*node.right);
    if (node.operation == Operation::And) return {std::min(left.md, right.md), std::min(left.mnd, right.mnd)};
    return {std::max(left.md, right.md), std::max(left.mnd, right.mnd)};
}

inline std::string explain(const Expression& node, bool trust) {
    if (node.operation == Operation::Fact) return formatNumber(trust ? node.value.md : node.value.mnd);
    if (node.operation == Operation::Not) return "(1 - " + explain(*node.left, trust) + ")";
    std::string operation = node.operation == Operation::And ? "MIN" : "MAX";
    return operation + "(" + explain(*node.left, trust) + ", " + explain(*node.right, trust) + ")";
}

inline bool readAssumptions(std::istream& in, std::vector<Assumption>& assumptions, std::string& error) {
    std::vector<Assumption> temp;
    std::string line;
    while (std::getline(in, line)) {
        if (line.size() >= 3 && static_cast<unsigned char>(line[0]) == 0xEF && static_cast<unsigned char>(line[1]) == 0xBB && static_cast<unsigned char>(line[2]) == 0xBF) line.erase(0, 3);
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;
        if (temp.size() >= 2) { error = "at most two assumptions (E1 and E2) are allowed"; return false; }
        try {
            Assumption assumption;
            assumption.expression = line;
            ExpressionParser parser(assumption.expression);
            assumption.root = parser.parse();
            assumption.result = evaluate(*assumption.root);
            temp.push_back(std::move(assumption));
        }
        catch (const ParseError& exception) {
            error = "E" + std::to_string(temp.size() + 1) + ": " + exception.what();
            return false;
        }
    }
    if (in.bad()) { error = "failed to read input"; return false; }
    if (temp.empty()) { error = "at least one assumption is required"; return false; }
    assumptions = std::move(temp);
    return true;
}

inline Measures accumulate(Measures first, Measures second) {
    return {first.md + second.md * (1 - first.md), first.mnd + second.mnd * (1 - first.mnd)};
}

inline void runExpertSystem(const std::vector<Assumption>& assumptions, std::ostream& out) {
    out << "Knowledge base:\n";
    const auto& rules = getRules();
    for (size_t i = 0; i < rules.size(); i++) out << i + 1 << ". " << rules[i] << (i + 1 == rules.size() ? " (H)" : "") << '\n';

    for (size_t i = 0; i < assumptions.size(); i++) {
        const Assumption& assumption = assumptions[i];
        std::string label = "H:E" + std::to_string(i + 1);
        out << "\nE" << i + 1 << ": IF " << assumption.expression << "\nTHEN " << rules.back() << '\n';
        out << "MD[" << label << "] = " << explain(*assumption.root, true) << " = " << formatNumber(assumption.result.md) << '\n';
        out << "MND[" << label << "] = " << explain(*assumption.root, false) << " = " << formatNumber(assumption.result.mnd) << '\n';
        out << "KU[" << label << "] = " << formatNumber(assumption.result.md) << " - " << formatNumber(assumption.result.mnd) << " = " << formatNumber(assumption.result.md - assumption.result.mnd) << '\n';
    }

    Measures result = assumptions.front().result;
    std::string label = "H:E1";
    if (assumptions.size() == 2) {
        Measures second = assumptions[1].result;
        result = accumulate(result, second);
        label = "H:E1,E2";
        out << "\nCombining E1 and E2:\n";
        out << "MD[" << label << "] = " << formatNumber(assumptions[0].result.md) << " + " << formatNumber(second.md) << " * (1 - " << formatNumber(assumptions[0].result.md) << ") = " << formatNumber(result.md) << '\n';
        out << "MND[" << label << "] = " << formatNumber(assumptions[0].result.mnd) << " + " << formatNumber(second.mnd) << " * (1 - " << formatNumber(assumptions[0].result.mnd) << ") = " << formatNumber(result.mnd) << '\n';
    }
    double ku = result.md - result.mnd;
    out << "\nFinal KU[" << label << "] = " << formatNumber(result.md) << " - " << formatNumber(result.mnd) << " = " << formatNumber(ku) << '\n';
    if (ku > 1e-12) out << "Conclusion: H has positive support.\n";
    else if (ku < -1e-12) out << "Conclusion: H has negative support.\n";
    else out << "Conclusion: H is neither supported nor refuted.\n";
}
