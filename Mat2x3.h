#pragma once

#include <cstddef>

#include "Vec3.h"

namespace ZSharp {

template<typename T>
class Mat2x3 final {
  public:

  enum class Axis {
    TWO_DIMENSIONS,
    X,
    Y,
    Z
  };

  Mat2x3() {}

  Mat2x3(const Mat2x3<T>& copy) {
    *this = copy;
  }

  void operator=(const Mat2x3<T>& matrix) {
    if (this == &matrix) {
      return;
    }

    for (std::size_t row = 0; row < Rows; row++) {
      mData[row] = matrix[row];
    }
  }

  Vec3<T>& operator[](std::size_t index) {
    return mData[index];
  }

  const Vec3<T>& operator[](std::size_t index) const {
    return mData[index];
  }

  Mat2x3<T> operator*(T scalar) {
    Mat2x3<T> result;

    for (std::size_t row = 0; row < Rows; row++) {
      result[row] = mData[row] * scalar;
    }

    return result;
  }

  Mat2x3<T> operator*(const Mat2x3<T>& matrix) {
    Mat2x3<T> result;

    Mat2x3<T> rhsTranspose(Mat2x3<T>::Transpose(matrix));

    for (std::size_t row = 0; row < Rows; row++) {
      for (std::size_t col = 0; col < Columns; col++) {
        result[row][col] = mData[row] * rhsTranspose[col];
      }
    }

    return result;
  }

  static void Identity(Mat2x3<T>& matrix) {
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

  static void Clear(Mat2x3<T>& matrix) {
    for (std::size_t row = 0; row < Rows; row++) {
      Vec3<T>::Clear(matrix[row]);
    }
  }

  static Mat2x3<T> Transpose(const Mat2x3<T>& matrix) {
    Mat2x3<T> result;

    for (std::size_t row = 0; row < Rows; row++) {
      for (std::size_t col = 0; col < Columns; col++) {
        result[row][col] = matrix[col][row];
      }
    }

    return result;
  }

  static void SetTranslation(Mat2x3<T>& matrix, const Vec3<T>& translation) {
    std::size_t lastColumn = Columns - 1;
    for (std::size_t row = 0; row < Columns; row++) {
      matrix[row][lastColumn] = translation[row];
    }
  }

  static void SetScale(Mat2x3<T>& matrix, const Vec3<T>& scale) {
    for (std::size_t row = 0; row < Columns; row++) {
      matrix[row][row] = scale[row];
    }
  }

  static void SetRotation(Mat2x3<T>& matrix, T angle, Axis axis) {
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

  static Vec3<T> ApplyTransform(const Mat2x3<T>& matrix, const Vec3<T>& domain) {
    Vec3<T> codomainResult;

    for (std::size_t row = 0; row < Rows; row++) {
      codomainResult[row] = domain * matrix[row];
    }

    return codomainResult;
  }

  private:
  static const std::size_t Rows = 2;
  static const std::size_t Columns = 3;
  Vec3<T> mData[Rows];
};

}
