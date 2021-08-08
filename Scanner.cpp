#include "Scanner.h"

#include "FileUtilities.h"

#include <iostream>
#include <string>

namespace ZSharp {

void Scanner::ScanTokens(const char* fileName, std::vector<Token>& tokens) {
  std::vector<char> fileContents;
  ReadFileToBuffer(fileName, fileContents);

  if(fileContents.empty()) {
    return;
  }
  
  while (!IsAtEnd(fileContents)) {
    mStart = mCurrent;
    ScanToken(tokens, fileContents);
  }
}

bool Scanner::IsAtEnd(std::vector<char>& fileBuffer) {
  return mCurrent >= fileBuffer.size();
}

char Scanner::Advance(std::vector<char>& fileBuffer) {
  mCurrent++;
  return fileBuffer[mCurrent - 1];
}

char Scanner::Peek(std::vector<char>& fileBuffer) {
  if (IsAtEnd(fileBuffer)) {
    return '\0';
  } else {
    return fileBuffer[mCurrent];
  }
}

void Scanner::AddToken(Token& token, std::vector<Token>& tokens) {
  tokens.push_back(token);
}

char Scanner::PeekNext(std::vector<char>& fileBuffer) {
  if (mCurrent + 1 >= fileBuffer.size()) {
    return '\0';
  } else {
    return fileBuffer[mCurrent + 1];
  }
}

}
