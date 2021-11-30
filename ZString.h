#pragma once

#include <cstdlib>
#include <cstring>

namespace ZSharp {
class String final {
  public:
  String(const char* str) {
    if (IsShort(str)) {
      CopyShort(str);
    }
    else {
      CopyLong(str);
    }
  }

  ~String() {
    if (!IsMarkedShort()) {
      FreeLong();
    }
  }

  const char* Str() {
    return GetString();
  }

  private:
  struct LongString {
    size_t capacity;
    size_t size;
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

  bool IsShort(const char* str) const {
    return strlen(str) < MinCapaity;
  }

  bool IsMarkedShort() const {
    return (mOverlapData.shortStr.size & 0x80) > 0;
  }

  void MarkShort(bool isShort) {
    mOverlapData.shortStr.size = (isShort) ? ((mOverlapData.shortStr.size & 0x7F) | 0x80) : (mOverlapData.shortStr.size & 0x7F);
  }

  // Short String helpers.
  void SetShortLength(const char* str) {
    mOverlapData.shortStr.size = static_cast<unsigned char>(strlen(str) + 1);
  }

  unsigned char GetShortLength() const {
    return mOverlapData.shortStr.size;
  }

  void CopyShort(const char* str) {
    SetShortLength(str);
    strncpy_s(mOverlapData.shortStr.data, GetShortLength(), str, GetShortLength());
    MarkShort(true);
  }

  // Long String helpers.
  void SetLongLength(const char* str) {
    size_t length = strlen(str) + 1;
    mOverlapData.longStr.size = length;
    mOverlapData.longStr.capacity = length;
  }

  size_t GetLongLength() const {
    return mOverlapData.longStr.size;
  }

  void AllocateLong() {
    mOverlapData.longStr.data = static_cast<char*>(malloc(GetLongLength()));
  }

  void FreeLong() {
    free(mOverlapData.longStr.data);
  }

  void CopyLong(const char* str) {
    SetLongLength(str);
    AllocateLong();
    strncpy_s(mOverlapData.longStr.data, GetLongLength(), str, GetLongLength());
    MarkShort(false);
  }

  // General helpers.
  const char* GetString() const {
    return (IsMarkedShort()) ? mOverlapData.shortStr.data : mOverlapData.longStr.data;
  }
};
}
