#include "JsonScanner.h"

#include <iostream>
#include <string>

#include "JsonObject.h"

namespace ZSharp {
void JsonScanner::PopulateJsonObject(JsonObject& jsonObject, std::vector<Token>& tokens) {
  for (auto iter = tokens.begin(); iter != tokens.end(); ++iter) {
    if (iter->GetType() == TokenType::OPEN_CURLY_BRACE) {
      ++iter;

      auto tmpEndIter = tokens.end();
      ProcessJsonData(jsonObject, iter, tmpEndIter, false);
      return;
    }
  }
}

void JsonScanner::ProcessJsonData(JsonObject& jsonObject, std::vector<Token>::iterator& begin, std::vector<Token>::iterator& end, bool isArray) {
  bool keyDetected = false;

  for (; begin != end; begin++) {
    JsonObject::JsonValue tmpValue;

    switch (begin->GetType()) {
      case TokenType::OPEN_CURLY_BRACE:
      {
        tmpValue.valueType = JsonObject::JsonValueType::OBJECT_VALUE;

        begin++;

        auto tmpObject(std::make_shared<JsonObject>());
        ProcessJsonData(*tmpObject, begin, end, false);
        tmpValue.dataObject = tmpObject;

        if (isArray) {
          jsonObject.GetValue().dataArray.push_back(tmpValue);
        }
        else {
          jsonObject.SetValue(tmpValue);
        }
      }
      break;
      case TokenType::OPEN_SQUARE_BRACE:
        tmpValue.valueType = JsonObject::JsonValueType::ARRAY_VALUE;
        begin++;

        if (isArray) {
          JsonObject tmpObject;
          ProcessJsonData(tmpObject, begin, end, true);

          tmpValue.dataArray = tmpObject.GetValue().dataArray;
          jsonObject.GetValue().dataArray.push_back(tmpValue);
        }
        else {
          jsonObject.GetValue().valueType = tmpValue.valueType;
          ProcessJsonData(jsonObject, begin, end, true);
        }
        break;
      case TokenType::CLOSE_SQUARE_BRACE:
      case TokenType::CLOSE_CURLY_BRACE:
        return;
      case TokenType::STRING:
        if (!isArray && !keyDetected) {
          keyDetected = true;
          jsonObject.SetKey(begin->GetData());
        }
        else {
          tmpValue.valueType = JsonObject::JsonValueType::STRING_VALUE;
          tmpValue.dataString = begin->GetData();

          if (isArray) {
            jsonObject.GetValue().dataArray.push_back(tmpValue);
          }
          else {
            jsonObject.SetValue(tmpValue);
          }
        }
        break;
      case TokenType::NUMBER_INT:
        tmpValue.valueType = JsonObject::JsonValueType::NUMBER_INT_VALUE;
        tmpValue.dataInt = TokenToInt(*begin);

        if (isArray) {
          jsonObject.GetValue().dataArray.push_back(tmpValue);
        }
        else {
          jsonObject.SetValue(tmpValue);
        }

        break;
      case TokenType::NUMBER_FLOAT:
        tmpValue.valueType = JsonObject::JsonValueType::NUMBER_FLOAT_VALUE;
        tmpValue.dataFloat = TokenToFloat(*begin);

        if (isArray) {
          jsonObject.GetValue().dataArray.push_back(tmpValue);
        }
        else {
          jsonObject.SetValue(tmpValue);
        }

        break;
      case TokenType::BOOL_VALUE:
        tmpValue.valueType = JsonObject::JsonValueType::BOOL_VALUE;
        tmpValue.dataBool = TokenToBool(*begin);

        if (isArray) {
          jsonObject.GetValue().dataArray.push_back(tmpValue);
        }
        else {
          jsonObject.SetValue(tmpValue);
        }

        break;
    }
  }
}

void JsonScanner::ScanToken(std::vector<Token>& tokens, std::vector<char>& fileBuffer) {
  char c = Advance(fileBuffer);
  switch (c) {
    case '{':
    {
      Token token(TokenType::OPEN_CURLY_BRACE);
      AddToken(token, tokens);
      break;
    }
    case '}':
    {
      Token token(TokenType::CLOSE_CURLY_BRACE);
      AddToken(token, tokens);
      break;
    }
    case '[':
    {
      Token token(TokenType::OPEN_SQUARE_BRACE);
      AddToken(token, tokens);
      break;
    }
    case ']':
    {
      Token token(TokenType::CLOSE_SQUARE_BRACE);
      AddToken(token, tokens);
      break;
    }
    case ',':
    {
      Token token(TokenType::COMMA);
      AddToken(token, tokens);
      break;
    }
    case ':':
    {
      Token token(TokenType::COLON);
      AddToken(token, tokens);
      break;
    }
    case '"':
      ScanString(tokens, fileBuffer);
      break;
    case '\n':
      mLine++;
      break;
    default:
      if ((c == '-' && IsDigit(Peek(fileBuffer))) || IsDigit(c)) {
        ScanNumber(tokens, fileBuffer);
      }
      else {
        std::cout << "Unknown token at position: " << mCurrent << ", Line: " << mLine << std::endl;
      }
  }
}

void JsonScanner::ScanString(std::vector<Token>& tokens, std::vector<char>& fileBuffer) {
  while (Peek(fileBuffer) != '"' && !IsAtEnd(fileBuffer)) {
    if (Peek(fileBuffer) == '\n') {
      mLine++;
    }

    Advance(fileBuffer);
  }

  if (IsAtEnd(fileBuffer)) {
    std::cout << "Non-terminated String detected at position: " << mCurrent << ", Line: " << mLine << std::endl;
    return;
  }

  Advance(fileBuffer);

  std::string scannedString(fileBuffer.data() + mStart + 1, fileBuffer.data() + mCurrent - 1);
  Token token(TokenType::STRING, scannedString);
  AddToken(token, tokens);
}

void JsonScanner::ScanNumber(std::vector<Token>& tokens, std::vector<char>& fileBuffer) {
  bool realNumber = false;

  while (IsDigit(Peek(fileBuffer))) {
    Advance(fileBuffer);
  }

  if (Peek(fileBuffer) == '.' && IsDigit(PeekNext(fileBuffer))) {
    realNumber = true;
    Advance(fileBuffer);

    while (IsDigit(Peek(fileBuffer))) {
      Advance(fileBuffer);
    }
  }

  TokenType tokenType;
  if (realNumber) {
    tokenType = TokenType::NUMBER_FLOAT;
  }
  else {
    tokenType = TokenType::NUMBER_INT;
  }

  std::string scannedNumber(fileBuffer.data() + mStart, fileBuffer.data() + mCurrent);

  Token token(tokenType, scannedNumber);
  AddToken(token, tokens);
}

bool JsonScanner::IsDigit(char letter) {
  return letter >= '0' && letter <= '9';
}

std::int64_t JsonScanner::TokenToInt(Token token) {
  return static_cast<std::int64_t>(std::atoll(token.GetData().c_str()));
}

double JsonScanner::TokenToFloat(Token token) {
  return std::atof(token.GetData().c_str());;
}

bool JsonScanner::TokenToBool(Token token) {
  const std::string& tokenString = token.GetData();

  if (tokenString.compare("true")) {
    return true;
  }
  else if (tokenString.compare("false")) {
    return false;
  }
  else {
    std::cout << "Unknown bool value." << std::endl;
  }

  return false;
}

}
