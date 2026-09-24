#include "solution.hpp"

#include <cmath>
#include <limits>
#include <fstream>
#include <sstream>
#include <stdexcept>


Solution::Solution() : _closestProds{-1, -1}, _closestUsers{-1, -1} {

}

void Solution::matrixInit(const std::string& filePath)
{
    std::ifstream file(filePath);

    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filePath);
    }

    _matrix.clear();
    std::string line;

    while (std::getline(file, line)) {
        std::stringstream lineStream(line);
        std::string value;

        std::vector<double> row;

        while (std::getline(lineStream, value, ',')) {
            row.push_back(std::stod(value));
        }

        if (!row.empty()) {
            _matrix.push_back(row);
        }
    }
}

std::vector<double> Solution::getMatrixColumn(int column) const {
    std::vector<double> result;

    for (const auto& row : _matrix) {
        result.push_back(row.at(column));
    }

    return result;
}

std::vector<double> Solution::getMatrixRow(int row) const {
    return _matrix.at(row);
}

void Solution::setClosestProds(int firstRow, int secondRow) {
    _closestProds = {firstRow, secondRow};
}

void Solution::setClosestUsers(int firstColumn, int secondColumn) {
    _closestUsers = {firstColumn, secondColumn};
}

double Solution::cosineSimilarity(const std::vector<double>& first, const std::vector<double>& second) const {
    if (first.size() != second.size()) {
        throw std::invalid_argument("Vectors must have the same size");
    }

    double dotProduct = 0.0;
    double firstLengthSquared = 0.0;
    double secondLengthSquared = 0.0;

    for (std::size_t i = 0; i < first.size(); ++i) {
        dotProduct += first[i] * second[i];

        firstLengthSquared += first[i] * first[i];
        secondLengthSquared += second[i] * second[i];
    }

    const double firstLength = std::sqrt(firstLengthSquared);
    const double secondLength = std::sqrt(secondLengthSquared);

    if (firstLength == 0.0 || secondLength == 0.0) {
        return 0.0;
    }

    return dotProduct / (firstLength * secondLength);
}

void Solution::calculateClosestProds()
{
    if (_matrix.size() < 2) {
        return;
    }

    double maxSimilarity = -std::numeric_limits<double>::infinity();

    for (std::size_t firstRow = 0; firstRow < _matrix.size(); ++firstRow)  {
        for (std::size_t secondRow = firstRow + 1; secondRow < _matrix.size(); ++secondRow) {
            const std::vector<double> firstProduct = getMatrixRow(firstRow);
            const std::vector<double> secondProduct = getMatrixRow(secondRow);
            const double similarity = cosineSimilarity(firstProduct, secondProduct);

            if (similarity > maxSimilarity) {
                maxSimilarity = similarity;
                setClosestProds(static_cast<int>(firstRow), static_cast<int>(secondRow));
            }
        }
    }
}

void Solution::calculateClosestUsers()
{
    if (_matrix.empty() || _matrix[0].size() < 2) {
        return;
    }

    double maxSimilarity = -std::numeric_limits<double>::infinity();
    const std::size_t columnsCount = _matrix[0].size();

    for (std::size_t firstColumn = 0; firstColumn < columnsCount; ++firstColumn) {
        for (std::size_t secondColumn = firstColumn + 1; secondColumn < columnsCount; ++secondColumn) {
            const std::vector<double> firstUser = getMatrixColumn(firstColumn);
            const std::vector<double> secondUser = getMatrixColumn(secondColumn);

            const double similarity = cosineSimilarity(firstUser, secondUser);

            if (similarity > maxSimilarity) {
                maxSimilarity = similarity;
                setClosestUsers(static_cast<int>(firstColumn), static_cast<int>(secondColumn));
            }
        }
    }
}


std::pair<int, int> Solution::getClosestProds() const {
    return _closestProds;
}

std::pair<int, int> Solution::getClosestUsers() const {
    return _closestUsers;
}