#include <algorithm>
#include <cmath>

#include "ZDrawing.h"

namespace ZSharp {

void ZDrawing::DrawRunSlice(Framebuffer& framebuffer, 
                  std::int32_t x1, 
                  std::int32_t y1, 
                  std::int32_t x2, 
                  std::int32_t y2, 
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
    double slope;
    double error = 0.0;
    std::int32_t slopeStep;
    std::int32_t delta;

    if (y2 < y1) {
      std::swap(y1, y2);
      std::swap(x1, x2);
    }

    if (std::abs(x2 - x1) >= std::abs(y2 - y1)) {
      delta = std::abs(y2 - y1);
      slope = std::abs(static_cast<double>((x2 - x1)) / (y2 - y1));

      for (std::size_t i = 0; i < delta; i++) {
        error = error + (slope - std::floor(slope));
        slopeStep = static_cast<std::int32_t>(std::floor(slope) + error);

        if (slopeStep > static_cast<std::int32_t>(slope)) {
          error = std::max(error - 1.0, 0.0);
        }

        if (x2 <= x1) {
          for (std::size_t j = x1 - slopeStep; j < x1; j++) {
            framebuffer.SetPixel(j, y1, color);
          }

          x1 -= slopeStep;
        }
        else {
          for (std::size_t j = x1; j < x1 + slopeStep; j++) {
            framebuffer.SetPixel(j, y1, color);
          }

          x1 += slopeStep;
        }

        y1++;
      }
    }
    else {
      delta = std::abs(x2 - x1);
      std::int32_t minorStep = (x2 - x1) / delta;
      slope = std::abs(static_cast<double>((y2 - y1)) / (x2 - x1));

      for (std::size_t i = 0; i < delta; i++) {
        error = error + (slope - std::floor(slope));
        slopeStep = static_cast<std::int32_t>(std::floor(slope) + error);

        if (slopeStep > static_cast<std::int32_t>(slope)) {
          error = std::max(error - 1.0, 0.0);
        }

        for (std::size_t j = y1; j < y1 + slopeStep; j++) {
          framebuffer.SetPixel(x1, j, color);
        }

        x1 += minorStep;
        y1 += slopeStep;
      }
    }
  }
}

void ZDrawing::TracePrimitive(GlobalEdgeTable& edgeTable, std::array<std::int32_t, 2>& p1, std::array<std::int32_t, 2>& p2, std::array<std::int32_t, 2>& p3, ZColor color, std::size_t primitiveIndex) {
  TraceLine(edgeTable, p1[0], p1[1], p2[0], p2[1], color, primitiveIndex);
  TraceLine(edgeTable, p2[0], p2[1], p3[0], p3[1], color, primitiveIndex);
  TraceLine(edgeTable, p3[0], p3[1], p1[0], p1[1], color, primitiveIndex);
}

void ZDrawing::TraceLine(GlobalEdgeTable& edgeTable, std::int32_t x1, std::int32_t y1, std::int32_t x2, std::int32_t y2, ZColor color, std::size_t primitiveIndex) {
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
    std::int32_t slopeStep;
    std::int32_t delta;

    if (y2 < y1) {
      std::swap(y1, y2);
      std::swap(x1, x2);
    }

    if (std::abs(x2 - x1) >= std::abs(y2 - y1)) {
      delta = std::abs(y2 - y1);
      slope = std::abs(static_cast<double>((x2 - x1)) / (y2 - y1));

      for (std::size_t i = 0; i < delta; i++) {
        error = error + (slope - std::floor(slope));
        slopeStep = static_cast<std::int32_t>(std::floor(slope) + error);

        if (slopeStep > static_cast<std::int32_t>(slope)) {
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
      std::int32_t minorStep = (x2 - x1) / delta;
      slope = std::abs(static_cast<double>((y2 - y1)) / (x2 - x1));

      for (std::size_t i = 0; i < delta; i++) {
        error = error + (slope - std::floor(slope));
        slopeStep = static_cast<std::int32_t>(std::floor(slope) + error);

        if (slopeStep > static_cast<std::int32_t>(slope)) {
          error = std::max(error - 1.0, 0.0);
        }

        for (std::size_t j = y1; j < y1 + slopeStep; j++) {
          edgeTable.AddPoint(j, x1, color, primitiveIndex);
        }

        x1 += minorStep;
        y1 += slopeStep;
      }
    }
  }
}

}