#include "ZDrawing.h"

#include <cmath>
#include <cstring>

#include "ZColor.h"
#include "Common.h"
#include "CommonMath.h"
#include "Constants.h"
#include "ScopedTimer.h"
#include "PlatformIntrinsics.h"

namespace ZSharp {

float BarycentricArea(const float v1[2], const float v2[2], const float v3[2]) {
  return ((v3[0] - v1[0]) * (v2[1] - v1[1])) - ((v3[1] - v1[1]) * (v2[0] - v1[0]));
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
      float R = Lerp(p0Attributes[0] * p0[2], p1Attributes[0] * p1[2], yT);
      float G = Lerp(p0Attributes[1] * p0[2], p1Attributes[1] * p1[2], yT);
      float B = Lerp(p0Attributes[2] * p0[2], p1Attributes[2] * p1[2], yT);
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
      float R = Lerp(p0Attributes[0] * p0[2], p1Attributes[0] * p1[2], xT);
      float G = Lerp(p0Attributes[1] * p0[2], p1Attributes[1] * p1[2], xT);
      float B = Lerp(p0Attributes[2] * p0[2], p1Attributes[2] * p1[2], xT);
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
            float R = Lerp(p0Attributes[0] * p0[2], p1Attributes[0] * p1[2], xT);
            float G = Lerp(p0Attributes[1] * p0[2], p1Attributes[1] * p1[2], xT);
            float B = Lerp(p0Attributes[2] * p0[2], p1Attributes[2] * p1[2], xT);
            ZColor color(R, G, B);
            framebuffer.SetPixel(j, y1, color);
          }

          x1 -= slopeStep;
        }
        else { // Drawing left to right
          for (int32 j = x1; j < x1 + slopeStep; ++j) {
            float xT = ParametricSolveForT(static_cast<float>(j), p1[0], p0[0]);
            float R = Lerp(p0Attributes[0] * p0[2], p1Attributes[0] * p1[2], xT);
            float G = Lerp(p0Attributes[1] * p0[2], p1Attributes[1] * p1[2], xT);
            float B = Lerp(p0Attributes[2] * p0[2], p1Attributes[2] * p1[2], xT);
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
          float R = Lerp(p0Attributes[0] * p0[2], p1Attributes[0] * p1[2], yT);
          float G = Lerp(p0Attributes[1] * p0[2], p1Attributes[1] * p1[2], yT);
          float B = Lerp(p0Attributes[2] * p0[2], p1Attributes[2] * p1[2], yT);
          ZColor color(R, G, B);
          framebuffer.SetPixel(x1, j, color);
        }

        x1 += minorStep;
        y1 += slopeStep;
      }
    }
  }
}

