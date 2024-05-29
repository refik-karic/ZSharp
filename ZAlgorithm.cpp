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

  const int32 numAttributes = vertexBuffer.GetNumAttributes();
  const int32 stride = 4 + numAttributes;

  const float* vertexBufferDataX = vertexBuffer.GetInputDataX(0);
  const float* vertexBufferDataY = vertexBuffer.GetInputDataY(0);
  const float* vertexBufferDataZ = vertexBuffer.GetInputDataZ(0);
  const float* vertexBufferDataW = vertexBuffer.GetInputDataW(0);

  for (int32 i = 0; i < indexBuffer.GetIndexSize(); i += TRI_VERTS) {
    const int32 i1 = indexBuffer[i];
    const int32 i2 = indexBuffer[i + 1];
    const int32 i3 = indexBuffer[i + 2];
    const float v1[] = { vertexBufferDataX[i1], vertexBufferDataY[i1], vertexBufferDataZ[i1], vertexBufferDataW[i1] };
    const float v2[] = { vertexBufferDataX[i2], vertexBufferDataY[i2], vertexBufferDataZ[i2], vertexBufferDataW[i2] };
    const float v3[] = { vertexBufferDataX[i3], vertexBufferDataY[i3], vertexBufferDataZ[i3], vertexBufferDataW[i3] };

    float clipBuffer[ScatchSize] = {};
    
    memcpy(clipBuffer, v1, sizeof(v1));
    memcpy(clipBuffer + stride, v2, sizeof(v2));
    memcpy(clipBuffer + (stride * 2), v3, sizeof(v3));

    for (int32 j = 0; j < numAttributes; ++j) {
      float* attribute0 = vertexBuffer.GetAttributeData(i1, j);
      float* attribute1 = vertexBuffer.GetAttributeData(i2, j);
      float* attribute2 = vertexBuffer.GetAttributeData(i3, j);
      clipBuffer[4 + j] = *attribute0;
      clipBuffer[(4 + j) + stride] = *attribute1;
      clipBuffer[(4 + j) + (stride * 2)] = *attribute2;
    }

    const int32 numClippedVerts = SutherlandHodgmanClip(clipBuffer, 3, nearEdge, edgeNormal, stride);

    if (numClippedVerts > 0) {
      ZAssert(numClippedVerts < 7);

      int32 currentClipIndex = vertexBuffer.GetClipLength();

      for (int32 j = 0; j < numClippedVerts; ++j) {
        int32 strideOffset = (j * stride);
        vertexBuffer.AppendClipDataAOS(clipBuffer + strideOffset, (4 + numAttributes) * sizeof(float), 1, numAttributes);
      }

      int32 nextTriangle[3] = { currentClipIndex, currentClipIndex + 1, currentClipIndex + 2 };
      indexBuffer.AppendClipData(nextTriangle, 3);

      if (numClippedVerts == 4) {
        const int32 clip0 = (currentClipIndex + 0);
        const int32 clip1 = (currentClipIndex + 2);
        const int32 clip2 = (currentClipIndex + 3);

        int32 clippedTriangle[3] = { clip0, clip1, clip2 };
        indexBuffer.AppendClipData(clippedTriangle, 3);
      }
      else if (numClippedVerts == 5) {
        const int32 clip0 = (currentClipIndex + 0);
        const int32 clip1 = (currentClipIndex + 2);
        const int32 clip2 = (currentClipIndex + 3);
        const int32 clip3 = (currentClipIndex + 3);
        const int32 clip4 = (currentClipIndex + 4);
        const int32 clip5 = (currentClipIndex + 0);

        int32 clippedTriangle[6] = { clip0, clip1, clip2, clip3, clip4, clip5 };
        indexBuffer.AppendClipData(clippedTriangle, 6);
      }
      else if (numClippedVerts == 6) {
        const int32 clip0 = (currentClipIndex + 0);
        const int32 clip1 = (currentClipIndex + 2);
        const int32 clip2 = (currentClipIndex + 3);
        const int32 clip3 = (currentClipIndex + 0);
        const int32 clip4 = (currentClipIndex + 3);
        const int32 clip5 = (currentClipIndex + 4);
        const int32 clip6 = (currentClipIndex + 4);
        const int32 clip7 = (currentClipIndex + 5);
        const int32 clip8 = (currentClipIndex + 0);

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

  const int32 numAttributes = vertexBuffer.GetNumAttributes();
  const int32 stride = 4 + numAttributes;
  const int32 indexBufferSize = indexBuffer.GetIndexSize();

  const float* vertexBufferDataX = vertexBuffer.GetInputDataX(0);
  const float* vertexBufferDataY = vertexBuffer.GetInputDataY(0);
  const float* vertexBufferDataZ = vertexBuffer.GetInputDataZ(0);
  const float* vertexBufferDataW = vertexBuffer.GetInputDataW(0);

  for (int32 i = 0; i < indexBufferSize; i += TRI_VERTS) {
    const int32 i1 = indexBuffer[i];
    const int32 i2 = indexBuffer[i + 1];
    const int32 i3 = indexBuffer[i + 2];
    const float v1[] = { vertexBufferDataX[i1], vertexBufferDataY[i1], vertexBufferDataZ[i1], vertexBufferDataW[i1] };
    const float v2[] = { vertexBufferDataX[i2], vertexBufferDataY[i2], vertexBufferDataZ[i2], vertexBufferDataW[i2] };
    const float v3[] = { vertexBufferDataX[i3], vertexBufferDataY[i3], vertexBufferDataZ[i3], vertexBufferDataW[i3] };

    float clipBuffer[ScatchSize] = {};

    memcpy(clipBuffer, v1, sizeof(v1));
    memcpy(clipBuffer + stride, v2, sizeof(v2));
    memcpy(clipBuffer + (stride * 2), v3, sizeof(v3));

    for (int32 j = 0; j < numAttributes; ++j) {
      float* attribute0 = vertexBuffer.GetAttributeData(i1, j);
      float* attribute1 = vertexBuffer.GetAttributeData(i2, j);
      float* attribute2 = vertexBuffer.GetAttributeData(i3, j);
      clipBuffer[4 + j] = *attribute0;
      clipBuffer[(4 + j) + stride] = *attribute1;
      clipBuffer[(4 + j) + (stride * 2)] = *attribute2;
    }

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
      ZAssert(numClippedVerts < 7);

      int32 currentClipIndex = vertexBuffer.GetClipLength();

      for (int32 j = 0; j < numClippedVerts; ++j) {
        int32 strideOffset = (j * stride);
        vertexBuffer.AppendClipDataAOS(clipBuffer + strideOffset, (4 + numAttributes) * sizeof(float), 1, numAttributes);
      }

      int32 nextTriangle[3] = { currentClipIndex, currentClipIndex + 1, currentClipIndex + 2 };
      indexBuffer.AppendClipData(nextTriangle, 3);

      if (numClippedVerts == 4) {
        const int32 clip0 = (currentClipIndex + 0);
        const int32 clip1 = (currentClipIndex + 2);
        const int32 clip2 = (currentClipIndex + 3);

        int32 clippedTriangle[3] = { clip0, clip1, clip2 };
        indexBuffer.AppendClipData(clippedTriangle, 3);
      }
      else if (numClippedVerts == 5) {
        const int32 clip0 = (currentClipIndex + 0);
        const int32 clip1 = (currentClipIndex + 2);
        const int32 clip2 = (currentClipIndex + 3);
        const int32 clip3 = (currentClipIndex + 3);
        const int32 clip4 = (currentClipIndex + 4);
        const int32 clip5 = (currentClipIndex + 0);

        int32 clippedTriangle[6] = { clip0, clip1, clip2, clip3, clip4, clip5 };
        indexBuffer.AppendClipData(clippedTriangle, 6);
      }
      else if (numClippedVerts == 6) {
        const int32 clip0 = (currentClipIndex + 0);
        const int32 clip1 = (currentClipIndex + 2);
        const int32 clip2 = (currentClipIndex + 3);
        const int32 clip3 = (currentClipIndex + 0);
        const int32 clip4 = (currentClipIndex + 3);
        const int32 clip5 = (currentClipIndex + 4);
        const int32 clip6 = (currentClipIndex + 4);
        const int32 clip7 = (currentClipIndex + 5);
        const int32 clip8 = (currentClipIndex + 0);

        int32 clippedTriangle[9] = { clip0, clip1, clip2, clip3, clip4, clip5, clip6, clip7, clip8 };
        indexBuffer.AppendClipData(clippedTriangle, 9);
      }
    }
#endif
  }
}

int32 SutherlandHodgmanClip(float* inputVerts, const int32 numInputVerts, const float clipEdge[3], const float edgeNormal[3], int32 stride) {
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
  
  vertexBuffer.Resize(8, 0);

  float aabbVertsX[8];
  float aabbVertsY[8];
  float aabbVertsZ[8];
  float aabbVertsW[8];
  float* aabbVerts[4] = { aabbVertsX, aabbVertsY, aabbVertsZ, aabbVertsW };

  // v1
  aabbVertsX[0] = minVec[0];
  aabbVertsY[0] = minVec[1];
  aabbVertsZ[0] = maxVec[2];
  aabbVertsW[0] = 1.f;

  // v2
  aabbVertsX[1] = maxVec[0];
  aabbVertsY[1] = minVec[1];
  aabbVertsZ[1] = maxVec[2];
  aabbVertsW[1] = 1.f;

  // v3
  aabbVertsX[2] = maxVec[0];
  aabbVertsY[2] = maxVec[1];
  aabbVertsZ[2] = maxVec[2];
  aabbVertsW[2] = 1.f;

  // v4
  aabbVertsX[3] = minVec[0];
  aabbVertsY[3] = maxVec[1];
  aabbVertsZ[3] = maxVec[2];
  aabbVertsW[3] = 1.f;

  // v5
  aabbVertsX[4] = minVec[0];
  aabbVertsY[4] = minVec[1];
  aabbVertsZ[4] = minVec[2];
  aabbVertsW[4] = 1.f;

  // v6
  aabbVertsX[5] = maxVec[0];
  aabbVertsY[5] = minVec[1];
  aabbVertsZ[5] = minVec[2];
  aabbVertsW[5] = 1.f;

  // v7
  aabbVertsX[6] = maxVec[0];
  aabbVertsY[6] = maxVec[1];
  aabbVertsZ[6] = minVec[2];
  aabbVertsW[6] = 1.f;

  // v8
  aabbVertsX[7] = minVec[0];
  aabbVertsY[7] = maxVec[1];
  aabbVertsZ[7] = minVec[2];
  aabbVertsW[7] = 1.f;

  vertexBuffer.CopyInputData((const float**)aabbVerts, 0, 8, 0);

  int32 indices[12 * 3];
  // Front Face
  indices[0] = 2;
  indices[1] = 0;
  indices[2] = 1;

  indices[3] = 2;
  indices[4] = 3;
  indices[5] = 0;

  // Right Face
  indices[6] = 6;
  indices[7] = 1;
  indices[8] = 5;

  indices[9] = 6;
  indices[10] = 2;
  indices[11] = 1;

  // Back Face
  indices[12] = 7;
  indices[13] = 5;
  indices[14] = 4;

  indices[15] = 7;
  indices[16] = 6;
  indices[17] = 5;

  // Left Face
  indices[18] = 3;
  indices[19] = 4;
  indices[20] = 0;

  indices[21] = 3;
  indices[22] = 7;
  indices[23] = 4;

  // Top Face
  indices[24] = 6;
  indices[25] = 3;
  indices[26] = 2;

  indices[27] = 6;
  indices[28] = 7;
  indices[29] = 3;

  // Bottom Face
  indices[30] = 1;
  indices[31] = 4;
  indices[32] = 5;

  indices[33] = 1;
  indices[34] = 0;
  indices[35] = 4;

  indexBuffer.Resize(12 * 3);
  indexBuffer.CopyInputData(indices, 0, 12 * 3);
}

void TriangulateAABBWithColor(const AABB& aabb, VertexBuffer& vertexBuffer, IndexBuffer& indexBuffer, const ZColor& color) {
  const float* minVec = *aabb.MinBounds();
  const float* maxVec = *aabb.MaxBounds();

  const float R = color.R() / 255.f;
  const float G = color.G() / 255.f;
  const float B = color.B() / 255.f;

  vertexBuffer.Resize(8, 3);

  float aabbVertsX[8];
  float aabbVertsY[8];
  float aabbVertsZ[8];
  float aabbVertsW[8];
  float aabbVertsR[8];
  float aabbVertsG[8];
  float aabbVertsB[8];
  const float* aabbVerts[7] = { aabbVertsX, aabbVertsY, aabbVertsZ, aabbVertsW, aabbVertsR, aabbVertsG, aabbVertsB };

  // v1
  aabbVertsX[0] = minVec[0];
  aabbVertsY[0] = minVec[1];
  aabbVertsZ[0] = maxVec[2];
  aabbVertsW[0] = 1.f;
  aabbVertsR[0] = R;
  aabbVertsG[0] = G;
  aabbVertsB[0] = B;

  // v2
  aabbVertsX[1] = maxVec[0];
  aabbVertsY[1] = minVec[1];
  aabbVertsZ[1] = maxVec[2];
  aabbVertsW[1] = 1.f;
  aabbVertsR[1] = R;
  aabbVertsG[1] = G;
  aabbVertsB[1] = B;

  // v3
  aabbVertsX[2] = maxVec[0];
  aabbVertsY[2] = maxVec[1];
  aabbVertsZ[2] = maxVec[2];
  aabbVertsW[2] = 1.f;
  aabbVertsR[2] = R;
  aabbVertsG[2] = G;
  aabbVertsB[2] = B;

  // v4
  aabbVertsX[3] = minVec[0];
  aabbVertsY[3] = maxVec[1];
  aabbVertsZ[3] = maxVec[2];
  aabbVertsW[3] = 1.f;
  aabbVertsR[3] = R;
  aabbVertsG[3] = G;
  aabbVertsB[3] = B;

  // v5
  aabbVertsX[4] = minVec[0];
  aabbVertsY[4] = minVec[1];
  aabbVertsZ[4] = minVec[2];
  aabbVertsW[4] = 1.f;
  aabbVertsR[4] = R;
  aabbVertsG[4] = G;
  aabbVertsB[4] = B;

  // v6
  aabbVertsX[5] = maxVec[0];
  aabbVertsY[5] = minVec[1];
  aabbVertsZ[5] = minVec[2];
  aabbVertsW[5] = 1.f;
  aabbVertsR[5] = R;
  aabbVertsG[5] = G;
  aabbVertsB[5] = B;

  // v7
  aabbVertsX[6] = maxVec[0];
  aabbVertsY[6] = maxVec[1];
  aabbVertsZ[6] = minVec[2];
  aabbVertsW[6] = 1.f;
  aabbVertsR[6] = R;
  aabbVertsG[6] = G;
  aabbVertsB[6] = B;

  // v8
  aabbVertsX[7] = minVec[0];
  aabbVertsY[7] = maxVec[1];
  aabbVertsZ[7] = minVec[2];
  aabbVertsW[7] = 1.f;
  aabbVertsR[7] = R;
  aabbVertsG[7] = G;
  aabbVertsB[7] = B;

  vertexBuffer.CopyInputData(aabbVerts, 0, 8, 3);

  int32 indices[12 * 3];
  // Front Face
  indices[0] = 2;
  indices[1] = 0;
  indices[2] = 1;

  indices[3] = 2;
  indices[4] = 3;
  indices[5] = 0;

  // Right Face
  indices[6] = 6;
  indices[7] = 1;
  indices[8] = 5;

  indices[9] = 6;
  indices[10] = 2;
  indices[11] = 1;

  // Back Face
  indices[12] = 7;
  indices[13] = 5;
  indices[14] = 4;

  indices[15] = 7;
  indices[16] = 6;
  indices[17] = 5;

  // Left Face
  indices[18] = 3;
  indices[19] = 4;
  indices[20] = 0;

  indices[21] = 3;
  indices[22] = 7;
  indices[23] = 4;

  // Top Face
  indices[24] = 6;
  indices[25] = 3;
  indices[26] = 2;

  indices[27] = 6;
  indices[28] = 7;
  indices[29] = 3;

  // Bottom Face
  indices[30] = 1;
  indices[31] = 4;
  indices[32] = 5;

  indices[33] = 1;
  indices[34] = 0;
  indices[35] = 4;

  indexBuffer.Resize(12 * 3);
  indexBuffer.CopyInputData(indices, 0, 12 * 3);
}

}
