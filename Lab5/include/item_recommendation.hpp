#pragma once

#include "matrix.hpp"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <limits>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

struct SimilarProduct {
    size_t product = 0;
    double similarity = 0;
};

struct PopularProduct {
    bool found = false;
    size_t product = 0;
    double rating = 0;
};

struct ItemPrediction {
    size_t user = 0;
    size_t product = 0;
    double user_average = 0;
    std::vector<SimilarProduct> neighbors;
    bool has_prediction = false;
    double predicted_rating = 0;
    double weighted_sum = 0;
    double similarity_sum = 0;
    bool recommend = false;
};

struct ItemOptions {
    size_t neighbor_count = 2;
    bool use_rating_threshold = false;
    double rating_threshold = 4.0;
};

template <typename T>
bool isNewUser(const Matrix<T>& matrix, size_t user) {
    for (size_t col = 0; col < matrix.get_cols(); col++) {
        if (matrix.has_value(user, col)) return false;
    }
    return true;
}

template <typename T>
bool ensureNewUser(Matrix<T>& matrix) {
    if (matrix.get_rows() == 0 || matrix.get_cols() == 0) return false;
    for (size_t row = 0; row < matrix.get_rows(); row++) {
        if (isNewUser(matrix, row)) return false;
    }

    Matrix<T> temp(matrix.get_rows() + 1, matrix.get_cols());
    for (size_t row = 0; row < matrix.get_rows(); row++) {
        for (size_t col = 0; col < matrix.get_cols(); col++) {
            temp(row, col) = matrix(row, col);
        }
    }
    matrix = std::move(temp);
    return true;
}

template <typename T>
double averageUserRating(const Matrix<T>& matrix, size_t user) {
    double sum = 0;
    size_t count = 0;
    for (size_t product = 0; product < matrix.get_cols(); product++) {
        if (!matrix.has_value(user, product)) continue;
        sum += static_cast<double>(matrix(user, product));
        count++;
    }
    return count ? sum / static_cast<double>(count) : 0;
}

template <typename T>
PopularProduct mostPopularProduct(const Matrix<T>& matrix) {
    PopularProduct best;
    for (size_t product = 0; product < matrix.get_cols(); product++) {
        double sum = 0;
        size_t count = 0;
        for (size_t user = 0; user < matrix.get_rows(); user++) {
            if (!matrix.has_value(user, product)) continue;
            sum += static_cast<double>(matrix(user, product));
            count++;
        }
        if (count == 0) continue;
        double average = sum / static_cast<double>(count);
        if (!best.found || average > best.rating) {
            best.found = true;
            best.product = product;
            best.rating = average;
        }
    }
    return best;
}

template <typename T>
double cosineProductSimilarity(const Matrix<T>& matrix, size_t first, size_t second) {
    double dot = 0, norm_first = 0, norm_second = 0;
    for (size_t user = 0; user < matrix.get_rows(); user++) {
        double a = static_cast<double>(matrix(user, first));
        double b = static_cast<double>(matrix(user, second));
        dot += a * b;
        norm_first += a * a;
        norm_second += b * b;
    }
    if (norm_first == 0 || norm_second == 0) return 0;
    return dot / (std::sqrt(norm_first) * std::sqrt(norm_second));
}

template <typename T>
Matrix<double> buildSimilarityMatrix(const Matrix<T>& matrix) {
    Matrix<double> similarities(matrix.get_cols(), matrix.get_cols());
    for (size_t row = 0; row < matrix.get_cols(); row++) {
        for (size_t col = row; col < matrix.get_cols(); col++) {
            double similarity = cosineProductSimilarity(matrix, row, col);
            similarities(row, col) = similarity;
            similarities(col, row) = similarity;
        }
    }
    return similarities;
}

inline std::vector<SimilarProduct> findNearestProducts(const Matrix<double>& similarities, size_t product, size_t count) {
    std::vector<SimilarProduct> neighbors;
    if (product >= similarities.get_rows()) return neighbors;

    for (size_t other = 0; other < similarities.get_cols(); other++) {
        if (other != product && similarities(product, other) > std::numeric_limits<double>::epsilon())
            neighbors.push_back({other, similarities(product, other)});
    }
    std::sort(neighbors.begin(), neighbors.end(), [](const SimilarProduct& a, const SimilarProduct& b) {
        if (a.similarity != b.similarity) return a.similarity > b.similarity;
        return a.product < b.product;
    });
    if (neighbors.size() > count) neighbors.resize(count);
    return neighbors;
}

template <typename T>
ItemPrediction predictRating(const Matrix<T>& matrix, const Matrix<double>& similarities, size_t user, size_t product, const ItemOptions& options) {
    ItemPrediction result;
    result.user = user;
    result.product = product;
    if (user >= matrix.get_rows() || product >= matrix.get_cols() || isNewUser(matrix, user) || matrix.has_value(user, product)) return result;

    result.user_average = averageUserRating(matrix, user);
    result.neighbors = findNearestProducts(similarities, product, options.neighbor_count);
    double numerator = 0, denominator = 0;
    for (const SimilarProduct& neighbor : result.neighbors) {
        if (!matrix.has_value(user, neighbor.product)) continue;
        numerator += neighbor.similarity * static_cast<double>(matrix(user, neighbor.product));
        denominator += std::abs(neighbor.similarity);
    }
    result.weighted_sum = numerator;
    result.similarity_sum = denominator;
    if (denominator <= std::numeric_limits<double>::epsilon()) return result;

    result.has_prediction = true;
    result.predicted_rating = numerator / denominator;
    result.recommend = options.use_rating_threshold
        ? result.predicted_rating + 1e-10 >= options.rating_threshold
        : result.predicted_rating + 1e-10 >= result.user_average;
    return result;
}

