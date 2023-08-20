#include "ZAlgorithm.h"

#include "Constants.h"
#include "CommonMath.h"

#include "ZAssert.h"
#include "ScopedTimer.h"

#include <cstring>

#define DEBUG_CLIPPING 0

#define ASSERT_CHECK 0

static const size_t MaxOutVerts = 3;
static const size_t NumInVerts = 3;
static const size_t NumAttributes = 3;

namespace ZSharp {

bool InsidePlane(const float point[3], const float clipEdge[3], const float normal[3]) {
  return (((point[0] - clipEdge[0]) * normal[0]) +
    ((point[1] - clipEdge[1]) * normal[1]) +
    ((point[2] - clipEdge[2]) * normal[2])) < 0.f;
}

Vec3 GetParametricVector3D(const float point, const Vec3& start, const Vec3& end) {
  return (start + ((end - start) * point));
}

void GetParametricVector4D(const float point, const float start[4], const float end[4], float outVec[4]) {
  outVec[0] = ((end[0] - start[0]) * point) + start[0];
  outVec[1] = ((end[1] - start[1]) * point) + start[1];
  outVec[2] = ((end[2] - start[2]) * point) + start[2];
  outVec[3] = ((end[3] - start[3]) * point) + start[3];
}

float ParametricLinePlaneIntersection(const float start[3], const float end[3], const float edgeNormal[3], const float edgePoint[3]) {
  float numerator = ((start[0] - edgePoint[0]) * edgeNormal[0]) +
    ((start[1] - edgePoint[1]) * edgeNormal[1]) +
    ((start[2] - edgePoint[2]) * edgeNormal[2]);

  float denominator = ((end[0] - start[0]) * (-edgeNormal[0])) +
    ((end[1] - start[1]) * (-edgeNormal[1])) +
    ((end[2] - start[2]) * (-edgeNormal[2]));
  return numerator / denominator;
}

void ClipTrianglesNearPlane(VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer, float nearClipZ) {
  NamedScopedTimer(NearClip);

  float nearEdge[3] = { 0.f, 0.f, nearClipZ };
  float edgeNormal[3] = { 0.f, 0.f, -1.f };

  const size_t stride = vertexBuffer.GetStride();
  const size_t vertByteSize = stride * sizeof(float);
  const size_t scatchSize = 128;

  for (size_t i = 0; i < indexBuffer.GetIndexSize(); i += TRI_VERTS) {
    const size_t i1 = indexBuffer[i];
    const size_t i2 = indexBuffer[i + 1];
    const size_t i3 = indexBuffer[i + 2];
    const float* v1 = vertexBuffer[i1];
    const float* v2 = vertexBuffer[i2];
    const float* v3 = vertexBuffer[i3];

    float clipBuffer[scatchSize];
    memset(clipBuffer, 0, sizeof(clipBuffer));
    memcpy(clipBuffer, v1, vertByteSize);
    memcpy(clipBuffer + stride, v2, vertByteSize);
    memcpy(clipBuffer + (stride * 2), v3, vertByteSize);

    size_t numClippedVerts = 3;
    numClippedVerts = SutherlandHodgmanClip(clipBuffer, numClippedVerts, nearEdge, edgeNormal, stride);

    if (numClippedVerts > 0) {
      size_t currentClipIndex = vertexBuffer.GetClipLength();

      vertexBuffer.AppendClipData(clipBuffer, numClippedVerts * stride * sizeof(float), numClippedVerts);

      Triangle nextTriangle(currentClipIndex, currentClipIndex + 1, currentClipIndex + 2);
      indexBuffer.AppendClipData(nextTriangle);

      if (numClippedVerts > 3) {
        ZAssert(numClippedVerts < 6);

        for (size_t j = 2; j < numClippedVerts; j += 3) {
          const size_t clip0 = currentClipIndex + (j % numClippedVerts);
          const size_t clip1 = currentClipIndex + ((j + 1) % numClippedVerts);
          const size_t clip2 = currentClipIndex + ((j + 2) % numClippedVerts);

          Triangle clippedTriangle(clip0, clip1, clip2);
          indexBuffer.AppendClipData(clippedTriangle);
        }
      }
    }
  }

  vertexBuffer.ShuffleClippedData();
  indexBuffer.ShuffleClippedData();
}

void ClipTriangles(VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer) {
  NamedScopedTimer(NDC_Clip);

  const size_t stride = vertexBuffer.GetStride();
  const size_t vertByteSize = stride * sizeof(float);
  const size_t scatchSize = 128;

  for (size_t i = 0; i < indexBuffer.GetIndexSize(); i += TRI_VERTS) {
    const size_t i1 = indexBuffer[i];
    const size_t i2 = indexBuffer[i + 1];
    const size_t i3 = indexBuffer[i + 2];
    const float* v1 = vertexBuffer[i1];
    const float* v2 = vertexBuffer[i2];
    const float* v3 = vertexBuffer[i3];

    float clipBuffer[scatchSize];
    memset(clipBuffer, 0, sizeof(clipBuffer));
    memcpy(clipBuffer, v1, vertByteSize);
    memcpy(clipBuffer + stride, v2, vertByteSize);
    memcpy(clipBuffer + (stride * 2), v3, vertByteSize);

    size_t numClippedVerts = 3;

#if DEBUG_CLIPPING
    size_t currentClipIndex = vertexBuffer.GetClipLength();

    // (XYZ, RGB) for 3 vertices.
    const size_t stride = 7;
    ZAssert(stride == vertexBuffer.GetStride());
    float clipData[stride * 3];
    const size_t vertByteSize = 7 * sizeof(float);
    memcpy(clipData, vertexBuffer[i1], vertByteSize);
    memcpy(clipData + stride, vertexBuffer[i2], vertByteSize);
    memcpy(clipData + (stride * 2), vertexBuffer[i3], vertByteSize);

    vertexBuffer.AppendClipData(clipData, sizeof(clipData), numClippedVerts);
    Triangle nextTriangle(currentClipIndex, currentClipIndex + 1, currentClipIndex + 2);
    indexBuffer.AppendClipData(nextTriangle);
#else

    float currentEdge[3] = {1.f, 0.f, 0.f};
    numClippedVerts = SutherlandHodgmanClip(clipBuffer, numClippedVerts, currentEdge, currentEdge, stride);

    currentEdge[0] = 0.f;
    currentEdge[1] = 1.f;
    numClippedVerts = SutherlandHodgmanClip(clipBuffer, numClippedVerts, currentEdge, currentEdge, stride);

    currentEdge[0] = -1.f;
    currentEdge[1] = 0.f;
    numClippedVerts = SutherlandHodgmanClip(clipBuffer, numClippedVerts, currentEdge, currentEdge, stride);

    currentEdge[0] = 0.f;
    currentEdge[1] = -1.f;
    numClippedVerts = SutherlandHodgmanClip(clipBuffer, numClippedVerts, currentEdge, currentEdge, stride);

    currentEdge[1] = 0.f;
    currentEdge[2] = -1.f;
    numClippedVerts = SutherlandHodgmanClip(clipBuffer, numClippedVerts, currentEdge, currentEdge, stride);

#if ASSERT_CHECK
    for (size_t clippedIndex = 0; clippedIndex < numClippedVerts; ++clippedIndex) {
      Vec3& vert = clippedVerts[clippedIndex];
      ZAssert(vert[0] >= -1.f);
      ZAssert(vert[0] <= 1.f);
      ZAssert(vert[1] >= -1.f);
      ZAssert(vert[1] <= 1.f);
      ZAssert(vert[2] >= -1.f);
      ZAssert(vert[2] <= 1.f);
    }
#endif

    if (numClippedVerts > 0) {
      size_t currentClipIndex = vertexBuffer.GetClipLength();

      vertexBuffer.AppendClipData(clipBuffer, numClippedVerts * stride * sizeof(float), numClippedVerts);

      Triangle nextTriangle(currentClipIndex, currentClipIndex + 1, currentClipIndex + 2);
      indexBuffer.AppendClipData(nextTriangle);

      if (numClippedVerts > 3) {
        for (size_t j = 2; j < numClippedVerts; j += 3) {
          const size_t clip0 = currentClipIndex + (j % numClippedVerts);
          const size_t clip1 = currentClipIndex + ((j + 1) % numClippedVerts);
          const size_t clip2 = currentClipIndex + ((j + 2) % numClippedVerts);

          Triangle clippedTriangle(clip0, clip1, clip2);
          indexBuffer.AppendClipData(clippedTriangle);
        }
      }
    }
#endif
  }
}

size_t SutherlandHodgmanClip(float* inputVerts, const size_t numInputVerts, const float clipEdge[3], const float edgeNormal[3], size_t stride) {
  size_t numOutputVerts = 0;

  const size_t vertByteSize = stride * sizeof(float);
  const size_t scratchSize = 128;

  float clipBuffer[scratchSize];
  memset(clipBuffer, 0, sizeof(clipBuffer));

  for (size_t i = 0; i < numInputVerts; ++i) {
    const size_t nextIndex = (i + 1) % numInputVerts;

    float* currentOffset = inputVerts + (i * stride);
    float* nextOffset = inputVerts + (nextIndex * stride);

    const bool p0Inside = InsidePlane(currentOffset, clipEdge, edgeNormal);
    const bool p1Inside = InsidePlane(nextOffset, clipEdge, edgeNormal);

    if (!p0Inside && !p1Inside) {
      continue;
    }
    else if (p0Inside && p1Inside) {
      // Unchanged input vertex.
      memcpy(clipBuffer + (numOutputVerts * stride),
        currentOffset,
        vertByteSize);
      ++numOutputVerts;
    }
    else {
      const float parametricValue = ParametricLinePlaneIntersection(currentOffset, nextOffset, edgeNormal, clipEdge);
      float clipPoint[4];
      GetParametricVector4D(parametricValue, currentOffset, nextOffset, clipPoint);

      const float* currentAttributes = currentOffset + 4;
      const float* nextAttributes = nextOffset + 4;

      float clippedAttributes[NumAttributes];
      memset(clippedAttributes, 0, sizeof(clippedAttributes));

      for (size_t j = 0; j < NumAttributes; ++j) {
        clippedAttributes[j] = Lerp(currentAttributes[j], nextAttributes[j], parametricValue);
      }

      if (!p0Inside) {
        // Clipped vertex.
        memcpy(clipBuffer + (numOutputVerts * stride),
          clipPoint, 
          sizeof(Vec4));
        // Clipped attributes.
        memcpy(clipBuffer + (numOutputVerts * stride) + 4,
          clippedAttributes,
          sizeof(clippedAttributes));
        ++numOutputVerts;

        // Unchanged input vertex.
        memcpy(clipBuffer + (numOutputVerts * stride), 
          nextOffset,
          vertByteSize);
        ++numOutputVerts;
      }
      else {
        // Unchanged input vertex.
        memcpy(clipBuffer + (numOutputVerts * stride),
          currentOffset,
          vertByteSize);
        ++numOutputVerts;

        // Clipped vertex.
        memcpy(clipBuffer + (numOutputVerts * stride),
          clipPoint, 
          sizeof(Vec4));
        // Clipped attributes.
        memcpy(clipBuffer + (numOutputVerts * stride) + 4,
          clippedAttributes,
          sizeof(clippedAttributes));
        ++numOutputVerts;
      }
    }
  }

  memcpy(inputVerts, clipBuffer, sizeof(clipBuffer));

  return numOutputVerts;
}

void CullBackFacingPrimitives(const VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer, const Vec3& viewer) {
  ZAssert((indexBuffer.GetIndexSize() % TRI_VERTS) == 0);
  
  if (indexBuffer.GetIndexSize() < TRI_VERTS) {
    return;
  }

  for (size_t i = indexBuffer.GetIndexSize(); i >= TRI_VERTS; i -= TRI_VERTS) {
    size_t i1 = indexBuffer[i - 3];
    size_t i2 = indexBuffer[i - 2];
    size_t i3 = indexBuffer[i - 1];
    const float* v1 = vertexBuffer[i1];
    const float* v2 = vertexBuffer[i2];
    const float* v3 = vertexBuffer[i3];
    const Vec3& firstEdge = *(reinterpret_cast<const Vec3*>(v1));
    const Vec3& secondEdge = *(reinterpret_cast<const Vec3*>(v2));
    const Vec3& thirdEdge = *(reinterpret_cast<const Vec3*>(v3));
    const Vec3 p1p0(secondEdge - firstEdge);
    const Vec3 p2p0(thirdEdge - firstEdge);
    const Vec3 triangleNormal(p1p0.Cross(p2p0));
    float dotResult = (viewer - firstEdge) * triangleNormal;
    if (FloatLessThanEqual(dotResult, 0.f)) {
      indexBuffer.RemoveTriangle(i - 3);
    }
  }
}
}
