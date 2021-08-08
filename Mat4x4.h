#pragma once

#include <cstddef>
#include <cmath>

#include "Vec3.h"
#include "Vec4.h"

namespace ZSharp {

template <typename T>
class Mat4x4 final {
  public:
  enum class Axis {
    TWO_DIMENSIONS,
    X,
    Y,
    Z
  };

  Mat4x4() {
  }

  Mat4x4(const Mat4x4<T>& copy) {
    *this = copy;
  }

  void operator=(const Mat4x4<T>& matrix) {
    if (this == &matrix) {
      return;
    }

    for (std::size_t row = 0; row < Rows; row++) {
      mData[row] = matrix[row];
    }
  }

  Vec4<T>& operator[](std::size_t index) {
    return mData[index];
  }

  const Vec4<T>& operator[](std::size_t index) const {
    return mData[index];
  }

  Mat4x4<T> operator*(T scalar) {
    Mat4x4<T> result;

    for (std::size_t row = 0; row < Rows; row++) {
      result[row] = mData[row] * scalar;
    }

    return result;
  }

  Mat4x4<T> operator*(const Mat4x4<T>& matrix) {
    Mat4x4<T> result;

    Mat4x4<T> rhsTranspose(Mat4x4<T>::Transpose(matrix));

    for (std::size_t row = 0; row < Rows; row++) {
      for (std::size_t col = 0; col < Columns; col++) {
        result[row][col] = mData[row] * rhsTranspose[col];
      }
    }

    return result;
  }

  static void Identity(Mat4x4<T>& matrix) {
    T zero{};
    T one{1};

    for (std::size_t row = 0; row < Rows; row++) {
      for (std::size_t col = 0; col < Columns; col++) {
        if (row == col) {
          matrix[row][col] = one;
        }
        else {
          matrix[row][col] = zero;
        }
      }
    }
  }
  
  static void Clear(Mat4x4<T>& matrix) {
    for (std::size_t row = 0; row < Rows; row++) {
      Vec4<T>::Clear(matrix[row]);
    }
  }

  static Mat4x4<T> Transpose(const Mat4x4<T>& matrix) {
    Mat4x4<T> result;

    for (std::size_t row = 0; row < Rows; row++) {
      for (std::size_t col = 0; col < Columns; col++) {
        result[row][col] = matrix[col][row];
      }
    }

    return result;
  }

  static void SetTranslation(Mat4x4<T>& matrix, const Vec3<T>& translation) {
    std::size_t lastColumn = Columns - 1;
    for (std::size_t row = 0; row < 3; row++) {
      matrix[row][lastColumn] = translation[row];
    }
  }

  static void SetTranslation(Mat4x4<T>& matrix, const Vec4<T>& translation) {
    std::size_t lastColumn = Columns - 1;
    for (std::size_t row = 0; row < Columns; row++) {
      matrix[row][lastColumn] = translation[row];
    }
  }

  static void SetScale(Mat4x4<T>& matrix, const Vec4<T>& scale) {
    for (std::size_t row = 0; row < Columns; row++) {
      matrix[row][row] = scale[row];
    }
  }

  static void SetRotation(Mat4x4<T>& matrix, T angle, Axis axis) {
    #pragma warning(disable: 4244)
    
    switch (axis) {
      case Axis::Z:
      case Axis::TWO_DIMENSIONS:
        matrix[0][0] = cos(static_cast<double>(angle));
        matrix[0][1] = -1 * sin(static_cast<double>(angle));
        matrix[1][0] = sin(static_cast<double>(angle));
        matrix[1][1] = cos(static_cast<double>(angle));
        matrix[2][2] = 1;

        if (axis == Axis::Z) {
          matrix[3][3] = 1;
        }
        break;
      case Axis::X:
        matrix[0][0] = 1;
        matrix[1][1] = cos(static_cast<double>(angle));
        matrix[1][2] = -1 * sin(static_cast<double>(angle));
        matrix[2][1] = sin(static_cast<double>(angle));
        matrix[2][2] = cos(static_cast<double>(angle));
        matrix[3][3] = 1;
        break;
      case Axis::Y:
        matrix[0][0] = cos(static_cast<double>(angle));
        matrix[0][2] = sin(static_cast<double>(angle));
        matrix[1][1] = 1;
        matrix[2][0] = -1 * sin(static_cast<double>(angle));
        matrix[2][2] = cos(static_cast<double>(angle));
        matrix[3][3] = 1;
        break;
    }

    #pragma warning(default: 4244)
  }

  static Vec4<T> ApplyTransform(const Mat4x4<T>& matrix, const Vec4<T>& domain) {
    Vec4<T> codomainResult;

    for (std::size_t row = 0; row < Rows; row++) {
      codomainResult[row] = domain * matrix[row];
    }

    return codomainResult;
  }

  private:
  static const std::size_t Rows = 4;
  static const std::size_t Columns = 4;
  Vec4<T> mData[Rows];
};

}
