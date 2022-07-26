#pragma once

#include "Array.h"
#include "ZString.h"

namespace ZSharp {
class FileString final {
  public:
  FileString(const String& absoluteFilePath);

  void operator=(const String& rhs);

  const String& GetVolume() const;

  const Array<String>& GetDirectories() const;

  const String& GetFilename() const;

  const String& GetExtension() const;

  const String& GetAbsolutePath();

  private:
  String mVolume;
  Array<String> mDirectories;
  String mFilename;
  String mExtension;

  // Cache the last absolute path string.
  bool mDirty = false;
  String mDirectorySeparator = "\\";
  String mExtensionSeparator = ".";
  String mAbsolutePath;

  void Initialize(const String& absoluteFilePath);

  void Reset();

  void CacheAbsolutePath();
};
}
