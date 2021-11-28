#include "FileString.h"

#include <cstdio>
#include <cstring>

// Exampe: C:\Users\kr\Desktop\ZSharp-Tools\src\Debug
namespace ZSharp {
FileString::FileString(const std::string& absoluteFilePath) {
  const char* volume = std::strchr(absoluteFilePath.c_str(), ':');

  if (volume != nullptr) {
    mVolume.append(absoluteFilePath.c_str(), 0, (volume - absoluteFilePath.c_str()) + 1);
  }

  for (const char* directory = std::strchr(absoluteFilePath.c_str(), '\\'); directory != nullptr;) {
    const char* nextDirectory = directory;
    nextDirectory++;

    if (nextDirectory != nullptr) {
      nextDirectory = std::strchr(nextDirectory, '\\');
    }
    else {
      directory = nullptr;
      continue;
    }

    if (nextDirectory != nullptr) {
      std::string parsedDirectory(absoluteFilePath.c_str(), (directory - absoluteFilePath.c_str()) + 1, (nextDirectory - directory) - 1);
      mDirectories.push_back(parsedDirectory);
    }
    else {
      directory++;
      const char* extension = std::strchr(directory, '.');
      if (extension != nullptr) {
        std::string parsedFilename(directory, 0, extension - directory);
        mFilename = parsedFilename;

        extension++;
        std::string parsedExtension(extension);
        mExtension = parsedExtension;
      }
      else {
        std::string parsedDirectory(directory);
        mDirectories.push_back(parsedDirectory);
      }
    }
    
    directory = nextDirectory;
  }

  CacheAbsolutePath();
}

const std::string& FileString::GetVolume() const {
  return mVolume;
}

const std::vector<std::string>& FileString::GetDirectories() const {
  return mDirectories;
}

const std::string& FileString::GetFilename() const {
  return mFilename;
}

const std::string& FileString::GetExtension() const {
  return mExtension;
}

const std::string& FileString::GetAbsolutePath() {
  if (mDirty) {
    CacheAbsolutePath();
  }

  return mAbsolutePath;
}

void FileString::CacheAbsolutePath() {
  mAbsolutePath.clear();

  mAbsolutePath.append(mVolume);
  mAbsolutePath.append(mDirectorySeparator);
  for (uint32_t i = 0; i < mDirectories.size(); ++i) {
    mAbsolutePath.append(mDirectories[i]);
    
    if (i != (mDirectories.size() - 1)) {
      mAbsolutePath.append(mDirectorySeparator);
    }
    else if(!mFilename.empty()) {
      mAbsolutePath.append(mDirectorySeparator);
    }
  }

  if (!mFilename.empty()) {
    mAbsolutePath.append(mFilename);

    if (!mExtension.empty()) {
      mAbsolutePath.append(mExtensionSeparator);
      mAbsolutePath.append(mExtension);
    }
  }

  mDirty = false;
}
}
