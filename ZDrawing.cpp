#include "ZDrawing.h"

#include <cmath>
#include <cstring>

#include "Common.h"
#include "CommonMath.h"
#include "Constants.h"
#include "ScopedTimer.h"
#include "PlatformIntrinsics.h"

namespace ZSharp {

void DrawRunSlice(Framebuffer& framebuffer,
  const float* p0,
  const float* p1,
  ZColor color) {
  int32 x1 = static_cast<int32>(p0[0]);
  int32 y1 = static_cast<int32>(p0[1]);
  int32 x2 = static_cast<int32>(p1[0]);
  int32 y2 = static_cast<int32>(p1[1]);

  // Vertical line
  if (x1 == x2) {
    float startY = p0[1];
    float endY = p1[1];

    if (y2 < y1) {
      Swap(y1, y2);
      Swap(startY, endY);
    }

    for (; y1 < y2; y1++) {
      framebuffer.SetPixel(x1, y1, color);
    }
  }
  else if (y1 == y2) { // Horizontal line
    float startX = p0[0];
    float endX = p1[0];

    if (x2 < x1) {
      Swap(x1, x2);
      Swap(startX, endX);
    }

    for (int32 i = x1; i < x2; ++i) {
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
            framebuffer.SetPixel(j, y1, color);
          }

          x1 -= slopeStep;
        }
        else { // Drawing left to right
          for (int32 j = x1; j < x1 + slopeStep; ++j) {
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
          framebuffer.SetPixel(x1, j, color);
        }

        x1 += minorStep;
        y1 += slopeStep;
      }
    }
  }
}

void RGBShader(Framebuffer& framebuffer, DepthBuffer& depthBuffer, const VertexBuffer& vertexBuffer, const IndexBuffer& indexBuffer, bool wasClipped) {
  NamedScopedTimer(DrawFlatTrianglesRGB);
  
  const size_t frameWidth = framebuffer.GetWidth();
  const float maxWidth = (float)frameWidth;

  const float* vertexClipData;
  const int32* indexClipData;
  int32 end;

  if (wasClipped) {
    vertexClipData = vertexBuffer.GetClipData(0);
    indexClipData = indexBuffer.GetClipData(0);
    end = indexBuffer.GetClipLength();
  }
  else {
    vertexClipData = vertexBuffer[0];
    indexClipData = indexBuffer.GetInputData();
    end = indexBuffer.GetIndexSize();
  }
  
  Unaligned_Shader_RGB(vertexClipData, indexClipData, end, maxWidth, framebuffer.GetBuffer(), depthBuffer.GetBuffer());
}

void TextureMappedShader(Framebuffer& framebuffer, DepthBuffer& depthBuffer, const VertexBuffer& vertexBuffer, const IndexBuffer& indexBuffer, bool wasClipped, const Texture* texture, size_t mipLevel) {
  NamedScopedTimer(DrawFlatTrianglesUV);

  const size_t frameWidth = framebuffer.GetWidth();
  const float maxWidth = (float)frameWidth;

  const float* vertexClipData;
  const int32* indexClipData;
  int32 end;

  if (wasClipped) {
    vertexClipData = vertexBuffer.GetClipData(0);
    indexClipData = indexBuffer.GetClipData(0);
    end = indexBuffer.GetClipLength();
  }
  else {
    vertexClipData = vertexBuffer[0];
    indexClipData = indexBuffer.GetInputData();
    end = indexBuffer.GetIndexSize();
  }

  Unaligned_Shader_UV(vertexClipData, indexClipData, end, maxWidth, framebuffer.GetBuffer(), depthBuffer.GetBuffer(), texture, mipLevel);
}

void WireframeShader(Framebuffer& framebuffer, const VertexBuffer& vertexBuffer, const IndexBuffer& indexBuffer, bool wasClipped, ZColor color) {
  NamedScopedTimer(DrawWireframeTriangles);

  const float* vertexClipData;
  const int32* indexClipData;
  int32 end;

  if (wasClipped) {
    vertexClipData = vertexBuffer.GetClipData(0);
    indexClipData = indexBuffer.GetClipData(0);
    end = indexBuffer.GetClipLength();
  }
  else {
    vertexClipData = vertexBuffer[0];
    indexClipData = indexBuffer.GetInputData();
    end = indexBuffer.GetIndexSize();
  }

  for (int32 i = 0; i < end; i += TRI_VERTS) {
    const float* v1 = vertexClipData + indexClipData[i];
    const float* v2 = vertexClipData + indexClipData[i + 1];
    const float* v3 = vertexClipData + indexClipData[i + 2];
    DrawRunSlice(framebuffer, v1, v2, color);
    DrawRunSlice(framebuffer, v2, v3, color);
    DrawRunSlice(framebuffer, v3, v1, color);
  }
}

}