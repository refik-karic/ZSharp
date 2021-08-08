#include "JsonObject.h"

namespace ZSharp {

void JsonObject::SetKey(const std::string& key) {
  mKey = key;
}

std::string& JsonObject::GetKey() {
  return mKey;
}

void JsonObject::SetValue(const JsonValue& value) {
  mValue = value;
}

JsonObject::JsonValue& JsonObject::GetValue() {
  return mValue;
}

}
