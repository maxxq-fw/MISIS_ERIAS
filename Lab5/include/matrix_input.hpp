#pragma once

#include "matrix.hpp"

#include <exception>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>

// Format: number of users, number of products, then one row per user.
template <typename T>
bool readMatrix(std::istream& in, Matrix<T>& matrix) {
    long long rows, cols;
    if (!(in >> rows >> cols) || rows <= 0 || cols <= 0) return false;

    const size_t max_size = std::vector<T>().max_size();
    if (static_cast<unsigned long long>(rows) >
        max_size / static_cast<unsigned long long>(cols)) return false;

    try {
        Matrix<T> temp(static_cast<size_t>(rows), static_cast<size_t>(cols));
        if (!(in >> temp)) return false;
        matrix = std::move(temp);
        return true;
    }
    catch (const std::exception&) {
        return false;
    }
}

template <typename T>
bool inputMatrix(Matrix<T>& matrix) {
    std::cout << "Enter the number of users and products: ";
    // Read dimensions here so the user can enter ratings row-by-row with clear prompts.
    long long rows, cols;
    if (!(std::cin >> rows >> cols) || rows <= 0 || cols <= 0) return false;

    const size_t max_size = std::vector<T>().max_size();
    if (static_cast<unsigned long long>(rows) >
        max_size / static_cast<unsigned long long>(cols)) return false;

    try {
        Matrix<T> temp(static_cast<size_t>(rows), static_cast<size_t>(cols));
        std::cout << "Enter " << cols << " ratings per user (0 = not rated):\n";
        for (size_t row = 0; row < temp.get_rows(); row++) {
            std::cout << "U" << row + 1 << ": ";
            for (size_t col = 0; col < temp.get_cols(); col++) {
                std::string token;
                if (!(std::cin >> token)) return false;
                if (token == "NULL" || token == "null") {
                    temp(row, col) = T{};
                    continue;
                }
                std::istringstream value_stream(token);
                T value;
                char extra;
                if (!(value_stream >> value) || (value_stream >> extra)) return false;
                temp(row, col) = value;
            }
        }
        matrix = std::move(temp);
        return true;
    }
    catch (const std::exception&) {
        return false;
    }
}

template <typename T>
bool loadMatrixFromFile(const std::string& filename, Matrix<T>& matrix) {
    std::ifstream file(filename);
    return file.is_open() && readMatrix(file, matrix);
}
