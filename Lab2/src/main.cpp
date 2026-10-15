
#include "solution.hpp"

#include <exception>
#include <iostream>

int main()
{
    try
    {
        Solution solution;
        solution.matrixInit("data/matrix.csv");

        double threshold;

        std::cout << "Enter similarity threshold R (0..1): ";

        if (!(std::cin >> threshold))
        {
            std::cerr << "Error: invalid threshold\n";
            return 1;
        }

        solution.calculateClusters(threshold);
    }
    catch (const std::exception& error)
    {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }

    return 0;
}
