#pragma once

#include "Array.h"
#include "FileString.h"
#include "Pair.h"
#include "ZString.h"

namespace ZSharp {

class IniFile final {
  public:
  IniFile(const FileString& fileName);

  private:
  FileString mFileName;

  typedef Pair<String, String> StringPair;

  struct IniSection {
    Array<StringPair> mPairs;
  };

  Array<IniSection> mSections;

  void ParseFile();
};

}
