#pragma once

#include "ZBaseTypes.h"
#include "Array.h"

namespace ZSharp {

class WideString;

class String final {
  private:
  class VariableArg {
    private:
    enum Type {
      SIZE_T,
      BOOL,
      INT32,
      UINT32,
      INT64,
      UINT64,
      FLOAT,
      DOUBLE,
      CONST_STRING
    };

    Type mType;

    union {
      size_t size_value;
      bool bool_value;
      int32 int32_value;
      uint32 uint32_value;
      int64 int64_value;
      uint64 uint64_value;
      float float_value;
      double double_value;
      const char* string_value;
    } mData;

    public:
    VariableArg() = delete; // Explicit type construction only.

    VariableArg(const size_t arg);

    VariableArg(const bool arg);

    VariableArg(const int32 arg);

    VariableArg(const int64 arg);

    VariableArg(const float arg);

    VariableArg(const char* arg);

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

  bool operator>(const String& rhs) const;

  bool operator<(const String& rhs) const;

  String operator+(const char* str);

  const char& operator[](const size_t index);

  void Append(const String& str);

  void Append(const char* str, size_t offset, size_t size);

  void Append(const char* str);

#if 0
  void Appendf(const char* formatStr, ...);
#endif

  /*
  Follows similar format to C#'s String::Format.
  i.e. "My format with {0}, {2}, and {1} args."
  Args do not have to be in consecutive order.
  Type is deduced during compile time, no type specifier is required.
  */
  template<typename... Args>
  void Appendf(const char* formatStr, const Args&... args) {
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

  WideString ToWide() const;

  private:
  struct LongString {
    size_t size;
    size_t capacity;
    char* data;
  };

  enum { MinCapacity = (sizeof(LongString) - sizeof(uint16)) /  sizeof(uint16) > 2 ? (sizeof(LongString) - sizeof(uint16)) : 2};

  struct ShortString {
    uint16 size;
    char data[MinCapacity];
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

  uint16 GetShortLength() const;

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

class WideString final {
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

    WideString ToString() const;
  };

public:
  WideString();

  WideString(const wchar_t* str);

  WideString(const wchar_t* str, size_t offset, size_t size);

  WideString(const WideString& rhs);

  ~WideString();

  const wchar_t* Str() const;

  WideString* operator=(const WideString& rhs);

  bool operator==(const WideString& rhs) const;

  bool operator>(const WideString& rhs) const;

  bool operator<(const WideString& rhs) const;

  WideString operator+(const wchar_t* str);

  const wchar_t& operator[](const size_t index);

  void Append(const WideString& str);

  void Append(const wchar_t* str, size_t offset, size_t size);

  void Append(const wchar_t* str);

#if 0
  void Appendf(const wchar_t* formatStr, ...);
#endif

  /*
  Follows similar format to C#'s String::Format.
  i.e. "My format with {0}, {2}, and {1} args."
  Args do not have to be in consecutive order.
  Type is deduced during compile time, no type specifier is required.
  */
  template<typename... Args>
  void Appendf(const wchar_t* formatStr, const Args&... args) {
    VariableArg inArgs[] = { args... };
    VariadicArgsAppend(formatStr, inArgs, sizeof...(args));
  }

  bool IsEmpty() const;

  void Clear();

  void Trim(wchar_t value);

  void Trim(const Array<wchar_t>& values);

  size_t Length() const;

  WideString SubStr(size_t start, size_t end);

  const wchar_t* FindFirst(wchar_t value) const;

  const wchar_t* FindLast(wchar_t value) const;

  const wchar_t* FindString(const wchar_t* pattern) const;

  uint8 ToUint8() const;

  uint16 ToUint16() const;

  uint32 ToUint32() const;

  uint64 ToUint64() const;

  int8 ToInt8() const;

  int16 ToInt16() const;

  int32 ToInt32() const;

  int64 ToInt64() const;

  float ToFloat() const;

  String ToNarrow() const;

private:
  struct LongString {
    size_t size;
    size_t capacity;
    wchar_t* data;
  };

  enum { MinCapacity = (sizeof(LongString) - 1) / sizeof(wchar_t) > 2 ? ((sizeof(LongString) - 1) / sizeof(wchar_t)) : 2 };

  struct ShortString {
    union {
      wchar_t size;
      wchar_t value;
    };
    wchar_t data[MinCapacity];
  };

  union Overlap {
    LongString longStr;
    ShortString shortStr;
  } mOverlapData;

  bool IsShort(const wchar_t* str) const;

  bool IsMarkedShort() const;

  void MarkShort(bool isShort);

  // Short String helpers.
  void SetShortLength(size_t length);

  wchar_t GetShortLength() const;

  void CopyShort(const wchar_t* str);

  void AppendShort(const wchar_t* str, size_t offset, size_t length);

  // Long String helpers.
  void SetLongLength(size_t length);

  size_t GetLongLength() const;

  void AllocateLong();

  void FreeLong();

  void CopyLong(const wchar_t* str);

  void AppendLong(const wchar_t* str, size_t offset, size_t length);

  // General helpers.
  void Copy(const wchar_t* str);

  const wchar_t* GetString() const;

  wchar_t* GetMutableString();

  size_t GetCombinedSize(const wchar_t* str);

  bool FitsInSmall(size_t size);

  void VariadicArgsAppend(const wchar_t* format, const VariableArg* args, size_t numArgs);
};

}
