#pragma once

#include "ZString.h"

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

  bool Exists() const;

  private:
  char mAbsolutePath[PLATFORM_MAX_PATH];
  uint16 mDrive = 0;
  uint16 mDirs = 0;
  uint16 mFilename = 0;
  uint16 mExtension = 0;
  uint16 mDriveLength = 0;
  uint16 mNumDirectories = 0;
  uint16 mDirectoryLength = 0;
  uint16 mFilenameLength = 0;
  uint16 mExtensionLength = 0;
  uint16 mPathLength = 0;

  void Initialize(const String& absoluteFilePath);

  void Reset();
};
}
