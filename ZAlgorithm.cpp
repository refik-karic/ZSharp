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
static const size_t ScatchSize = 128;

namespace ZSharp {

bool InsidePlane(const float point[3], const float clipEdge[3], const float normal[3]) {
  return (((point[0] - clipEdge[0]) * normal[0]) +
    ((point[1] - clipEdge[1]) * normal[1]) +
    ((point[2] - clipEdge[2]) * normal[2])) < 0.f;
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

  const float nearEdge[3] = { 0.f, 0.f, nearClipZ };
  const float edgeNormal[3] = { 0.f, 0.f, -1.f };

  const size_t stride = vertexBuffer.GetStride();
  const size_t vertByteSize = stride * sizeof(float);

  for (size_t i = 0; i < indexBuffer.GetIndexSize(); i += TRI_VERTS) {
    const size_t i1 = indexBuffer[i];
    const size_t i2 = indexBuffer[i + 1];
    const size_t i3 = indexBuffer[i + 2];
    const float* v1 = vertexBuffer[i1];
    const float* v2 = vertexBuffer[i2];
    const float* v3 = vertexBuffer[i3];

    float clipBuffer[ScatchSize];
    memset(clipBuffer, 0, sizeof(clipBuffer));
    memcpy(clipBuffer, v1, vertByteSize);
    memcpy(clipBuffer + stride, v2, vertByteSize);
    memcpy(clipBuffer + (stride * 2), v3, vertByteSize);

    const size_t numClippedVerts = SutherlandHodgmanClip(clipBuffer, 3, nearEdge, edgeNormal, stride);

    if (numClippedVerts > 0) {
      size_t currentClipIndex = vertexBuffer.GetClipLength();

      vertexBuffer.AppendClipData(clipBuffer, numClippedVerts * vertByteSize, numClippedVerts);

      Triangle nextTriangle(currentClipIndex, currentClipIndex + 1, currentClipIndex + 2);
      indexBuffer.AppendClipData(nextTriangle);

      if (numClippedVerts > 3) {
        ZAssert(numClippedVerts < 6);

        for (size_t j = 2; j < numClippedVerts; j+=3) {
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
  const size_t indexBufferSize = indexBuffer.GetIndexSize();

  for (size_t i = 0; i < indexBufferSize; i += TRI_VERTS) {
    const size_t i1 = indexBuffer[i];
    const size_t i2 = indexBuffer[i + 1];
    const size_t i3 = indexBuffer[i + 2];
    const float* v1 = vertexBuffer[i1];
    const float* v2 = vertexBuffer[i2];
    const float* v3 = vertexBuffer[i3];

    float clipBuffer[ScatchSize] = {};
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

      vertexBuffer.AppendClipData(clipBuffer, numClippedVerts * vertByteSize, numClippedVerts);

      Triangle nextTriangle(currentClipIndex, currentClipIndex + 1, currentClipIndex + 2);
      indexBuffer.AppendClipData(nextTriangle);

      if (numClippedVerts > 3) {
        for (size_t j = 2; j < numClippedVerts; j+=3) {
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
  float clipBuffer[ScatchSize];

  for (size_t i = 0; i < numInputVerts; ++i) {
    const size_t nextIndex = (i + 1) % numInputVerts;

    const float* currentOffset = inputVerts + (i * stride);
    const float* nextOffset = inputVerts + (nextIndex * stride);

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

      if (!p0Inside) {
        // Clipped vertex.
        memcpy(clipBuffer + (numOutputVerts * stride),
          clipPoint, 
          sizeof(clipPoint));

        // Clipped attributes.
        float* attributeOffset = (clipBuffer + (numOutputVerts * stride)) + 4;
        for (size_t j = 0; j < stride - 4; ++j) {
          attributeOffset[j] = Lerp(currentAttributes[j], nextAttributes[j], parametricValue);
        }
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
          sizeof(clipPoint));

        // Clipped attributes.
        float* attributeOffset = (clipBuffer + (numOutputVerts * stride)) + 4;
        for (size_t j = 0; j < stride - 4; ++j) {
          attributeOffset[j] = Lerp(currentAttributes[j], nextAttributes[j], parametricValue);
        }
        ++numOutputVerts;
      }
    }
  }

  memcpy(inputVerts, clipBuffer, numOutputVerts * vertByteSize);

  return numOutputVerts;
}

void CullBackFacingPrimitives(const VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer, const Vec3& viewer) {
  NamedScopedTimer(BackfaceCull);
  
  ZAssert((indexBuffer.GetIndexSize() % TRI_VERTS) == 0);
  
  if (indexBuffer.GetIndexSize() < TRI_VERTS) {
    return;
  }

  const float* view = *viewer;

  for (size_t i = indexBuffer.GetIndexSize(); i >= TRI_VERTS; i -= TRI_VERTS) {
    size_t i1 = indexBuffer[i - 3];
    size_t i2 = indexBuffer[i - 2];
    size_t i3 = indexBuffer[i - 1];
    const float* v1 = vertexBuffer[i1];
    const float* v2 = vertexBuffer[i2];
    const float* v3 = vertexBuffer[i3];

    // TODO: Move this into a dedicate math header.
    // We're doing vec3 sub, cross product, and dot product.
    const float p1p0[3] = { v2[0] - v1[0], v2[1] - v1[1], v2[2] - v1[2] };
    const float p2p0[3] = { v3[0] - v1[0], v3[1] - v1[1], v3[2] - v1[2] };

    float normal[3] = {
      (p1p0[1] * p2p0[2]) - (p1p0[2] * p2p0[1]),
      (p1p0[2] * p2p0[0]) - (p1p0[0] * p2p0[2]),
      (p1p0[0] * p2p0[1]) - (p1p0[1] * p2p0[0]),
    };

    float dotResult = ((v1[0] - view[0]) * normal[0]) +
      ((v1[1] - view[1]) * normal[1]) +
      ((v1[2] - view[2]) * normal[2]);
    if (dotResult < 0.f) {
      indexBuffer.RemoveTriangle(i - 3);
    }
  }
}

void TriangulateAABB(const AABB& aabb, VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer) {
  const size_t aabbStride = 4;
  vertexBuffer.Resize(8 * aabbStride, aabbStride);

  float aabbVerts[8 * aabbStride];
  const float* minVec = *aabb.MinBounds();
  const float* maxVec = *aabb.MaxBounds();

  // v1
  aabbVerts[0] = minVec[0];
  aabbVerts[1] = minVec[1];
  aabbVerts[2] = minVec[2];
  aabbVerts[3] = 1.f;

  // v2
  aabbVerts[4] = maxVec[0];
  aabbVerts[5] = minVec[1];
  aabbVerts[6] = minVec[2];
  aabbVerts[7] = 1.f;

  // v3
  aabbVerts[8] = maxVec[0];
  aabbVerts[9] = maxVec[1];
  aabbVerts[10] = minVec[2];
  aabbVerts[11] = 1.f;

  // v4
  aabbVerts[12] = minVec[0];
  aabbVerts[13] = maxVec[1];
  aabbVerts[14] = minVec[2];
  aabbVerts[15] = 1.f;

  // v5
  aabbVerts[16] = minVec[0];
  aabbVerts[17] = minVec[1];
  aabbVerts[18] = maxVec[2];
  aabbVerts[19] = 1.f;

  // v6
  aabbVerts[20] = maxVec[0];
  aabbVerts[21] = minVec[1];
  aabbVerts[22] = maxVec[2];
  aabbVerts[23] = 1.f;

  // v7
  aabbVerts[24] = maxVec[0];
  aabbVerts[25] = maxVec[1];
  aabbVerts[26] = maxVec[2];
  aabbVerts[27] = 1.f;

  // v8
  aabbVerts[28] = minVec[0];
  aabbVerts[29] = maxVec[1];
  aabbVerts[30] = maxVec[2];
  aabbVerts[31] = 1.f;

  vertexBuffer.CopyInputData(aabbVerts, 0, 8 * aabbStride);

  size_t indices[12 * 3];
  // Back Face
  indices[0] = 0;
  indices[1] = 1;
  indices[2] = 2;

  indices[3] = 2;
  indices[4] = 3;
  indices[5] = 0;

  // Right Face
  indices[6] = 5;
  indices[7] = 1;
  indices[8] = 2;

  indices[9] = 2;
  indices[10] = 6;
  indices[11] = 5;

  // Front Face
  indices[12] = 4;
  indices[13] = 5;
  indices[14] = 6;

  indices[15] = 6;
  indices[16] = 7;
  indices[17] = 4;

  // Left Face
  indices[18] = 4;
  indices[19] = 0;
  indices[20] = 3;

  indices[21] = 3;
  indices[22] = 7;
  indices[23] = 4;

  // Top Face
  indices[24] = 7;
  indices[25] = 6;
  indices[26] = 2;

  indices[27] = 2;
  indices[28] = 3;
  indices[29] = 7;

  // Bottom Face
  indices[30] = 4;
  indices[31] = 5;
  indices[32] = 1;

  indices[33] = 1;
  indices[34] = 0;
  indices[35] = 4;

  indexBuffer.Resize(12 * 3);
  indexBuffer.CopyInputData(indices, 0, 12 * 3);
}
}
