#pragma once

#include <cstdlib>

#include <string>
#include <vector>

class FileString {
  public:
  FileString(const std::string& absoluteFilePath);

  const std::string& GetVolume() const;

  const std::vector<std::string>& GetDirectories() const;

  const std::string& GetFilename() const;

  const std::string& GetExtension() const;

  const std::string& GetAbsolutePath();

  protected:

  private:
  std::string mVolume;
  std::vector<std::string> mDirectories;
  std::string mFilename;
  std::string mExtension;

  // Cache the last absolute path string.
  bool mDirty = false;
  std::string mDirectorySeparator = "\\";
  std::string mExtensionSeparator = ".";
  std::string mAbsolutePath;

  void CacheAbsolutePath();
};
