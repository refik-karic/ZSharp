#include "FileString.h"

#include "ZAssert.h"
#include "ZBaseTypes.h"

#include <cstring>

#if PLATFORM_WINDOWS
static const char PlatformDirectorySeparator = '\\';
static const char PlatformExtensionSeparator = '.';
static const size_t PlatformMaxDrive = 3;
#endif

// Exampe: C:\Users\kr\Desktop\ZSharp-Tools\src\Debug
namespace ZSharp {
FileString::FileString() {
  Reset();
}

FileString::FileString(const String& absoluteFilePath) {
  Initialize(absoluteFilePath);
}

FileString::FileString(const FileString& rhs) {
  memcpy(mAbsolutePath, rhs.mAbsolutePath, sizeof(mAbsolutePath));
  mDrive = mAbsolutePath;
  mDriveLength = rhs.mDriveLength;
  mDirs = mAbsolutePath + mDriveLength + 1;
  mDirectoryLength = rhs.mDirectoryLength;
  mNumDirectories = rhs.mNumDirectories;
  mFilename = mDirs + mDirectoryLength;
  mFilenameLength = rhs.mFilenameLength;
  mExtension = (rhs.HasExtension()) ? (mFilename + mFilenameLength + 1) : (mFilename + mFilenameLength);
  mExtensionLength = rhs.mExtensionLength;
  mPathLength = rhs.mPathLength;
}

void FileString::operator=(const String& rhs) {
  Initialize(rhs);
}

String FileString::GetVolume() const {
  String drive(mDrive, 0, mDriveLength);
  return drive;
}

String FileString::GetFilename() const {
  String filename(mFilename, 0, mFilenameLength);
  return filename;
}

String FileString::GetExtension() const {
  String extension(mExtension, 0, mExtensionLength);
  return mExtension;
}

String FileString::GetAbsolutePath() const {
  return mAbsolutePath;
}

void FileString::SetFilename(const String& filename) {
  size_t totalFilenameLength = mFilenameLength;
  if (HasExtension()) {
    totalFilenameLength += (1 + mExtensionLength);
  }

  if ((mPathLength - totalFilenameLength) + filename.Length() >= PLATFORM_MAX_PATH) {
    ZAssert(false);
    return;
  }

  memset(mAbsolutePath + (mPathLength - totalFilenameLength), 0, PLATFORM_MAX_PATH - (mPathLength - totalFilenameLength));
  mPathLength -= totalFilenameLength;

  const char* extension = filename.FindFirst('.');
  if (extension != nullptr) {
    mDirs[mDirectoryLength] = PlatformDirectorySeparator;
    mDirectoryLength++;
    mPathLength++;

    size_t length = extension - filename.Str();

    mFilename = mAbsolutePath + mPathLength;
    memcpy(mFilename, filename.Str(), length);
    mFilename[length] = PlatformExtensionSeparator;
    mFilenameLength = length;
    mPathLength += (length + 1);

    extension++;
    mExtension = mAbsolutePath + mPathLength;
    size_t extensionLength = strlen(extension);
    memcpy(mExtension, extension, extensionLength);
    mExtensionLength = extensionLength;
    mPathLength += extensionLength;
  }
  else {
    if (mNumDirectories != 0) {
      if (mAbsolutePath[mPathLength - 1] == PlatformDirectorySeparator) {
        mAbsolutePath[mPathLength - 1] = NULL;
        mPathLength--;
      }
    }

    mFilename = mAbsolutePath + mPathLength;
    mExtension = mAbsolutePath + mPathLength;
    mFilenameLength = 0;
    mExtensionLength = 0;
  }
}

void FileString::AddDirectory(const String& directory) {
  if (mPathLength + directory.Length() >= PLATFORM_MAX_PATH) {
    ZAssert(false);
    return;
  }

  char* directoryBuffer = mDirs + mDirectoryLength;
  (*directoryBuffer) = PlatformDirectorySeparator;
  mDirectoryLength++;
  directoryBuffer++;
  mPathLength++;
  const size_t directoryLength = directory.Length();

  if (HasFilename()) {
    size_t filenameLength = mFilenameLength;
    if (HasExtension()) {
      filenameLength += (mExtensionLength + 1);
    }

    memmove(directoryBuffer + (directoryLength + 1), directoryBuffer, filenameLength);
    memcpy(directoryBuffer, directory.Str(), directoryLength);
    directoryBuffer[directoryLength] = PlatformDirectorySeparator;
    mDirectoryLength += (directoryLength + 1);
    mPathLength += (directoryLength + 1);
    mFilename = mDirs + mDirectoryLength;
    mExtension = mFilename + mFilenameLength + 1;
  }
  else {
    memcpy(directoryBuffer, directory.Str(), directoryLength);
    mDirectoryLength += directoryLength;
    mPathLength += directoryLength;
    mFilename = mDirs + mDirectoryLength;
    mExtension = mFilename;
  }

  mNumDirectories++;
}

bool FileString::HasFilename() const {
  return mFilenameLength != 0;
}

bool FileString::HasExtension() const {
  return mExtensionLength != 0;
}

bool FileString::IsEmpty() const {
  return mPathLength == 0;
}

void FileString::Initialize(const String& absoluteFilePath) {
  Reset();

  if (absoluteFilePath.Length() >= PLATFORM_MAX_PATH) {
    ZAssert(false);
    return;
  }

  const char* volume = absoluteFilePath.FindFirst(':');

  if (volume != nullptr) {
    const char* str = absoluteFilePath.Str();
    size_t length = (volume - str) + 1;

    if (length >= PlatformMaxDrive) {
      ZAssert(false);
      return;
    }

    memcpy(mAbsolutePath, str, length);
    mAbsolutePath[length] = PlatformDirectorySeparator;
    mDrive = mAbsolutePath;
    mDriveLength = length;
    mPathLength += (length + 1); // Account for directory separator.
    mDirs = mAbsolutePath + mPathLength;
  }
  else {
    return;
  }

  for (const char* directory = strchr(absoluteFilePath.Str(), PlatformDirectorySeparator); directory != nullptr;) {
    const char* nextDirectory = directory;
    nextDirectory++;

    if (nextDirectory != nullptr) {
      nextDirectory = strchr(nextDirectory, PlatformDirectorySeparator);
    }
    else {
      directory = nullptr;
      continue;
    }

    if (nextDirectory != nullptr) {
      const char* str = absoluteFilePath.Str();
      size_t start = (directory - str) + 1;
      size_t length = (nextDirectory - directory) - 1;

      char* directoryBuffer = mAbsolutePath + mPathLength;
      memcpy(directoryBuffer, str + start, length);
      directoryBuffer[length] = PlatformDirectorySeparator;
      mDirectoryLength += (length + 1);
      mPathLength += (length + 1);
      mNumDirectories++;
    }
    else {
      directory++;
      const char* extension = strchr(directory, PlatformExtensionSeparator);
      if (extension != nullptr) {
        size_t length = extension - directory;
        
        mFilename = mAbsolutePath + mPathLength;

        memcpy(mFilename, directory, length);
        mFilename[length] = PlatformExtensionSeparator;
        mFilenameLength = length;
        mPathLength += (length + 1);

        extension++;
        mExtension = mAbsolutePath + mPathLength;
        size_t extensionLength = strlen(extension);
        memcpy(mExtension, extension, extensionLength);
        mExtensionLength = extensionLength;
        mPathLength += extensionLength;
      }
      else {
        char* directoryBuffer = mAbsolutePath + mPathLength;
        size_t directoryLength = strlen(directory);
        memcpy(directoryBuffer, directory, directoryLength);
        mNumDirectories++;
        mDirectoryLength += directoryLength;
        mPathLength += directoryLength;
        mFilename = mAbsolutePath + mPathLength;
        mExtension = mAbsolutePath + mPathLength;
      }
    }

    directory = nextDirectory;
  }
}

void FileString::Reset() {
  memset(mAbsolutePath, 0, sizeof(mAbsolutePath));
  mDrive = mAbsolutePath;
  mDirs = mAbsolutePath;
  mFilename = mAbsolutePath;
  mExtension = mAbsolutePath;
  mDriveLength = 0;
  mNumDirectories = 0;
  mDirectoryLength = 0;
  mFilenameLength = 0;
  mExtensionLength = 0;
  mPathLength = 0;
}
}
