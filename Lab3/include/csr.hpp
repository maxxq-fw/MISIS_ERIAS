#pragma once

#include "matrix.hpp"

#include <iostream>
#include <vector>

template <typename T>
class CSR {
private:
    std::vector<T> values_;
    std::vector<size_t> col_indices_;
    std::vector<size_t> row_ptr_;

public:
    CSR(const Matrix<T>& matrix) {
        row_ptr_.push_back(0);

        for (size_t row = 0; row < matrix.get_rows(); row++) {
            for (size_t col = 0; col < matrix.get_cols(); col++) {
                if (!matrix.has_value(row, col)) continue;

                values_.push_back(matrix(row, col));
                col_indices_.push_back(col);
            }
            row_ptr_.push_back(values_.size());
        }
    }

    void print(std::ostream& out) const {
        out << "Values: ";
        for (const T& value : values_) out << value << ' ';
        out << "\nColumn indices: ";
        for (size_t col : col_indices_) out << col << ' ';
        out << "\nRow pointers: ";
        for (size_t ptr : row_ptr_) out << ptr << ' ';
        out << "\nNonzero entries: " << values_.size() << "\n";
    }

    const std::vector<T>& get_values() const { return values_; }
    const std::vector<size_t>& get_col_indices() const { return col_indices_; }
    const std::vector<size_t>& get_row_ptr() const { return row_ptr_; }
};
