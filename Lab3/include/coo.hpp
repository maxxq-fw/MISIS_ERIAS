#pragma once

#include "matrix.hpp"

#include <iostream>
#include <vector>

template <typename T>
class COO {
private:
    std::vector<T> values_;
    std::vector<size_t> row_indices_;
    std::vector<size_t> col_indices_;

public:
    COO(const Matrix<T>& matrix) {
        for (size_t row = 0; row < matrix.get_rows(); row++) {
            for (size_t col = 0; col < matrix.get_cols(); col++) {
                if (!matrix.has_value(row, col)) continue;

                values_.push_back(matrix(row, col));
                row_indices_.push_back(row);
                col_indices_.push_back(col);
            }
        }
    }

    void print(std::ostream& out) const {
        out << "Values: ";
        for (const T& value : values_) out << value << ' ';
        out << "\nRow indices: ";
        for (size_t row : row_indices_) out << row << ' ';
        out << "\nColumn indices: ";
        for (size_t col : col_indices_) out << col << ' ';
        out << "\nNonzero entries: " << values_.size() << "\n";
    }

    const std::vector<T>& get_values() const { return values_; }
    const std::vector<size_t>& get_row_indices() const { return row_indices_; }
    const std::vector<size_t>& get_col_indices() const { return col_indices_; }
};
