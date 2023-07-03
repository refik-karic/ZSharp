#define _CRT_SECURE_NO_WARNINGS 1

#include "ZString.h"
#include "Array.h"

#include "ZAssert.h"
#include "PlatformMemory.h"

#include <cfloat>
#include <cstdarg>
#include <cstdlib>
#include <cstring>
#include <cwchar>

namespace ZSharp {

static const uint16 ShortMaskSet = 0x8000U;
static const uint16 ShortMaskClear = 0x7FFFU;
static const size_t LongMaskClear = 0x7FFFFFFFFFFFFFFFULL;

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

bool String::operator>(const String& rhs) const {
  return strcmp(Str(), rhs.Str()) > 0;
}

bool String::operator<(const String& rhs) const {
  return strcmp(Str(), rhs.Str()) < 0;
}

bool String::operator<=(const String& rhs) const {
  return ((*this) < rhs) || ((*this) == rhs);
}

bool String::operator>=(const String& rhs) const {
  return ((*this) > rhs) || ((*this) == rhs);
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

#if 0 // Leaving here as an example of how C-style va_args work.
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
#endif

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

const char* String::FindString(const char* pattern) const {
  size_t inLength = strlen(pattern);
  const char* str = Str();
  const size_t length = Length();
  if (inLength > length) {
    return nullptr;
  }

  // Boyer-Moore string search.
  // Only implements the "bad character rule", does not do "good suffix".
  
  // Create a LUT big enough for the alphabet and size each letter row to the pattern length.
  Array<Array<size_t>> lut(256);
  for (size_t i = 0; i < lut.Size(); ++i) {
    lut[i].Resize(inLength);
  }

  // Save each unique character index for each index of the pattern starting from the end.
  // Any characters that don't exist will be 0.
  for (size_t i = inLength; i > 0; --i) {
    for (size_t j = i - 1; j > 0; --j) {
      size_t currentChar = static_cast<size_t>(pattern[j - 1]);
      Array<size_t>& charRow = lut[currentChar];
      // Only save the last index.
      if (charRow[i - 1] == 0) {
        charRow[i - 1] = j;
      }
    }
  }

  const char* match = nullptr;
  for (size_t position = inLength, skipAhead = inLength; (position <= length) && (match == nullptr); position += skipAhead) {
    bool matched = true;
    for (size_t i = 0; (i < inLength) && matched; ++i) {
      // Check each character of the pattern starting from the end.
      // If any character doesn't match, skip ahead and try again.
      const size_t inPosition = inLength - i - 1;
      const char patternChar = pattern[inPosition];
      const char matchChar = str[position - i - 1];
      if (patternChar != matchChar) {
        // Check the LUT and skip ahead by the known amount.
        skipAhead = (inLength - i) - (lut[static_cast<size_t>(matchChar)][inPosition]);
        matched = false;
      }
    }

    if (matched) {
      match = str + position - inLength;
    }
  }

  return match;
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

WideString String::ToWide() const {
  if (IsEmpty()) {
    WideString result;
    return result;
  }
  
  const size_t maxLength = Length() + 1;
  Array<wchar_t> dest(maxLength);
  mbstowcs(dest.GetData(), Str(), maxLength);
  WideString result(dest.GetData());
  return result;
}

void String::Serialize(ISerializer& serializer) {
  size_t length = Length();
  const char* str = Str();
  serializer.Serialize(&length, sizeof(length));
  serializer.Serialize(str, length);
}

void String::Deserialize(IDeserializer& deserializer) {
  size_t length = 0;
  deserializer.Deserialize(&length, sizeof(length));

  char* buffer = (char*)PlatformMalloc(length + 1);
  deserializer.Deserialize(buffer, length);
  buffer[length] = '\0';

  Append(buffer);

  PlatformFree(buffer);
}

bool String::IsShort(const char* str) const {
  return (strlen(str) + 1) < MinCapacity;
}

bool String::IsMarkedShort() const {
  return (mOverlapData.shortStr.size & ShortMaskSet) > 0;
}

void String::MarkShort(bool isShort) {
  mOverlapData.shortStr.size = (isShort) ? ((mOverlapData.shortStr.size & ShortMaskClear) | ShortMaskSet) : (mOverlapData.shortStr.size & ShortMaskClear);
}

void String::SetShortLength(size_t length) {
  mOverlapData.shortStr.size = (static_cast<uint16>(length) & ShortMaskClear);
}

uint16 String::GetShortLength() const {
  return mOverlapData.shortStr.size & ShortMaskClear;
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
  mOverlapData.longStr.size = (length & LongMaskClear);
  mOverlapData.longStr.capacity = length;
}

size_t String::GetLongLength() const {
  return mOverlapData.longStr.size & LongMaskClear;
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
    char oldData[MinCapacity];
    size_t shortLength = GetShortLength();
    memcpy(oldData, GetString(), shortLength);
    SetLongLength(shortLength);
    AllocateLong();
    MarkShort(false);

    char* mutableStr = GetMutableString();
    memcpy(mutableStr, oldData, shortLength);
    mutableStr[shortLength] = NULL;
  }

  char* resizedStr = static_cast<char*>(PlatformReAlloc(mOverlapData.longStr.data, combinedLength + 1));
  ZAssert(resizedStr != nullptr);
  mOverlapData.longStr.data = resizedStr;
  MarkShort(false);
  SetLongLength(combinedLength);

  size_t currentPosition = strlen(GetMutableString());

  char* mutableStr = GetMutableString() + currentPosition;
  const char* strOffset = str + offset;
  memcpy(mutableStr, strOffset, length);
  mutableStr[length] = NULL;
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
  return size < MinCapacity;
}

void String::VariadicArgsAppend(const char* format, const VariableArg* args, size_t numArgs) {
  ZAssert(format != nullptr);

  const char* str = format;
  const char* lastPosition = format;
  const char* lastChar = nullptr;
  for (; *str != '\0'; ++str) {
    char currentChar = *str;
    bool isEscaped = (lastChar != nullptr) ? ((*lastChar) == '\\') : false;

    if (!isEscaped && (currentChar == '{')) {
      size_t jumpAhead = 0;

      for (const char* endFormat = str + 1; *endFormat != '\0'; ++endFormat) {
        if (*endFormat == '}') {
          jumpAhead = (endFormat - str);
          break;
        }
      }

      if (jumpAhead == 0) {
        ZAssert(false); // Invalid format.
        break;
      }

      // Get the index specified by the argument.
      int32 argIndex = atoi(str + 1);
      ZAssert(argIndex >= 0);

      if (argIndex >= numArgs) {
        ZAssert(false); // OOB
        continue;
      }

      // Append format str up until the current position.
      Append(lastPosition, 0, str - lastPosition);

      // Append the VariableArg based on its type.
      const VariableArg& arg = args[argIndex];
      Append(arg.ToString().Str());

      str += jumpAhead;
      lastPosition = str + 1;
      lastChar = str + 1;
    }
    else {
      lastChar = str;
    }
  }

  // Append trailing characters that may be present after the last format specifier.
  Append(lastPosition, 0, str - lastPosition);
}

String::VariableArg::VariableArg(const size_t arg)
  : mType(Type::SIZE_T) {
  mData.size_value = arg;
}

String::VariableArg::VariableArg(const bool arg)
  : mType(Type::BOOL) {
  mData.bool_value = arg;
}

String::VariableArg::VariableArg(const int32 arg)
  : mType(Type::INT32) {
  mData.int32_value = arg;
}

String::VariableArg::VariableArg(const int64 arg) 
  : mType(Type::INT64) {
  mData.int64_value = arg;
}

String::VariableArg::VariableArg(const float arg)
  : mType(Type::FLOAT) {
  mData.float_value = arg;
}

String::VariableArg::VariableArg(const char* arg) 
  : mType(Type::CONST_STRING) {
  mData.string_value = arg;
}

String::VariableArg::VariableArg(const String& arg)
  : mType(Type::STRING_CLASS) {
  mData.string_class_value = &arg;
}

String String::VariableArg::ToString() const {
  String result;

  switch (mType) {
    case Type::SIZE_T:
    {
      const size_t bufferSize = 64;
      char buffer[bufferSize];
      buffer[0] = '\0';
      const char* str = _ui64toa(mData.size_value, buffer, 10);
      result.Append(str);
    }
      break;
    case Type::BOOL:
    {
      const char* str = (mData.bool_value) ? "1" : "0";
      result.Append(str);
    }
    break;
    case Type::INT32:
    {
      const size_t bufferSize = 64;
      char buffer[bufferSize];
      buffer[0] = '\0';
      const char* str = _itoa(mData.int32_value, buffer, 10);
      result.Append(str);
    }
      break;
    case Type::UINT32:
    {
      ZAssert(false); // Not implemented.
    }
      break;
    case Type::INT64:
    {
      const size_t bufferSize = 64;
      char buffer[bufferSize];
      buffer[0] = '\0';
      const char* str = _i64toa(mData.int64_value, buffer, 10);
      result.Append(str);
    }
      break;
    case Type::UINT64:
    {
      ZAssert(false); // Not implemented.
    }
      break;
    case Type::FLOAT:
    {
      const size_t bufferSize = 64;
      char buffer[bufferSize];
      buffer[0] = '\0';
      const int32 digits = FLT_DECIMAL_DIG;
      const float val = mData.float_value;
      const char* str = _gcvt(val, digits, buffer);
      result.Append(str);
    }
      break;
    case Type::DOUBLE:
    {
      ZAssert(false); // Not implemented.
    }
      break;
    case Type::CONST_STRING:
    {
      result.Append(mData.string_value);
    }
      break;
    case Type::STRING_CLASS:
    {
      result.Append(*(mData.string_class_value));
    }
    break;
  }

  return result;
}

WideString::WideString() {
  Copy(L"");
}

WideString::WideString(const wchar_t* str) {
  Copy(str);
}

WideString::WideString(const wchar_t* str, size_t offset, size_t end) {
  Copy(L"");
  Append(str, offset, end);
}

WideString::WideString(const WideString& rhs) {
  Copy(rhs.Str());
}

WideString::~WideString() {
  if (!IsMarkedShort()) {
    FreeLong();
  }
}

const wchar_t* WideString::Str() const {
  return GetString();
}

WideString* WideString::operator=(const WideString& rhs) {
  if (this != &rhs) {
    if (!IsMarkedShort()) {
      FreeLong();
    }

    Copy(rhs.Str());
  }

  return this;
}

bool WideString::operator==(const WideString& rhs) const {
  return wcscmp(Str(), rhs.Str()) == 0;
}

bool WideString::operator>(const WideString& rhs) const {
  return wcscmp(Str(), rhs.Str()) > 0;
}

bool WideString::operator<(const WideString& rhs) const {
  return wcscmp(Str(), rhs.Str()) < 0;
}

WideString WideString::operator+(const wchar_t* str) {
  WideString result(*this);
  result.Append(str);
  return result;
}

const wchar_t& WideString::operator[](const size_t index) {
  ZAssert(index < Length());
  return GetString()[index];
}

void WideString::Append(const WideString& str) {
  Append(str.Str());
}

void WideString::Append(const wchar_t* str, size_t offset, size_t size) {
  size_t totalSize = Length() + size + 1;
  if (FitsInSmall(totalSize)) {
    AppendShort(str, offset, size);
  }
  else {
    AppendLong(str, offset, size);
  }
}

void WideString::Append(const wchar_t* str) {
  size_t length = wcslen(str);
  if (FitsInSmall(GetCombinedSize(str) + 1)) {
    AppendShort(str, 0, length);
  }
  else {
    AppendLong(str, 0, length);
  }
}

#if 0 // Leaving here as an example of how C-style va_args work.
void WideString::Appendf(const wchar_t* formatStr, ...) {
  ZAssert(formatStr != nullptr);

  va_list arglist;
  va_start(arglist, formatStr);

  const wchar_t* lastPosition = formatStr;
  for (const wchar_t* str = formatStr; *str != '\0'; ++str) {
    wchar_t currentChar = *str;
    if (currentChar == '%') {
      wchar_t nextChar = *(str + 1);
      size_t jumpAhead = 0;
      switch (nextChar) {
      case '\0':
        continue;
      case 'd':
      {
        const size_t bufferSize = 64;
        wchar_t buffer[bufferSize];
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
        wchar_t buffer[bufferSize];
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
#endif

bool WideString::IsEmpty() const {
  return Length() == 0;
}

void WideString::Clear() {
  if (!IsMarkedShort()) {
    FreeLong();
  }

  Copy(L"");
}

void WideString::Trim(wchar_t value) {
  const wchar_t* lastLocation = FindLast(value);
  while ((lastLocation != nullptr) && (*lastLocation == value) && (lastLocation - GetString() > 0)) {
    --lastLocation;
  }

  if (lastLocation != nullptr) {
    size_t length = (lastLocation + 1) - GetString();
    WideString temp(GetString(), 0, length);
    *this = temp;
  }
}

void WideString::Trim(const Array<wchar_t>& values) {
  for (wchar_t& value : values) {
    Trim(value);
  }
}

size_t WideString::Length() const {
  return IsMarkedShort() ? GetShortLength() : GetLongLength();
}

WideString WideString::SubStr(size_t start, size_t end) {
  WideString temp(*this);
  wchar_t* subStr = temp.GetMutableString() + start;
  if (end > Length()) {
    end = Length();
  }
  temp.GetMutableString()[end] = '\0';

  WideString result(subStr);
  return result;
}

const wchar_t* WideString::FindFirst(const wchar_t value) const {
  return wcschr(GetString(), value);
}

const wchar_t* WideString::FindLast(const wchar_t value) const {
  return wcschr(GetString(), value);
}

const wchar_t* WideString::FindString(const wchar_t* pattern) const {
  size_t inLength = wcslen(pattern);
  const wchar_t* str = Str();
  const size_t length = Length();
  if (inLength > length) {
    return nullptr;
  }

  // Boyer-Moore string search.
  // Only implements the "bad character rule", does not do "good suffix".

  // Create a LUT big enough for the alphabet and size each letter row to the pattern length.
  Array<Array<size_t>> lut(WCHAR_MAX);
  for (size_t i = 0; i < lut.Size(); ++i) {
    lut[i].Resize(inLength);
  }

  // Save each unique character index for each index of the pattern starting from the end.
  // Any characters that don't exist will be 0.
  for (size_t i = inLength; i > 0; --i) {
    for (size_t j = i - 1; j > 0; --j) {
      size_t currentChar = static_cast<size_t>(pattern[j - 1]);
      Array<size_t>& charRow = lut[currentChar];
      // Only save the last index.
      if (charRow[i - 1] == 0) {
        charRow[i - 1] = j;
      }
    }
  }

  const wchar_t* match = nullptr;
  for (size_t position = inLength, skipAhead = inLength; (position <= length) && (match == nullptr); position += skipAhead) {
    bool matched = true;
    for (size_t i = 0; (i < inLength) && matched; ++i) {
      // Check each character of the pattern starting from the end.
      // If any character doesn't match, skip ahead and try again.
      const size_t inPosition = inLength - i - 1;
      const wchar_t patternChar = pattern[inPosition];
      const wchar_t matchChar = str[position - i - 1];
      if (patternChar != matchChar) {
        // Check the LUT and skip ahead by the known amount.
        skipAhead = (inLength - i) - (lut[static_cast<size_t>(matchChar)][inPosition]);
        matched = false;
      }
    }

    if (matched) {
      match = str + position - inLength;
    }
  }

  return match;
}

uint8 WideString::ToUint8() const {
  return static_cast<uint8>(wcstoul(GetString(), NULL, 10));
}

uint16 WideString::ToUint16() const {
  return static_cast<uint16>(wcstoul(GetString(), NULL, 10));
}

uint32 WideString::ToUint32() const {
  return static_cast<uint32>(wcstoul(GetString(), NULL, 10));
}

uint64 WideString::ToUint64() const {
  return static_cast<uint64>(wcstoul(GetString(), NULL, 10));
}

int8 WideString::ToInt8() const {
  return static_cast<int8>(wcstoul(GetString(), NULL, 10));
}

int16 WideString::ToInt16() const {
  return static_cast<int16>(wcstoul(GetString(), NULL, 10));
}

int32 WideString::ToInt32() const {
  return static_cast<int32>(wcstoul(GetString(), NULL, 10));
}

int64 WideString::ToInt64() const {
  return static_cast<int64>(wcstoul(GetString(), NULL, 10));
}

float WideString::ToFloat() const {
  return static_cast<float>(wcstof(GetString(), NULL));
}

String WideString::ToNarrow() const {
  if (IsEmpty()) {
    String result;
    return result;
  }

  Array<char> dest(Length() + 1);
  wcstombs(dest.GetData(), Str(), Length() + 1);
  String result(dest.GetData());
  return result;
}

void WideString::Serialize(ISerializer& serializer) {
  size_t length = Length();
  const wchar_t* str = Str();
  serializer.Serialize(&length, sizeof(length));
  serializer.Serialize(str, length * sizeof(wchar_t));
}

void WideString::Deserialize(IDeserializer& deserializer) {
  size_t length = 0;
  deserializer.Deserialize(&length, sizeof(length));

  wchar_t* buffer = (wchar_t*)PlatformMalloc(length + 1);
  deserializer.Deserialize(buffer, length * sizeof(wchar_t));
  buffer[length] = L'\0';

  Append(buffer);

  PlatformFree(buffer);
}

bool WideString::IsShort(const wchar_t* str) const {
  return (wcslen(str) + 1) < MinCapacity;
}

bool WideString::IsMarkedShort() const {
  return (mOverlapData.shortStr.size & ShortMaskSet) > 0;
}

void WideString::MarkShort(bool isShort) {
  mOverlapData.shortStr.size = (isShort) ? ((mOverlapData.shortStr.size & ShortMaskClear) | ShortMaskSet) : (mOverlapData.shortStr.size & ShortMaskClear);
}

void WideString::SetShortLength(size_t length) {
  mOverlapData.shortStr.size = (static_cast<wchar_t>(length) & ShortMaskClear);
}

wchar_t WideString::GetShortLength() const {
  return mOverlapData.shortStr.size & ShortMaskClear;
}

void WideString::CopyShort(const wchar_t* str) {
  size_t length = wcslen(str);
  SetShortLength(length);
  if (length > 0) {
    memcpy(mOverlapData.shortStr.data, str, GetShortLength() * sizeof(wchar_t));
    mOverlapData.shortStr.data[GetShortLength()] = L'\0';
  }
  else {
    mOverlapData.shortStr.data[0] = L'\0';
  }
  MarkShort(true);
}

void WideString::AppendShort(const wchar_t* str, size_t offset, size_t length) {
  wchar_t shortLength = static_cast<wchar_t>(Length());
  const wchar_t* offsetString = str + offset;
  memcpy(mOverlapData.shortStr.data + shortLength, offsetString, length * sizeof(wchar_t));
  mOverlapData.shortStr.data[shortLength + length] = L'\0';
  SetShortLength(wcslen(mOverlapData.shortStr.data));
  MarkShort(true);
}

void WideString::SetLongLength(size_t length) {
  mOverlapData.longStr.size = (length & LongMaskClear);
  mOverlapData.longStr.capacity = length;
}

size_t WideString::GetLongLength() const {
  return mOverlapData.longStr.size & LongMaskClear;
}

void WideString::AllocateLong() {
  size_t length = GetLongLength() + 1;
  mOverlapData.longStr.data = static_cast<wchar_t*>(PlatformMalloc(length * sizeof(wchar_t)));
}

void WideString::FreeLong() {
  if (mOverlapData.longStr.data != nullptr) {
    PlatformFree(mOverlapData.longStr.data);
    mOverlapData.longStr.data = nullptr;
  }
}

void WideString::CopyLong(const wchar_t* str) {
  SetLongLength(wcslen(str));
  AllocateLong();
  memcpy(mOverlapData.longStr.data, str, GetLongLength() * sizeof(wchar_t));
  mOverlapData.longStr.data[GetLongLength()] = L'\0';
  MarkShort(false);
}

void WideString::AppendLong(const wchar_t* str, size_t offset, size_t length) {
  size_t combinedLength = Length() + length;

  if (IsMarkedShort()) {
    wchar_t oldData[MinCapacity];
    size_t shortLength = GetShortLength();
    memcpy(oldData, GetString(), shortLength * sizeof(wchar_t));
    SetLongLength(shortLength);
    AllocateLong();
    MarkShort(false);

    wchar_t* mutableStr = GetMutableString();
    memcpy(mutableStr, oldData, shortLength * sizeof(wchar_t));
    mutableStr[shortLength] = L'\0';
  }

  SetLongLength(combinedLength);
  wchar_t* resizedStr = static_cast<wchar_t*>(PlatformReAlloc(mOverlapData.longStr.data, (combinedLength + 1) * sizeof(wchar_t)));
  ZAssert(resizedStr != nullptr);
  mOverlapData.longStr.data = resizedStr;

  size_t currentPosition = wcslen(GetMutableString());

  wchar_t* mutableStr = GetMutableString() + currentPosition;
  const wchar_t* strOffset = str + offset;
  memcpy(mutableStr, strOffset, length * sizeof(wchar_t));
  mutableStr[length] = L'\0';
  MarkShort(false);
}

void WideString::Copy(const wchar_t* str) {
  if (IsShort(str)) {
    CopyShort(str);
  }
  else {
    CopyLong(str);
  }
}

const wchar_t* WideString::GetString() const {
  return (IsMarkedShort()) ? mOverlapData.shortStr.data : mOverlapData.longStr.data;
}

wchar_t* WideString::GetMutableString() {
  return (IsMarkedShort()) ? mOverlapData.shortStr.data : mOverlapData.longStr.data;
}

size_t WideString::GetCombinedSize(const wchar_t* str) {
  return wcslen(str) + Length();
}

bool WideString::FitsInSmall(size_t size) {
  return size < MinCapacity;
}

void WideString::VariadicArgsAppend(const wchar_t* format, const VariableArg* args, size_t numArgs) {
  ZAssert(format != nullptr);

  const wchar_t* lastPosition = format;
  const wchar_t* lastChar = nullptr;
  for (const wchar_t* str = format; *str != '\0'; ++str) {
    wchar_t currentChar = *str;
    bool isEscaped = (lastChar != nullptr) ? ((*lastChar) == '\\') : false;

    if (!isEscaped && (currentChar == '{')) {
      size_t jumpAhead = 0;

      for (const wchar_t* endFormat = str + 1; *endFormat != '\0'; ++endFormat) {
        if (*endFormat == '}') {
          jumpAhead = (endFormat - str) + 1;
          break;
        }
      }

      if (jumpAhead == 0) {
        ZAssert(false); // Invalid format.
        break;
      }

      // Get the index specified by the argument.
      int32 argIndex = static_cast<int32>(wcstol(str + 1, NULL, 10));
      ZAssert(argIndex >= 0);

      if (argIndex >= numArgs) {
        ZAssert(false); // OOB
        continue;
      }

      // Append format str up until the current position.
      Append(lastPosition, 0, str - lastPosition);

      // Append the VariableArg based on its type.
      const VariableArg& arg = args[argIndex];
      Append(arg.ToString().Str());

      str += jumpAhead;
      lastPosition = str;
    }

    lastChar = str;
  }
}

WideString::VariableArg::VariableArg(const int32 arg)
  : mType(Type::INT32) {
  mData.int32_value = arg;
}

WideString::VariableArg::VariableArg(const float arg)
  : mType(Type::FLOAT) {
  mData.float_value = arg;
}

WideString WideString::VariableArg::ToString() const {
  WideString result;

  switch (mType) {
  case Type::INT32:
  {
    const size_t bufferSize = 64;
    wchar_t buffer[bufferSize];
    buffer[0] = '\0';
    const wchar_t* str = _itow(mData.int32_value, buffer, 10);
    result.Append(str);
  }
  break;
  case Type::UINT32:
  {
    ZAssert(false); // Not implemented.
  }
  break;
  case Type::INT64:
  {
    ZAssert(false); // Not implemented.
  }
  break;
  case Type::UINT64:
  {
    ZAssert(false); // Not implemented.
  }
  break;
  case Type::FLOAT:
  {
#if 0
    const size_t bufferSize = 64;
    wchar_t buffer[bufferSize];
    buffer[0] = '\0';
    const int32 digits = FLT_DECIMAL_DIG;
    const float val = mData.float_value;
    const wchar_t* str = _gcvt(val, digits, buffer);
    result.Append(str);
#endif
    ZAssert(false); // TODO: Is this even necessary?
  }
  break;
  case Type::DOUBLE:
  {
    ZAssert(false); // Not implemented.
  }
  break;
  }

  return result;
}

}
