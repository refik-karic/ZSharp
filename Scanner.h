#pragma once

#include <cstdint>
#include <cstdlib>

#include <vector>

#include "Token.h"

namespace ZSharp {

class Scanner {
  public:

  Scanner() {

  }

  void ScanTokens(const char* fileName, std::vector<Token>& tokens);

  protected:
  std::size_t mStart = 0;
  
  std::size_t mCurrent = 0;
  
  std::size_t mLine = 0;

  bool IsAtEnd(std::vector<char>& fileBuffer);

  virtual void ScanToken(std::vector<Token>& tokens, std::vector<char>& fileBuffer) = 0;

  char Advance(std::vector<char>& fileBuffer);

  char Peek(std::vector<char>& fileBuffer);

  void AddToken(Token& token, std::vector<Token>& tokens);

  char PeekNext(std::vector<char>& fileBuffer);
};

}
