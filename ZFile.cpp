#include "ZFile.h"

#include <cstring>

#include "PlatformFile.h"
#include "PlatformMemory.h"

namespace ZSharp {

BaseFile::BaseFile(const FileString& fileName, size_t flags)
: mFile(fileName), mFileHandle(nullptr), mOpen(false) {
  if (!fileName.IsEmpty()) {
    mFileHandle = PlatformOpenFile(mFile, flags);
    mOpen = (mFileHandle != nullptr);
  }
}

BaseFile::~BaseFile() {
  if (IsOpen()) {
    PlatformCloseFile(mFileHandle);
  }
}

bool BaseFile::IsOpen() const {
  return mOpen;
}

size_t BaseFile::GetSize() {
  return PlatformGetFileSize(mFileHandle);
}

BufferedFileReader::BufferedFileReader(const FileString& fileName) 
  : BaseFile(fileName, static_cast<size_t>(FileFlags::READ)) {
  mBuffer = static_cast<char*>(PlatformCalloc(mBufferSize));
}

BufferedFileReader::~BufferedFileReader() {
  if (mBuffer != nullptr) {
    PlatformFree(mBuffer);
  }
}

size_t BufferedFileReader::Read(void* buffer, size_t length) {
  if (!IsOpen() || buffer == nullptr) {
    return 0;
  }

  return PlatformReadFile(mFileHandle, buffer, length);
}

size_t BufferedFileReader::ReadLine() {
  if (!IsOpen() || mBuffer == nullptr) {
    return 0;
  }

  if (mBufferSize != mDefaultBufferSize) {
    ResetBuffer(mDefaultBufferSize);
  }

  bool keepReading = true;
  size_t numRead = 0;
  for (size_t i = 0; keepReading; ++i) {
    if (i == (mBufferSize - 1)) {
      size_t newLength = mBufferSize + mDefaultBufferSize;
      char* resizedBuf = static_cast<char*>(PlatformReAlloc(mBuffer, newLength));
      if (resizedBuf == nullptr) {
        return 0;
      }

      mBuffer = resizedBuf;
      mBufferSize = newLength;
    }

    char nextChar[1];
    if (PlatformReadFile(mFileHandle, nextChar, 1) > 0) {
      mBuffer[i] = nextChar[0];
      numRead++;
    }
    else {
      mBuffer[i] = NULL;
      keepReading = false;
    }

    if (mBuffer[i] == '\n') {
      mBuffer[i + 1] = NULL;
      keepReading = false;
    }
  }

  return numRead;
}

const char* BufferedFileReader::GetBuffer() const {
  return mBuffer;
}

bool BufferedFileReader::ResetBuffer(size_t size) {
  char* resizedBuf = static_cast<char*>(PlatformReAlloc(mBuffer, size));
  if (resizedBuf == nullptr) {
    return false;
  }

  mBuffer = resizedBuf;
  mBufferSize = size;
  memset(mBuffer, 0, mBufferSize);
  return true;
}

BufferedFileWriter::BufferedFileWriter(const FileString& fileName, size_t flags)
  : BaseFile(fileName, static_cast<size_t>(FileFlags::WRITE) | flags) {
  mBuffer = static_cast<char*>(PlatformCalloc(mBufferSize));
}

BufferedFileWriter::~BufferedFileWriter() {
  Flush();

  if (mBuffer != nullptr) {
    PlatformFree(mBuffer);
  }
}

SystemBufferedFileWriter::SystemBufferedFileWriter(const FileString& fileName, size_t flags) 
  : BaseFile(fileName, static_cast<size_t>(FileFlags::WRITE) | flags) {

}

SystemBufferedFileWriter::~SystemBufferedFileWriter() {
  Flush();
}

bool SystemBufferedFileWriter::Write(const void* data, size_t length) {
  if (!IsOpen()) {
    return false;
  }

  return PlatformWriteFile(mFileHandle, data, length) == length;
}

bool SystemBufferedFileWriter::Flush() {
  if (!IsOpen()) {
    return false;
  }

  return PlatformFileFlush(mFileHandle);
}

bool BufferedFileWriter::Write(const void* data, size_t length) {
  if (!IsOpen() || mBuffer == nullptr) {
    return false;
  }

  // Unlike buffered reader, writer should not reallocate data.
  if ((length > mBufferSize) || (mBufferedDataSize + length > mBufferSize)) {
    // Write any buffered data.
    if (mBufferedDataSize > 0) {
      bool success = PlatformWriteFile(mFileHandle, mBuffer, mBufferedDataSize) == mBufferedDataSize;
      if (!success) {
        return false;
      }

      mBufferedDataSize = 0;
      memset(mBuffer, 0, mBufferSize);
    }

    // Write out the rest.
    bool success = PlatformWriteFile(mFileHandle, data, length) == length;
    if (!success) {
      return false;
    }
  }
  else {
    // Buffer data internally.
    memcpy(mBuffer + mBufferedDataSize, data, length);
    mBufferedDataSize += length;
  }

  return true;
}

bool BufferedFileWriter::Flush() {
  if (!IsOpen()) {
    return false;
  }

  // Write any buffered data.
  if (mBufferedDataSize > 0) {
    bool success = PlatformWriteFile(mFileHandle, mBuffer, mBufferedDataSize) == mBufferedDataSize;
    if (!success) {
      return false;
    }

    mBufferedDataSize = 0;
    memset(mBuffer, 0, mBufferSize);
  }

  return PlatformFileFlush(mFileHandle);
}

MemoryMappedFileReader::MemoryMappedFileReader(const FileString& fileName) 
  : BaseFile(fileName, static_cast<size_t>(FileFlags::READ)) {
  if (IsOpen()) {
    mFileData = PlatformOpenMemoryMapFile(mFileHandle, static_cast<size_t>(FileFlags::READ), mMappedFileHandle, 0);
    mOpen = (mFileData != nullptr);

    if (mOpen) {
      PlatformUpdateFileAccessTime(mFileHandle);
    }
  }
}

MemoryMappedFileReader::~MemoryMappedFileReader() {
  if (IsOpen()) {
    PlatformCloseMemoryMapFile(mMappedFileHandle);
  }
}

const char* MemoryMappedFileReader::GetBuffer() const {
  return static_cast<char*>(mFileData);
}

MemoryMappedFileWriter::MemoryMappedFileWriter(const FileString& fileName, size_t maxSize)
  : BaseFile(fileName, static_cast<size_t>(FileFlags::READ) | static_cast<size_t>(FileFlags::WRITE)) {
  if (IsOpen()) {
    mFileData = PlatformOpenMemoryMapFile(mFileHandle, static_cast<size_t>(FileFlags::READ) | static_cast<size_t>(FileFlags::WRITE), mMappedFileHandle, maxSize);
    mOpen = (mFileData != nullptr);

    if (mOpen) {
      // TODO: File access time for memory mapped files is inconsistent.
      // Why does this not update properly?
      PlatformUpdateFileAccessTime(mFileHandle);
    }
  }
}

MemoryMappedFileWriter::~MemoryMappedFileWriter() {
  if (IsOpen()) {
    Flush();
    PlatformCloseMemoryMapFile(mMappedFileHandle);
    PlatformUpdateFileModificationTime(mFileHandle);
  }
}

char* MemoryMappedFileWriter::GetBuffer() {
  return static_cast<char*>(mFileData);
}

bool MemoryMappedFileWriter::Flush() {
  if (!IsOpen()) {
    return false;
  }
  
  return PlatformFlushMemoryMapFile(mMappedFileHandle);
}

}
