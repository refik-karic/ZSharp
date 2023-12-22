#pragma once

#include "ZBaseTypes.h"
#include "Delegate.h"
#include "ZString.h"
#include "HashTable.h"

namespace ZSharp {

extern HashTable<String, Delegate<const String&>> GlobalConsoleCommands;

template<typename T>
struct ConsoleVariableConverter {
  void operator()(const String& str, T& var) const {
    (void)str;
    (void)var;
  }
};

template<>
struct ConsoleVariableConverter<int32> {
  void operator()(const String& str, int32& var) const {
    var = str.ToInt32();
  }
};

template<typename T>
class ConsoleVariable {
  public:

  ConsoleVariable(const String& name) {
    if (!GlobalConsoleCommands.HasKey(name)) {
      GlobalConsoleCommands.Add(name, Delegate<const String&>::FromMember<ConsoleVariable, &ConsoleVariable::Set>(this));
    }
  }

  ConsoleVariable(const String& name, const T& value)
    : mName(name), mValue(value) {
    if (!GlobalConsoleCommands.HasKey(name)) {
      GlobalConsoleCommands.Add(name, Delegate<const String&>::FromMember<ConsoleVariable, &ConsoleVariable::Set>(this));
    }
  }

  ~ConsoleVariable() {
    if (GlobalConsoleCommands.HasKey(mName)) {
      GlobalConsoleCommands.Remove(mName);
    }
  }

  T& operator*() {
    return mValue;
  }

  void Set(const String& value) {
    ConsoleVariableConverter<T> converter;
    converter(value, mValue);
  }

  private:
  T mValue;
  String mName;
};

}
