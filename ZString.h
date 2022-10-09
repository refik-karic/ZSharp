#pragma once

#include "ZBaseTypes.h"
#include "Array.h"

namespace ZSharp {
class String final {
  private:
  class VariableArg {
    private:
    enum Type {
      INT32,
      UINT32,
      INT64,
      UINT64,
      FLOAT,
      DOUBLE
    };

    Type mType;

    union {
      int32 int32_value;
      uint32 uint32_value;
      int64 int64_value;
      uint64 uint64_value;
      float float_value;
      double double_value;
    } mData;

    public:
    VariableArg() = delete; // Explicit type construction only.

    VariableArg(const int32 arg);

    VariableArg(const float arg);

    String ToString() const;
  };

  public:
  String();

  String(const char* str);

  String(const char* str, size_t offset, size_t size);

  String(const String& rhs);

  ~String();

  const char* Str() const;

  String* operator=(const String& rhs);

  bool operator==(const String& rhs) const;

  String operator+(const char* str);

  const char& operator[](const size_t index);

  void Append(const String& str);

  void Append(const char* str, size_t offset, size_t size);

  void Append(const char* str);

#if 0
  void Appendf(const char* formatStr, ...);
#endif

  template<typename... Args>
  void VariadicAppend(const char* formatStr, const Args&... args) {
    VariableArg inArgs[] = {args...};
    VariadicArgsAppend(formatStr, inArgs, sizeof...(args));
  }

  bool IsEmpty() const;

  void Clear();

  void Trim(char value);

  void Trim(const Array<char>& values);

  size_t Length() const;

  String SubStr(size_t start, size_t end);

  const char* FindFirst(char value) const;

  const char* FindLast(char value) const;

  const char* FindString(const char* pattern) const;

  uint8 ToUint8() const;
  
  uint16 ToUint16() const;

  uint32 ToUint32() const;

  uint64 ToUint64() const;

  int8 ToInt8() const;

  int16 ToInt16() const;

  int32 ToInt32() const;

  int64 ToInt64() const;

  float ToFloat() const;

  private:
  struct LongString {
    size_t size;
    size_t capacity;
    char* data;
  };

  enum { MinCapaity = (sizeof(LongString) - 1) /  sizeof(char) > 2 ? ((sizeof(LongString) - 1) / sizeof(char)) : 2};

  struct ShortString {
    union {
      unsigned char size;
      char value;
    };
    char data[MinCapaity];
  };

  union Overlap {
    LongString longStr;
    ShortString shortStr;
  } mOverlapData;

  bool IsShort(const char* str) const;

  bool IsMarkedShort() const;

  void MarkShort(bool isShort);

  // Short String helpers.
  void SetShortLength(size_t length);

  unsigned char GetShortLength() const;

  void CopyShort(const char* str);

  void AppendShort(const char* str, size_t offset, size_t length);

  // Long String helpers.
  void SetLongLength(size_t length);

  size_t GetLongLength() const;

  void AllocateLong();

  void FreeLong();

  void CopyLong(const char* str);

  void AppendLong(const char* str, size_t offset, size_t length);

  // General helpers.
  void Copy(const char* str);

  const char* GetString() const;

  char* GetMutableString();

  size_t GetCombinedSize(const char* str);

  bool FitsInSmall(size_t size);

  void VariadicArgsAppend(const char* format, const VariableArg* args, size_t numArgs);
};
}
