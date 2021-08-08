#pragma once

#include <cstdint>

#include <string>
#include <vector>

#include "JsonObject.h"
#include "Scanner.h"
#include "Token.h"

namespace ZSharp {

class JsonScanner : public Scanner {
  public:

  void PopulateJsonObject(JsonObject& jsonObject, std::vector<Token>& tokens);

  void ProcessJsonData(JsonObject& jsonObject, std::vector<Token>::iterator& begin, std::vector<Token>::iterator& end, bool isArray);

  private:
  void ScanToken(std::vector<Token>& tokens, std::vector<char>& fileBuffer) override;

  void ScanString(std::vector<Token>& tokens, std::vector<char>& fileBuffer);

  void ScanNumber(std::vector<Token>& tokens, std::vector<char>& fileBuffer);

  bool IsDigit(char letter);

  std::int64_t TokenToInt(Token token);

  double TokenToFloat(Token token);

  bool TokenToBool(Token token);
};
}
