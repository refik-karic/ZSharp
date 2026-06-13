#include "FileString.h"

#include "ZAssert.h"
#include "ZBaseTypes.h"
#include "PlatformFile.h"

#include <cstring>

#if PLATFORM_WINDOWS
static const char PlatformDirectorySeparator = '\\';
static const char PlatformExtensionSeparator = '.';
static const ZSharp::uint16 PlatformMaxDrive = 3;
#endif

// Exampe: C:\Users\kr\Desktop\ZSharp-Tools\src\Debug
namespace ZSharp {
FileString::FileString() {
  Reset();
}

FileString::FileString(const String& absoluteFilePath) {
  Initialize(absoluteFilePath);
}

FileString::FileString(const FileString& rhs)
  : mDrive(rhs.mDrive),
    mDirs(rhs.mDirs),
    mFilename(rhs.mFilename),
    mExtension(rhs.mExtension),
    mDriveLength(rhs.mDriveLength),
    mNumDirectories(rhs.mNumDirectories),
    mDirectoryLength(rhs.mDirectoryLength),
    mFilenameLength(rhs.mFilenameLength),
    mExtensionLength(rhs.mExtensionLength),
    mPathLength(rhs.mPathLength)
{
  memcpy(mAbsolutePath, rhs.mAbsolutePath, sizeof(mAbsolutePath));
}

void FileString::operator=(const String& rhs) {
  Initialize(rhs);
}

String FileString::GetVolume() const {
  String drive(mAbsolutePath + mDrive, 0, mDriveLength);
  return drive;
}

String FileString::GetFilename() const {
  String filename(mAbsolutePath + mFilename, 0, mFilenameLength);
  return filename;
}

String FileString::GetExtension() const {
  String extension(mAbsolutePath + mExtension, 0, mExtensionLength);
  return extension;
}

Span<const char> FileString::GetAbsolutePath() const {
  Span<const char> path(mAbsolutePath, mPathLength);
  return path;
}

void FileString::SetFilename(const String& filename) {
  bool changingFilename = HasFilename();

  uint16 totalFilenameLength = mFilenameLength;
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
    if (!changingFilename) {
      mAbsolutePath[mDirs + mDirectoryLength] = PlatformDirectorySeparator;
      mDirectoryLength++;
      mPathLength++;
    }

    uint16 length = (uint16)(extension - filename.Str());

    mFilename = mPathLength;
    memcpy(mAbsolutePath + mFilename, filename.Str(), length);
    mAbsolutePath[mFilename + length] = PlatformExtensionSeparator;
    mFilenameLength = length;
    mPathLength += (length + 1);

    extension++;
    mExtension = mPathLength;
    uint16 extensionLength = (uint16)strlen(extension);
    memcpy(mAbsolutePath + mExtension, extension, extensionLength);
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

    mFilename = mPathLength;
    mExtension = mPathLength;
    mFilenameLength = 0;
    mExtensionLength = 0;
  }
}

void FileString::AddDirectory(const String& directory) {
  if (mPathLength + directory.Length() >= PLATFORM_MAX_PATH) {
    ZAssert(false);
    return;
  }

  char* directoryBuffer = mAbsolutePath + mDirs + mDirectoryLength;
  (*directoryBuffer) = PlatformDirectorySeparator;
  mDirectoryLength++;
  directoryBuffer++;
  mPathLength++;
  const uint16 directoryLength = (uint16)directory.Length();

  if (HasFilename()) {
    uint16 filenameLength = mFilenameLength;
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

bool FileString::Exists() const {
  return PlatformFileExists(*this);
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
    uint16 length = (uint16)((volume - str) + 1);

    if (length >= PlatformMaxDrive) {
      ZAssert(false);
      return;
    }

    memcpy(mAbsolutePath, str, length);
    mAbsolutePath[length] = PlatformDirectorySeparator;
    mDrive = 0;
    mDriveLength = length;
    mPathLength += (length + 1); // Account for directory separator.
    mDirs = mPathLength;
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
      uint16 start = (uint16)((directory - str) + 1);
      uint16 length = (uint16)((nextDirectory - directory) - 1);

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
        uint16 length = (uint16)(extension - directory);
        
        mFilename = mPathLength;

        memcpy(mAbsolutePath + mFilename, directory, length);
        mAbsolutePath[mFilename + length] = PlatformExtensionSeparator;
        mFilenameLength = length;
        mPathLength += (length + 1);

        extension++;
        mExtension = mPathLength;
        uint16 extensionLength = (uint16)strlen(extension);
        memcpy(mAbsolutePath + mExtension, extension, extensionLength);
        mExtensionLength = extensionLength;
        mPathLength += extensionLength;
      }
      else {
        char* directoryBuffer = mAbsolutePath + mPathLength;
        uint16 directoryLength = (uint16)strlen(directory);
        memcpy(directoryBuffer, directory, directoryLength);
        mNumDirectories++;
        mDirectoryLength += directoryLength;
        mPathLength += directoryLength;
        mFilename = mPathLength;
        mExtension = mPathLength;
      }
    }

    directory = nextDirectory;
  }
}

void FileString::Reset() {
  memset(mAbsolutePath, 0, sizeof(mAbsolutePath));
  mDrive = 0;
  mDirs = 0;
  mFilename = 0;
  mExtension = 0;
  mDriveLength = 0;
  mNumDirectories = 0;
  mDirectoryLength = 0;
  mFilenameLength = 0;
  mExtensionLength = 0;
  mPathLength = 0;
}
}
