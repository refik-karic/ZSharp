#pragma once

#include "ZBaseTypes.h"
#include "FileString.h"
#include "ZFile.h"

namespace ZSharp {

/*
TODO:
  1) CRC checksum?
  2) Endian swap?
*/

class Serializer final {
  public:

  Serializer(const FileString& path);

  bool Serialize(void* memory, size_t sizeBytes);

  private:
  size_t mOffset;
  BufferedFileWriter mWriter;

  // NOTE: Although Win32 docs state that you _can_ grow files using a writable memory map, this does not apply to 0 length files.
  // Which means that we must write to the file using PlatformWriteFile.
};

class Deserializer final {
  public:

  Deserializer(const FileString& path);

  bool Deserialize(void* memory, size_t sizeBytes);

  private:
  size_t mOffset;
  MemoryMappedFileReader mReader;
};

}
