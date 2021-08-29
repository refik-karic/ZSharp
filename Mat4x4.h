#pragma once

#include <cstddef>
#include <cmath>

#include "Vec3.h"
#include "Vec4.h"

namespace ZSharp {

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

  Mat4x4(const Mat4x4& copy) {
    *this = copy;
  }

  void operator=(const Mat4x4& matrix) {
    if (this == &matrix) {
      return;
    }

    for (std::size_t row = 0; row < Rows; row++) {
      mData[row] = matrix[row];
    }
  }

  Vec4& operator[](std::size_t index) {
    return mData[index];
  }

  const Vec4& operator[](std::size_t index) const {
    return mData[index];
  }

  Mat4x4 operator*(float scalar) {
    Mat4x4 result;

    for (std::size_t row = 0; row < Rows; row++) {
      result[row] = mData[row] * scalar;
    }

    return result;
  }

  Mat4x4 operator*(const Mat4x4& matrix) {
    Mat4x4 result;

    Mat4x4 rhsTranspose(Mat4x4::Transpose(matrix));

    for (std::size_t row = 0; row < Rows; row++) {
      for (std::size_t col = 0; col < Columns; col++) {
        result[row][col] = mData[row] * rhsTranspose[col];
      }
    }

    return result;
  }

  static void Identity(Mat4x4& matrix) {
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
  
  static void Clear(Mat4x4& matrix) {
    for (std::size_t row = 0; row < Rows; row++) {
      Vec4::Clear(matrix[row]);
    }
  }

  static Mat4x4 Transpose(const Mat4x4& matrix) {
    Mat4x4 result;

    for (std::size_t row = 0; row < Rows; row++) {
      for (std::size_t col = 0; col < Columns; col++) {
        result[row][col] = matrix[col][row];
      }
    }

    return result;
  }

  static void SetTranslation(Mat4x4& matrix, const Vec3& translation) {
    std::size_t lastColumn = Columns - 1;
    for (std::size_t row = 0; row < 3; row++) {
      matrix[row][lastColumn] = translation[row];
    }
  }

  static void SetTranslation(Mat4x4& matrix, const Vec4& translation) {
    std::size_t lastColumn = Columns - 1;
    for (std::size_t row = 0; row < Columns; row++) {
      matrix[row][lastColumn] = translation[row];
    }
  }

  static void SetScale(Mat4x4& matrix, const Vec4& scale) {
    for (std::size_t row = 0; row < Columns; row++) {
      matrix[row][row] = scale[row];
    }
  }

  static void SetRotation(Mat4x4& matrix, float angle, Axis axis) {
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

  static Vec4 ApplyTransform(const Mat4x4& matrix, const Vec4& domain) {
    Vec4 codomainResult;

    for (std::size_t row = 0; row < Rows; row++) {
      codomainResult[row] = domain * matrix[row];
    }

    return codomainResult;
  }

  private:
  static const std::size_t Rows = 4;
  static const std::size_t Columns = 4;
  Vec4 mData[Rows];
};

}
