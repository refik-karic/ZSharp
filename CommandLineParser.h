#pragma once

#include "Array.h"
#include "HashTable.h"
#include "ZString.h"

#include "Delegate.h"
#include "Pair.h"

namespace ZSharp {

class CLICommand;
class CLIParser;

typedef HashTable<String, String> CLICommandOptions;
typedef ResultDelegate<int32, const CLICommand&, const CLICommandOptions&> CLICallback;

class CLICommand final {
  public:
  CLICommand() {}

  CLICommand(const CLICommand& rhs)
    : mName(rhs.mName), mHelp(rhs.mHelp), mCallback(rhs.mCallback), mOptions(rhs.mOptions) {

  }

  CLICommand(const String& name, const String& help, const CLICallback& callback);

  void AddOption(const String& option);

  String GetOption(const String& option) const;

  bool operator==(const CLICommand& rhs) const {
    if (this == &rhs) {
      return true;
    }

    return mName == rhs.mName;
  }

  void operator=(const CLICommand& rhs) {
    if (this == &rhs) {
      return;
    }

    mName = rhs.mName;
    mHelp = rhs.mHelp;
    mCallback = rhs.mCallback;
    mOptions = rhs.mOptions;
  }

  private:
  String mName;
  String mHelp;
  CLICallback mCallback;
  HashTable<String, String> mOptions;

  friend class CLIParser;
};

class CLIParser final {
  public:

  CLIParser(const Array<CLICommand>& commands, const Array<String>& globalOptions);

  int32 Evaluate(int argc, const char** argv);

  private:
  Array<CLICommand> mCommands;
  CLICommandOptions mGlobalOptions;
};

}
