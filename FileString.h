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

  const String& GetAbsolutePath() const;

  void SetFilename(const String& filename);

  void AddDirectory(const String& directory);

  private:
  String mVolume;
  Array<String> mDirectories;
  String mFilename;
  String mExtension;

  String mDirectorySeparator = "\\";
  String mExtensionSeparator = ".";
  String mAbsolutePath;

  void Initialize(const String& absoluteFilePath);

  void Reset();

  void UpdateAbsolutePath();
};
}
