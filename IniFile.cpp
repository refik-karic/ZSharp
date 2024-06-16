#include "IniFile.h"

#include "ZFile.h"

namespace ZSharp {

IniFile::IniFile(const FileString& fileName)
: mFileName(fileName) {
  ParseFile();
}

const Array<IniFile::IniSection>& IniFile::GetSections() const {
  return mSections;
}

void IniFile::GetAllValuesForSection(const String& section, Array<String>& outValues) const {
  for (const IniSection& currentSection : mSections) {
    if (currentSection.mSectionName == section) {
      for (const StringPair& pair : currentSection.mPairs) {
        outValues.PushBack(pair.mValue);
      }
      break;
    }
  }
}

String IniFile::FindValue(const String& section, const String& key) const {
  String foundValue;

  for (const IniSection& currentSection : mSections) {
    if (!foundValue.IsEmpty()) {
      break;
    }

    if (currentSection.mSectionName == section) {
      for (const StringPair& stringPair : currentSection.mPairs) {
        if (stringPair.mKey == key) {
          foundValue = stringPair.mValue;
          break;
        }
      }
    }
  }

  return foundValue;
}

bool IniFile::Loaded() const {
  return mLoaded;
}

void IniFile::ParseFile() {
  MemoryMappedFileReader reader(mFileName);
  if (!reader.IsOpen()) {
    return;
  }

  mLoaded = true;
  IniSection* activeSection = nullptr;

  const char* fileBuffer = reader.GetBuffer();
  size_t fileSize = reader.GetSize();
  size_t lastLineIndex = 0;

  for (size_t offset = 0; offset < fileSize; ++offset) {
    if (fileBuffer[offset] == '\n') {
      size_t lineLength = offset - lastLineIndex;
      if ((offset > 0) && (fileBuffer[offset - 1] == '\r')) {
        --lineLength;
      }

      if (lineLength == 0) {
        lastLineIndex = offset + 1;
        continue;
      }

      String currentLine(fileBuffer + lastLineIndex, 0, lineLength);
      lastLineIndex = offset + 1;

      switch (currentLine[0]) {
        case '[': // Section
        {
          const char* endSection = currentLine.FindLast(']');
          if (endSection != nullptr) {
            size_t sectionLength = endSection - currentLine.Str();
            String sectionString(currentLine.Str(), 1, sectionLength - 1);
            activeSection = &mSections.EmplaceBack(sectionString);
          }
        }
        break;
        case ';': // Comment
          break;
        default: // Key value pair
          const char* delimLocation = currentLine.FindFirst('=');
          if (delimLocation != nullptr) {
            size_t keyLength = delimLocation - currentLine.Str();
            String key(currentLine.Str(), 0, keyLength);
            String value(currentLine.Str(), keyLength + 1, currentLine.Length() - keyLength - 1);

            if (activeSection != nullptr) {
              activeSection->mPairs.EmplaceBack(key, value);
            }
          }
          break;
      }
    }
  }
}

}
