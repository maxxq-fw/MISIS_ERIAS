
#pragma once

#include <string>
#include <utility>
#include <vector>

class Solution {
private:
    std::vector<std::vector<double>> _matrix;
    std::vector<std::vector<int>> _clusters;

private:
    std::pair<int, int> findMaxSimilarity() const;
    void mergeClusters(int first, int second);
    std::string getClusterName(int index) const;

public:
    Solution();

    void matrixInit(const std::string& filePath);
    void calculateClusters(double threshold);

    void printMatrix() const;
    void printClusters() const;
};
