#pragma once

#include "matrix.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <limits>
#include <utility>
#include <vector>

struct RecommendationOptions {
    bool use_top_n = false;
    double similarity_threshold = 0.5;
    size_t neighbor_count = 3;
    bool use_rating_threshold = false;
    double rating_threshold = 4.0;
};

struct Neighbor {
    size_t user = 0;
    double similarity = 0;
};

struct PopularProduct {
    bool found = false;
    size_t product = 0;
    double rating = 0;
    size_t rating_count = 0;
};

struct RecommendationResult {
    size_t user = 0;
    size_t product = 0;
    double user_average = 0;
    std::vector<size_t> distance_users;
    Matrix<double> distances;
    std::vector<Neighbor> neighbors;
    bool has_prediction = false;
    double numerator = 0;
    double denominator = 0;
    double predicted_rating = 0;
    bool recommend = false;
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

    for (size_t col = 0; col < matrix.get_cols(); col++) {
        if (!matrix.has_value(user, col)) continue;
        sum += static_cast<double>(matrix(user, col));
        count++;
    }
    return count == 0 ? 0.0 : sum / static_cast<double>(count);
}

template <typename T>
PopularProduct mostPopularProduct(const Matrix<T>& matrix) {
    PopularProduct best;

    for (size_t col = 0; col < matrix.get_cols(); col++) {
        double sum = 0;
        size_t count = 0;

        for (size_t row = 0; row < matrix.get_rows(); row++) {
            if (!matrix.has_value(row, col)) continue;
            sum += static_cast<double>(matrix(row, col));
            count++;
        }

        if (count == 0) continue;
        double average = sum / static_cast<double>(count);
        if (!best.found || average > best.rating) {
            best.found = true;
            best.product = col;
            best.rating = average;
            best.rating_count = count;
        }
    }
    return best;
}

template <typename T>
double cosineSimilarity(const Matrix<T>& matrix, size_t first, size_t second) {
    double dot = 0;
    double first_norm = 0;
    double second_norm = 0;

    for (size_t col = 0; col < matrix.get_cols(); col++) {
        double a = static_cast<double>(matrix(first, col));
        double b = static_cast<double>(matrix(second, col));
        dot += a * b;
        first_norm += a * a;
        second_norm += b * b;
    }
    if (first_norm == 0 || second_norm == 0) return 0;

    return dot / (std::sqrt(first_norm) * std::sqrt(second_norm));
}

template <typename T>
RecommendationResult analyzeProduct(const Matrix<T>& matrix, size_t user, size_t product,
                                    const RecommendationOptions& options) {
    RecommendationResult result;
    result.user = user;
    result.product = product;
    if (user >= matrix.get_rows() || product >= matrix.get_cols()) return result;
    result.user_average = averageUserRating(matrix, user);
    if (isNewUser(matrix, user) || matrix.has_value(user, product)) return result;

    // Only users who rated the target product can contribute to the prediction.
    result.distance_users.push_back(user);
    for (size_t row = 0; row < matrix.get_rows(); row++) {
        if (row != user && matrix.has_value(row, product))
            result.distance_users.push_back(row);
    }

    size_t size = result.distance_users.size();
    result.distances = Matrix<double>(size, size);
    for (size_t row = 0; row < size; row++) {
        for (size_t col = 0; col < size; col++) {
            result.distances(row, col) = cosineSimilarity(
                matrix, result.distance_users[row], result.distance_users[col]);
        }
    }

    for (size_t index = 1; index < size; index++) {
        double similarity = result.distances(0, index);
        if (options.use_top_n || similarity >= options.similarity_threshold)
            result.neighbors.push_back({result.distance_users[index], similarity});
    }

    std::sort(result.neighbors.begin(), result.neighbors.end(),
              [](const Neighbor& a, const Neighbor& b) {
                  if (a.similarity != b.similarity) return a.similarity > b.similarity;
                  return a.user < b.user;
              });

    if (options.use_top_n && result.neighbors.size() > options.neighbor_count)
        result.neighbors.resize(options.neighbor_count);

    for (const Neighbor& neighbor : result.neighbors) {
        double neighbor_average = averageUserRating(matrix, neighbor.user);
        double rating = static_cast<double>(matrix(neighbor.user, product));
        result.numerator += neighbor.similarity * (rating - neighbor_average);
        result.denominator += std::abs(neighbor.similarity);
    }

    if (result.denominator <= std::numeric_limits<double>::epsilon()) return result;

    result.has_prediction = true;
    result.predicted_rating = result.user_average + result.numerator / result.denominator;
    result.recommend = options.use_rating_threshold
        ? result.predicted_rating > options.rating_threshold
        : result.predicted_rating >= result.user_average;
    return result;
}

