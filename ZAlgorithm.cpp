#include "ZAlgorithm.h"

#include "Constants.h"
#include "CommonMath.h"

#include "ZAssert.h"
#include "ScopedTimer.h"
#include "PlatformIntrinsics.h"

#include <cstring>

#define DEBUG_CLIPPING 0

#define ASSERT_CHECK 0

static const size_t MaxOutVerts = 3;
static const size_t NumInVerts = 3;
static const size_t ScatchSize = 128;

namespace ZSharp {

bool InsidePlane(const float point[4], const float clipEdge[4], const float normal[4]) {
  return Unaligned_InsidePlane(point, clipEdge, normal);
}

void GetParametricVector4D(const float point, const float start[4], const float end[4], float outVec[4]) {
  Unaligned_ParametricVector4D(point, start, end, outVec);
}

float ParametricLinePlaneIntersection(const float start[4], const float end[4], const float edgeNormal[4], const float edgePoint[4]) {
  return Unaligned_ParametricLinePlaneIntersection(start, end, edgeNormal, edgePoint);
}

void ClipTrianglesNearPlane(VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer, float nearClipZ) {
  NamedScopedTimer(NearClip);

  const float nearEdge[4] = { 0.f, 0.f, nearClipZ, 0.f };
  const float edgeNormal[4] = { 0.f, 0.f, -1.f, 0.f };

  const int32 stride = vertexBuffer.GetStride();
  const int32 vertByteSize = stride * sizeof(float);

  const float* vertexBufferData = vertexBuffer[0];

  for (int32 i = 0; i < indexBuffer.GetIndexSize(); i += TRI_VERTS) {
    const int32 i1 = indexBuffer[i];
    const int32 i2 = indexBuffer[i + 1];
    const int32 i3 = indexBuffer[i + 2];
    const float* v1 = vertexBufferData + i1;
    const float* v2 = vertexBufferData + i2;
    const float* v3 = vertexBufferData + i3;

    float clipBuffer[ScatchSize] = {};
    memcpy(clipBuffer, v1, vertByteSize);
    memcpy(clipBuffer + stride, v2, vertByteSize);
    memcpy(clipBuffer + (stride * 2), v3, vertByteSize);

    const int32 numClippedVerts = SutherlandHodgmanClip(clipBuffer, 3, nearEdge, edgeNormal, stride);

    if (numClippedVerts > 0) {
      ZAssert(numClippedVerts < 7);

      int32 currentClipIndex = vertexBuffer.GetClipLength();

      vertexBuffer.AppendClipData(clipBuffer, numClippedVerts * vertByteSize, numClippedVerts);

      int32 nextTriangle[3] = { currentClipIndex * stride, (currentClipIndex + 1) * stride, (currentClipIndex + 2) * stride };
      indexBuffer.AppendClipData(nextTriangle, 3);

      if (numClippedVerts == 4) {
        const int32 clip0 = (currentClipIndex + 0) * stride;
        const int32 clip1 = (currentClipIndex + 2) * stride;
        const int32 clip2 = (currentClipIndex + 3) * stride;

        int32 clippedTriangle[3] = { clip0, clip1, clip2 };
        indexBuffer.AppendClipData(clippedTriangle, 3);
      }
      else if (numClippedVerts == 5) {
        const int32 clip0 = (currentClipIndex + 0) * stride;
        const int32 clip1 = (currentClipIndex + 2) * stride;
        const int32 clip2 = (currentClipIndex + 3) * stride;
        const int32 clip3 = (currentClipIndex + 3) * stride;
        const int32 clip4 = (currentClipIndex + 4) * stride;
        const int32 clip5 = (currentClipIndex + 0) * stride;

        int32 clippedTriangle[6] = { clip0, clip1, clip2, clip3, clip4, clip5 };
        indexBuffer.AppendClipData(clippedTriangle, 6);
      }
      else if (numClippedVerts == 6) {
        const int32 clip0 = (currentClipIndex + 0) * stride;
        const int32 clip1 = (currentClipIndex + 2) * stride;
        const int32 clip2 = (currentClipIndex + 3) * stride;
        const int32 clip3 = (currentClipIndex + 0) * stride;
        const int32 clip4 = (currentClipIndex + 3) * stride;
        const int32 clip5 = (currentClipIndex + 4) * stride;
        const int32 clip6 = (currentClipIndex + 4) * stride;
        const int32 clip7 = (currentClipIndex + 5) * stride;
        const int32 clip8 = (currentClipIndex + 0) * stride;

        int32 clippedTriangle[9] = { clip0, clip1, clip2, clip3, clip4, clip5, clip6, clip7, clip8 };
        indexBuffer.AppendClipData(clippedTriangle, 9);
      }
    }
  }

  vertexBuffer.ShuffleClippedData();
  indexBuffer.ShuffleClippedData();
}

void ClipTriangles(VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer) {
  NamedScopedTimer(NDC_Clip);

  const int32 stride = vertexBuffer.GetStride();
  const int32 vertByteSize = stride * sizeof(float);
  const int32 indexBufferSize = indexBuffer.GetIndexSize();

  const float* vertexBufferData = vertexBuffer[0];

  for (int32 i = 0; i < indexBufferSize; i += TRI_VERTS) {
    const int32 i1 = indexBuffer[i];
    const int32 i2 = indexBuffer[i + 1];
    const int32 i3 = indexBuffer[i + 2];
    const float* v1 = vertexBufferData + i1;
    const float* v2 = vertexBufferData + i2;
    const float* v3 = vertexBufferData + i3;

    float clipBuffer[ScatchSize] = {};
    memcpy(clipBuffer, v1, vertByteSize);
    memcpy(clipBuffer + stride, v2, vertByteSize);
    memcpy(clipBuffer + (stride * 2), v3, vertByteSize);

    int32 numClippedVerts = 3;

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

    float currentEdge[4] = {1.f, 0.f, 0.f, 0.f};
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
      ZAssert(numClippedVerts < 7);

      int32 currentClipIndex = vertexBuffer.GetClipLength();

      vertexBuffer.AppendClipData(clipBuffer, numClippedVerts * vertByteSize, numClippedVerts);

      int32 nextTriangle[3] = { currentClipIndex * stride, (currentClipIndex + 1) * stride, (currentClipIndex + 2) * stride };
      indexBuffer.AppendClipData(nextTriangle, 3);

      if (numClippedVerts == 4) {
        const int32 clip0 = (currentClipIndex + 0) * stride;
        const int32 clip1 = (currentClipIndex + 2) * stride;
        const int32 clip2 = (currentClipIndex + 3) * stride;

        int32 clippedTriangle[3] = { clip0, clip1, clip2 };
        indexBuffer.AppendClipData(clippedTriangle, 3);
      }
      else if (numClippedVerts == 5) {
        const int32 clip0 = (currentClipIndex + 0) * stride;
        const int32 clip1 = (currentClipIndex + 2) * stride;
        const int32 clip2 = (currentClipIndex + 3) * stride;
        const int32 clip3 = (currentClipIndex + 3) * stride;
        const int32 clip4 = (currentClipIndex + 4) * stride;
        const int32 clip5 = (currentClipIndex + 0) * stride;

        int32 clippedTriangle[6] = { clip0, clip1, clip2, clip3, clip4, clip5 };
        indexBuffer.AppendClipData(clippedTriangle, 6);
      }
      else if (numClippedVerts == 6) {
        const int32 clip0 = (currentClipIndex + 0) * stride;
        const int32 clip1 = (currentClipIndex + 2) * stride;
        const int32 clip2 = (currentClipIndex + 3) * stride;
        const int32 clip3 = (currentClipIndex + 0) * stride;
        const int32 clip4 = (currentClipIndex + 3) * stride;
        const int32 clip5 = (currentClipIndex + 4) * stride;
        const int32 clip6 = (currentClipIndex + 4) * stride;
        const int32 clip7 = (currentClipIndex + 5) * stride;
        const int32 clip8 = (currentClipIndex + 0) * stride;

        int32 clippedTriangle[9] = { clip0, clip1, clip2, clip3, clip4, clip5, clip6, clip7, clip8 };
        indexBuffer.AppendClipData(clippedTriangle, 9);
      }
    }
#endif
  }
}

