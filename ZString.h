#pragma once

#include "ZBaseTypes.h"
#include "ISerializable.h"
#include "Span.h"

namespace ZSharp {

template<typename T>
class Array;

class WideString;

class String final : public ISerializable {
  private:
  class VariableArg {
    private:
    enum Type {
      CHAR,
      SIZE_T,
      BOOL,
      INT32,
      UINT32,
      INT64,
      UINT64,
      FLOAT,
      DOUBLE,
      CONST_STRING,
      STRING_CLASS,
      SPAN_CLASS
    };

    Type mType;

    union {
      char char_value;
      size_t size_value;
      bool bool_value;
      int32 int32_value;
      uint32 uint32_value;
      int64 int64_value;
      uint64 uint64_value;
      float float_value;
      double double_value;
      const char* string_value;
      const String* string_class_value;
      const Span<const char>* span_class_value;
    } mData;

    public:
    VariableArg() = delete; // Explicit type construction only.

    VariableArg(const char arg);

    VariableArg(const size_t arg);

    VariableArg(const bool arg);

    VariableArg(const int32 arg);

    VariableArg(const int64 arg);

    VariableArg(const float arg);

    VariableArg(const char* arg);

    VariableArg(const String& arg);

    VariableArg(const Span<const char>& arg);

    void ToString(String& str, int32 numDigits = 0) const;
  };

  public:

  class Iterator {
  public:
    Iterator(const char* data) : mPtr(data) {}

    Iterator& operator++() {
      mPtr++;
      return *this;
    }

    Iterator operator++(int) {
      Iterator temp(*this);
      ++(*this);
      return temp;
    }

    Iterator& operator--() {
      mPtr--;
      return *this;
    }

    Iterator operator--(int) {
      Iterator temp(mPtr);
      --(*this);
      return temp;
    }

    bool operator==(const Iterator& rhs) {
      return mPtr == rhs.mPtr;
    }

    bool operator!=(const Iterator& rhs) {
      return mPtr != rhs.mPtr;
    }

    const char& operator*() const {
      return *mPtr;
    }

    const char* operator->() {
      return mPtr;
    }

    private:
    const char* mPtr;
  };

  String();

  String(const char* str);

  String(const char* str, size_t offset, size_t length);

  String(const String& rhs);

  ~String();

  const char* Str() const;

  String* operator=(const String& rhs);

  bool operator==(const String& rhs) const;

  bool operator!=(const String& rhs) const;

  bool operator==(const char* rhs) const;

  bool operator>(const String& rhs) const;

  bool operator<(const String& rhs) const;

  bool operator<=(const String& rhs) const;

  bool operator>=(const String& rhs) const;

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

  template<typename... Args>
  static String FromFormat(const char* formatStr, const Args&... args) {
    String str;
    str.Appendf(formatStr, args...);
    return str;
  }

  bool IsEmpty() const;

  void Clear();

  void Trim(char value);

  void Trim(const Array<char>& values);

  void Reverse();

  size_t Length() const;

  String SubStr(size_t start, size_t end) const;

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

  void ToLower();

  void ToUpper();

  bool CompareIgnoreCase(const String& rhs) const;

  bool CompareIgnoreCase(const char* rhs) const;

  virtual void Serialize(ISerializer& serializer) override;

  virtual void Deserialize(IDeserializer& deserializer) override;

  Iterator begin() const {
    return Iterator(GetString());
  }

  Iterator end() const {
    return Iterator(GetString() + Length());
  }

  Iterator rbegin() const {
    return Iterator(GetString() + Length() - 1);
  }

  Iterator rend() const {
    return Iterator(GetString() - 1);
  }

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

  bool IsShort(const char* str, size_t& length) const;

  bool IsMarkedShort() const;

  void MarkShort(bool isShort);

  // Short String helpers.
  void SetShortLength(size_t length);

  uint16 GetShortLength() const;

  void CopyShort(const char* str, size_t length);

  void AppendShort(const char* str, size_t offset, size_t length);

  // Long String helpers.
  void SetLongLength(size_t length);

  size_t GetLongLength() const;

  void AllocateLong();

  void FreeLong();

  void CopyLong(const char* str, size_t length);

  void AppendLong(const char* str, size_t offset, size_t length);

  // General helpers.
  void Copy(const char* str);

