#pragma once

#include "ZBaseTypes.h"
#include "Delegate.h"
#include "ZString.h"
#include "HashTable.h"

namespace ZSharp {

HashTable<String, Delegate<const String&>>& GlobalConsoleCommands();

HashTable<String, Delegate<void>>& GlobalConsoleCommandsValueless();

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

template<>
struct ConsoleVariableConverter<bool> {
  void operator()(const String& str, bool& var) const {
    var = (str.ToInt32() != 0);
  }
};

template<>
struct ConsoleVariableConverter<float> {
  void operator()(const String& str, float& var) const {
    var = str.ToFloat();
  }
};

template<typename T>
class ConsoleVariable {
  public:

  ConsoleVariable(const String& name, Delegate<void>& callback) 
    : mName(name), mCallback(callback) {
    if (!GlobalConsoleCommands().HasKey(name)) {
      GlobalConsoleCommands().Add(name, Delegate<const String&>::FromMember<ConsoleVariable, &ConsoleVariable::Set>(this));
    }
  }

  ConsoleVariable(const String& name, const T& value)
    : mValue(value), mName(name)  {
    if (!GlobalConsoleCommands().HasKey(name)) {
      GlobalConsoleCommands().Add(name, Delegate<const String&>::FromMember<ConsoleVariable, &ConsoleVariable::Set>(this));
    }
  }

  ConsoleVariable(const String& name, const T& value, Delegate<void>& callback)
    : mName(name), mValue(value), mCallback(callback) {
    if (!GlobalConsoleCommands().HasKey(name)) {
      GlobalConsoleCommands().Add(name, Delegate<const String&>::FromMember<ConsoleVariable, &ConsoleVariable::Set>(this));
    }
  }

  ~ConsoleVariable() {
    if (GlobalConsoleCommands().HasKey(mName)) {
      GlobalConsoleCommands().Remove(mName);
    }
  }

  T& operator*() {
    return mValue;
  }

  void Set(const String& value) {
    ConsoleVariableConverter<T> converter;
    converter(value, mValue);

    if (mCallback.IsBound()) {
      mCallback();
    }
  }

  private:
  T mValue;
  String mName;
  Delegate<void> mCallback;
};

template<>
class ConsoleVariable<void> {
  public:

  ConsoleVariable(const String& name, Delegate<void>& callback)
    : mName(name), mCallback(callback) {
    if (!GlobalConsoleCommandsValueless().HasKey(name)) {
      GlobalConsoleCommandsValueless().Add(name, Delegate<void>::FromMember<ConsoleVariable, &ConsoleVariable::Invoke>(this));
    }
  }

  ConsoleVariable(const String& name, Delegate<void> callback)
    : mName(name), mCallback(callback) {
    if (!GlobalConsoleCommandsValueless().HasKey(name)) {
      GlobalConsoleCommandsValueless().Add(name, Delegate<void>::FromMember<ConsoleVariable, &ConsoleVariable::Invoke>(this));
    }
  }

  ~ConsoleVariable() {
    if (GlobalConsoleCommandsValueless().HasKey(mName)) {
      GlobalConsoleCommandsValueless().Remove(mName);
    }
  }

  void Invoke() {
    if (mCallback.IsBound()) {
      mCallback();
    }
  }

  private:
  String mName;
  Delegate<void> mCallback;
};

}
