#include "Serializer.h"

#include "ZAssert.h"
#include "PlatformMemory.h"

#include <cstring>

namespace ZSharp {

// File serializers
FileSerializer::FileSerializer(const FileString& path)
  : mOffset(0), mWriter(path, 0)  {
}

bool FileSerializer::Serialize(const void* memory, size_t sizeBytes) {
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

FileDeserializer::FileDeserializer(const FileString& path)
  : mOffset(0), mReader(path) {
}

bool FileDeserializer::Deserialize(void* memory, size_t sizeBytes) {
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

// Memory serializers
MemorySerializer::MemorySerializer(size_t initialSize)
  : mOffset(0), mSize(initialSize), mMemory(nullptr) {
  if (mSize > 0) {
    mMemory = (uint8*)PlatformMalloc(mSize);
  }
}

MemorySerializer::~MemorySerializer() {
  if (mMemory != nullptr) {
    PlatformFree(mMemory);
  }
}

bool MemorySerializer::Serialize(const void* memory, size_t sizeBytes) {
  const size_t typeSize = sizeof(sizeBytes);

  if (mMemory == nullptr) {
    mSize = sizeBytes + typeSize;
    mMemory = (uint8*)PlatformMalloc(mSize);
  }
  else if (mOffset + sizeBytes + typeSize >= mSize) {
    mSize = mOffset + sizeBytes + typeSize;
    mMemory = (uint8*)PlatformReAlloc(mMemory, mSize);
  }

  // Write size and data following it.
  memcpy(mMemory + mOffset, &sizeBytes, typeSize);
  mOffset += typeSize;
  memcpy(mMemory + mOffset, memory, sizeBytes);
  mOffset += sizeBytes;
  return true;
}

const uint8* MemorySerializer::Data() const {
  return mMemory;
}

size_t MemorySerializer::Size() const {
  return mSize;
}

MemoryDeserializer::MemoryDeserializer() : mOffset(0), mBaseAddress(0) {
}

MemoryDeserializer::MemoryDeserializer(const void* baseAddress)
  : mOffset(0), mBaseAddress((uint8*)baseAddress) {
}

MemoryDeserializer::MemoryDeserializer(const MemoryDeserializer& rhs)
  : mOffset(0), mBaseAddress(rhs.mBaseAddress) {
}

bool MemoryDeserializer::Deserialize(void* memory, size_t sizeBytes) {
  if (mBaseAddress == nullptr) {
    return false;
  }

  const size_t typeSize = sizeof(sizeBytes);

  size_t savedBytes = 0;
  memcpy(&savedBytes, mBaseAddress + mOffset, typeSize);

  ZAssert(savedBytes == sizeBytes);
  
  mOffset += typeSize;
  memcpy(memory, mBaseAddress + mOffset, sizeBytes);
  mOffset += sizeBytes;

  return true;
}

uint8* MemoryDeserializer::BaseAddress() {
  return mBaseAddress;
}

size_t MemoryDeserializer::Offset() const {
  return mOffset;
}

void MemoryDeserializer::Reassign(void* baseAddress) {
  mBaseAddress = (uint8*)baseAddress;
  mOffset = 0;
}

}
