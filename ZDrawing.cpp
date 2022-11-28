#include "ZDrawing.h"

#include <cmath>

#include "Common.h"
#include "Constants.h"
#include "Triangle.h"

namespace ZSharp {

float ParametricSolveForT(const float step, const float p0, const float p1) {
  const float numerator = step - p0;
  const float denominator = p1 - p0;
  return numerator / denominator;
}

float PerspectiveLerp(const float p0, const float p1, const float p0Z, const float p1Z, const float t) {
  // Assuming projection onto -1 Z plane.
  float numeratorP0 = (p0 / -p0Z) * (t);
  float numeratorP1 = (p1 / -p1Z) * (1 - t);

  const float finalNumerator = numeratorP0 + numeratorP1;

  const float denominatorP0 = (1 / -p0Z) * t;
  const float denominatorP1 = (1 / -p1Z) * (1 - t);

  const float finalDenominator = denominatorP0 + denominatorP1;

  const float result = finalNumerator / finalDenominator;

  return result;
}

void DrawRunSlice(Framebuffer& framebuffer, 
                  const Vec4& p0,
                  const Vec4& p1,
                  const float* p0Attributes,
                  const float* p1Attributes) {
  int32 x1 = static_cast<int32>(p0[0]);
  int32 y1 = static_cast<int32>(p0[1]);
  int32 x2 = static_cast<int32>(p1[0]);
  int32 y2 = static_cast<int32>(p1[1]);

  // TODO: Add logic to calculate parametric T value for the vertex pairs and use PerspectiveLerp() to obtain the final color, texture, etc values.

  (void)p1Attributes;

  if (x1 == x2) {
    if (y2 < y1) {
      Swap(y1, y2);
    }

    for (; y1 < y2; y1++) {
      ZColor color;
      color.FloatToRGB(p0Attributes[0], p0Attributes[1], p0Attributes[2]);
      framebuffer.SetPixel(x1, y1, color);
    }
  }
  else if (y1 == y2) {
    if (x2 < x1) {
      Swap(x1, x2);
    }

    ZColor color;
    color.FloatToRGB(p0Attributes[0], p0Attributes[1], p0Attributes[2]);
    framebuffer.SetRow(y1, x1, x2, color);
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

    if (abs(x2 - x1) >= abs(y2 - y1)) {
      delta = abs(y2 - y1);
      slope = fabs(static_cast<float>((x2 - x1)) / (y2 - y1));

      for (size_t i = 0; i < delta; i++) {
        error = error + (slope - floor(slope));
        slopeStep = static_cast<int32>(floor(slope) + error);

        if (slopeStep > static_cast<int32>(slope)) {
          error = fmax(error - 1.f, 0.f);
        }

        if (x2 <= x1) {
          // TODO: Calculate perspective correct UV's here using ParametricSolveForT() and PerspectiveLerp().
          ZColor color;
          color.FloatToRGB(p0Attributes[0], p0Attributes[1], p0Attributes[2]);
          framebuffer.SetRow(y1, x1 - slopeStep, x1, color);
          x1 -= slopeStep;
        }
        else {
          ZColor color;
          color.FloatToRGB(p0Attributes[0], p0Attributes[1], p0Attributes[2]);
          framebuffer.SetRow(y1, x1, x1 + slopeStep, color);
          x1 += slopeStep;
        }

        y1++;
      }
    }
    else {
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
          ZColor color;
          color.FloatToRGB(p0Attributes[0], p0Attributes[1], p0Attributes[2]);
          framebuffer.SetPixel(x1, j, color);
        }

        x1 += minorStep;
        y1 += slopeStep;
      }
    }
  }
}

void TracePrimitive(GlobalEdgeTable& edgeTable, FixedArray<size_t, 2>& p1, FixedArray<size_t, 2>& p2, FixedArray<size_t, 2>& p3, ZColor color, size_t primitiveIndex) {
  TraceLine(edgeTable, (int32)p1[0], (int32)p1[1], (int32)p2[0], (int32)p2[1], color, primitiveIndex);
  TraceLine(edgeTable, (int32)p2[0], (int32)p2[1], (int32)p3[0], (int32)p3[1], color, primitiveIndex);
  TraceLine(edgeTable, (int32)p3[0], (int32)p3[1], (int32)p1[0], (int32)p1[1], color, primitiveIndex);
}

