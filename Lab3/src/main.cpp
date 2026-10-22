#include "matrix.hpp"
#include "matrix_input.hpp"
#include "coo.hpp"
#include "csr.hpp"
#include "ellpack.hpp"
#include "conf_matrix.hpp"

#include <iostream>
#include <limits>

int main() {
    int choice = -1;
    Matrix<int> matrix;

    do {
        std::cout << "\nMenu:\n\n" << "1. Load matrix from file\n" << "2. Enter matrix manually\n\n" << "3. Coordinate format (COO)\n"
            << "4. Compressed Sparse Row (CSR)\n" << "5. ELLPACK\n" << "6. Reduce preference matrix\n\n" << "0. Exit\n\n"
            << "Choose an option: ";

        if (!(std::cin >> choice)) {
            if (std::cin.eof()) break;
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid menu option!\n";
            continue;
        }

        if (choice >= 3 && choice <= 6 && (matrix.get_rows() == 0 || matrix.get_cols() == 0)) {
            std::cout << "Load or enter a matrix first!\n";
            continue;
        }

        switch (choice) {
            case 1:
                if (loadMatrixFromFile(matrix)) {
                    std::cout << "Matrix loaded from file.\n";
                    std::cout << "Original matrix:\n" << matrix;
                }
                else std::cout << "Failed to load matrix.\n";
                break;

            case 2:
                if (inputMatrix(matrix)) {
                    std::cout << "Matrix entered successfully.\n";
                    std::cout << "Original matrix:\n" << matrix;
                }
                else {
                    std::cout << "Invalid matrix input.\n";
                    if (std::cin.eof()) return 1;
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                }
                break;

            case 3: {
                std::cout << "Original matrix:\n" << matrix;
                COO<int> coo(matrix);
                std::cout << "Coordinate format (COO):\n";
                coo.print(std::cout);
                break;
            }

            case 4: {
                std::cout << "Original matrix:\n" << matrix;
                CSR<int> csr(matrix);
                std::cout << "Compressed Sparse Row (CSR):\n";
                csr.print(std::cout);
                break;
            }

            case 5: {
                std::cout << "Original matrix:\n" << matrix;
                ELLPACK<int> ellpack(matrix);
                std::cout << "ELLPACK:\n";
                ellpack.print(std::cout);
                break;
            }

            case 6: {
                const double threshold = 3.0;
                std::cout << "Original matrix:\n" << matrix;
                std::cout << "Remove unrated products and products with average rating <= " << threshold
                    << ", then remove inactive users.\n";
                ReducedMatrix<int> result = reduceMatrix(matrix, threshold);
                printReducedMatrix(result, std::cout);
                break;
            }

            case 0:
                break;

            default:
                std::cout << "Invalid menu option!\n";
                break;
        }

    } while (choice != 0);

    return 0;
}
