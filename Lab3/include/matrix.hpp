#pragma once

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

template <typename T>
class Matrix {
private:
    size_t rows_ = 0, cols_ = 0;
    std::vector<T> matrix_;

public:
    Matrix() = default;
    Matrix(size_t rows, size_t cols) : rows_(rows), cols_(cols), matrix_(rows * cols) {}

    friend std::istream& operator>>(std::istream& in, Matrix<T>& matrix) {
        for (size_t row = 0; row < matrix.rows_; row++) {
            for (size_t col = 0; col < matrix.cols_; col++) {
                std::string token;
                if (!(in >> token)) return in;

                if (token == "NULL" || token == "null") {
                    matrix(row, col) = T{};
                    continue;
                }

                std::istringstream value_stream(token);
                T value;
                char extra;

                if (!(value_stream >> value) || (value_stream >> extra)) {
                    in.setstate(std::ios::failbit);
                    return in;
                }
                matrix(row, col) = value;
            }
        }
        return in;
    }

    friend std::ostream& operator<<(std::ostream& out, const Matrix<T>& matrix) {
        for (size_t row = 0; row < matrix.rows_; row++) {
            for (size_t col = 0; col < matrix.cols_; col++)
                out << std::setw(5) << matrix(row, col);
            out << '\n';
        }
        return out;
    }

    T& operator()(size_t row, size_t col) { return matrix_[row * cols_ + col]; }
    const T& operator()(size_t row, size_t col) const { return matrix_[row * cols_ + col]; }

    bool has_value(size_t row, size_t col) const { return matrix_[row * cols_ + col] != T{}; }

    size_t get_rows() const { return this->rows_; }
    size_t get_cols() const { return this->cols_; }
};
