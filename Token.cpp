#include "Token.h"

namespace ZSharp {
Token::Token(TokenType token) :
  mToken(token),
  mData("") {
}

Token::Token(TokenType token, std::string& data)
: mToken(token),
  mData(data) {

}

Token::Token(const Token& rhs) {
  if (this != &rhs) {
    *this = rhs;
  }
}

void Token::SetType(TokenType type) {
  mToken = type;
}

void Token::SetData(const std::string& data) {
  mData = data;
}

TokenType Token::GetType() {
  return mToken;
}

const std::string& Token::GetData() {
  return mData;
}

}
