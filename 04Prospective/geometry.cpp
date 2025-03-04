// geometry.cpp
#include "geometry.h"
#include <iostream>
#include <stdexcept>

Matrix::Matrix(int rows, int cols) : rows(rows), cols(cols), m(rows, std::vector<float>(cols, 0)) {}

Matrix::Matrix(const std::vector<std::vector<float>>& values) : m(values), rows(values.size()), cols(values[0].size()) {}

int Matrix::getRows() const { return rows; }
int Matrix::getCols() const { return cols; }

Matrix Matrix::operator+(const Matrix& other) const {
    if (rows != other.rows || cols != other.cols) throw std::invalid_argument("Matrix dimensions must match for addition");
    Matrix result(rows, cols);
    for (int i = 0; i < rows; ++i)
        for (int j = 0; j < cols; ++j)
            result.m[i][j] = m[i][j] + other.m[i][j];
    return result;
}

Matrix Matrix::operator-(const Matrix& other) const {
    if (rows != other.rows || cols != other.cols) throw std::invalid_argument("Matrix dimensions must match for subtraction");
    Matrix result(rows, cols);
    for (int i = 0; i < rows; ++i)
        for (int j = 0; j < cols; ++j)
            result.m[i][j] = m[i][j] - other.m[i][j];
    return result;
}

Matrix Matrix::operator*(const Matrix& other) const {
    if (cols != other.rows) throw std::invalid_argument("Matrix multiplication requires matching inner dimensions");
    Matrix result(rows, other.cols);
    for (int i = 0; i < rows; ++i)
        for (int j = 0; j < other.cols; ++j)
            for (int k = 0; k < cols; ++k)
                result.m[i][j] += m[i][k] * other.m[k][j];
    return result;
}

Matrix Matrix::operator*(float scalar) const {
    Matrix result(rows, cols);
    for (int i = 0; i < rows; ++i)
        for (int j = 0; j < cols; ++j)
            result.m[i][j] = m[i][j] * scalar;
    return result;
}

std::vector<float>& Matrix::operator[](int index) { return m[index]; }
const std::vector<float>& Matrix::operator[](int index) const { return m[index]; }

Matrix Matrix::transpose() const {
    Matrix result(cols, rows);
    for (int i = 0; i < rows; ++i)
        for (int j = 0; j < cols; ++j)
            result.m[j][i] = m[i][j];
    return result;
}

float Matrix::determinantRecursive(const std::vector<std::vector<float>>& mat) const {
    int n = mat.size();
    if (n == 1) return mat[0][0];
    if (n == 2) return mat[0][0] * mat[1][1] - mat[0][1] * mat[1][0];
    float det = 0;
    for (int i = 0; i < n; ++i) {
        Matrix cofactor = getCofactorMatrix(0, i);
        det += (i % 2 == 0 ? 1 : -1) * mat[0][i] * cofactor.determinant();
    }
    return det;
}

Matrix Matrix::getCofactorMatrix(int p, int q) const {
    std::vector<std::vector<float>> temp;
    for (int i = 0; i < rows; ++i) {
        if (i == p) continue;
        std::vector<float> row;
        for (int j = 0; j < cols; ++j) {
            if (j == q) continue;
            row.push_back(m[i][j]);
        }
        temp.push_back(row);
    }
    return Matrix(temp);
}

float Matrix::determinant() const {
    if (rows != cols) throw std::invalid_argument("Determinant is only defined for square matrices");
    return determinantRecursive(m);
}

Matrix Matrix::inverse() const {
    float det = determinant();
    if (det == 0) throw std::runtime_error("Matrix is singular and cannot be inverted");
    Matrix adj(rows, cols);
    for (int i = 0; i < rows; ++i)
        for (int j = 0; j < cols; ++j)
            adj.m[j][i] = ((i + j) % 2 == 0 ? 1 : -1) * getCofactorMatrix(i, j).determinant();
    return adj * (1 / det);
}

Matrix Matrix::identity(int dimensions) {
    Matrix m(dimensions, dimensions);
    for (int i = 0; i < dimensions; i++) {
        for (int j = 0; j < dimensions; j++){
            if(i == j) {
                m[i][j] = 1;
            }else{
                m[i][j] = 0;
            }
        }
    }

    return m;
}

void Matrix::display() const {
    for (const auto& row : m) {
        for (float val : row)
            std::cout << val << " ";
        std::cout << std::endl;
    }
}