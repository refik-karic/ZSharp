#include "ZFile.h"

#include <cstdlib>
#include <cstring>

#pragma warning(disable : 4996)

namespace ZSharp {

BaseFile::BaseFile(const FileString& fileName, const char* mode)
: mFile(fileName) {
  mFileHandle = fopen(mFile.GetAbsolutePath().Str(), mode);
  mOpen = (mFileHandle != nullptr);
}

BaseFile::~BaseFile() {
  if (IsOpen()) {
    fclose(mFileHandle);
  }
}

bool BaseFile::IsOpen() const {
  return mOpen;
}

BufferedFileReader::BufferedFileReader(const FileString& fileName) 
  : BaseFile(fileName, "rb") {
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

  return fread(buffer, 1, length, mFileHandle);
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

    char nextChar = static_cast<char>(fgetc(mFileHandle));
    if (nextChar == EOF) {
      mBuffer[i] = NULL;
      keepReading = false;
    }
    else {
      mBuffer[i] = nextChar;
      numRead++;
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
  : BaseFile(fileName, "wb") {
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
      bool success = fwrite(mBuffer, sizeof(char), mBufferedDataSize, mFileHandle) == mBufferedDataSize;
      if (!success) {
        return false;
      }

      mBufferedDataSize = 0;
      memset(mBuffer, 0, mBufferSize);
    }

    // Write out the rest.
    bool success = fwrite(data, sizeof(char), length, mFileHandle) == mBufferedDataSize;
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
    bool success = fwrite(mBuffer, sizeof(char), mBufferedDataSize, mFileHandle) == mBufferedDataSize;
    if (!success) {
      return false;
    }

    mBufferedDataSize = 0;
    memset(mBuffer, 0, mBufferSize);
  }

  return fflush(mFileHandle) == 0;
}

}

#pragma warning(default : 4996)
