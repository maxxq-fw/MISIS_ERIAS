#include "matrix.hpp"
#include "matrix_input.hpp"
#include "item_recommendation.hpp"

#include <fstream>
#include <iostream>
#include <string>

int main() {
    Matrix<int> matrix;
    const std::string filename = "data/matrix.txt";

    std::ifstream file(filename);
    if (file.is_open()) {
        if (!readMatrix(file, matrix)) {
            std::cerr << "Error: file " << filename << " contains an invalid matrix.\n";
            return 1;
        }
        std::cout << "Matrix loaded from " << filename << ".\n";
    }
    else {
        std::cout << "File " << filename << " not found. Enter the matrix manually.\n";
        if (!inputMatrix(matrix)) {
            std::cerr << "Failed to read the matrix.\n";
            return 1;
        }
    }

    if (ensureNewUser(matrix)) std::cout << "New user U" << matrix.get_rows() << " added.\n";

    ItemOptions options;
    options.neighbor_count = 2;
    options.use_rating_threshold = false;

    runRecommendations(matrix, options, std::cout);
    return 0;
}
