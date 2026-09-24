#include <iostream>

#include "solution.hpp"

int main()
{
    Solution solution;

    solution.matrixInit();

    solution.calculateClosestProds();
    solution.calculateClosestUsers();

    const auto closestProds = solution.getClosestProds();
    const auto closestUsers = solution.getClosestUsers();

    std::cout << "Closest products: P" << closestProds.first + 1 << " and P" << closestProds.second + 1 << '\n';
    std::cout << "Closest users: U" << closestUsers.first + 1 << " and U" << closestUsers.second + 1 << '\n';

    return 0;
}