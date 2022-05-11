#include "FileString.h"

#include "ZBaseTypes.h"

#include <cstdio>
#include <cstring>

// Exampe: C:\Users\kr\Desktop\ZSharp-Tools\src\Debug
namespace ZSharp {
FileString::FileString(const String& absoluteFilePath) {
  const char* volume = strchr(absoluteFilePath.Str(), ':');

  if (volume != nullptr) {
    const char* str = absoluteFilePath.Str();
    size_t length = (volume - absoluteFilePath.Str()) + 1;
    mVolume.Append(str, 0, length);
  }

  for (const char* directory = strchr(absoluteFilePath.Str(), '\\'); directory != nullptr;) {
    const char* nextDirectory = directory;
    nextDirectory++;

    if (nextDirectory != nullptr) {
      nextDirectory = strchr(nextDirectory, '\\');
    }
    else {
      directory = nullptr;
      continue;
    }

    if (nextDirectory != nullptr) {
      const char* str = absoluteFilePath.Str();
      size_t start = (directory - absoluteFilePath.Str()) + 1;
      size_t length = (nextDirectory - directory) - 1;
      String parsedDirectory(str, start, length);
      mDirectories.PushBack(parsedDirectory);
    }
    else {
      directory++;
      const char* extension = strchr(directory, '.');
      if (extension != nullptr) {
        size_t length = extension - directory;
        String parsedFilename(directory, 0, length);
        mFilename = parsedFilename;

        extension++;
        String parsedExtension(extension);
        mExtension = parsedExtension;
      }
      else {
        String parsedDirectory(directory);
        mDirectories.PushBack(parsedDirectory);
      }
    }
    
    directory = nextDirectory;
  }

  CacheAbsolutePath();
}

const String& FileString::GetVolume() const {
  return mVolume;
}

const Array<String>& FileString::GetDirectories() const {
  return mDirectories;
}

const String& FileString::GetFilename() const {
  return mFilename;
}

const String& FileString::GetExtension() const {
  return mExtension;
}

const String& FileString::GetAbsolutePath() {
  if (mDirty) {
    CacheAbsolutePath();
  }

  return mAbsolutePath;
}

void FileString::CacheAbsolutePath() {
  mAbsolutePath.Clear();

  mAbsolutePath.Append(mVolume);
  mAbsolutePath.Append(mDirectorySeparator);
  for (size_t i = 0; i < mDirectories.Size(); ++i) {
    mAbsolutePath.Append(mDirectories[i]);
    
    if (i != (mDirectories.Size() - 1)) {
      mAbsolutePath.Append(mDirectorySeparator);
    }
    else if(!mFilename.IsEmpty()) {
      mAbsolutePath.Append(mDirectorySeparator);
    }
  }

  if (!mFilename.IsEmpty()) {
    mAbsolutePath.Append(mFilename);

    if (!mExtension.IsEmpty()) {
      mAbsolutePath.Append(mExtensionSeparator);
      mAbsolutePath.Append(mExtension);
    }
  }

  mDirty = false;
}
}
