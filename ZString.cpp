#define _CRT_SECURE_NO_WARNINGS 1

#include "ZString.h"

#include "ZAssert.h"
#include "PlatformMemory.h"

#include <cfloat>
#include <cstdarg>
#include <cstdlib>
#include <cstring>

namespace ZSharp {
String::String() {
  Copy("");
}

String::String(const char* str) {
  Copy(str);
}

String::String(const char* str, size_t offset, size_t end) {
  Copy("");
  Append(str, offset, end);
}

String::String(const String& rhs) {
  Copy(rhs.Str());
}

String::~String() {
  if (!IsMarkedShort()) {
    FreeLong();
  }
}

const char* String::Str() const {
  return GetString();
}

String* String::operator=(const String& rhs) {
  if (this != &rhs) {
    if (!IsMarkedShort()) {
      FreeLong();
    }

    Copy(rhs.Str());
  }

  return this;
}

bool String::operator==(const String& rhs) const {
  return strcmp(Str(), rhs.Str()) == 0;
}

String String::operator+(const char* str) {
  String result(*this);
  result.Append(str);
  return result;
}

const char& String::operator[](const size_t index) {
  ZAssert(index < Length());
  return GetString()[index];
}

void String::Append(const String& str) {
  Append(str.Str());
}

void String::Append(const char* str, size_t offset, size_t size) {
  size_t totalSize = Length() + size + 1;
  if (FitsInSmall(totalSize)) {
    AppendShort(str, offset, size);
  }
  else {
    AppendLong(str, offset, size);
  }
}

void String::Append(const char* str) {
  size_t length = strlen(str);
  if (FitsInSmall(GetCombinedSize(str) + 1)) {
    AppendShort(str, 0, length);
  }
  else {
    AppendLong(str, 0, length);
  }
}

void String::Appendf(const char* formatStr, ...) {
  ZAssert(formatStr != nullptr);

  va_list arglist;
  va_start(arglist, formatStr);

  const char* lastPosition = formatStr;
  for (const char* str = formatStr; *str != '\0'; ++str) {
    char currentChar = *str;
    if (currentChar == '%') {
      char nextChar = *(str + 1);
      size_t jumpAhead = 0;
      switch (nextChar) {
        case '\0':
          continue;
        case 'd':
        {
          const size_t bufferSize = 64;
          char buffer[bufferSize];
          buffer[0] = '\0';
          const int32 vaValue = va_arg(arglist, int32);
          Append(lastPosition, 0, str - lastPosition);
          Append(_itoa(vaValue, buffer, 10));
          jumpAhead = 2;
        }
          break;
        case 'f':
        {
          const size_t bufferSize = 64;
          char buffer[bufferSize];
          buffer[0] = '\0';
          const int32 digits = FLT_DECIMAL_DIG;
          const float vaValue = static_cast<float>(va_arg(arglist, double));
          Append(lastPosition, 0, str - lastPosition);
          Append(_gcvt(vaValue, digits, buffer));
          jumpAhead = 2;
        }
          break;
        default:
          break;
      }

      if (*(str + jumpAhead) != '\0') {
        lastPosition = str + jumpAhead;
      }
    }
  }

  va_end(arglist);
}

bool String::IsEmpty() const {
  return Length() == 0;
}

void String::Clear() {
  if (!IsMarkedShort()) {
    FreeLong();
  }

  Copy("");
}

void String::Trim(char value) {
  const char* lastLocation = FindLast(value);
  while ((lastLocation != nullptr) && (*lastLocation == value) && (lastLocation - GetString() > 0)) {
    --lastLocation;
  }

  if (lastLocation != nullptr) {
    size_t length = (lastLocation + 1) - GetString();
    String temp(GetString(), 0, length);
    *this = temp;
  }
}

void String::Trim(const Array<char>& values) {
  for (char& value : values) {
    Trim(value);
  }
}

size_t String::Length() const {
  return IsMarkedShort() ? GetShortLength() : GetLongLength();
}

String String::SubStr(size_t start, size_t end) {
  String temp(*this);
  char* subStr = temp.GetMutableString() + start;
  if (end > Length()) {
    end = Length();
  }
  temp.GetMutableString()[end] = '\0';

  String result(subStr);
  return result;
}

const char* String::FindFirst(const char value) const {
  return strchr(GetString(), value);
}

const char* String::FindLast(const char value) const {
  return strrchr(GetString(), value);
}

uint8 String::ToUint8() const {
  return static_cast<uint8>(strtoul(GetString(), NULL, 10));
}

uint16 String::ToUint16() const {
  return static_cast<uint16>(strtoul(GetString(), NULL, 10));
}

uint32 String::ToUint32() const {
  return static_cast<uint32>(strtoul(GetString(), NULL, 10));
}

uint64 String::ToUint64() const {
  return static_cast<uint64>(strtoull(GetString(), NULL, 10));
}

int8 String::ToInt8() const {
  return static_cast<int8>(strtol(GetString(), NULL, 10));
}

int16 String::ToInt16() const {
  return static_cast<int16>(strtol(GetString(), NULL, 10));
}

int32 String::ToInt32() const {
  return static_cast<int32>(strtol(GetString(), NULL, 10));
}

int64 String::ToInt64() const {
  return static_cast<int64>(strtoll(GetString(), NULL, 10));
}

float String::ToFloat() const {
  return static_cast<float>(strtof(GetString(), NULL));
}

bool String::IsShort(const char* str) const {
  return (strlen(str) + 1) < MinCapaity;
}

bool String::IsMarkedShort() const {
  return (mOverlapData.shortStr.size & 0x80) > 0;
}

void String::MarkShort(bool isShort) {
  mOverlapData.shortStr.size = (isShort) ? ((mOverlapData.shortStr.size & 0x7F) | 0x80) : (mOverlapData.shortStr.size & 0x7F);
}

void String::SetShortLength(size_t length) {
  mOverlapData.shortStr.size = (static_cast<unsigned char>(length) & 0x7F);
}

unsigned char String::GetShortLength() const {
  return mOverlapData.shortStr.size & 0x7F;
}

void String::CopyShort(const char* str) {
  size_t length = strlen(str);
  SetShortLength(length);
  if (length > 0) {
    memcpy(mOverlapData.shortStr.data, str, GetShortLength());
    mOverlapData.shortStr.data[GetShortLength()] = NULL;
  }
  else {
    mOverlapData.shortStr.data[0] = NULL;
  }
  MarkShort(true);
}

void String::AppendShort(const char* str, size_t offset, size_t length) {
  unsigned char shortLength = static_cast<unsigned char>(Length());
  const char* offsetString = str + offset;
  memcpy(mOverlapData.shortStr.data + shortLength, offsetString, length);
  mOverlapData.shortStr.data[shortLength + length] = NULL;
  SetShortLength(strlen(mOverlapData.shortStr.data));
  MarkShort(true);
}

void String::SetLongLength(size_t length) {
  mOverlapData.longStr.size = (length & 0x7FFFFFFFFFFFFFFF);
  mOverlapData.longStr.capacity = length;
}

size_t String::GetLongLength() const {
  return mOverlapData.longStr.size & 0x7FFFFFFFFFFFFFFF;
}

void String::AllocateLong() {
  size_t length = GetLongLength() + 1;
  mOverlapData.longStr.data = static_cast<char*>(PlatformMalloc(length));
}

void String::FreeLong() {
  if (mOverlapData.longStr.data != nullptr) {
    PlatformFree(mOverlapData.longStr.data);
    mOverlapData.longStr.data = nullptr;
  }
}

void String::CopyLong(const char* str) {
  SetLongLength(strlen(str));
  AllocateLong();
  memcpy(mOverlapData.longStr.data, str, GetLongLength());
  mOverlapData.longStr.data[GetLongLength()] = NULL;
  MarkShort(false);
}

void String::AppendLong(const char* str, size_t offset, size_t length) {
  size_t combinedLength = Length() + length;

  if (IsMarkedShort()) {
    char oldData[MinCapaity];
    size_t shortLength = GetShortLength();
    memcpy(oldData, GetString(), shortLength);
    SetLongLength(shortLength);
    AllocateLong();
    MarkShort(false);

    char* mutableStr = GetMutableString();
    memcpy(mutableStr, oldData, shortLength);
    mutableStr[shortLength] = NULL;
  }

  SetLongLength(combinedLength);
  char* resizedStr = static_cast<char*>(PlatformReAlloc(mOverlapData.longStr.data, combinedLength + 1));
  ZAssert(resizedStr != nullptr);
  mOverlapData.longStr.data = resizedStr;

  size_t currentPosition = strlen(GetMutableString());

  char* mutableStr = GetMutableString() + currentPosition;
  const char* strOffset = str + offset;
  memcpy(mutableStr, strOffset, length);
  mutableStr[length] = NULL;
  MarkShort(false);
}

void String::Copy(const char* str) {
  if (IsShort(str)) {
    CopyShort(str);
  }
  else {
    CopyLong(str);
  }
}

const char* String::GetString() const {
  return (IsMarkedShort()) ? mOverlapData.shortStr.data : mOverlapData.longStr.data;
}

char* String::GetMutableString() {
  return (IsMarkedShort()) ? mOverlapData.shortStr.data : mOverlapData.longStr.data;
}

size_t String::GetCombinedSize(const char* str) {
  return strlen(str) + Length();
}

bool String::FitsInSmall(size_t size) {
  return size < MinCapaity;
}
}
