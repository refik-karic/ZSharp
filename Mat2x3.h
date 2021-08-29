#pragma once

#include <cstddef>

#include "Vec3.h"

namespace ZSharp {

class Mat2x3 final {
  public:

  enum class Axis {
    TWO_DIMENSIONS,
    X,
    Y,
    Z
  };

  Mat2x3() {}

  Mat2x3(const Mat2x3& copy) {
    *this = copy;
  }

  void operator=(const Mat2x3& matrix) {
    if (this == &matrix) {
      return;
    }

    for (std::size_t row = 0; row < Rows; row++) {
      mData[row] = matrix[row];
    }
  }

  Vec3& operator[](std::size_t index) {
    return mData[index];
  }

  const Vec3& operator[](std::size_t index) const {
    return mData[index];
  }

  Mat2x3 operator*(float scalar) {
    Mat2x3 result;

    for (std::size_t row = 0; row < Rows; row++) {
      result[row] = mData[row] * scalar;
    }

    return result;
  }

  Mat2x3 operator*(const Mat2x3& matrix) {
    Mat2x3 result;

    Mat2x3 rhsTranspose(Mat2x3::Transpose(matrix));

    for (std::size_t row = 0; row < Rows; row++) {
      for (std::size_t col = 0; col < Columns; col++) {
        result[row][col] = mData[row] * rhsTranspose[col];
      }
    }

    return result;
  }

  static void Identity(Mat2x3& matrix) {
    for (std::size_t row = 0; row < Rows; row++) {
      for (std::size_t col = 0; col < Columns; col++) {
        if (row == col) {
          matrix[row][col] = 1.f;
        }
        else {
          matrix[row][col] = 0.f;
        }
      }
    }
  }

  static void Clear(Mat2x3& matrix) {
    for (std::size_t row = 0; row < Rows; row++) {
      Vec3::Clear(matrix[row]);
    }
  }

  static Mat2x3 Transpose(const Mat2x3& matrix) {
    Mat2x3 result;

    for (std::size_t row = 0; row < Rows; row++) {
      for (std::size_t col = 0; col < Columns; col++) {
        result[row][col] = matrix[col][row];
      }
    }

    return result;
  }

  static void SetTranslation(Mat2x3& matrix, const Vec3& translation) {
    std::size_t lastColumn = Columns - 1;
    for (std::size_t row = 0; row < Columns; row++) {
      matrix[row][lastColumn] = translation[row];
    }
  }

  static void SetScale(Mat2x3& matrix, const Vec3& scale) {
    for (std::size_t row = 0; row < Columns; row++) {
      matrix[row][row] = scale[row];
    }
  }

  static void SetRotation(Mat2x3& matrix, float angle, Axis axis) {
#pragma warning(disable: 4244)

    switch (axis) {
      case Axis::Z:
      case Axis::TWO_DIMENSIONS:
        matrix[0][0] = cosf(angle);
        matrix[0][1] = -1.f * sinf(angle);
        matrix[1][0] = sinf(angle);
        matrix[1][1] = cosf(angle);
        matrix[2][2] = 1.f;

        if (axis == Axis::Z) {
          matrix[3][3] = 1.f;
        }
        break;
      case Axis::X:
        matrix[0][0] = 1.f;
        matrix[1][1] = cosf(angle);
        matrix[1][2] = -1.f * sinf(angle);
        matrix[2][1] = sinf(angle);
        matrix[2][2] = cosf(angle);
        matrix[3][3] = 1.f;
        break;
      case Axis::Y:
        matrix[0][0] = cosf(angle);
        matrix[0][2] = sinf(angle);
        matrix[1][1] = 1.f;
        matrix[2][0] = -1.f * sinf(angle);
        matrix[2][2] = cosf(angle);
        matrix[3][3] = 1.f;
        break;
    }

#pragma warning(default: 4244)
  }

  static Vec3 ApplyTransform(const Mat2x3& matrix, const Vec3& domain) {
    Vec3 codomainResult;

    for (std::size_t row = 0; row < Rows; row++) {
      codomainResult[row] = domain * matrix[row];
    }

    return codomainResult;
  }

  private:
  static const std::size_t Rows = 2;
  static const std::size_t Columns = 3;
  Vec3 mData[Rows];
};

}
