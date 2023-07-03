#pragma once

#include "ZBaseTypes.h"
#include "FileString.h"
#include "ZFile.h"
#include "ISerializable.h"

namespace ZSharp {

/*
TODO:
  1) CRC checksum?
  2) Endian swap?
*/

// File Serialization
class FileSerializer : public ISerializer {
  public:

  FileSerializer(const FileString& path);

  virtual bool Serialize(const void* memory, size_t sizeBytes) override;

  private:
  size_t mOffset;
  BufferedFileWriter mWriter;

  // NOTE: Although Win32 docs state that you _can_ grow files using a writable memory map, this does not apply to 0 length files.
  // Which means that we must write to the file using PlatformWriteFile.
};

class FileDeserializer : public IDeserializer {
  public:

  FileDeserializer(const FileString& path);

  virtual bool Deserialize(void* memory, size_t sizeBytes) override;

  private:
  size_t mOffset;
  MemoryMappedFileReader mReader;
};

// Memory serialization
class MemorySerializer : public ISerializer {
  public:

  MemorySerializer(size_t initialSize = 0);

  ~MemorySerializer();

  virtual bool Serialize(const void* memory, size_t sizeBytes) override;

  const uint8* Data() const;

  size_t Size() const;

  private:
  size_t mOffset;
  size_t mSize;
  uint8* mMemory;
};

class MemoryDeserializer : public IDeserializer {
  public:

  MemoryDeserializer();

  MemoryDeserializer(const void* baseAddress);

  MemoryDeserializer(const MemoryDeserializer& rhs);

  virtual bool Deserialize(void* memory, size_t sizeBytes) override;

  void Reassign(void* baseAddress);

  uint8* BaseAddress();

  size_t Offset() const;

  private:
  size_t mOffset;
  uint8* mBaseAddress;
};

}