int32 SutherlandHodgmanClip(float* inputVerts, const int32 numInputVerts, const float clipEdge[4], const float edgeNormal[4], int32 stride) {
  int32 numOutputVerts = 0;
  const int32 vertByteSize = stride * sizeof(float);
  float clipBuffer[ScatchSize];

  for (int32 i = 0; i < numInputVerts; ++i) {
    const int32 nextIndex = (i + 1) % numInputVerts;

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
        /*
          NOTE: We intentionaly do not add the second vertex when the first is outside the clip region.
            This avoids duplicating vertices since only those inside the clip region are added.
        */

        // Clipped vertex.
        memcpy(clipBuffer + (numOutputVerts * stride),
          clipPoint, 
          sizeof(clipPoint));

        // Clipped attributes.
        float* attributeOffset = (clipBuffer + (numOutputVerts * stride)) + 4;
        for (int32 j = 0; j < stride - 4; ++j) {
          attributeOffset[j] = Lerp(currentAttributes[j], nextAttributes[j], parametricValue);
        }
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
        for (int32 j = 0; j < stride - 4; ++j) {
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
  
  ZAssert((indexBuffer.GetIndexSize() % 3) == 0);
  
  if (indexBuffer.GetIndexSize() < 3) {
    return;
  }

  const float* view = *viewer;

  Aligned_BackfaceCull(indexBuffer, vertexBuffer, view);
}

void TriangulateAABB(const AABB& aabb, VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer) {
  const float* minVec = *aabb.MinBounds();
  const float* maxVec = *aabb.MaxBounds();
  
  const int32 aabbStride = 4; // XYZW
  vertexBuffer.Resize(8 * aabbStride, aabbStride);

  float aabbVerts[8 * aabbStride];

  // v1
  aabbVerts[0] = minVec[0];
  aabbVerts[1] = minVec[1];
  aabbVerts[2] = maxVec[2];
  aabbVerts[3] = 1.f;

  // v2
  aabbVerts[4] = maxVec[0];
  aabbVerts[5] = minVec[1];
  aabbVerts[6] = maxVec[2];
  aabbVerts[7] = 1.f;

  // v3
  aabbVerts[8] = maxVec[0];
  aabbVerts[9] = maxVec[1];
  aabbVerts[10] = maxVec[2];
  aabbVerts[11] = 1.f;

  // v4
  aabbVerts[12] = minVec[0];
  aabbVerts[13] = maxVec[1];
  aabbVerts[14] = maxVec[2];
  aabbVerts[15] = 1.f;

  // v5
  aabbVerts[16] = minVec[0];
  aabbVerts[17] = minVec[1];
  aabbVerts[18] = minVec[2];
  aabbVerts[19] = 1.f;

  // v6
  aabbVerts[20] = maxVec[0];
  aabbVerts[21] = minVec[1];
  aabbVerts[22] = minVec[2];
  aabbVerts[23] = 1.f;

  // v7
  aabbVerts[24] = maxVec[0];
  aabbVerts[25] = maxVec[1];
  aabbVerts[26] = minVec[2];
  aabbVerts[27] = 1.f;

  // v8
  aabbVerts[28] = minVec[0];
  aabbVerts[29] = maxVec[1];
  aabbVerts[30] = minVec[2];
  aabbVerts[31] = 1.f;

  vertexBuffer.CopyInputData(aabbVerts, 0, 8 * aabbStride);

  int32 indices[12 * 3];
  // Front Face
  indices[0] = 2 * aabbStride;
  indices[1] = 0 * aabbStride;
  indices[2] = 1 * aabbStride;

  indices[3] = 2 * aabbStride;
  indices[4] = 3 * aabbStride;
  indices[5] = 0 * aabbStride;

  // Right Face
  indices[6] = 6 * aabbStride;
  indices[7] = 1 * aabbStride;
  indices[8] = 5 * aabbStride;

  indices[9] = 6 * aabbStride;
  indices[10] = 2 * aabbStride;
  indices[11] = 1 * aabbStride;

  // Back Face
  indices[12] = 7 * aabbStride;
  indices[13] = 5 * aabbStride;
  indices[14] = 4 * aabbStride;

  indices[15] = 7 * aabbStride;
  indices[16] = 6 * aabbStride;
  indices[17] = 5 * aabbStride;

  // Left Face
  indices[18] = 3 * aabbStride;
  indices[19] = 4 * aabbStride;
  indices[20] = 0 * aabbStride;

  indices[21] = 3 * aabbStride;
  indices[22] = 7 * aabbStride;
  indices[23] = 4 * aabbStride;

  // Top Face
  indices[24] = 6 * aabbStride;
  indices[25] = 3 * aabbStride;
  indices[26] = 2 * aabbStride;

  indices[27] = 6 * aabbStride;
  indices[28] = 7 * aabbStride;
  indices[29] = 3 * aabbStride;

  // Bottom Face
  indices[30] = 1 * aabbStride;
  indices[31] = 4 * aabbStride;
  indices[32] = 5 * aabbStride;

  indices[33] = 1 * aabbStride;
  indices[34] = 0 * aabbStride;
  indices[35] = 4 * aabbStride;

  indexBuffer.Resize(12 * 3);
  indexBuffer.CopyInputData(indices, 0, 12 * 3);
}

void TriangulateAABBWithColor(const AABB& aabb, VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer, const ZColor& color) {
  const float* minVec = *aabb.MinBounds();
  const float* maxVec = *aabb.MaxBounds();

  const float R = color.R() / 255.f;
  const float G = color.G() / 255.f;
  const float B = color.B() / 255.f;

  const int32 aabbStride = 4 + 3; // XYZW, RGB
  vertexBuffer.Resize(8 * aabbStride, aabbStride);

  float aabbVerts[8 * aabbStride];

  // v1
  aabbVerts[0] = minVec[0];
  aabbVerts[1] = minVec[1];
  aabbVerts[2] = maxVec[2];
  aabbVerts[3] = 1.f;
  aabbVerts[4] = R;
  aabbVerts[5] = G;
  aabbVerts[6] = B;

  // v2
  aabbVerts[7] = maxVec[0];
  aabbVerts[8] = minVec[1];
  aabbVerts[9] = maxVec[2];
  aabbVerts[10] = 1.f;
  aabbVerts[11] = R;
  aabbVerts[12] = G;
  aabbVerts[13] = B;

  // v3
  aabbVerts[14] = maxVec[0];
  aabbVerts[15] = maxVec[1];
  aabbVerts[16] = maxVec[2];
  aabbVerts[17] = 1.f;
  aabbVerts[18] = R;
  aabbVerts[19] = G;
  aabbVerts[20] = B;

  // v4
  aabbVerts[21] = minVec[0];
  aabbVerts[22] = maxVec[1];
  aabbVerts[23] = maxVec[2];
  aabbVerts[24] = 1.f;
  aabbVerts[25] = R;
  aabbVerts[26] = G;
  aabbVerts[27] = B;

  // v5
  aabbVerts[28] = minVec[0];
  aabbVerts[29] = minVec[1];
  aabbVerts[30] = minVec[2];
  aabbVerts[31] = 1.f;
  aabbVerts[32] = R;
  aabbVerts[33] = G;
  aabbVerts[34] = B;

  // v6
  aabbVerts[35] = maxVec[0];
  aabbVerts[36] = minVec[1];
  aabbVerts[37] = minVec[2];
  aabbVerts[38] = 1.f;
  aabbVerts[39] = R;
  aabbVerts[40] = G;
  aabbVerts[41] = B;

  // v7
  aabbVerts[42] = maxVec[0];
  aabbVerts[43] = maxVec[1];
  aabbVerts[44] = minVec[2];
  aabbVerts[45] = 1.f;
  aabbVerts[46] = R;
  aabbVerts[47] = G;
  aabbVerts[48] = B;

  // v8
  aabbVerts[49] = minVec[0];
  aabbVerts[50] = maxVec[1];
  aabbVerts[51] = minVec[2];
  aabbVerts[52] = 1.f;
  aabbVerts[53] = R;
  aabbVerts[54] = G;
  aabbVerts[55] = B;

  vertexBuffer.CopyInputData(aabbVerts, 0, 8 * aabbStride);

  int32 indices[12 * 3];
  // Front Face
  indices[0] = 2 * aabbStride;
  indices[1] = 0 * aabbStride;
  indices[2] = 1 * aabbStride;

  indices[3] = 2 * aabbStride;
  indices[4] = 3 * aabbStride;
  indices[5] = 0 * aabbStride;

  // Right Face
  indices[6] = 6 * aabbStride;
  indices[7] = 1 * aabbStride;
  indices[8] = 5 * aabbStride;

  indices[9] = 6 * aabbStride;
  indices[10] = 2 * aabbStride;
  indices[11] = 1 * aabbStride;

  // Back Face
  indices[12] = 7 * aabbStride;
  indices[13] = 5 * aabbStride;
  indices[14] = 4 * aabbStride;

  indices[15] = 7 * aabbStride;
  indices[16] = 6 * aabbStride;
  indices[17] = 5 * aabbStride;

  // Left Face
  indices[18] = 3 * aabbStride;
  indices[19] = 4 * aabbStride;
  indices[20] = 0 * aabbStride;

  indices[21] = 3 * aabbStride;
  indices[22] = 7 * aabbStride;
  indices[23] = 4 * aabbStride;

  // Top Face
  indices[24] = 6 * aabbStride;
  indices[25] = 3 * aabbStride;
  indices[26] = 2 * aabbStride;

  indices[27] = 6 * aabbStride;
  indices[28] = 7 * aabbStride;
  indices[29] = 3 * aabbStride;

  // Bottom Face
  indices[30] = 1 * aabbStride;
  indices[31] = 4 * aabbStride;
  indices[32] = 5 * aabbStride;

  indices[33] = 1 * aabbStride;
  indices[34] = 0 * aabbStride;
  indices[35] = 4 * aabbStride;

  indexBuffer.Resize(12 * 3);
  indexBuffer.CopyInputData(indices, 0, 12 * 3);
}

}
