#pragma once

#include "FileString.h"

#include <cstdio>

namespace ZSharp {

class BaseFile {
  public:
  BaseFile(const FileString& fileName, const char* mode);

  BaseFile(const BaseFile& rhs) = delete;

  void operator=(const BaseFile& rhs) = delete;

  bool IsOpen() const;

  protected:
  FileString mFile;
  FILE* mFileHandle = nullptr;
  bool mOpen = false;

  ~BaseFile();
};

class BufferedFileReader final : public BaseFile {
  public:
  BufferedFileReader(const FileString& fileName);

  BufferedFileReader(const BufferedFileReader& rhs) = delete;

  void operator=(const BufferedFileReader& rhs) = delete;

  ~BufferedFileReader();

  size_t Read(void* buffer, size_t length);

  size_t ReadLine();

  const char* GetBuffer() const;

  private:
  const size_t mDefaultBufferSize = 4096;
  size_t mBufferSize = mDefaultBufferSize;
  char* mBuffer = nullptr;

  bool ResetBuffer(size_t size);
};

class BufferedFileWriter final : public BaseFile {
  public:
  BufferedFileWriter(const FileString& fileName);

  BufferedFileWriter(const BufferedFileReader& rhs) = delete;

  void operator=(const BufferedFileReader& rhs) = delete;

  ~BufferedFileWriter();

  bool Write(const void* data, size_t length);

  bool Flush();

  private:
  const size_t mBufferSize = 4096;
  size_t mBufferedDataSize = 0;
  char* mBuffer = nullptr;
};

}
