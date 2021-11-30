#pragma once

#include <cstdlib>

#include <string>

#include "Array.h"

namespace ZSharp {
class FileString {
  public:
  FileString(const std::string& absoluteFilePath);

  const std::string& GetVolume() const;

  const Array<std::string>& GetDirectories() const;

  const std::string& GetFilename() const;

  const std::string& GetExtension() const;

  const std::string& GetAbsolutePath();

  protected:

  private:
  std::string mVolume;
  Array<std::string> mDirectories;
  std::string mFilename;
  std::string mExtension;

  // Cache the last absolute path string.
  bool mDirty = false;
  std::string mDirectorySeparator = "\\";
  std::string mExtensionSeparator = ".";
  std::string mAbsolutePath;

  void CacheAbsolutePath();
};
}
