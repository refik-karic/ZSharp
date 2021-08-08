#pragma once

#include <cstdint>

#include <string>

namespace ZSharp {

enum class TokenType {
  OPEN_CURLY_BRACE,
  CLOSE_CURLY_BRACE,
  OPEN_SQUARE_BRACE,
  CLOSE_SQUARE_BRACE,
  COMMA,
  COLON,
  STRING,
  NUMBER_FLOAT,
  NUMBER_INT,
  BOOL_VALUE,
  NULL_VALUE
};

class Token final {
  public:
  Token(TokenType token);

  Token(TokenType token, std::string& data);

  Token(const Token& rhs);

  void operator=(const Token& rhs) {
    if (this == &rhs) {
      return;
    }

    mToken = rhs.mToken;
    mData = rhs.mData;
  }

  void SetType(TokenType type);

  void SetData(const std::string& data);

  TokenType GetType();

  const std::string& GetData();

  private:
  TokenType mToken;
  std::string mData;
};

}
