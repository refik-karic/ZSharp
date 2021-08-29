#include "UtilMath.h"

#include "Constants.h"

namespace ZSharp {

float DegreesToRadians(float degrees) {
  return degrees * Constants::PI_OVER_180;
}

}
