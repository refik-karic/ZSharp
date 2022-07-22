#include "IniFile.h"

#include "ZFile.h"

namespace ZSharp {

IniFile::IniFile(const FileString& fileName)
: mFileName(fileName) {
  ParseFile();
}

void IniFile::ParseFile() {
  BufferedFileReader fileReader(mFileName);
  if (!fileReader.IsOpen()) {
    return;
  }

  for (size_t bytesRead = fileReader.ReadLine(); bytesRead > 0; bytesRead = fileReader.ReadLine()) {
    String currentLine(fileReader.GetBuffer());

    switch (currentLine[0]) {
    case '[': // Section
    {
      const char* endSection = currentLine.FindLast(']');

      if (endSection != nullptr) {
        size_t sectionLength = endSection - currentLine.Str();
        String sectionString(currentLine.SubStr(1, sectionLength));
      }
    }
      break;
    case ';': // Comment
      break;
    default: // Key value pair
      break;
    }
  }
}

}
