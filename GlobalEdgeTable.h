#pragma once

#include <cstddef>

#include <map>
#include <vector>

#include "Framebuffer.h"
#include "ZColor.h"

namespace ZSharp {

class GlobalEdgeTable final {
  public:

  GlobalEdgeTable() = default;
  ~GlobalEdgeTable() = default;
  GlobalEdgeTable(const GlobalEdgeTable&) = delete;
  void operator=(const GlobalEdgeTable&) = delete;

  void AddPoint(std::size_t yIndex, std::size_t x, ZColor color, std::size_t primitiveIndex);
  void Clear();
  void Draw(Framebuffer& frameBuffer);

  private:
  
  struct ScanLine {
    std::size_t x1;
    std::size_t x2;
    std::size_t primitiveIndex;
    ZColor color;
  };

  typedef std::vector<ScanLine> ScanLineList;

  std::map<std::size_t, ScanLineList> mEdgeTable;
};

}
