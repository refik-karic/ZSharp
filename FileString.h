#pragma once

#include "ZString.h"
#include <cstdlib>

namespace ZSharp {
class FileString final {
  public:
  FileString();

  FileString(const String& absoluteFilePath);

  FileString(const FileString& rhs);

  void operator=(const String& rhs);

  String GetVolume() const;

  String GetFilename() const;

  String GetExtension() const;

  String GetAbsolutePath() const;

  void SetFilename(const String& filename);

  void AddDirectory(const String& directory);

  bool HasFilename() const;

  bool HasExtension() const;

  bool IsEmpty() const;

  private:
  char* mDrive = nullptr;
  char* mDirs = nullptr;
  char* mFilename = nullptr;
  char* mExtension = nullptr;
  char mAbsolutePath[_MAX_PATH];
  size_t mDriveLength = 0;
  size_t mNumDirectories = 0;
  size_t mDirectoryLength = 0;
  size_t mFilenameLength = 0;
  size_t mExtensionLength = 0;
  size_t mPathLength = 0;

  static const char mDirectorySeparator;
  static const char mExtensionSeparator;

  void Initialize(const String& absoluteFilePath);

  void Reset();
};
}
