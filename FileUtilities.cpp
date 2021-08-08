#include "FileUtilities.h"

#include <fstream>

namespace ZSharp {

  void ReadFileToBuffer(const char* fileName, std::vector<char>& fileBuffer) {
    std::ifstream inputStream(fileName, std::ios::binary);

    if (!inputStream.is_open()) {
      return;
    }

    fileBuffer = std::vector<char>(std::istreambuf_iterator<char>(inputStream), std::istreambuf_iterator<char>());
  }


}