template <typename T>
void printPreferenceMatrix(const Matrix<T>& matrix, std::ostream& out) {
    out << "\nPreference matrix (0 = not rated):\n" << std::setw(8) << "P/U";
    for (size_t user = 0; user < matrix.get_rows(); user++) out << std::setw(6) << ("U" + std::to_string(user + 1));
    out << '\n';
    for (size_t product = 0; product < matrix.get_cols(); product++) {
        out << std::setw(8) << ("P" + std::to_string(product + 1));
        for (size_t user = 0; user < matrix.get_rows(); user++) out << std::setw(6) << matrix(user, product);
        out << '\n';
    }
}

inline void printSimilarityMatrix(const Matrix<double>& similarities, std::ostream& out) {
    out << "\nProduct similarity matrix (cosine similarity):\n" << std::setw(8) << "P/P";
    for (size_t product = 0; product < similarities.get_cols(); product++) out << std::setw(8) << ("P" + std::to_string(product + 1));
    out << '\n';
    for (size_t product = 0; product < similarities.get_rows(); product++) {
        out << std::setw(8) << ("P" + std::to_string(product + 1));
        for (size_t other = 0; other < similarities.get_cols(); other++) out << std::setw(8) << similarities(product, other);
        out << '\n';
    }
}

template <typename T>
void printPrediction(const Matrix<T>& matrix, const ItemPrediction& result, std::ostream& out) {
    out << "\nU" << result.user + 1 << " -> P" << result.product + 1 << ":\n";
    out << "Nearest products (Top-" << result.neighbors.size() << "):\n";
    for (const SimilarProduct& neighbor : result.neighbors)
        out << "  P" << neighbor.product + 1 << ": similarity = " << neighbor.similarity << '\n';

    out << "Nearest products (ratings matrix):\n" << std::setw(8) << "P/U";
    for (size_t user = 0; user < matrix.get_rows(); user++) out << std::setw(6) << ("U" + std::to_string(user + 1));
    out << '\n';

    auto print_row = [&](size_t product) {
        out << std::setw(8) << ("P" + std::to_string(product + 1));
        for (size_t user = 0; user < matrix.get_rows(); user++) out << std::setw(6) << matrix(user, product);
        out << '\n';
    };
    print_row(result.product);
    for (const SimilarProduct& neighbor : result.neighbors) print_row(neighbor.product);

    out << "Prediction calculation: ";
    bool has_rated_neighbor = false;
    for (const SimilarProduct& neighbor : result.neighbors) {
        if (!matrix.has_value(result.user, neighbor.product)) continue;
        if (has_rated_neighbor) out << " + ";
        out << matrix(result.user, neighbor.product) << " * " << neighbor.similarity;
        has_rated_neighbor = true;
    }
    if (!has_rated_neighbor) out << "no rated similar products";
    out << '\n';

    if (!result.has_prediction) {
        out << "Prediction unavailable (no rated similar products). Decision: NO RECOMMENDATION.\n";
        return;
    }
    out << "Weighted sum: " << result.weighted_sum << "; similarity sum: " << result.similarity_sum << ".\n";
    out << "Predicted rating: " << result.predicted_rating << "; average rating of U" << result.user + 1 << ": " << result.user_average;
    out << ". Decision: " << (result.recommend ? "RECOMMEND" : "DO NOT RECOMMEND") << ".\n";
}

template <typename T>
void runRecommendations(const Matrix<T>& matrix, const ItemOptions& options, std::ostream& out) {
    out << std::fixed << std::setprecision(3);
    printPreferenceMatrix(matrix, out);
    PopularProduct popular = mostPopularProduct(matrix);
    if (popular.found)
        out << "\nTop-rated product: P" << popular.product + 1 << " (average rating " << popular.rating << ").\n";
    else
        out << "\nNo rated products.\n";

    Matrix<double> similarities = buildSimilarityMatrix(matrix);
    printSimilarityMatrix(similarities, out);

    out << "\nDetailed calculations for all unrated products (" << options.neighbor_count << " nearest products):\n";
    for (size_t user = 0; user < matrix.get_rows(); user++) {
        if (isNewUser(matrix, user)) continue;
        bool has_unrated = false;
        for (size_t product = 0; product < matrix.get_cols(); product++) {
            if (matrix.has_value(user, product)) continue;
            has_unrated = true;
            printPrediction(matrix, predictRating(matrix, similarities, user, product, options), out);
        }
        if (!has_unrated) out << "\nU" << user + 1 << " has rated all products.\n";
    }

    out << "\nFinal recommendations (" << options.neighbor_count << " nearest products):\n";
    out << "      U      P  Prediction  Decision\n";
    for (size_t user = 0; user < matrix.get_rows(); user++) {
        if (isNewUser(matrix, user)) {
            out << "U" << user + 1 << " is a new user: ";
            if (popular.found) out << "recommend P" << popular.product + 1 << " (Popularity Based).\n";
            else out << "no recommendation.\n";
            continue;
        }
        bool has_unrated = false;
        for (size_t product = 0; product < matrix.get_cols(); product++) {
            if (matrix.has_value(user, product)) continue;
            has_unrated = true;
            ItemPrediction result = predictRating(matrix, similarities, user, product, options);
            out << std::setw(7) << ("U" + std::to_string(user + 1)) << std::setw(7) << ("P" + std::to_string(product + 1));
            if (result.has_prediction)
                out << std::setw(12) << result.predicted_rating << "  " << (result.recommend ? "RECOMMEND" : "DO NOT RECOMMEND") << '\n';
            else
                out << std::setw(12) << "-" << "  Not enough data\n";
        }
        if (!has_unrated) out << "U" << user + 1 << " has rated all products.\n";
    }
}
