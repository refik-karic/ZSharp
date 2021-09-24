#include "ZDrawing.h"

#include <algorithm>
#include <cmath>

#include "Constants.h"
#include "Triangle.h"


namespace ZSharp {

void DrawRunSlice(Framebuffer& framebuffer, 
                  int32_t x1, 
                  int32_t y1, 
                  int32_t x2, 
                  int32_t y2, 
                  ZColor color) {
  if (x1 == x2) {
    if (y2 < y1) {
      std::swap(y1, y2);
    }

    for (; y1 < y2; y1++) {
      framebuffer.SetPixel(x1, y1, color);
    }
  }
  else if (y1 == y2) {
    if (x2 < x1) {
      std::swap(x1, x2);
    }

    for (; x1 < x2; x1++) {
      framebuffer.SetPixel(x1, y1, color);
    }
  }
  else {
    float slope;
    float error = 0.f;
    int32_t slopeStep;
    int32_t delta;

    if (y2 < y1) {
      std::swap(y1, y2);
      std::swap(x1, x2);
    }

    if (abs(x2 - x1) >= abs(y2 - y1)) {
      delta = abs(y2 - y1);
      slope = fabs(static_cast<float>((x2 - x1)) / (y2 - y1));

      for (size_t i = 0; i < delta; i++) {
        error = error + (slope - floor(slope));
        slopeStep = static_cast<int32_t>(floor(slope) + error);

        if (slopeStep > static_cast<int32_t>(slope)) {
          error = fmax(error - 1.f, 0.f);
        }

        if (x2 <= x1) {
          for (int32_t j = x1 - slopeStep; j < x1; j++) {
            framebuffer.SetPixel(j, y1, color);
          }

          x1 -= slopeStep;
        }
        else {
          for (int32_t j = x1; j < x1 + slopeStep; j++) {
            framebuffer.SetPixel(j, y1, color);
          }

          x1 += slopeStep;
        }

        y1++;
      }
    }
    else {
      delta = abs(x2 - x1);
      int32_t minorStep = (x2 - x1) / delta;
      slope = fabs(static_cast<float>((y2 - y1)) / (x2 - x1));

      for (size_t i = 0; i < delta; i++) {
        error = error + (slope - floor(slope));
        slopeStep = static_cast<int32_t>(floor(slope) + error);

        if (slopeStep > static_cast<int32_t>(slope)) {
          error = fmax(error - 1.f, 0.f);
        }

        for (int32_t j = y1; j < y1 + slopeStep; j++) {
          framebuffer.SetPixel(x1, j, color);
        }

        x1 += minorStep;
        y1 += slopeStep;
      }
    }
  }
}

void TracePrimitive(GlobalEdgeTable& edgeTable, std::array<int32_t, 2>& p1, std::array<int32_t, 2>& p2, std::array<int32_t, 2>& p3, ZColor color, size_t primitiveIndex) {
  TraceLine(edgeTable, p1[0], p1[1], p2[0], p2[1], color, primitiveIndex);
  TraceLine(edgeTable, p2[0], p2[1], p3[0], p3[1], color, primitiveIndex);
  TraceLine(edgeTable, p3[0], p3[1], p1[0], p1[1], color, primitiveIndex);
}

void TraceLine(GlobalEdgeTable& edgeTable, int32_t x1, int32_t y1, int32_t x2, int32_t y2, ZColor color, size_t primitiveIndex) {
  if (x1 == x2) {
    if (y2 < y1) {
      std::swap(y1, y2);
    }

    while (y1 < y2) {
      edgeTable.AddPoint(y1, x1, color, primitiveIndex);
      y1++;
    }
  }
  else if (y1 == y2) {
    if (x2 < x1) {
      std::swap(x1, x2);
    }

    edgeTable.AddPoint(y1, x1, color, primitiveIndex);
    edgeTable.AddPoint(y1, x2, color, primitiveIndex);
  }
  else {
    double slope;
    double error = 0.0;
    int32_t slopeStep;
    int32_t delta;

    if (y2 < y1) {
      std::swap(y1, y2);
      std::swap(x1, x2);
    }

    if (std::abs(x2 - x1) >= std::abs(y2 - y1)) {
      delta = std::abs(y2 - y1);
      slope = std::abs(static_cast<double>((x2 - x1)) / (y2 - y1));

      for (size_t i = 0; i < delta; i++) {
        error = error + (slope - std::floor(slope));
        slopeStep = static_cast<int32_t>(std::floor(slope) + error);

        if (slopeStep > static_cast<int32_t>(slope)) {
          error = std::max(error - 1.0, 0.0);
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
      delta = std::abs(x2 - x1);
      int32_t minorStep = (x2 - x1) / delta;
      slope = std::abs(static_cast<double>((y2 - y1)) / (x2 - x1));

      for (size_t i = 0; i < delta; i++) {
        error = error + (slope - std::floor(slope));
        slopeStep = static_cast<int32_t>(std::floor(slope) + error);

        if (slopeStep > static_cast<int32_t>(slope)) {
          error = std::max(error - 1.0, 0.0);
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
    GlobalEdgeTable edgeTable;

    size_t inputStride = vertexBuffer.GetInputStride();
    size_t end = indexBuffer.GetClipLength();
    for (size_t i = 0; i < end; i += TRI_VERTS) {
        const float* v1 = vertexBuffer.GetClipData(indexBuffer.GetClipData(i), inputStride);
        const float* v2 = vertexBuffer.GetClipData(indexBuffer.GetClipData(i + 1), inputStride);
        const float* v3 = vertexBuffer.GetClipData(indexBuffer.GetClipData(i + 2), inputStride);

        std::array<int32_t, 2> p1{ static_cast<int32_t>(*v1), static_cast<int32_t>(*(v1 + 1)) };
        std::array<int32_t, 2> p2{ static_cast<int32_t>(*v2), static_cast<int32_t>(*(v2 + 1)) };
        std::array<int32_t, 2> p3{ static_cast<int32_t>(*v3), static_cast<int32_t>(*(v3 + 1)) };

        TracePrimitive(edgeTable, p1, p2, p3, color, i);
    }

    edgeTable.Draw(framebuffer);
}

void DrawTrianglesWireframe(Framebuffer& framebuffer, const VertexBuffer& vertexBuffer, const IndexBuffer& indexBuffer, ZColor color) {
    size_t inputStride = vertexBuffer.GetInputStride();
    size_t end = indexBuffer.GetClipLength();
    for (size_t i = 0; i < end; i += TRI_VERTS) {
        const float* v1 = vertexBuffer.GetClipData(indexBuffer.GetClipData(i), inputStride);
        const float* v2 = vertexBuffer.GetClipData(indexBuffer.GetClipData(i + 1), inputStride);
        const float* v3 = vertexBuffer.GetClipData(indexBuffer.GetClipData(i + 2), inputStride);

        DrawRunSlice(framebuffer,
            static_cast<size_t>(*(v1)),
            static_cast<size_t>(*(v1 + 1)),
            static_cast<size_t>(*(v2)),
            static_cast<size_t>(*(v2 + 1)),
            color);

        DrawRunSlice(framebuffer,
            static_cast<size_t>(*(v2)),
            static_cast<size_t>(*(v2 + 1)),
            static_cast<size_t>(*(v3)),
            static_cast<size_t>(*(v3 + 1)),
            color);

        DrawRunSlice(framebuffer,
            static_cast<size_t>(*(v3)),
            static_cast<size_t>(*(v3 + 1)),
            static_cast<size_t>(*(v1)),
            static_cast<size_t>(*(v1 + 1)),
            color);
    }
}

}