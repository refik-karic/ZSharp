#pragma once

#include "ZBaseTypes.h"

#include "Constants.h"

namespace ZSharp {

class Triangle final {
  public:

  Triangle();

  Triangle(int32 p1, int32 p2, int32 p3);

  Triangle(const Triangle& copy);

  void operator=(const Triangle& rhs);

  int32 operator[](int32 index) const;

  int32& operator[](int32 index);

  const int32* GetData();

  const int32* GetData() const;

  private:
  int32 mIndicies[TRI_VERTS] = {};
};

}