template <typename T>
void printPreferenceMatrix(const Matrix<T>& matrix, std::ostream& out) {
    out << "\nPreference matrix (0 = not rated):\n";
    out << std::setw(8) << "P/U";
    for (size_t user = 0; user < matrix.get_rows(); user++)
        out << std::setw(6) << ("U" + std::to_string(user + 1));
    out << '\n';

    for (size_t product = 0; product < matrix.get_cols(); product++) {
        out << std::setw(8) << ("P" + std::to_string(product + 1));
        for (size_t user = 0; user < matrix.get_rows(); user++)
            out << std::setw(6) << matrix(user, product);
        out << '\n';
    }
}

template <typename T>
void printRecommendation(const Matrix<T>& matrix, const RecommendationResult& result,
                         std::ostream& out) {
    out << "\nU" << result.user + 1 << " -> P" << result.product + 1 << ":\n";
    out << std::fixed << std::setprecision(3);

    out << "Similarity matrix (cosine similarity):\n";
    out << std::setw(8) << " ";
    for (size_t id : result.distance_users)
        out << std::setw(8) << ("U" + std::to_string(id + 1));
    out << '\n';
    for (size_t row = 0; row < result.distance_users.size(); row++) {
        out << std::setw(8) << ("U" + std::to_string(result.distance_users[row] + 1));
        for (size_t col = 0; col < result.distance_users.size(); col++)
            out << std::setw(8) << result.distances(row, col);
        out << '\n';
    }

    out << "Nearest users (rating matrix):\n";
    out << std::setw(8) << "P/U" << std::setw(6)
        << ("U" + std::to_string(result.user + 1));
    for (const Neighbor& neighbor : result.neighbors)
        out << std::setw(6) << ("U" + std::to_string(neighbor.user + 1));
    out << '\n';
    for (size_t product = 0; product < matrix.get_cols(); product++) {
        out << std::setw(8) << ("P" + std::to_string(product + 1))
            << std::setw(6) << matrix(result.user, product);
        for (const Neighbor& neighbor : result.neighbors)
            out << std::setw(6) << matrix(neighbor.user, product);
        out << '\n';
    }

    if (!result.has_prediction) {
        out << "Prediction unavailable (not enough ratings). DO NOT RECOMMEND.\n";
        return;
    }
    out << "Predicted rating: " << result.predicted_rating
        << "; average rating of U" << result.user + 1 << ": " << result.user_average
        << ". Decision: " << (result.recommend ? "RECOMMEND" : "DO NOT RECOMMEND")
        << ".\n";
}

template <typename T>
void runRecommendations(const Matrix<T>& matrix, const RecommendationOptions& options,
                        std::ostream& out) {
    printPreferenceMatrix(matrix, out);
    PopularProduct popular = mostPopularProduct(matrix);
    out << std::fixed << std::setprecision(3);
    if (popular.found)
        out << "\nTop-rated product: P" << popular.product + 1
            << " (average rating " << popular.rating << ").\n";
    else
        out << "\nNo rated products.\n";

    // Print at most two detailed examples; prefer U1/P5 and U2/P4 from the task.
    size_t details = 0;
    if (matrix.get_rows() > 0 && matrix.get_cols() > 4 &&
        !isNewUser(matrix, 0) && !matrix.has_value(0, 4)) {
        printRecommendation(matrix, analyzeProduct(matrix, 0, 4, options), out);
        details++;
    }
    if (matrix.get_rows() > 1 && matrix.get_cols() > 3 &&
        !isNewUser(matrix, 1) && !matrix.has_value(1, 3)) {
        printRecommendation(matrix, analyzeProduct(matrix, 1, 3, options), out);
        details++;
    }
    for (size_t user = 0; user < matrix.get_rows() && details < 2; user++) {
        if (isNewUser(matrix, user)) continue;
        for (size_t product = 0; product < matrix.get_cols() && details < 2; product++) {
            if (matrix.has_value(user, product)) continue;
            if ((user == 0 && product == 4) || (user == 1 && product == 3)) continue;
            printRecommendation(matrix, analyzeProduct(matrix, user, product, options), out);
            details++;
        }
    }

    out << "\nFinal recommendations (2 nearest users):\n";
    out << "      U      P  Prediction  Decision\n";
    for (size_t user = 0; user < matrix.get_rows(); user++) {
        if (isNewUser(matrix, user)) {
            out << "U" << user + 1 << " is a new user: ";
            if (popular.found)
                out << "recommend P" << popular.product + 1 << " (Popularity Based).\n";
            else
                out << "no recommendation.\n";
            continue;
        }

        bool has_unrated = false;
        for (size_t product = 0; product < matrix.get_cols(); product++) {
            if (matrix.has_value(user, product)) continue;
            has_unrated = true;
            RecommendationResult result = analyzeProduct(matrix, user, product, options);
            out << std::setw(7) << ("U" + std::to_string(user + 1))
                << std::setw(7) << ("P" + std::to_string(product + 1));
            if (result.has_prediction)
                out << std::setw(12) << result.predicted_rating << "  "
                    << (result.recommend ? "RECOMMEND" : "DO NOT RECOMMEND") << '\n';
            else
                out << std::setw(12) << "-" << "  Not enough data\n";
        }
        if (!has_unrated)
            out << "U" << user + 1 << " has rated all products.\n";
    }
}