void TraceLine(GlobalEdgeTable& edgeTable, int32 x1, int32 y1, int32 x2, int32 y2, ZColor color, size_t primitiveIndex) {
  if (x1 == x2) {
    if (y2 < y1) {
      Swap(y1, y2);
    }

    while (y1 < y2) {
      edgeTable.AddPoint(y1, x1, color, primitiveIndex);
      y1++;
    }
  }
  else if (y1 == y2) {
    if (x2 < x1) {
      Swap(x1, x2);
    }

    edgeTable.AddPoint(y1, x1, color, primitiveIndex);
    edgeTable.AddPoint(y1, x2, color, primitiveIndex);
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

    if (abs(x2 - x1) >= abs(y2 - y1)) {
      delta = abs(y2 - y1);
      slope = fabs(static_cast<float>((x2 - x1)) / (y2 - y1));

      for (size_t i = 0; i < delta; i++) {
        error = error + (slope - floor(slope));
        slopeStep = static_cast<int32>(floor(slope) + error);

        if (slopeStep > static_cast<int32>(slope)) {
          error = fmax(error - 1.f, 0.f);
        }

        if (x2 <= x1) {
          edgeTable.AddPoint(y1, x1, color, primitiveIndex);
          x1 -= slopeStep;
        } else {
          edgeTable.AddPoint(y1, x1, color, primitiveIndex);
          x1 += slopeStep;
        }

        y1++;
      }
    } else {
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
          edgeTable.AddPoint(j, x1, color, primitiveIndex);
        }

        x1 += minorStep;
        y1 += slopeStep;
      }
    }
  }
}

void DrawTrianglesFlat(Framebuffer& framebuffer, const VertexBuffer& vertexBuffer, const IndexBuffer& indexBuffer, ZColor color) {
    GlobalEdgeTable edgeTable(framebuffer.GetHeight());

    size_t end = indexBuffer.GetClipLength();
    for (size_t i = 0; i < end; i += TRI_VERTS) {
        const float* v1 = vertexBuffer.GetClipData(indexBuffer.GetClipData(i));
        const float* v2 = vertexBuffer.GetClipData(indexBuffer.GetClipData(i + 1));
        const float* v3 = vertexBuffer.GetClipData(indexBuffer.GetClipData(i + 2));

        FixedArray<size_t, 2> p1;
        p1[0] = static_cast<size_t>(*v1);
        p1[1] = static_cast<size_t>(*(v1 + 1));
        FixedArray<size_t, 2> p2;
        p2[0] = static_cast<size_t>(*v2);
        p2[1] = static_cast<size_t>(*(v2 + 1));
        FixedArray<size_t, 2> p3;
        p3[0] = static_cast<size_t>(*v3);
        p3[1] = static_cast<size_t>(*(v3 + 1));

        TracePrimitive(edgeTable, p1, p2, p3, color, i);
    }

    edgeTable.Draw(framebuffer);
}

void DrawTrianglesWireframe(Framebuffer& framebuffer, const VertexBuffer& vertexBuffer, const IndexBuffer& indexBuffer) {
    size_t end = indexBuffer.GetClipLength();
    for (size_t i = 0; i < end; i += TRI_VERTS) {
        const Vec3& v1 = *reinterpret_cast<const Vec3*>(vertexBuffer.GetClipData(indexBuffer.GetClipData(i)));
        const Vec3& v2 = *reinterpret_cast<const Vec3*>(vertexBuffer.GetClipData(indexBuffer.GetClipData(i + 1)));
        const Vec3& v3 = *reinterpret_cast<const Vec3*>(vertexBuffer.GetClipData(indexBuffer.GetClipData(i + 2)));

        Vec4 v1Vec(v1);
        Vec4 v2Vec(v2);
        Vec4 v3Vec(v3);
        // W axis is inverse perspective Z for each vertex.
        v1Vec[3] = 1 / v1Vec[2];
        v2Vec[3] = 1 / v2Vec[2];
        v3Vec[3] = 1 / v3Vec[2];

        const float red[] = { 1.f, 0.f, 0.f };
        const float green[] = { 0.f, 1.f, 0.f };
        const float blue[] = { 0.f, 0.f, 1.f };

        DrawRunSlice(framebuffer,
            v1Vec,
            v2Vec,
            red,
            green);

        DrawRunSlice(framebuffer,
            v2Vec,
            v3Vec,
            green,
            blue);

        DrawRunSlice(framebuffer,
            v3Vec,
            v1Vec,
            blue,
            red);
    }
}

}