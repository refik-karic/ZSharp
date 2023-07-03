#include "ZAlgorithm.h"

#include "Constants.h"
#include "CommonMath.h"

#include "ZAssert.h"

#include <cstring>

#define DEBUG_CLIPPING 0

#define ASSERT_CHECK 0

static const size_t MaxOutVerts = 3;
static const size_t NumInVerts = 3;
static const size_t NumAttributes = 3;

namespace ZSharp {
bool InsidePlane(const Vec3& point, const Vec3& clipEdge, const Vec3& normal) {
  return FloatLessThan((point - clipEdge) * normal, 0.f);
}

Vec3 GetParametricVector3D(const float point, const Vec3& start, const Vec3& end) {
  return (start + ((end - start) * point));
}

Vec4 GetParametricVector4D(const float point, const Vec4& start, const Vec4& end) {
  return (start + ((end - start) * point));
}

float ParametricLinePlaneIntersection(const Vec3& start, const Vec3& end, const Vec3& edgeNormal, const Vec3& edgePoint) {
  float numerator = edgeNormal * (start - edgePoint);
  float denominator = (-edgeNormal) * (end - start);
  return (numerator / denominator);
}

void ClipTrianglesNearPlane(VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer, float nearClipZ) {
  const Vec3 nearEdge(0.f, 0.f, nearClipZ);
  const Vec3 edgeNormal(0.f, 0.f, -1.f);

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
  Vec3 currentEdge;

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
    currentEdge[0] = 1.f;
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
    currentEdge.Clear();

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

size_t SutherlandHodgmanClip(float* inputVerts, const size_t numInputVerts, const Vec3& clipEdge, const Vec3& edgeNormal, size_t stride) {
  size_t numOutputVerts = 0;

  const size_t vertByteSize = stride * sizeof(float);
  const size_t scratchSize = 128;

  float clipBuffer[scratchSize];
  memset(clipBuffer, 0, sizeof(clipBuffer));

  for (size_t i = 0; i < numInputVerts; ++i) {
    const size_t nextIndex = (i + 1) % numInputVerts;

    float* currentOffset = inputVerts + (i * stride);
    float* nextOffset = inputVerts + (nextIndex * stride);

    Vec3& currentVert = *reinterpret_cast<Vec3*>(currentOffset);
    Vec3& nextVert = *reinterpret_cast<Vec3*>(nextOffset);

    Vec4& currentVert4D = *reinterpret_cast<Vec4*>(currentOffset);
    Vec4& nextVert4D = *reinterpret_cast<Vec4*>(nextOffset);

    const bool p0Inside = InsidePlane(currentVert, clipEdge, edgeNormal);
    const bool p1Inside = InsidePlane(nextVert, clipEdge, edgeNormal);

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
      const float parametricValue = ParametricLinePlaneIntersection(currentVert, nextVert, edgeNormal, clipEdge);
      const Vec4 clipPoint(GetParametricVector4D(parametricValue, currentVert4D, nextVert4D));

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
          reinterpret_cast<const float*>(&clipPoint), 
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
          reinterpret_cast<const float*>(&clipPoint), 
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