  const char* GetString() const;

  char* GetMutableString();

  size_t GetCombinedSize(size_t length);

  bool FitsInSmall(size_t size);

  void VariadicArgsAppend(const char* format, const VariableArg* args, size_t numArgs);
};

class WideString final : public ISerializable {
  private:
  class VariableArg {
    private:
    enum Type {
      WCHAR,
      INT32,
      UINT32,
      INT64,
      UINT64,
      FLOAT,
      DOUBLE
    };

    Type mType;

    union {
      wchar_t wchar_value;
      int32 int32_value;
      uint32 uint32_value;
      int64 int64_value;
      uint64 uint64_value;
      float float_value;
      double double_value;
    } mData;

    public:
    VariableArg() = delete; // Explicit type construction only.

    VariableArg(const wchar_t arg);

    VariableArg(const int32 arg);

    VariableArg(const uint32 arg);

    VariableArg(const int64 arg);

    VariableArg(const uint64 arg);

    VariableArg(const float arg);

    VariableArg(const double arg);

    void ToString(WideString& str, int32 numDigits = 0) const;
  };

  public:

  class Iterator {
    public:
    Iterator(const wchar_t* data) : mPtr(data) {}

    Iterator& operator++() {
      mPtr++;
      return *this;
    }

    Iterator operator++(int) {
      Iterator temp(*this);
      ++(*this);
      return temp;
    }

    Iterator& operator--() {
      mPtr--;
      return *this;
    }

    Iterator operator--(int) {
      Iterator temp(mPtr);
      --(*this);
      return temp;
    }

    bool operator==(const Iterator& rhs) {
      return mPtr == rhs.mPtr;
    }

    bool operator!=(const Iterator& rhs) {
      return mPtr != rhs.mPtr;
    }

    const wchar_t& operator*() const {
      return *mPtr;
    }

    const wchar_t* operator->() {
      return mPtr;
    }

    private:
    const wchar_t* mPtr;
  };

  WideString();

  WideString(const wchar_t* str);

  WideString(const wchar_t* str, size_t offset, size_t size);

  WideString(const WideString& rhs);

  ~WideString();

  const wchar_t* Str() const;

  WideString* operator=(const WideString& rhs);

  bool operator==(const WideString& rhs) const;

  bool operator==(const wchar_t* rhs) const;

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

  template<typename... Args>
  static WideString FromFormat(const wchar_t* formatStr, const Args&... args) {
    WideString str;
    str.Appendf(formatStr, args...);
    return str;
  }

  bool IsEmpty() const;

  void Clear();

  void Trim(wchar_t value);

  void Trim(const Array<wchar_t>& values);

  void Reverse();

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

  void ToLower();

  void ToUpper();

  bool CompareIgnoreCase(const WideString& rhs) const;

  bool CompareIgnoreCase(const wchar_t* rhs) const;

  virtual void Serialize(ISerializer& serializer) override;

  virtual void Deserialize(IDeserializer& deserializer) override;

  Iterator begin() const {
    return Iterator(GetString());
  }

  Iterator end() const {
    return Iterator(GetString() + Length());
  }

  Iterator rbegin() const {
    return Iterator(GetString() + Length() - 1);
  }

  Iterator rend() const {
    return Iterator(GetString() - 1);
  }

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

  bool IsShort(const wchar_t* str, size_t& length) const;

  bool IsMarkedShort() const;

  void MarkShort(bool isShort);

  // Short String helpers.
  void SetShortLength(size_t length);

  wchar_t GetShortLength() const;

  void CopyShort(const wchar_t* str, size_t length);

  void AppendShort(const wchar_t* str, size_t offset, size_t length);

  // Long String helpers.
  void SetLongLength(size_t length);

  size_t GetLongLength() const;

  void AllocateLong();

  void FreeLong();

  void CopyLong(const wchar_t* str, size_t length);

  void AppendLong(const wchar_t* str, size_t offset, size_t length);

  // General helpers.
  void Copy(const wchar_t* str);

  const wchar_t* GetString() const;

  wchar_t* GetMutableString();

  size_t GetCombinedSize(size_t length);

  bool FitsInSmall(size_t size);

  void VariadicArgsAppend(const wchar_t* format, const VariableArg* args, size_t numArgs);
};

}
