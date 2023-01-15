#include "Serializer.h"

#include "ZAssert.h"

namespace ZSharp {

Serializer::Serializer(const FileString& path)
  : mWriter(path, 0), mOffset(0) {
}

bool Serializer::Serialize(void* memory, size_t sizeBytes) {
  if (!mWriter.IsOpen()) {
    return false;
  }

  // Write size and data following it.
  const size_t typeSize = sizeof(sizeBytes);
  if (!mWriter.Write(&sizeBytes, typeSize)) {
    return false;
  }

  if (!mWriter.Write(memory, sizeBytes)) {
    return false;
  }

  mOffset += typeSize;
  mOffset += sizeBytes;

  return true;
}

Deserializer::Deserializer(const FileString& path)
  : mReader(path), mOffset(0) {
}

bool Deserializer::Deserialize(void* memory, size_t sizeBytes) {
  if (!mReader.IsOpen()) {
    return false;
  }

  const char* buffer = mReader.GetBuffer();
  buffer += mOffset;

  const size_t typeSize = sizeof(sizeBytes);

  size_t savedBytes = 0;
  memcpy(&savedBytes, buffer, typeSize);
  ZAssert(savedBytes == sizeBytes);
  buffer += typeSize;
  memcpy(memory, buffer, sizeBytes);

  mOffset += typeSize;
  mOffset += sizeBytes;

  return true;
}

}
