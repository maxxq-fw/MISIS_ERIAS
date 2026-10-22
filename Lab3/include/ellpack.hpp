#pragma once

#include "matrix.hpp"

#include <iostream>
#include <vector>

template <typename T>
class ELLPACK {
private:
    std::vector<std::vector<T>> values_;
    std::vector<std::vector<long long>> col_indices_;
    size_t width_ = 0;

public:
    ELLPACK(const Matrix<T>& matrix) {
        for (size_t row = 0; row < matrix.get_rows(); row++) {
            size_t count = 0;
            for (size_t col = 0; col < matrix.get_cols(); col++) {
                if (matrix.has_value(row, col)) count++;
            }
            if (count > width_) width_ = count;
        }

        values_.assign(matrix.get_rows(), std::vector<T>(width_, T{}));
        col_indices_.assign(matrix.get_rows(), std::vector<long long>(width_, -1));

        for (size_t row = 0; row < matrix.get_rows(); row++) {
            size_t index = 0;
            for (size_t col = 0; col < matrix.get_cols(); col++) {
                if (!matrix.has_value(row, col)) continue;

                values_[row][index] = matrix(row, col);
                col_indices_[row][index] = static_cast<long long>(col);
                index++;
            }
        }
    }

    void print(std::ostream& out) const {
        out << "ELLPACK width: " << width_ << "\n";
        out << "Values:\n";
        for (const auto& row : values_) {
            for (const T& value : row) out << value << ' ';
            out << '\n';
        }

        out << "Column indices (-1 means padding):\n";
        for (const auto& row : col_indices_) {
            for (long long col : row) out << col << ' ';
            out << '\n';
        }
    }

    const std::vector<std::vector<T>>& get_values() const { return values_; }
    const std::vector<std::vector<long long>>& get_col_indices() const { return col_indices_; }
    size_t get_width() const { return width_; }
};
