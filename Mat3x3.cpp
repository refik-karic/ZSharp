#include "Mat3x3.h"

namespace ZSharp {
Mat3x3::Mat3x3() {
}

Mat3x3::Mat3x3(const Mat3x3& copy) {
  *this = copy;
}

void Mat3x3::Identity() {
  for (std::size_t row = 0; row < Rows; row++) {
    for (std::size_t col = 0; col < Columns; col++) {
      if (row == col) {
        mData[row][col] = 1.f;
      }
      else {
        mData[row][col] = 0.f;
      }
    }
  }
}

void Mat3x3::Clear() {
  for (std::size_t row = 0; row < Rows; row++) {
    mData[row].Clear();
  }
}

Mat3x3 Mat3x3::Transpose() const {
  Mat3x3 result;

  for (std::size_t row = 0; row < Rows; row++) {
    for (std::size_t col = 0; col < Columns; col++) {
      result[row][col] = mData[col][row];
    }
  }

  return result;
}

}
