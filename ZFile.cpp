#include "ZFile.h"

#include <cstdlib>
#include <cstring>

#include "PlatformFile.h"

namespace ZSharp {

BaseFile::BaseFile(const FileString& fileName, size_t flags)
: mFile(fileName) {
  mFileHandle = PlatformOpenFile(mFile, flags);
  mOpen = (mFileHandle != nullptr);
}

BaseFile::~BaseFile() {
  if (IsOpen()) {
    PlatformCloseFile(mFileHandle);
  }
}

bool BaseFile::IsOpen() const {
  return mOpen;
}

BufferedFileReader::BufferedFileReader(const FileString& fileName) 
  : BaseFile(fileName, static_cast<size_t>(FileFlags::READ)) {
  mBuffer = static_cast<char*>(malloc(mBufferSize));
  if (mBuffer != nullptr) {
    memset(mBuffer, 0, mBufferSize);
  }
}

BufferedFileReader::~BufferedFileReader() {
  if (mBuffer != nullptr) {
    free(mBuffer);
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
      char* resizedBuf = static_cast<char*>(realloc(mBuffer, newLength));
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
  char* resizedBuf = static_cast<char*>(realloc(mBuffer, size));
  if (resizedBuf == nullptr) {
    return false;
  }

  mBuffer = resizedBuf;
  mBufferSize = size;
  memset(mBuffer, 0, mBufferSize);
  return true;
}

BufferedFileWriter::BufferedFileWriter(const FileString& fileName) 
  : BaseFile(fileName, static_cast<size_t>(FileFlags::WRITE)) {
  mBuffer = static_cast<char*>(malloc(mBufferSize));
  if (mBuffer != nullptr) {
    memset(mBuffer, 0, mBufferSize);
  }
}

BufferedFileWriter::~BufferedFileWriter() {
  Flush();

  if (mBuffer != nullptr) {
    free(mBuffer);
  }
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
    bool success = PlatformWriteFile(mFileHandle, data, length) == mBufferedDataSize;
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

}
