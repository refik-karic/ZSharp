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

void IniFile::ParseFile() {
  BufferedFileReader fileReader(mFileName);
  if (!fileReader.IsOpen()) {
    return;
  }

  IniSection* activeSection = nullptr;

  for (size_t bytesRead = fileReader.ReadLine(); bytesRead > 0; bytesRead = fileReader.ReadLine()) {
    String currentLine(fileReader.GetBuffer());
    currentLine.Trim('\n');
    currentLine.Trim('\r');

    switch (currentLine[0]) {
    case '[': // Section
    {
      const char* endSection = currentLine.FindLast(']');
      if (endSection != nullptr) {
        size_t sectionLength = endSection - currentLine.Str();
        String sectionString(currentLine.SubStr(1, sectionLength));

        IniSection section(sectionString);
        mSections.PushBack(section);
        activeSection = &mSections[mSections.Size() - 1];
      }
    }
      break;
    case ';': // Comment
      break;
    default: // Key value pair
      const char* delimLocation = currentLine.FindFirst('=');
      if (delimLocation != nullptr) {
        size_t keyLength = delimLocation - currentLine.Str();

        String key(currentLine.SubStr(0, keyLength));
        String value(currentLine.SubStr(keyLength + 1, currentLine.GetLength()));

        if (activeSection != nullptr) {
          StringPair kvp(key, value);
          activeSection->mPairs.PushBack(kvp);
        }
      }
      break;
    }
  }
}

}
