
#include "solution.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>

Solution::Solution() = default;


void Solution::matrixInit(const std::string& filePath)
{
    std::ifstream file(filePath);

    if (!file.is_open())
    {
        throw std::runtime_error("Cannot open file: " + filePath);
    }

    _matrix.clear();
    _clusters.clear();

    std::string line;

    while (std::getline(file, line))
    {
        if (line.find_first_not_of(" \t\r") == std::string::npos)
        {
            continue;
        }

        std::stringstream lineStream(line);
        std::string cell;
        std::vector<double> row;

        while (std::getline(lineStream, cell, ','))
        {
            std::size_t parsed = 0;
            const double value = std::stod(cell, &parsed);

            if (cell.find_first_not_of(" \t\r", parsed) != std::string::npos ||
                !std::isfinite(value) || value < 0.0 || value > 1.0)
            {
                throw std::runtime_error("Invalid similarity value in CSV");
            }

            row.push_back(value);
        }

        _matrix.push_back(row);
    }

    const std::size_t n = _matrix.size();

    if (n == 0)
    {
        throw std::runtime_error("Matrix is empty");
    }

    for (std::size_t i = 0; i < n; ++i)
    {
        if (_matrix[i].size() != n)
        {
            throw std::runtime_error("Matrix must be square (N x N)");
        }

        if (std::abs(_matrix[i][i]) > 1e-9)
        {
            throw std::runtime_error("Matrix diagonal must contain zeros");
        }

        for (std::size_t j = i + 1; j < n; ++j)
        {
            if (std::abs(_matrix[i][j] - _matrix[j][i]) > 1e-9)
            {
                throw std::runtime_error("Matrix must be symmetric");
            }
        }

        _clusters.push_back({static_cast<int>(i) + 1});
    }
}


std::pair<int, int> Solution::findMaxSimilarity() const
{
    if (_matrix.size() < 2)
    {
        return {-1, -1};
    }

    double maxSimilarity = -1.0;
    std::pair<int, int> result{-1, -1};

    for (std::size_t i = 0; i < _matrix.size(); ++i)
    {
        for (std::size_t j = i + 1; j < _matrix.size(); ++j)
        {
            if (_matrix[i][j] > maxSimilarity)
            {
                maxSimilarity = _matrix[i][j];
                result = {
                    static_cast<int>(i),
                    static_cast<int>(j)
                };
            }
        }
    }

    return result;
}


void Solution::mergeClusters(int first, int second)
{
    for (std::size_t i = 0; i < _matrix.size(); ++i)
    {
        if (static_cast<int>(i) == first ||
            static_cast<int>(i) == second)
        {
            continue;
        }

        const double similarity = std::max(
            _matrix[first][i],
            _matrix[second][i]
        );

        _matrix[first][i] = similarity;
        _matrix[i][first] = similarity;
    }

    _matrix[first][first] = 0.0;

    for (auto& row : _matrix)
    {
        row.erase(row.begin() + second);
    }

    _matrix.erase(_matrix.begin() + second);

    _clusters[first].insert(
        _clusters[first].end(),
        _clusters[second].begin(),
        _clusters[second].end()
    );

    std::sort(
        _clusters[first].begin(),
        _clusters[first].end()
    );

    _clusters.erase(_clusters.begin() + second);
}


std::string Solution::getClusterName(int index) const
{
    std::string name = "{";

    for (std::size_t i = 0; i < _clusters[index].size(); ++i)
    {
        if (i != 0)
        {
            name += ",";
        }

        name += "U" + std::to_string(_clusters[index][i]);
    }

    return name + "}";
}


void Solution::printMatrix() const
{
    std::size_t columnWidth = 12;

    for (std::size_t i = 0; i < _clusters.size(); ++i)
    {
        columnWidth = std::max(
            columnWidth,
            getClusterName(static_cast<int>(i)).size() + 2
        );
    }

    std::cout << "Matrix (" << _matrix.size()
              << "x" << _matrix.size() << "):\n";

    std::cout << std::setw(static_cast<int>(columnWidth)) << "";

    for (std::size_t i = 0; i < _clusters.size(); ++i)
    {
        std::cout << std::setw(static_cast<int>(columnWidth))
                  << getClusterName(static_cast<int>(i));
    }

    std::cout << '\n';
    std::cout << std::fixed << std::setprecision(2);

    for (std::size_t i = 0; i < _matrix.size(); ++i)
    {
        std::cout << std::setw(static_cast<int>(columnWidth))
                  << getClusterName(static_cast<int>(i));

        for (double similarity : _matrix[i])
        {
            std::cout << std::setw(static_cast<int>(columnWidth))
                      << similarity;
        }

        std::cout << '\n';
    }

    std::cout << '\n';
}


void Solution::printClusters() const
{
    std::cout << "Final clusters:\n";

    for (std::size_t i = 0; i < _clusters.size(); ++i)
    {
        std::cout << "Cluster " << i + 1 << ": "
                  << getClusterName(static_cast<int>(i))
                  << '\n';
    }
}


void Solution::calculateClusters(double threshold)
{
    if (!std::isfinite(threshold) ||
        threshold < 0.0 || threshold > 1.0)
    {
        throw std::invalid_argument(
            "Threshold R must be between 0 and 1"
        );
    }

    if (_matrix.empty())
    {
        throw std::runtime_error(
            "Load the matrix before clustering"
        );
    }

    std::cout << "Initial matrix:\n";
    printMatrix();

    int step = 1;

    while (_matrix.size() > 1)
    {
        const auto [first, second] = findMaxSimilarity();
        const double similarity = _matrix[first][second];

        if (threshold > 0.0 && similarity < threshold)
        {
            std::cout << "Stop: maximum similarity "
                      << similarity
                      << " < R = " << threshold
                      << "\n\n";
            break;
        }

        std::cout << "Step " << step++ << ": merge "
                  << getClusterName(first) << " and "
                  << getClusterName(second)
                  << " (similarity = " << similarity << ")\n";

        mergeClusters(first, second);
        printMatrix();
    }

    printClusters();
}
