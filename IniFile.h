#pragma once

#include "Array.h"
#include "FileString.h"
#include "Pair.h"
#include "ZString.h"

namespace ZSharp {

class IniFile final {
  public:
  typedef Pair<String, String> StringPair;

  struct IniSection {
    IniSection() = default;

    IniSection(const String& sectionName)
      : mSectionName(sectionName) {

    }

    String mSectionName;
    Array<StringPair> mPairs;
  };

  IniFile(const FileString& fileName);

  const Array<IniFile::IniSection>& GetSections() const;

  void GetAllValuesForSection(const String& section, Array<String>& outValues) const;

  String FindValue(const String& section, const String& key) const;

  bool Loaded() const;

  private:
  FileString mFileName;
  Array<IniSection> mSections;
  bool mLoaded = false;

  void ParseFile();
};

}
