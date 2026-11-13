#include "expert_system.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

int main() {
    const std::string filename = "data/assumptions.txt";
    std::vector<Assumption> assumptions;
    std::string error;

    std::ifstream file(filename);
    if (file.is_open()) {
        if (!readAssumptions(file, assumptions, error)) {
            std::cerr << "Error in " << filename << ": " << error << '\n';
            return 1;
        }
        std::cout << "Assumptions loaded from " << filename << ".\n";
    }
    else {
        std::cout << "File " << filename << " not found.\n";
        std::cout << "Use N(MD, MND) with AND, OR, NOT (rules 1-4).\n";
        std::string e1, e2;
        std::cout << "E1: ";
        if (!std::getline(std::cin, e1)) return 1;
        std::cout << "E2 (press Enter to skip): ";
        if (!std::getline(std::cin, e2)) return 1;
        std::istringstream in(e1 + "\n" + e2 + "\n");
        if (!readAssumptions(in, assumptions, error)) {
            std::cerr << "Invalid assumptions: " << error << '\n';
            return 1;
        }
    }

    runExpertSystem(assumptions, std::cout);
    return 0;
}
