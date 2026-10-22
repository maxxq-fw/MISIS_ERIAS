#pragma once

#include "matrix.hpp"

#include <fstream>
#include <iostream>
#include <exception>
#include <string>
#include <utility>

template <typename T>
bool readMatrix(std::istream& in, Matrix<T>& matrix, bool keyboard = false) {
    long long rows, cols;
    if (keyboard) std::cout << "Enter rows and columns: ";
    if (!(in >> rows >> cols) || rows <= 0 || cols <= 0) return false;

    const size_t max_size = std::vector<T>().max_size();
    if (static_cast<unsigned long long>(rows) > max_size / static_cast<unsigned long long>(cols))
        return false;

    try {
        Matrix<T> temp(static_cast<size_t>(rows), static_cast<size_t>(cols));
        if (keyboard) std::cout << "Enter matrix elements (0 or NULL means no rating):\n";
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
    return readMatrix(std::cin, matrix, true);
}

template <typename T>
bool loadMatrixFromFile(Matrix<T>& matrix) {
    std::string filename;
    std::cout << "Enter file path: ";
    std::getline(std::cin >> std::ws, filename);

    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cout << "Cannot open file.\n";
        return false;
    }

    return readMatrix(file, matrix);
}
