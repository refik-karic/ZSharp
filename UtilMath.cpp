#include "UtilMath.h"

#include "Constants.h"

namespace ZSharp {

double DegreesToRadians(double degrees) {
  return degrees * Constants::PI_OVER_180;
}

}