void DrawTrianglesFlat(Framebuffer& framebuffer, const VertexBuffer& vertexBuffer, const IndexBuffer& indexBuffer, const ShadingModeOrder& order, const Texture* texture) {
  NamedScopedTimer(DrawFlatTriangles);
  
  if (order.Size() == 0) {
    ZAssert(false);
    return;
  }

  size_t stride = 0;
  for (ShadingMode& mode : order) {
    stride += mode.length;
  }

  const size_t frameHeight = framebuffer.GetHeight();
  const size_t frameWidth = framebuffer.GetWidth();
  const float maxHeight = (float)frameHeight;
  const float maxWidth = (float)frameWidth;

  const size_t vertexStride = vertexBuffer.GetStride();
  const float* vertexClipData = vertexBuffer.GetClipData(0);
  const size_t* indexClipData = indexBuffer.GetClipData(0);
  const size_t end = indexBuffer.GetClipLength();
  
  // TODO: Come up with a better long term solution to different shading modes.
  // Maybe a callback function for flat shading with known attributes?
  ShadingModes mode = order[0].mode;
  if (mode == ShadingModes::RGB) {
    for (size_t i = 0; i < end; i += TRI_VERTS) {
      const float* v1 = vertexClipData + (indexClipData[i] * vertexStride);
      const float* v2 = vertexClipData + (indexClipData[i + 1] * vertexStride);
      const float* v3 = vertexClipData + (indexClipData[i + 2] * vertexStride);

      const float* v1Attr = v1 + 4;
      const float* v2Attr = v2 + 4;
      const float* v3Attr = v3 + 4;

      // Calculate the amount of scan lines a triangle occupies.
      // Round to prev/next to avoid flooring.
      float minX = Min(Min(v1[0], v2[0]), v3[0]);
      float maxX = Max(Max(v1[0], v2[0]), v3[0]);
      float minY = Min(Min(v1[1], v2[1]), v3[1]);
      float maxY = Max(Max(v1[1], v2[1]), v3[1]);

      minX -= 1.f;
      maxX += 1.f;
      minY -= 1.f;
      maxY += 1.f;

      Clamp(minX, 0.f, maxWidth);
      Clamp(maxX, 0.f, maxWidth);
      Clamp(minY, 0.f, maxHeight);
      Clamp(maxY, 0.f, maxHeight);

      const size_t sMinX = (size_t)minX;
      const size_t sMaxX = (size_t)maxX;
      const size_t sMinY = (size_t)minY;
      const size_t sMaxY = (size_t)maxY;

      // Factor out some of the BarycentricArea equation that's constant for each vertex. 
      const float factor10 = v2[0] - v1[0];
      const float factor11 = v2[1] - v1[1];

      const float factor20 = v3[0] - v2[0];
      const float factor21 = v3[1] - v2[1];

      const float factor30 = v1[0] - v3[0];
      const float factor31 = v1[1] - v3[1];

      const float invArea = 1.f / ((v3[0] - v1[0]) * factor11 - ((v3[1] - v1[1]) * factor10));

      const float invV1 = v1[3] * invArea;
      const float invV2 = v2[3] * invArea;
      const float invV3 = v3[3] * invArea;

      const float invV1Attr0 = v1Attr[0] * invArea;
      const float invV1Attr1 = v1Attr[1] * invArea;
      const float invV1Attr2 = v1Attr[2] * invArea;
      const float invV2Attr0 = v2Attr[0] * invArea;
      const float invV2Attr1 = v2Attr[1] * invArea;
      const float invV2Attr2 = v2Attr[2] * invArea;
      const float invV3Attr0 = v3Attr[0] * invArea;
      const float invV3Attr1 = v3Attr[1] * invArea;
      const float invV3Attr2 = v3Attr[2] * invArea;

      float p0 = minX;
      float p1 = minY;

      for (size_t h = sMinY; h < sMaxY; ++h, p1++) {
        p0 = minX;

        const float p1Factor0 = (p1 - v1[1]) * factor10;
        const float p1Factor1 = (p1 - v2[1]) * factor20;
        const float p1Factor2 = (p1 - v3[1]) * factor30;

        for (size_t w = sMinX; w < sMaxX; ++w, p0++) {
          const float w0 = (((p0 - v1[0]) * factor11) - p1Factor0);
          const float w1 = (((p0 - v2[0]) * factor21) - p1Factor1);
          const float w2 = (((p0 - v3[0]) * factor31) - p1Factor2);

          // Ignore winding order for now. We need a long term solution to get models in the right order.
          // Backface culling is handled earlier in the pipeline.
          if ((w0 <= 0.f && w1 <= 0.f && w2 <= 0.f) || (w0 >= 0.f && w1 >= 0.f && w2 >= 0.f)) {
            const float invDenominator = 1.f / ((w0 * invV1) + (w1 * invV2) + (w2 * invV3));

            const float r = invDenominator * ((w0 * invV1Attr0) + (w1 * invV2Attr0) + (w2 * invV3Attr0));
            const float g = invDenominator * ((w0 * invV1Attr1) + (w1 * invV2Attr1) + (w2 * invV3Attr1));
            const float b = invDenominator * ((w0 * invV1Attr2) + (w1 * invV2Attr2) + (w2 * invV3Attr2));

            const ZColor color(r, g, b);
            framebuffer.SetPixel(w, h, color);
          }
        }
      }
    }
  }
  else if (mode == ShadingModes::UV) {
    for (size_t i = 0; i < end; i += TRI_VERTS) {
      const float* v1 = vertexClipData + (indexClipData[i] * vertexStride);
      const float* v2 = vertexClipData + (indexClipData[i + 1] * vertexStride);
      const float* v3 = vertexClipData + (indexClipData[i + 2] * vertexStride);

#if 0
      const float* v1Attr = v1 + 4;
      const float* v2Attr = v2 + 4;
      const float* v3Attr = v3 + 4;

      // Calculate the amount of scan lines a triangle occupies.
      // Round to prev/next to avoid flooring.
      float minX = Min(Min(v1[0], v2[0]), v3[0]);
      float maxX = Max(Max(v1[0], v2[0]), v3[0]);
      float minY = Min(Min(v1[1], v2[1]), v3[1]);
      float maxY = Max(Max(v1[1], v2[1]), v3[1]);

      minX -= 1.f;
      maxX += 1.f;
      minY -= 1.f;
      maxY += 1.f;

      Clamp(minX, 0.f, maxWidth);
      Clamp(maxX, 0.f, maxWidth);
      Clamp(minY, 0.f, maxHeight);
      Clamp(maxY, 0.f, maxHeight);

      const size_t sMinX = (size_t)minX;
      const size_t sMaxX = (size_t)maxX;
      const size_t sMinY = (size_t)minY;
      const size_t sMaxY = (size_t)maxY;

      // Factor out some of the BarycentricArea equation that's constant for each vertex. 
      const float factor10 = v2[0] - v1[0];
      const float factor11 = v2[1] - v1[1];

      const float factor20 = v3[0] - v2[0];
      const float factor21 = v3[1] - v2[1];

      const float factor30 = v1[0] - v3[0];
      const float factor31 = v1[1] - v3[1];

      const float invArea = 1.f / ((v3[0] - v1[0]) * factor11 - ((v3[1] - v1[1]) * factor10));

      const float invV1 = v1[3] * invArea;
      const float invV2 = v2[3] * invArea;
      const float invV3 = v3[3] * invArea;

      const float invV1Attr0 = v1Attr[0] * invArea;
      const float invV1Attr1 = v1Attr[1] * invArea;
      const float invV2Attr0 = v2Attr[0] * invArea;
      const float invV2Attr1 = v2Attr[1] * invArea;
      const float invV3Attr0 = v3Attr[0] * invArea;
      const float invV3Attr1 = v3Attr[1] * invArea;

      float p0 = minX;
      float p1 = minY;

      for (size_t h = sMinY; h < sMaxY; ++h, p1++) {
        p0 = minX;

        const float p1Factor0 = (p1 - v1[1]) * factor10;
        const float p1Factor1 = (p1 - v2[1]) * factor20;
        const float p1Factor2 = (p1 - v3[1]) * factor30;

        for (size_t w = sMinX; w < sMaxX; ++w, p0++) {
          const float w0 = (((p0 - v1[0]) * factor11) - p1Factor0);
          const float w1 = (((p0 - v2[0]) * factor21) - p1Factor1);
          const float w2 = (((p0 - v3[0]) * factor31) - p1Factor2);
          
          // Ignore winding order for now. We need a long term solution to get models in the right order.
          // Backface culling is handled earlier in the pipeline.
          if ((w0 <= 0.f && w1 <= 0.f && w2 <= 0.f) || (w0 >= 0.f && w1 >= 0.f && w2 >= 0.f)) {
            const float invDenominator = 1.f / ((w0 * invV1) + (w1 * invV2) + (w2 * invV3));

            const float u = invDenominator * ((w0 * invV1Attr0) + (w1 * invV2Attr0) + (w2 * invV3Attr0));
            const float v = invDenominator * ((w0 * invV1Attr1) + (w1 * invV2Attr1) + (w2 * invV3Attr1));

            framebuffer.SetPixel(w, h, texture->Sample(u, v));
          }
        }
      }
#else
      Unaligned_FlatShadeUVs(v1, v2, v3, maxWidth, maxHeight, framebuffer.GetBuffer(), texture);
#endif
    }
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