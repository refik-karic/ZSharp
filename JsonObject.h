#pragma once

#include <cstdint>

#include <memory>
#include <string>
#include <vector>

namespace ZSharp {

class JsonObject final {
  public:
  enum class JsonValueType {
    STRING_VALUE,
    NUMBER_INT_VALUE,
    NUMBER_FLOAT_VALUE,
    OBJECT_VALUE,
    ARRAY_VALUE,
    BOOL_VALUE,
    NULL_VALUE
  };

  struct JsonValue {
    JsonValueType valueType;
    std::string dataString;
    union {
      bool dataBool;
      double dataFloat;
      std::int64_t dataInt;
    };
    std::shared_ptr<JsonObject> dataObject;
    std::vector<JsonValue> dataArray;
  };

  void SetKey(const std::string& key);

  std::string& GetKey();

  void SetValue(const JsonValue& value);

  JsonValue& GetValue();

  private:
  std::string mKey;

  JsonValue mValue;
};

}
