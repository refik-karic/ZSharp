#include "FileString.h"

#include "ZAssert.h"
#include "ZBaseTypes.h"

#include <cstring>

// Exampe: C:\Users\kr\Desktop\ZSharp-Tools\src\Debug
namespace ZSharp {
const char FileString::mDirectorySeparator = '\\';
const char FileString::mExtensionSeparator = '.';

FileString::FileString(const String& absoluteFilePath) {
  Initialize(absoluteFilePath);
}

void FileString::operator=(const String& rhs) {
  Initialize(rhs);
}

String FileString::GetVolume() const {
  return mDrive;
}

String FileString::GetFilename() const {
  return mFilename;
}

String FileString::GetExtension() const {
  return mExtension;
}

String FileString::GetAbsolutePath() const {
  return mAbsolutePath;
}

void FileString::SetFilename(const String& filename) {
  const size_t filenameLength = strlen(mFilename);
  const size_t extensionLength = strlen(mExtension);

  size_t totalFilenameLength = filenameLength;
  if (extensionLength != 0) {
    totalFilenameLength += 1 + extensionLength;
  }

  if ((mPathLength - totalFilenameLength) + filename.Length() >= _MAX_PATH) {
    ZAssert(false);
    return;
  }

  const char* extension = filename.FindFirst('.');
  if (extension != nullptr) {
    size_t length = extension - filename.Str();

    memcpy(mFilename, filename.Str(), length);
    mFilename[length] = NULL;

    extension++;

    strcpy(mExtension, extension);
  }
  else {
    memset(mFilename, 0, sizeof(mFilename));
    memset(mExtension, 0, sizeof(mExtension));
  }

  UpdateAbsolutePath();
}

void FileString::AddDirectory(const String& directory) {
  if (mPathLength + directory.Length() >= _MAX_PATH) {
    ZAssert(false);
    return;
  }

  char* directoryBuffer = mDirs;
  for (size_t i = 0; i < mNumDirectories; ++i) {
    directoryBuffer += strlen(directoryBuffer) + 1;
  }

  const size_t directoryLength = directory.Length();
  memcpy(directoryBuffer, directory.Str(), directoryLength);
  directoryBuffer[directoryLength] = NULL;
  mNumDirectories++;

  UpdateAbsolutePath();
}

void FileString::Initialize(const String& absoluteFilePath) {
  Reset();

  if (absoluteFilePath.Length() >= _MAX_PATH) {
    ZAssert(false);
    return;
  }

  const char* volume = absoluteFilePath.FindFirst(':');

  if (volume != nullptr) {
    const char* str = absoluteFilePath.Str();
    size_t length = (volume - str) + 1;

    if (length >= _MAX_DRIVE) {
      ZAssert(false);
      return;
    }

    memcpy(mDrive, str, length);
    mDrive[length] = NULL;
  }
  else {
    return;
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
      size_t start = (directory - str) + 1;
      size_t length = (nextDirectory - directory) - 1;
      
      char* directoryBuffer = mDirs;
      for (size_t i = 0; i < mNumDirectories; ++i) {
        directoryBuffer += strlen(directoryBuffer) + 1;
      }

      memcpy(directoryBuffer, str + start, length);
      directoryBuffer[length] = NULL;
      mNumDirectories++;
    }
    else {
      directory++;
      const char* extension = strchr(directory, '.');
      if (extension != nullptr) {
        size_t length = extension - directory;
        
        memcpy(mFilename, directory, length);
        mFilename[length] = NULL;

        extension++;
        strcpy(mExtension, extension);
      }
      else {
        char* directoryBuffer = mDirs;
        for (size_t i = 0; i < mNumDirectories; ++i) {
          directoryBuffer += strlen(directoryBuffer) + 1;
        }

        strcpy(directoryBuffer, directory);
        mNumDirectories++;
      }
    }

    directory = nextDirectory;
  }

  UpdateAbsolutePath();
}

void FileString::Reset() {
  memset(mDrive, 0, sizeof(mDrive));
  memset(mDirs, 0, sizeof(mDirs));
  memset(mFilename, 0, sizeof(mFilename));
  memset(mExtension, 0, sizeof(mExtension));
  memset(mAbsolutePath, 0, sizeof(mAbsolutePath));
  mPathLength = 0;
  mNumDirectories = 0;
}

void FileString::UpdateAbsolutePath() {
  char* position = mAbsolutePath;

  size_t driveLength = strlen(mDrive);
  memcpy(position, mDrive, driveLength);

  position += driveLength;

  (*position) = mDirectorySeparator;
  position++;

  const size_t filenameLength = strlen(mFilename);
  bool hasFilename = filenameLength != 0;

  char* directoryBuffer = mDirs;
  for (size_t i = 0; i < mNumDirectories; ++i) {
    const size_t dirLength = strlen(directoryBuffer);
    memcpy(position, directoryBuffer, dirLength);

    if (i != (mNumDirectories - 1)) {
      position[dirLength] = mDirectorySeparator;
      position += dirLength + 1;
    }
    else if(hasFilename) {
      position[dirLength] = mDirectorySeparator;
      position += dirLength + 1;
    }
    else {
      position += dirLength;
    }

    directoryBuffer += dirLength + 1;
  }

  if (hasFilename) {
    memcpy(position, mFilename, filenameLength);
    position += filenameLength;

    const size_t extensionLength = strlen(mExtension);

    if (extensionLength != 0) {
      (*position) = mExtensionSeparator;
      position++;

      strcpy(position, mExtension);
    }
  }

  mPathLength = position - mAbsolutePath;
}
}
