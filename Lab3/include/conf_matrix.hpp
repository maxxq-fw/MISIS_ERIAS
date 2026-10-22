#pragma once

#include "matrix.hpp"

#include <cstddef>
#include <iostream>
#include <vector>

template <typename T>
struct ReducedMatrix {
    Matrix<T> matrix;
    std::vector<size_t> users;
    std::vector<size_t> products;
};

template <typename T>
ReducedMatrix<T> reduceMatrix(const Matrix<T>& matrix, double threshold = 3.0) {
    ReducedMatrix<T> result;

    for (size_t col = 0; col < matrix.get_cols(); col++) {
        double sum = 0;
        size_t count = 0;

        for (size_t row = 0; row < matrix.get_rows(); row++) {
            if (!matrix.has_value(row, col)) continue;
            sum += static_cast<double>(matrix(row, col));
            count++;
        }

        if (count > 0 && sum / count > threshold) result.products.push_back(col);
    }

    for (size_t row = 0; row < matrix.get_rows(); row++) {
        bool active = false;
        for (size_t col : result.products) {
            if (matrix.has_value(row, col)) {
                active = true;
                break;
            }
        }
        if (active) result.users.push_back(row);
    }

    result.matrix = Matrix<T>(result.users.size(), result.products.size());

    for (size_t row = 0; row < result.users.size(); row++) {
        for (size_t col = 0; col < result.products.size(); col++) {
            size_t old_row = result.users[row];
            size_t old_col = result.products[col];

            if (matrix.has_value(old_row, old_col))
                result.matrix(row, col) = matrix(old_row, old_col);
        }
    }

    return result;
}

template <typename T>
void printReducedMatrix(const ReducedMatrix<T>& result, std::ostream& out) {
    out << "Remaining users (original indices): ";
    for (size_t row : result.users) out << row << ' ';
    out << "\nRemaining products (original indices): ";
    for (size_t col : result.products) out << col << ' ';
    out << "\nReduced matrix size: " << result.matrix.get_rows()
        << " x " << result.matrix.get_cols() << "\n";

    if (result.matrix.get_rows() == 0 || result.matrix.get_cols() == 0)
        out << "No entries meet the reduction criteria.\n";
    else
        out << result.matrix;
}
