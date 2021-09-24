#pragma once

#include <cstddef>

#include "Vec3.h"

#ifdef FORCE_SSE
#include "IntelIntrinsics.h"
#endif

namespace ZSharp {

class alignas(16) Vec4 final {
  public:
  Vec4();

  Vec4(float x, float y, float z, float w);

  Vec4(const Vec4& copy);

  Vec4(const Vec3& copy);

  void operator=(const Vec3& vector) {
    std::memcpy(mData, *vector, sizeof(mData));
    mData[3] = 1.f;
  }

  void operator=(const Vec4& vector) {
    if (this == &vector) {
      return;
    }
    
    std::memcpy(mData, *vector, sizeof(mData));
  }

  bool operator==(const Vec4& vector) const {
    if (this == &vector) {
      return true;
    }

    return std::memcmp(mData, *vector, sizeof(mData)) == 0;
  }

  float* operator*() {
    return &mData[0];
  }

  const float* operator*() const {
    return &mData[0];
  }

  float operator[](const size_t index) const {
    return mData[index];
  }

  float& operator[](const size_t index) {
    return mData[index];
  }

  Vec4 operator+(const Vec4& vector) const {
#ifdef FORCE_SSE
    Vec4 result;
    aligned_sse128addfloats(mData, vector.mData, result.mData);
    return result;
#else
    Vec4 result(
      mData[0] + vector[0],
      mData[1] + vector[1],
      mData[2] + vector[2],
      mData[3] + vector[3]
    );
    return result;
#endif
  }

  Vec4 operator-(const Vec4& vector) const {
#ifdef FORCE_SSE
    Vec4 result;
    aligned_sse128subfloats(mData, vector.mData, result.mData);
    return result;
#else
    Vec4 result(
      mData[0] - vector[0],
      mData[1] - vector[1],
      mData[2] - vector[2],
      mData[3] - vector[3]
    );
    return result;
#endif
  }

  Vec4 operator-() const {
    Vec4 result(
      -mData[0],
      -mData[1],
      -mData[2],
      -mData[3]
    );
    return result;
  }

  Vec4 operator*(float scalar) const {
#ifdef FORCE_SSE
    Vec4 result;
    aligned_sse128mulfloat(mData, scalar, result.mData);
    return result;
#else
    Vec4 result(
      mData[0] * scalar,
      mData[1] * scalar,
      mData[2] * scalar,
      mData[3] * scalar
    );
    return result;
#endif
  }

  float operator*(const Vec4& vector) {
#ifdef FORCE_SSE
    return aligned_sse128mulfloatssum(mData, vector.mData);
#else
    float result = (mData[0] * vector[0]);
    result += (mData[1] * vector[1]);
    result += (mData[2] * vector[2]);
    result += (mData[3] * vector[3]);
    return result;
#endif
  }

  float operator*(const Vec4& vector) const {
#ifdef FORCE_SSE
    return aligned_sse128mulfloatssum(mData, vector.mData);
#else
    float result = (mData[0] * vector[0]);
    result += (mData[1] * vector[1]);
    result += (mData[2] * vector[2]);
    result += (mData[3] * vector[3]);
    return result;
#endif
  }

  float Length() const;

  void Normalize();

  void Homogenize(size_t element);

  void Clear();

  private:
  static const size_t Elements = 4;
  float mData[Elements];
};

}
