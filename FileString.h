#pragma once

#include <cstdlib>

#include "Array.h"
#include "ZString.h"

namespace ZSharp {
class FileString {
  public:
  FileString(const String& absoluteFilePath);

  const String& GetVolume() const;

  const Array<String>& GetDirectories() const;

  const String& GetFilename() const;

  const String& GetExtension() const;

  const String& GetAbsolutePath();

  protected:

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

  void CacheAbsolutePath();
};
}
