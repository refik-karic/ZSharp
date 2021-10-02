#include "Mat2x3.h"

#include <cmath>

namespace ZSharp {
Mat2x3::Mat2x3() {
}

Mat2x3::Mat2x3(const Mat2x3& copy) {
  *this = copy;
}

void Mat2x3::Identity() {
  Clear();
  mData[0][0] = 1.f;
  mData[1][1] = 1.f;
}

void Mat2x3::Clear() {
  mData[0].Clear();
  mData[1].Clear();
}

Vec3 Mat2x3::ApplyTransform(const Vec3& domain) const {
  Vec3 codomainResult(
    domain * mData[0],
    domain * mData[1],
    0.f
  );
  return codomainResult;
}
}
