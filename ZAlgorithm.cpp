#include "ZAlgorithm.h"

#include "Constants.h"
#include "CommonMath.h"

#include "ZAssert.h"
#include "ScopedTimer.h"
#include "PlatformIntrinsics.h"

#include <cstring>

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

  float clipBuffer[ScatchSize * 2];

  for (int32 i = 0; i < indexBuffer.GetIndexSize(); i += TRI_VERTS) {
    const int32 i1 = indexBuffer[i];
    const int32 i2 = indexBuffer[i + 1];
    const int32 i3 = indexBuffer[i + 2];
    const float* v1 = vertexBufferData + i1;
    const float* v2 = vertexBufferData + i2;
    const float* v3 = vertexBufferData + i3;

    memcpy(clipBuffer, v1, vertByteSize);
    memcpy(clipBuffer + stride, v2, vertByteSize);
    memcpy(clipBuffer + (stride * 2), v3, vertByteSize);

    const int32 numClippedVerts = SutherlandHodgmanClip(clipBuffer, clipBuffer + ScatchSize, 3, nearEdge, edgeNormal, stride);

    if (numClippedVerts > 0) {
      // There should never be more than 4 verts after a near clip pass since we don't clip against any other planes.
      ZAssert(numClippedVerts < 5);

      int32 currentClipIndex = vertexBuffer.GetClipLength();

      vertexBuffer.AppendClipData(clipBuffer + ScatchSize, numClippedVerts * vertByteSize, numClippedVerts);

      int32 clippedTriangle[6];
      int32 clipLength = 3;

      clippedTriangle[0] = currentClipIndex * stride;
      clippedTriangle[1] = (currentClipIndex + 1) * stride;
      clippedTriangle[2] = (currentClipIndex + 2) * stride;

      if (numClippedVerts == 4) {
        clippedTriangle[3] = (currentClipIndex + 0) * stride;
        clippedTriangle[4] = (currentClipIndex + 2) * stride;
        clippedTriangle[5] = (currentClipIndex + 3) * stride;
        clipLength += 3;
      }

      indexBuffer.AppendClipData(clippedTriangle, clipLength);
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

  float clipBuffer[ScatchSize * 2];

  for (int32 i = 0; i < indexBufferSize; i += TRI_VERTS) {
    const int32 i1 = indexBuffer[i];
    const int32 i2 = indexBuffer[i + 1];
    const int32 i3 = indexBuffer[i + 2];
    const float* v1 = vertexBufferData + i1;
    const float* v2 = vertexBufferData + i2;
    const float* v3 = vertexBufferData + i3;

    memcpy(clipBuffer, v1, vertByteSize);
    memcpy(clipBuffer + stride, v2, vertByteSize);
    memcpy(clipBuffer + (stride * 2), v3, vertByteSize);

    int32 numClippedVerts = 3;

    float currentEdge[4] = {1.f, 0.f, 0.f, 0.f};
    numClippedVerts = SutherlandHodgmanClip(clipBuffer, clipBuffer + ScatchSize, numClippedVerts, currentEdge, currentEdge, stride);

    currentEdge[0] = 0.f;
    currentEdge[1] = 1.f;
    numClippedVerts = SutherlandHodgmanClip(clipBuffer + ScatchSize, clipBuffer, numClippedVerts, currentEdge, currentEdge, stride);

    currentEdge[0] = -1.f;
    currentEdge[1] = 0.f;
    numClippedVerts = SutherlandHodgmanClip(clipBuffer, clipBuffer + ScatchSize, numClippedVerts, currentEdge, currentEdge, stride);

    currentEdge[0] = 0.f;
    currentEdge[1] = -1.f;
    numClippedVerts = SutherlandHodgmanClip(clipBuffer + ScatchSize, clipBuffer, numClippedVerts, currentEdge, currentEdge, stride);

    currentEdge[1] = 0.f;
    currentEdge[2] = -1.f;
    numClippedVerts = SutherlandHodgmanClip(clipBuffer, clipBuffer + ScatchSize, numClippedVerts, currentEdge, currentEdge, stride);

    if (numClippedVerts > 0) {
      ZAssert(numClippedVerts < 7);

      int32 currentClipIndex = vertexBuffer.GetClipLength();

      vertexBuffer.AppendClipData(clipBuffer + ScatchSize, numClippedVerts * vertByteSize, numClippedVerts);

      int32 clippedTriangle[12];
      int32 clipLength = 3;

      clippedTriangle[0] = currentClipIndex * stride;
      clippedTriangle[1] = (currentClipIndex + 1) * stride;
      clippedTriangle[2] = (currentClipIndex + 2) * stride;

      if (numClippedVerts >= 4) {
        clippedTriangle[3] = (currentClipIndex + 0) * stride;
        clippedTriangle[4] = (currentClipIndex + 2) * stride;
        clippedTriangle[5] = (currentClipIndex + 3) * stride;
        clipLength += 3;
      }

      if (numClippedVerts >= 5) {
        clippedTriangle[6] = (currentClipIndex + 0) * stride;
        clippedTriangle[7] = (currentClipIndex + 3) * stride;
        clippedTriangle[8] = (currentClipIndex + 4) * stride;
        clipLength += 3;
      }

      if (numClippedVerts >= 6) {
        clippedTriangle[9] = (currentClipIndex + 0) * stride;
        clippedTriangle[10] = (currentClipIndex + 4) * stride;
        clippedTriangle[11] = (currentClipIndex + 5) * stride;
        clipLength += 3;
      }

      indexBuffer.AppendClipData(clippedTriangle, clipLength);
    }
  }
}

int32 SutherlandHodgmanClip(float* inputVerts, float* outputVerts, const int32 numInputVerts, const float clipEdge[4], const float edgeNormal[4], int32 stride) {
  int32 numOutputVerts = 0;
  const int32 vertByteSize = stride * sizeof(float);

  bool p0Inside = InsidePlane(inputVerts, clipEdge, edgeNormal);

  for (int32 i = 0; i < numInputVerts; ++i) {
    // Mod causes an idiv, this instead generates a cmov which is actually much faster.
    const int32 nextIndex = ((i + 1) == numInputVerts) ? 0 : (i + 1);

    const float* currentOffset = inputVerts + (i * stride);
    const float* nextOffset = inputVerts + (nextIndex * stride);

    const bool p1Inside = InsidePlane(nextOffset, clipEdge, edgeNormal);

    if (p0Inside) {
      // Unchanged input vertex.
      memcpy(outputVerts + (numOutputVerts * stride),
        currentOffset,
        vertByteSize);
      ++numOutputVerts;
    }
    
    if((p0Inside && !p1Inside) || (!p0Inside && p1Inside)) {
      const float parametricValue = ParametricLinePlaneIntersection(currentOffset, nextOffset, edgeNormal, clipEdge);
      
      // Clipped vertex, implicitly stores the clipped vertex in the clip buffer.
      float* outputOffset = (outputVerts + (numOutputVerts * stride));
      GetParametricVector4D(parametricValue, currentOffset, nextOffset, outputOffset);

      // Clipped attributes.
      for (int32 j = 4; j < stride; j+=4) {
        Unaligned_LerpAttribute(currentOffset + j, nextOffset + j, outputOffset + j, parametricValue);
      }

      ++numOutputVerts;
    }

    p0Inside = p1Inside;
  }

  return numOutputVerts;
}

void CullBackFacingPrimitives(const VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer) {
  NamedScopedTimer(BackfaceCull);
  
  ZAssert((indexBuffer.GetIndexSize() % 3) == 0);
  
  if (indexBuffer.GetIndexSize() < 3) {
    return;
  }

  Aligned_BackfaceCull(indexBuffer, vertexBuffer);
}

void TriangulateAABB(const AABB& aabb, VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer, bool shouldResize) {
  const float* minVec = *aabb.MinBounds();
  const float* maxVec = *aabb.MaxBounds();
  
  const int32 aabbStride = 4; // XYZW
  if (shouldResize) {
    vertexBuffer.Resize(8 * aabbStride, aabbStride);
    indexBuffer.Resize(12 * 3);
  }

  float aabbVerts[8 * aabbStride];
  int32 indices[12 * 3];

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

  vertexBuffer.CopyInputData(aabbVerts, 0, 8 * aabbStride);
  indexBuffer.CopyInputData(indices, 0, 12 * 3);
}

}
