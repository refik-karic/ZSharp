#include "PhysicsAlgorithms.h"

#include "Common.h"
#include "CommonMath.h"

namespace ZSharp {

// TODO: This is arbitrarily assigned. We need a cohesive unit system for this to make sense.
const float GravityPerSecond = -1.0e-4f;

// TODO: Does this value make sense?
const float IntervalEpsilon = 1.0e-4f;

void CorrectOverlappingObjects(PhysicsObject& a, PhysicsObject& b) {
  (void)b;

  // TODO: We need to apply a realistic counter force here.
  //  Depending on the mass and object properties, we need to handle elasticity.
  const float hackyCounterForce = fabsf(a.Velocity()[1]) * 0.5f;
  a.Velocity() = { 0.f, hackyCounterForce, 0.f };
}

float MinDistanceForTime(PhysicsObject& a, PhysicsObject& b, float t) {
  const size_t numAABBVerts = 8;

  Vec3 aPoints[numAABBVerts] = {};
  Vec3 bPoints[numAABBVerts] = {};
  size_t simplexIndices[numAABBVerts] = {};
  GJKTestAABB(a, b, t, aPoints, bPoints, simplexIndices);

  const Vec3 bTestTriangle[3] = {
    bPoints[simplexIndices[3]],
    bPoints[simplexIndices[5]],
    bPoints[simplexIndices[7]],
  };

  const Vec3 closestDistances[4] = {
    ClosestPointToTriangle(aPoints[simplexIndices[0]], bTestTriangle) - aPoints[simplexIndices[0]],
    ClosestPointToTriangle(aPoints[simplexIndices[2]], bTestTriangle) - aPoints[simplexIndices[2]],
    ClosestPointToTriangle(aPoints[simplexIndices[4]], bTestTriangle) - aPoints[simplexIndices[4]],
    ClosestPointToTriangle(aPoints[simplexIndices[6]], bTestTriangle) - aPoints[simplexIndices[6]],
  };

  float distA = closestDistances[0].Length();
  float distB = closestDistances[1].Length();
  float distC = closestDistances[2].Length();
  float distD = closestDistances[3].Length();

  float leastDistance = distA;
  if (distB < leastDistance) {
    leastDistance = distB;
  }

  if (distC < leastDistance) {
    leastDistance = distC;
  }

  if (distD < leastDistance) {
    leastDistance = distD;
  }

  return leastDistance;
}

float MaxMovementForTime(PhysicsObject& object, float t0, float t1) {
  const Vec3 scaledStart(object.Velocity() * t0);
  const Vec3 scaledEnd(object.Velocity() * t1);

  const Vec3 distance(scaledEnd - scaledStart);

  return distance.Length();
}

bool StaticContinuousTest(PhysicsObject& a, PhysicsObject& b, float t0, float t1, float& timeOfImpact) {
  /*
    Perform a recursive binary search over the movement for the current timestep.

    If the objects never reach each other for the timestep we can early out.
    Otherwise, we check the first half of the space and then the second.
    Eventually A will come very close to B but not actually collide with it.
    At that point, we can use something like GJK to approximate an impact point and handle the collision.
  */
  
  float maxMoveA = MaxMovementForTime(a, t0, t1);

  // NOTE: In a dynamic/dynamic case we must consider the other moving object.
  //  Since we're dealing with dynamic/static here, we can assume B is fixed.
  //float maxMoveB = MaxDistanceForTime(b, t0, t1);
  float maxMoveB = 0.f;

  float maxMoveDistance = maxMoveA + maxMoveB;

  float minDistanceStart = MinDistanceForTime(a, b, t0);
  if (minDistanceStart > maxMoveDistance) {
    return false;
  }

  float minDistanceEnd = MinDistanceForTime(a, b, t1);
  if (minDistanceEnd > maxMoveDistance) {
    return false;
  }

  if ((t1 - t0) < IntervalEpsilon) {
    timeOfImpact = t0;
    return true;
  }

  float midTime = (t0 + t1) * 0.5f;

  if (StaticContinuousTest(a, b, t0, midTime, timeOfImpact)) {
    return true;
  }
  else {
    return StaticContinuousTest(a, b, midTime, t1, timeOfImpact);
  }
}

bool DynamicDynamicIntersectionTest(PhysicsObject& a, PhysicsObject& b, float& timeOfImpact) {
  (void)a;
  (void)b;
  (void)timeOfImpact;

  return false;
}

bool GJKTestAABB(PhysicsObject& a, PhysicsObject& b, float t, Vec3 aPoints[8], Vec3 bPoints[8], size_t simplexIndices[8]) {
  Vec3 simplex[4] = {};

  const size_t numAABBVerts = 8;

  AABB translatedA(a.TransformedAABB());
  translatedA.Translate(a.Velocity() * t);
  translatedA.ToPoints(aPoints);

  AABB translatedB(b.TransformedAABB());
  translatedB.Translate(b.Velocity() * t);
  translatedB.ToPoints(bPoints);

  // Pick first point.
  Vec3 nextSearch(1.f, 0.f, 0.f);
  size_t simplexLength = 0;

  simplex[simplexLength] = (GJKSupportPoint(aPoints, numAABBVerts, nextSearch, simplexIndices[simplexLength]) -
    GJKSupportPoint(bPoints, numAABBVerts, -nextSearch, simplexIndices[simplexLength + 1]));

  simplexLength++;

  const size_t maxGJKIterations = 5;

  bool intersection = false;

  // Find the next supporting points for this iteration in the opposite direction.
  simplex[simplexLength] = (GJKSupportPoint(aPoints, numAABBVerts, -simplex[0], simplexIndices[2 * simplexLength]) -
    GJKSupportPoint(bPoints, numAABBVerts, simplex[0], simplexIndices[(2 * simplexLength) + 1]));

  simplexLength++;

  // We could early out here if the line segment doesn't cross the origin.
  // Instead, we continue to compute the closest features because we're interested in minimum distance even in the non-overlapping case.
  const Vec3 origin(0.f);

  GJKHandleSimplex(simplex, simplexIndices, nextSearch, simplexLength);

  for(size_t i = 0; i < maxGJKIterations; ++i) {
    Vec3 nextSimplexPoint((GJKSupportPoint(aPoints, numAABBVerts, nextSearch, simplexIndices[2 * simplexLength]) -
      GJKSupportPoint(bPoints, numAABBVerts, -nextSearch, simplexIndices[(2 * simplexLength) + 1])));

    bool foundSamePoint = false;

    if (simplexLength == 2) {
      foundSamePoint = nextSimplexPoint == simplex[2] ||
        nextSimplexPoint == simplex[1] ||
        nextSimplexPoint == simplex[0];
    }
    else if (simplexLength == 3) {
      foundSamePoint = nextSimplexPoint == simplex[3] ||
        nextSimplexPoint == simplex[2] ||
        nextSimplexPoint == simplex[1] ||
        nextSimplexPoint == simplex[0];
    }
    else {
      ZAssert(false);
      break;
    }

    if (foundSamePoint) {
      break;
    }
    else {
      simplex[simplexLength] = nextSimplexPoint;
    }

    if (simplexLength < 3) {
      simplexLength++;
    }

    intersection = GJKHandleSimplex(simplex, simplexIndices, nextSearch, simplexLength);
  }

  return intersection;
}

bool GJKHandleSimplex(Vec3 simplex[4], size_t simplexIndices[8], Vec3& direction, size_t& simplexLength) {
  // Using 0 based indexing here to match array index
  switch (simplexLength - 1) {
    case 1:
    {
      const Vec3 ao(-simplex[1]);
      const Vec3 ab(simplex[0] - simplex[1]);

      const float abaoDot = ab * ao;

      if (abaoDot > 0.f) {
        direction = ab.TripleCross(ao, ab);
      }
      else {
        direction = ao;
      }
    }
    break;
    case 2:
    {
      const Vec3 nextAO(-simplex[2]);
      const Vec3 nextAB(simplex[1] - simplex[2]);
      const Vec3 nextAC(simplex[0] - simplex[2]);

      const Vec3 abacPerp(nextAB.Cross(nextAC));

      Vec3 nextSearchAC(abacPerp.Cross(nextAC));
      const float nextACDot = nextSearchAC * nextAO;

      if (nextACDot > 0.f) {
        const float dotResult = nextAC * nextAO;
        if (dotResult > 0.f) {
          // Drop B, search in AC perp.
          simplex[1] = simplex[2];
          simplexIndices[2] = simplexIndices[4];
          simplexIndices[3] = simplexIndices[5];
          simplexLength = 2;

          direction = nextAC.TripleCross(nextAO, nextAC);
        }
        else {
          // Drop C, search for new line.
          simplexLength = 2;
          return GJKHandleSimplex(simplex, simplexIndices, direction, simplexLength);
        }
      }
      else {
        Vec3 nextSearchAB(nextAB.Cross(abacPerp));

        const float dotResult = nextSearchAB * nextAO;
        if (dotResult > 0.f) {
          // Drop C, search for new line.
          simplexLength = 2;
          return GJKHandleSimplex(simplex, simplexIndices, direction, simplexLength);
        }
        else {
          const float abacPerpDot = abacPerp * nextAO;
          if (abacPerpDot > 0.f) {
            direction = abacPerp;
          }
          else {
            // Swap B and C, search in negative direction.
            Swap(simplex[1], simplex[2]);
            Swap(simplexIndices[2], simplexIndices[4]);
            Swap(simplexIndices[3], simplexIndices[5]);
            direction = -abacPerp;
          }
        }
      }
    }
    break;
    case 3:
    {
      const Vec3 nextAO(-simplex[3]);
      const Vec3 nextAB(simplex[2] - simplex[3]);
      const Vec3 nextAC(simplex[1] - simplex[3]);
      const Vec3 nextAD(simplex[0] - simplex[3]);
      Vec3 nextSearchAB(nextAB.Cross(nextAC));
      Vec3 nextSearchAC(nextAC.Cross(nextAD));
      Vec3 nextSearchAD(nextAD.Cross(nextAB));

      const float nextABDot = nextSearchAB * nextAO;
      const float nextACDot = nextSearchAC * nextAO;
      const float nextADDot = nextSearchAD * nextAO;

      if (nextABDot > 0.f) {
        // Drop D, search in ABC.
        simplex[0] = simplex[1];
        simplex[1] = simplex[2];
        simplex[2] = simplex[3];

        simplexIndices[0] = simplexIndices[2];
        simplexIndices[1] = simplexIndices[3];

        simplexIndices[2] = simplexIndices[4];
        simplexIndices[3] = simplexIndices[5];

        simplexIndices[4] = simplexIndices[6];
        simplexIndices[5] = simplexIndices[7];

        simplexLength = 3;
        return GJKHandleSimplex(simplex, simplexIndices, direction, simplexLength);
      }
      else if (nextACDot > 0.f) {
        // Drop B, search in ACD.
        simplex[2] = simplex[3];

        simplexIndices[4] = simplexIndices[6];
        simplexIndices[5] = simplexIndices[7];

        simplexLength = 3;
        return GJKHandleSimplex(simplex, simplexIndices, direction, simplexLength);
      }
      else if (nextADDot > 0.f) {
        // Drop C, search in ABD.
        simplex[1] = simplex[2];
        simplex[2] = simplex[3];

        simplexIndices[2] = simplexIndices[4];
        simplexIndices[3] = simplexIndices[5];

        simplexIndices[4] = simplexIndices[6];
        simplexIndices[5] = simplexIndices[7];

        simplexLength = 3;
        return GJKHandleSimplex(simplex, simplexIndices, direction, simplexLength);
      }
      else {
        return true;
      }
    }
    break;
    default:
    {
      ZAssert(false);
      return false;
    }
      break;
  }

  return false;
}

Vec3 GJKSupportPoint(const Vec3* verts, size_t length, const Vec3& direction, size_t& index) {
  float highestDot = verts[0] * direction;
  Vec3 result(verts[0]);

  for (size_t i = 1; i < length; ++i) {
    float dot = verts[i] * direction;
    if (dot > highestDot) {
      highestDot = dot;
      result = verts[i];
      index = i;
    }
  }
  
  return result;
}

Vec3 ClosestPointToLine(const Vec3& point, const Vec3 line[2], float& t) {
  Vec3 ab(line[1] - line[0]);
  t = ((point - line[0]) * ab) / (ab * ab);

  t = Clamp(t, 0.f, 1.f);

  return line[0].Parametric(ab, t);
}

Vec3 ClosestPointToTriangle(const Vec3& point, const Vec3 triangle[3]) {
  Vec3 ab(triangle[1] - triangle[0]);
  Vec3 ac(triangle[2] - triangle[0]);

  Vec3 ap(point - triangle[0]);
  float d1 = ab * ap;
  float d2 = ac * ap;
  if (d1 <= 0.f && d2 <= 0.f) {
    return triangle[0];
  }

  Vec3 bp(point - triangle[1]);
  float d3 = ab * bp;
  float d4 = ac * bp;
  if (d3 >= 0.f && d4 <= d3) {
    return triangle[1];
  }

  float vc = (d1 * d4) - (d3 * d2);
  if (vc <= 0.f && d1 >= 0.f && d3 <= 0.f) {
    float v = d1 / (d1 - d3);
    return triangle[0].Parametric(ab, v);
  }

  Vec3 cp(point - triangle[2]);
  float d5 = ab * cp;
  float d6 = ac * cp;
  if (d6 >= 0.f && d5 <= d6) {
    return triangle[2];
  }

  float vb = (d5 * d2) - (d1 * d6);
  if (vb <= 0.f && d2 >= 0.f && d6 <= 0.f) {
    float w = d2 / (d2 - d6);
    return triangle[0].Parametric(ac, w);
  }

  float va = (d3 * d6) - (d5 * d4);
  if (va <= 0.f && (d4 - d3) >= 0.f && (d5 - d6) >= 0.f) {
    float w = (d4 - d3) / ((d4 - d3) + (d5 - d6));
    return triangle[1].Parametric((triangle[2] - triangle[1]), w);
  }

  float denom = 1.f / (va + vb + vc);
  float v = vb * denom;
  float w = vc * denom;
  return triangle[0] + (ab * v) + (ac * w);
}

}
