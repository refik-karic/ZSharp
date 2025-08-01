#pragma once

#include "FileString.h"
#include "PlatformFile.h"

namespace ZSharp {

class BaseFile {
  public:
  BaseFile(const FileString& fileName, size_t flags);

  BaseFile(const BaseFile& rhs) = delete;

  void operator=(const BaseFile& rhs) = delete;

  bool IsOpen() const;

  size_t GetSize();

  protected:
  FileString* mFile;
  PlatformFileHandle* mFileHandle;
  bool mOpen = false;

  virtual ~BaseFile();
};

class BufferedFileReader final : public BaseFile {
  public:
  BufferedFileReader(const FileString& fileName);

  BufferedFileReader(const BufferedFileReader& rhs) = delete;

  void operator=(const BufferedFileReader& rhs) = delete;

  virtual ~BufferedFileReader();

  size_t Read(void* buffer, size_t length);

  size_t ReadLine();

  const char* GetBuffer() const;

  private:
  static const size_t mDefaultBufferSize;
  size_t mBufferSize = mDefaultBufferSize;
  char* mBuffer = nullptr;

  bool ResetBuffer(size_t size);
};

/*
Write buffering is handled by the OS.
*/
class SystemBufferedFileWriter final : public BaseFile {
  public:
  SystemBufferedFileWriter(const FileString& fileName, size_t flags);

  SystemBufferedFileWriter(const BufferedFileReader& rhs) = delete;

  void operator=(const BufferedFileReader& rhs) = delete;

  virtual ~SystemBufferedFileWriter();

  bool Write(const void* data, size_t length);

  bool Flush();

  private:
};

/*
Write buffering is handled by the app. Prefer SystemBufferedFileWriter where possible.
*/
class BufferedFileWriter final : public BaseFile {
  public:
  BufferedFileWriter(const FileString& fileName, size_t flags);

  BufferedFileWriter(const BufferedFileReader& rhs) = delete;

  void operator=(const BufferedFileReader& rhs) = delete;

  virtual ~BufferedFileWriter();

  bool Write(const void* data, size_t length);

  bool Flush();

  private:
  static const size_t mBufferSize;
  size_t mBufferedDataSize = 0;
  char* mBuffer = nullptr;
};

class MemoryMappedFileReader final : public BaseFile {
public:
  MemoryMappedFileReader(const FileString& fileName);

  MemoryMappedFileReader(const MemoryMappedFileReader& rhs) = delete;

  void operator=(const MemoryMappedFileReader& rhs) = delete;

  virtual ~MemoryMappedFileReader();

  const char* GetBuffer() const;

private:
  PlatformMemoryMappedFileHandle* mMappedFileHandle = nullptr;
  void* mFileData = nullptr;
};

class MemoryMappedFileWriter final : public BaseFile {
public:
  MemoryMappedFileWriter(const FileString& fileName, size_t maxSize);

  MemoryMappedFileWriter(const MemoryMappedFileWriter& rhs) = delete;

  void operator=(const MemoryMappedFileWriter& rhs) = delete;

  virtual ~MemoryMappedFileWriter();

  char* GetBuffer();

  bool Flush();

private:
  PlatformMemoryMappedFileHandle* mMappedFileHandle = nullptr;
  void* mFileData = nullptr;
};

}
