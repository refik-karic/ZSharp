#include "ZString.h"

#include <cstring>

namespace ZSharp {
String::String() {
  MarkShort(true);
  Clear();
}

String::String(const char* str) {
  MarkShort(true);
  Clear();
  Copy(str);
}

String::String(const char* str, size_t offset, size_t end) {
  MarkShort(true);
  Clear();
  Append(str, offset, end);
}

String::String(const String& rhs) {
  MarkShort(true);
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

void String::Append(const String& str) {
  Append(str.Str());
}

void String::Append(const char* str, size_t offset, size_t size) {
  size_t totalSize = GetSize(false) + size;
  if (FitsInSmall(totalSize)) {
    AppendShort(str, offset, size);
  }
  else {
    AppendLong(str, offset, size);
  }
}

void String::Append(const char* str) {
  size_t length = strlen(str) + 1;
  if (FitsInSmall(GetCombinedSize(str))) {
    AppendShort(str, 0, length);
  }
  else {
    AppendLong(str, 0, length);
  }
}

bool String::IsEmpty() {
  return (GetSize(false) == 0);
}

void String::Clear() {
  if (!IsMarkedShort()) {
    FreeLong();
  }

  SetShortLength(0);
  MarkShort(true);
}

size_t String::GetSize(bool includeNull) const {
  size_t length = IsMarkedShort() ? GetShortLength() : GetLongLength();
  if (length > 0 && !includeNull) {
    length -= 1;
  }
  return length;
}

String String::SubStr(size_t start, size_t end) {
  String temp(*this);
  char* subStr = temp.GetMutableString() + start;
  if (end > (GetSize(false))) {
    end = GetSize(false);
  }
  temp.GetMutableString()[end] = '\0';

  String result(subStr);
  return result;
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
  size_t length = strlen(str) + 1;
  SetShortLength(length);
  strncpy_s(mOverlapData.shortStr.data, GetShortLength(), str, GetShortLength());
  MarkShort(true);
}

void String::AppendShort(const char* str, size_t offset, size_t length) {
  unsigned char shortLength = GetShortLength();

  if (shortLength > 0) {
    shortLength -= 1;
  }

  const char* offsetString = str + offset;
  strncpy_s(mOverlapData.shortStr.data + shortLength, sizeof(mOverlapData.shortStr.data), offsetString, length);
  SetShortLength(strlen(mOverlapData.shortStr.data) + 1);
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
  mOverlapData.longStr.data = static_cast<char*>(malloc(GetLongLength()));
}

void String::FreeLong() {
  free(mOverlapData.longStr.data);
}

void String::CopyLong(const char* str) {
  size_t length = strlen(str) + 1;
  SetLongLength(length);
  AllocateLong();
  strncpy_s(mOverlapData.longStr.data, GetLongLength(), str, GetLongLength());
  MarkShort(false);
}

void String::AppendLong(const char* str, size_t offset, size_t length) {
  size_t combinedLength = GetSize(false) + length;

  if (IsMarkedShort()) {
    char oldData[MinCapaity];
    size_t shortLength = GetShortLength();
    strncpy_s(oldData, shortLength, GetString(), shortLength);
    SetLongLength(shortLength);
    AllocateLong();
    MarkShort(false);
    strncpy_s(GetMutableString(), shortLength, oldData, shortLength);
  }

  SetLongLength(combinedLength);
  mOverlapData.longStr.data = static_cast<char*>(realloc(GetMutableString(), combinedLength));
  size_t currentPosition = strlen(GetMutableString());
  strncpy_s(GetMutableString() + currentPosition, length, str + offset, length);
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
  size_t inLength = strlen(str) + 1;
  return inLength + GetSize(false);
}

bool String::FitsInSmall(size_t size) {
  return size < MinCapaity;
}
}
