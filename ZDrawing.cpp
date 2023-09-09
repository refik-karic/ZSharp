#include "ZDrawing.h"

#include <cmath>
#include <cstring>

#include "ZColor.h"
#include "Common.h"
#include "CommonMath.h"
#include "Constants.h"
#include "ScopedTimer.h"
#include "Triangle.h"

namespace ZSharp {

static GlobalEdgeTable globalEdgeTable(1080, 3);

float PerspectiveLerp(const float p0,
  const float p1,
  const float p0Z,
  const float invP0z,
  const float p1Z,
  const float invP1z,
  const float t) {
  // Assuming projection onto +1 Z plane.
  float numeratorP0 = (p0 / p0Z) * (t);
  float numeratorP1 = (p1 / p1Z) * (1 - t);

  const float finalNumerator = numeratorP0 + numeratorP1;
  const float finalDenominator = fmaf(invP0z, t, invP1z * (1 - t));
  const float result = finalNumerator / finalDenominator;

  return result;
}

void DrawRunSlice(Framebuffer& framebuffer, 
                  const float* p0,
                  const float* p1) {
  int32 x1 = static_cast<int32>(p0[0]);
  int32 y1 = static_cast<int32>(p0[1]);
  int32 x2 = static_cast<int32>(p1[0]);
  int32 y2 = static_cast<int32>(p1[1]);

  const float* p0Attributes = p0 + 4;
  const float* p1Attributes = p1 + 4;

  // Vertical line
  if (x1 == x2) {
    if (y2 < y1) {
      Swap(y1, y2);
    }

    for (; y1 < y2; y1++) {
      float yT = ParametricSolveForT(static_cast<float>(y1), p1[1], p0[1]);
      float R = PerspectiveLerp(p0Attributes[0], p1Attributes[0], p0[2], p0[3], p1[2], p1[3], yT);
      float G = PerspectiveLerp(p0Attributes[1], p1Attributes[1], p0[2], p0[3], p1[2], p1[3], yT);
      float B = PerspectiveLerp(p0Attributes[2], p1Attributes[2], p0[2], p0[3], p1[2], p1[3], yT);
      ZColor color(R, G, B);
      framebuffer.SetPixel(x1, y1, color);
    }
  }
  else if (y1 == y2) { // Horizontal line
    if (x2 < x1) {
      Swap(x1, x2);
    }

    for (int32 i = x1; i < x2; ++i) {
      float xT = ParametricSolveForT(static_cast<float>(i), p1[0], p0[0]);
      float R = PerspectiveLerp(p0Attributes[0], p1Attributes[0], p0[2], p0[3], p1[2], p1[3], xT);
      float G = PerspectiveLerp(p0Attributes[1], p1Attributes[1], p0[2], p0[3], p1[2], p1[3], xT);
      float B = PerspectiveLerp(p0Attributes[2], p1Attributes[2], p0[2], p0[3], p1[2], p1[3], xT);
      ZColor color(R, G, B);
      framebuffer.SetPixel(i, y1, color);
    }
  }
  else {
    float slope;
    float error = 0.f;
    int32 slopeStep;
    int32 delta;

    if (y2 < y1) { // Always draw up and to left/right
      Swap(y1, y2);
      Swap(x1, x2);
    }

    if (abs(x2 - x1) >= abs(y2 - y1)) { // Horizontal slope
      delta = abs(y2 - y1);
      slope = fabs(static_cast<float>((x2 - x1)) / (y2 - y1));

      for (size_t i = 0; i < delta; i++) {
        error = error + (slope - floor(slope));
        slopeStep = static_cast<int32>(floor(slope) + error);

        if (slopeStep > static_cast<int32>(slope)) {
          error = fmax(error - 1.f, 0.f);
        }

        if (x2 <= x1) { // Drawing right to left, writing pixels left to right for cache coherency.
          for (int32 j = x1 - slopeStep; j < x1; ++j) {
            float xT = ParametricSolveForT(static_cast<float>(j), p1[0], p0[0]);
            float R = PerspectiveLerp(p0Attributes[0], p1Attributes[0], p0[2], p0[3], p1[2], p1[3], xT);
            float G = PerspectiveLerp(p0Attributes[1], p1Attributes[1], p0[2], p0[3], p1[2], p1[3], xT);
            float B = PerspectiveLerp(p0Attributes[2], p1Attributes[2], p0[2], p0[3], p1[2], p1[3], xT);
            ZColor color(R, G, B);
            framebuffer.SetPixel(j, y1, color);
          }

          x1 -= slopeStep;
        }
        else { // Drawing left to right
          for (int32 j = x1; j < x1 + slopeStep; ++j) {
            float xT = ParametricSolveForT(static_cast<float>(j), p1[0], p0[0]);
            float R = PerspectiveLerp(p0Attributes[0], p1Attributes[0], p0[2], p0[3], p1[2], p1[3], xT);
            float G = PerspectiveLerp(p0Attributes[1], p1Attributes[1], p0[2], p0[3], p1[2], p1[3], xT);
            float B = PerspectiveLerp(p0Attributes[2], p1Attributes[2], p0[2], p0[3], p1[2], p1[3], xT);
            ZColor color(R, G, B);
            framebuffer.SetPixel(j, y1, color);
          }

          x1 += slopeStep;
        }

        y1++; // Move vertical in either direction
      }
    }
    else { // Vertical slope
      delta = abs(x2 - x1);
      int32 minorStep = (x2 - x1) / delta;
      slope = fabs(static_cast<float>((y2 - y1)) / (x2 - x1));

      for (size_t i = 0; i < delta; i++) {
        error = error + (slope - floor(slope));
        slopeStep = static_cast<int32>(floor(slope) + error);

        if (slopeStep > static_cast<int32>(slope)) {
          error = fmax(error - 1.f, 0.f);
        }

        // Draw a vertical span for the current X
        for (int32 j = y1; j < y1 + slopeStep; j++) {
          float yT = ParametricSolveForT(static_cast<float>(j), p1[1], p0[1]);
          float R = PerspectiveLerp(p0Attributes[0], p1Attributes[0], p0[2], p0[3], p1[2], p1[3], yT);
          float G = PerspectiveLerp(p0Attributes[1], p1Attributes[1], p0[2], p0[3], p1[2], p1[3], yT);
          float B = PerspectiveLerp(p0Attributes[2], p1Attributes[2], p0[2], p0[3], p1[2], p1[3], yT);
          ZColor color(R, G, B);
          framebuffer.SetPixel(x1, j, color);
        }

        x1 += minorStep;
        y1 += slopeStep;
      }
    }
  }
}

void TraceLine(const float* p0, const float* p1, size_t attributeStride) {
  int32 x1 = static_cast<int32>(p0[0]);
  int32 y1 = static_cast<int32>(p0[1]);
  int32 x2 = static_cast<int32>(p1[0]);
  int32 y2 = static_cast<int32>(p1[1]);

  const float* p0Attributes = p0 + 4;
  const float* p1Attributes = p1 + 4;

  float* attributeBuffer = globalEdgeTable.AttributeScratchBuffer();

  // Vertical Line
  if (x1 == x2) {
    if (y2 < y1) {
      Swap(y1, y2);
    }

    for (int32 y = y1; y < y2; y++) {
      float yT = ParametricSolveForT(static_cast<float>(y), p1[1], p0[1]);

      for (size_t a = 0; a < attributeStride; ++a) {
        attributeBuffer[a] = PerspectiveLerp(p0Attributes[a], p1Attributes[a], p0[2], p0[3], p1[2], p1[3], yT);
      }

      globalEdgeTable.AddPoint(y, x1, attributeBuffer);
    }
  }
  else if (y1 == y2) { // Horizontal line.
    if (x2 < x1) {
      Swap(x1, x2);
    }

    memcpy(attributeBuffer, p0Attributes, attributeStride * sizeof(float));
    globalEdgeTable.AddPoint(y1, x1, attributeBuffer);

    memcpy(attributeBuffer, p1Attributes, attributeStride * sizeof(float));
    globalEdgeTable.AddPoint(y1, x2, attributeBuffer);
  }
  else {
    float slope;
    float error = 0.f;
    int32 slopeStep;
    int32 delta;

    if (y2 < y1) {
      Swap(y1, y2);
      Swap(x1, x2);
    }

    if (abs(x2 - x1) >= abs(y2 - y1)) { // Horizontal slope.
      delta = abs(y2 - y1);
      slope = fabs(static_cast<float>((x2 - x1)) / (y2 - y1));

      for (size_t i = 0; i < delta; i++) {
        error = error + (slope - floor(slope));
        slopeStep = static_cast<int32>(floor(slope) + error);

        if (slopeStep > static_cast<int32>(slope)) {
          error = fmax(error - 1.f, 0.f);
        }

        if (x2 <= x1) { // Tracing right to left.
          float xT = ParametricSolveForT(static_cast<float>(x1), p1[0], p0[0]);

          for (size_t a = 0; a < attributeStride; ++a) {
            attributeBuffer[a] = PerspectiveLerp(p0Attributes[a], p1Attributes[a], p0[2], p0[3], p1[2], p1[3], xT);
          }

          globalEdgeTable.AddPoint(y1, x1, attributeBuffer);

          x1 -= slopeStep;
        }
        else { // Tracing left to right.
          float xT = ParametricSolveForT(static_cast<float>(x1), p1[0], p0[0]);

          for (size_t a = 0; a < attributeStride; ++a) {
            attributeBuffer[a] = PerspectiveLerp(p0Attributes[a], p1Attributes[a], p0[2], p0[3], p1[2], p1[3], xT);
          }

          globalEdgeTable.AddPoint(y1, x1, attributeBuffer);

          x1 += slopeStep;
        }

        y1++;
      }
    }
    else { // Vertical slope.
      delta = abs(x2 - x1);
      int32 minorStep = (x2 - x1) / delta;
      slope = fabs(static_cast<float>((y2 - y1)) / (x2 - x1));

      for (size_t i = 0; i < delta; i++) {
        error = error + (slope - floor(slope));
        slopeStep = static_cast<int32>(floor(slope) + error);

        if (slopeStep > static_cast<int32>(slope)) {
          error = fmax(error - 1.f, 0.f);
        }

        for (int32 j = y1; j < y1 + slopeStep; j++) {
          float yT = ParametricSolveForT(static_cast<float>(j), p1[1], p0[1]);

          for (size_t a = 0; a < attributeStride; ++a) {
            attributeBuffer[a] = PerspectiveLerp(p0Attributes[a], p1Attributes[a], p0[2], p0[3], p1[2], p1[3], yT);
          }

          globalEdgeTable.AddPoint(j, x1, attributeBuffer);
        }

        x1 += minorStep;
        y1 += slopeStep;
      }
    }
  }
}

void DrawTrianglesFlat(Framebuffer& framebuffer, const VertexBuffer& vertexBuffer, const IndexBuffer& indexBuffer, const ShadingModeOrder& order, const Texture* texture) {
  NamedScopedTimer(DrawFlatTriangles);
  
  size_t stride = 0;
  for (ShadingMode& mode : order) {
    stride += mode.length;
  }
  
  const size_t frameHeight = framebuffer.GetHeight();
  const float maxHeight = (float)frameHeight;

  if ((globalEdgeTable.TableHeight() != frameHeight) || (globalEdgeTable.AttributeSize() != stride)) {
    globalEdgeTable.Resize(frameHeight, stride);
  }

  const size_t vertexStride = vertexBuffer.GetStride();
  const float* vertexClipData = vertexBuffer.GetClipData(0);
  const size_t* indexClipData = indexBuffer.GetClipData(0);
  const size_t end = indexBuffer.GetClipLength();
  for (size_t i = 0; i < end; i += TRI_VERTS) {
    const float* v1 = vertexClipData + (indexClipData[i] * vertexStride);
    const float* v2 = vertexClipData + (indexClipData[i + 1] * vertexStride);
    const float* v3 = vertexClipData + (indexClipData[i + 2] * vertexStride);

    // Calculate the amount of scan lines a triangle occupies.
    // Round to prev/next to avoid flooring.
    float minY = Min(Min(v1[1], v2[1]), v3[1]);
    float maxY = Max(Max(v1[1], v2[1]), v3[1]);

    minY -= 1.f;
    maxY += 1.f;

    TraceLine(v1, v2, stride);
    TraceLine(v2, v3, stride);
    TraceLine(v3, v1, stride);

    Clamp(minY, 0.f, maxHeight);
    Clamp(maxY, 0.f, maxHeight);

    size_t startY = (size_t)minY;
    size_t endY = (size_t)maxY;

    globalEdgeTable.Draw(framebuffer, order, texture, startY, endY);
  }
}

void DrawTrianglesWireframe(Framebuffer& framebuffer, const VertexBuffer& vertexBuffer, const IndexBuffer& indexBuffer) {
  NamedScopedTimer(DrawWireframeTriangles);
  
  const size_t vertexStride = vertexBuffer.GetStride();
  const float* vertexClipData = vertexBuffer.GetClipData(0);
  const size_t* indexClipData = indexBuffer.GetClipData(0);
  const size_t end = indexBuffer.GetClipLength();
  for (size_t i = 0; i < end; i += TRI_VERTS) {
    const float* v1 = vertexClipData + (indexClipData[i] * vertexStride);
    const float* v2 = vertexClipData + (indexClipData[i + 1] * vertexStride);
    const float* v3 = vertexClipData + (indexClipData[i + 2] * vertexStride);
    DrawRunSlice(framebuffer, v1, v2);
    DrawRunSlice(framebuffer, v2, v3);
    DrawRunSlice(framebuffer, v3, v1);
  }
}

}