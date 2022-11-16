#include "ZDrawing.h"

#include <cmath>

#include "Common.h"
#include "Constants.h"
#include "Triangle.h"

namespace ZSharp {

void DrawRunSlice(Framebuffer& framebuffer, 
                  int32 x1, 
                  int32 y1, 
                  int32 x2, 
                  int32 y2, 
                  ZColor color) {
  if (x1 == x2) {
    if (y2 < y1) {
      Swap(y1, y2);
    }

    for (; y1 < y2; y1++) {
      framebuffer.SetPixel(x1, y1, color);
    }
  }
  else if (y1 == y2) {
    if (x2 < x1) {
      Swap(x1, x2);
    }

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
          framebuffer.SetRow(y1, x1 - slopeStep, x1, color);
          x1 -= slopeStep;
        }
        else {
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

void DrawTrianglesWireframe(Framebuffer& framebuffer, const VertexBuffer& vertexBuffer, const IndexBuffer& indexBuffer, ZColor color) {
    size_t end = indexBuffer.GetClipLength();
    for (size_t i = 0; i < end; i += TRI_VERTS) {
        const float* v1 = vertexBuffer.GetClipData(indexBuffer.GetClipData(i));
        const float* v2 = vertexBuffer.GetClipData(indexBuffer.GetClipData(i + 1));
        const float* v3 = vertexBuffer.GetClipData(indexBuffer.GetClipData(i + 2));

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