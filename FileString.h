#pragma once

#include "ZString.h"
#include <cstdlib>

namespace ZSharp {
class FileString final {
  public:
  FileString(const String& absoluteFilePath);

  void operator=(const String& rhs);

  String GetVolume() const;

  String GetFilename() const;

  String GetExtension() const;

  String GetAbsolutePath() const;

  void SetFilename(const String& filename);

  void AddDirectory(const String& directory);

  private:
  char mDrive[_MAX_DRIVE];
  char mDirs[_MAX_DIR];
  char mFilename[_MAX_FNAME];
  char mExtension[_MAX_EXT];
  char mAbsolutePath[_MAX_PATH];
  size_t mPathLength = 0;
  size_t mNumDirectories = 0;

  static const char mDirectorySeparator;
  static const char mExtensionSeparator;

  void Initialize(const String& absoluteFilePath);

  void Reset();

  void UpdateAbsolutePath();
};
}
