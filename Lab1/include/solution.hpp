#include <iostream>
#include <vector>
#include <utility>
#include <string>

class Solution {
private:
    std::vector<std::vector<double>> _matrix;
    std::pair<int, int> _closestProds;
    std::pair<int, int> _closestUsers;

private:
    std::vector<double> getMatrixColumn(int column) const; 
    std::vector<double> getMatrixRow(int row) const;
    void setClosestProds(int firstRow, int secondRow);
    void setClosestUsers(int firstColumn, int secondColumn);
    double cosineSimilarity(const std::vector<double>& first, const std::vector<double>& second) const;
    
public:
    Solution();
    void matrixInit(const std::string& filePath);
    void calculateClosestProds();
    void calculateClosestUsers();
    std::pair<int, int> getClosestProds() const;
    std::pair<int, int> getClosestUsers() const;
};