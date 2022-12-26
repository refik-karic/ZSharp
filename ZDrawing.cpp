#include "ZDrawing.h"

#include <cmath>

#include "ZColor.h"
#include "Common.h"
#include "CommonMath.h"
#include "Constants.h"
#include "Triangle.h"

namespace ZSharp {

float PerspectiveLerp(const float p0,
  const float p1,
  const float p0Z,
  const float invP0z,
  const float p1Z,
  const float invP1z,
  const float t) {
  // Assuming projection onto -1 Z plane.
  float numeratorP0 = (p0 / -p0Z) * (t);
  float numeratorP1 = (p1 / -p1Z) * (1 - t);

  const float finalNumerator = numeratorP0 + numeratorP1;

  const float denominatorP0 = invP0z * t;
  const float denominatorP1 = invP1z * (1 - t);

  const float finalDenominator = denominatorP0 + denominatorP1;

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
      float yT = ParametricSolveForT(static_cast<float>(y1), p0[1], p1[1]);
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
      float xT = ParametricSolveForT(static_cast<float>(i), p0[0], p1[0]);
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
            float xT = ParametricSolveForT(static_cast<float>(j), p0[0], p1[0]);
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
            float xT = ParametricSolveForT(static_cast<float>(j), p0[0], p1[0]);
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
          float yT = ParametricSolveForT(static_cast<float>(j), p0[1], p1[1]);
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

void TraceLine(GlobalEdgeTable& edgeTable, const float* p0, const float* p1, size_t primitiveIndex) {
  int32 x1 = static_cast<int32>(p0[0]);
  int32 y1 = static_cast<int32>(p0[1]);
  int32 x2 = static_cast<int32>(p1[0]);
  int32 y2 = static_cast<int32>(p1[1]);
  
  const float* p0Attributes = p0 + 4;
  const float* p1Attributes = p1 + 4;

  // Vertical Line
  if (x1 == x2) {
    if (y2 < y1) {
      Swap(y1, y2);
    }

    for (int32 y = y1; y < y2; y++) {
      float yT = ParametricSolveForT(static_cast<float>(y), p0[1], p1[1]);
      float R = PerspectiveLerp(p0Attributes[0], p1Attributes[0], p0[2], p0[3], p1[2], p1[3], yT);
      float G = PerspectiveLerp(p0Attributes[1], p1Attributes[1], p0[2], p0[3], p1[2], p1[3], yT);
      float B = PerspectiveLerp(p0Attributes[2], p1Attributes[2], p0[2], p0[3], p1[2], p1[3], yT);
      ZColor color(R, G, B);
      edgeTable.AddPoint(y1, x1, color, primitiveIndex);
    }
  }
  else if (y1 == y2) { // Horizontal line.
    if (x2 < x1) {
      Swap(x1, x2);
    }

    ZColor x1Color(p0Attributes[0], p0Attributes[1], p0Attributes[2]);
    ZColor x2Color(p1Attributes[0], p1Attributes[1], p1Attributes[2]);
    edgeTable.AddPoint(y1, x1, x1Color, primitiveIndex);
    edgeTable.AddPoint(y1, x2, x2Color, primitiveIndex);
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
          float xT = ParametricSolveForT(static_cast<float>(x1), p0[0], p1[0]);
          float R = PerspectiveLerp(p0Attributes[0], p1Attributes[0], p0[2], p0[3], p1[2], p1[3], xT);
          float G = PerspectiveLerp(p0Attributes[1], p1Attributes[1], p0[2], p0[3], p1[2], p1[3], xT);
          float B = PerspectiveLerp(p0Attributes[2], p1Attributes[2], p0[2], p0[3], p1[2], p1[3], xT);
          ZColor color(R, G, B);
          edgeTable.AddPoint(y1, x1, color, primitiveIndex);
          x1 -= slopeStep;
        }
        else { // Tracing left to right.
          float xT = ParametricSolveForT(static_cast<float>(x1), p0[0], p1[0]);
          float R = PerspectiveLerp(p0Attributes[0], p1Attributes[0], p0[2], p0[3], p1[2], p1[3], xT);
          float G = PerspectiveLerp(p0Attributes[1], p1Attributes[1], p0[2], p0[3], p1[2], p1[3], xT);
          float B = PerspectiveLerp(p0Attributes[2], p1Attributes[2], p0[2], p0[3], p1[2], p1[3], xT);
          ZColor color(R, G, B);
          edgeTable.AddPoint(y1, x1, color, primitiveIndex);
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

        for (size_t j = y1; j < y1 + slopeStep; j++) {
          float yT = ParametricSolveForT(static_cast<float>(j), p0[1], p1[1]);
          float R = PerspectiveLerp(p0Attributes[0], p1Attributes[0], p0[2], p0[3], p1[2], p1[3], yT);
          float G = PerspectiveLerp(p0Attributes[1], p1Attributes[1], p0[2], p0[3], p1[2], p1[3], yT);
          float B = PerspectiveLerp(p0Attributes[2], p1Attributes[2], p0[2], p0[3], p1[2], p1[3], yT);
          ZColor color(R, G, B);
          edgeTable.AddPoint(j, x1, color, primitiveIndex);
        }

        x1 += minorStep;
        y1 += slopeStep;
      }
    }
  }
}

void DrawTrianglesFlat(Framebuffer& framebuffer, const VertexBuffer& vertexBuffer, const IndexBuffer& indexBuffer) {
    GlobalEdgeTable edgeTable(framebuffer.GetHeight());

    size_t end = indexBuffer.GetClipLength();
    for (size_t i = 0; i < end; i += TRI_VERTS) {
      const float* v1 = vertexBuffer.GetClipData(indexBuffer.GetClipData(i));
      const float* v2 = vertexBuffer.GetClipData(indexBuffer.GetClipData(i + 1));
      const float* v3 = vertexBuffer.GetClipData(indexBuffer.GetClipData(i + 2));

      TraceLine(edgeTable, v1, v2, i);
      TraceLine(edgeTable, v2, v3, i);
      TraceLine(edgeTable, v3, v1, i);
    }

    edgeTable.Draw(framebuffer);
}

void DrawTrianglesWireframe(Framebuffer& framebuffer, const VertexBuffer& vertexBuffer, const IndexBuffer& indexBuffer) {
    size_t end = indexBuffer.GetClipLength();
    for (size_t i = 0; i < end; i += TRI_VERTS) {
        const float* v1 = vertexBuffer.GetClipData(indexBuffer.GetClipData(i));
        const float* v2 = vertexBuffer.GetClipData(indexBuffer.GetClipData(i + 1));
        const float* v3 = vertexBuffer.GetClipData(indexBuffer.GetClipData(i + 2));
        DrawRunSlice(framebuffer, v1, v2);
        DrawRunSlice(framebuffer, v2, v3);
        DrawRunSlice(framebuffer, v3, v1);
    }
}

}