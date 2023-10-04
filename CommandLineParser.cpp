#include "CommandLineParser.h"

#include "ZAssert.h"
#include "Logger.h"

namespace ZSharp {

CLICommand::CLICommand(const String& name, const String& help, const CLICallback& callback) 
  : mName(name), mHelp(help), mCallback(callback) {
}

void CLICommand::AddOption(const String& option) {
  if (mOptions.HasKey(option)) {
    Logger::Log(LogCategory::Warning, String::FromFormat("Option {0} already exists, skipping.\n", option));
    return;
  }
  else {
    mOptions.Add(option, "");
  }
}

String CLICommand::GetOption(const String& option) const {
  return mOptions.GetValue(option);
}

CLIParser::CLIParser(const Array<CLICommand>& commands, const Array<String>& globalOptions)
  : mCommands(commands), mGlobalOptions(globalOptions.Size()) {
  for (const String& option : globalOptions) {
    mGlobalOptions.Add(option, "");
  }
}

int32 CLIParser::Evaluate(int argc, const char** argv, bool justOptions) {
  if (argc == 1) {
    Logger::Log(LogCategory::Error, "No args specified.\n");
    Logger::Log(LogCategory::Error, "Use --help for info on commands.\n");
    return -1;
  }
  
  String commandName(argv[1]);

  if (commandName == "--help") {
    Logger::Log(LogCategory::Info, "Usage:\n");

    for (const CLICommand& command : mCommands) {
      Logger::Log(LogCategory::Info, String::FromFormat("\t{0} ", command.mName.Str()));
      for (const Pair<String, String>& item : command.mOptions) {
        Logger::Log(LogCategory::Info, String::FromFormat("{0} ", item.mKey.Str()));
      }

      Logger::Log(LogCategory::Info, String::FromFormat("\n\t\t{0}\n", command.mHelp.Str()));
    }

    Logger::Log(LogCategory::Info, "Global Options:\n\t");

    for (const Pair<String, String>& item : mGlobalOptions) {
      Logger::Log(LogCategory::Info, String::FromFormat("{0} ", item.mKey.Str()));
    }

    Logger::Log(LogCategory::Info, "\n");
    return 0;
  }

  CLICommand* commandPtr = nullptr;
  
  if (!justOptions) {
    for (CLICommand& command : mCommands) {
      if (command.mName == commandName) {
        commandPtr = &command;
        break;
      }
    }

    if (commandPtr == nullptr) {
      Logger::Log(LogCategory::Error, String::FromFormat("[{0}] not found in saved commands.\n", commandName));
      return -1;
    }
  }

  const int32 firstOption = (justOptions) ? 1 : 2;

  for (int32 i = firstOption; i < argc; ++i) {
    const char* arg = argv[i];
    if (arg[0] == '-') {
      arg++;
    }
    
    String argName(arg);
    const char* optionDelimiter = argName.FindFirst('=');
    String argValue;

    if (optionDelimiter != nullptr) {
      size_t equalsLocation = optionDelimiter - argName.Str();

      String originalName = argName;
      optionDelimiter = originalName.Str() + equalsLocation;
      argName = argName.SubStr(0, equalsLocation);

      optionDelimiter++;

      argValue = optionDelimiter;
    }
    else {
      argValue.Append("1");
    }

    if (!justOptions) {
      // Break if command-specific options clash with global options.
      ZAssert(!(commandPtr->mOptions.HasKey(argName) && mGlobalOptions.HasKey(argName)));

      if (commandPtr->mOptions.HasKey(argName)) {
        commandPtr->mOptions[argName] = argValue;
      }
      else if (mGlobalOptions.HasKey(argName)) {
        mGlobalOptions[argName] = argValue;
      }
      else {
        Logger::Log(LogCategory::Warning, String::FromFormat("Unknown option: {0}={1}\n", argName, argValue));
      }
    }
    else {
      if (mGlobalOptions.HasKey(argName)) {
        mGlobalOptions[argName] = argValue;
      }
      else {
        Logger::Log(LogCategory::Warning, String::FromFormat("Unknown option: {0}={1}\n", argName, argValue));
      }
    }
  }

  if (justOptions) {
    return 0;
  }

  bool optionsPopulated = true;

  for (const Pair<String, String>& option : commandPtr->mOptions) {
    if (option.mValue.IsEmpty()) {
      optionsPopulated = false;
      Logger::Log(LogCategory::Warning, String::FromFormat("Option [{0}] was not set.\n", option.mKey.Str()));
    }
  }

  if (optionsPopulated) {
    return commandPtr->mCallback(*commandPtr, mGlobalOptions);
  }
  else {
    Logger::Log(LogCategory::Error, String::FromFormat("Missing required options for command [{0}].\n", commandPtr->mName.Str()));
    return -1;
  }
}

int32 CLIParser::Evaluate(int argc, const wchar_t** argv, bool justOptions) {
  if (argc == 1) {
    Logger::Log(LogCategory::Error, "No args specified.\n");
    Logger::Log(LogCategory::Error, "Use --help for info on commands.\n");
    return -1;
  }

  String commandName(WideString(argv[1]).ToNarrow());

  if (commandName == "--help") {
    Logger::Log(LogCategory::Info, "Usage:\n");

    for (const CLICommand& command : mCommands) {
      Logger::Log(LogCategory::Info, String::FromFormat("\t{0} ", command.mName.Str()));
      for (const Pair<String, String>& item : command.mOptions) {
        Logger::Log(LogCategory::Info, String::FromFormat("{0} ", item.mKey.Str()));
      }

      Logger::Log(LogCategory::Info, String::FromFormat("\n\t\t{0}\n", command.mHelp.Str()));
    }

    Logger::Log(LogCategory::Info, "Global Options:\n\t");

    for (const Pair<String, String>& item : mGlobalOptions) {
      Logger::Log(LogCategory::Info, String::FromFormat("{0} ", item.mKey.Str()));
    }

    Logger::Log(LogCategory::Info, "\n");
    return 0;
  }

  CLICommand* commandPtr = nullptr;

  if (!justOptions) {
    for (CLICommand& command : mCommands) {
      if (command.mName == commandName) {
        commandPtr = &command;
        break;
      }
    }

    if (commandPtr == nullptr) {
      Logger::Log(LogCategory::Error, String::FromFormat("[{0}] not found in saved commands.\n", commandName));
      return -1;
    }
  }

  const int32 firstOption = (justOptions) ? 1 : 2;

  for (int32 i = firstOption; i < argc; ++i) {
    const wchar_t* arg = argv[i];
    if (arg[0] == L'-') {
      arg++;
    }

    String argName(WideString(arg).ToNarrow());
    const char* optionDelimiter = argName.FindFirst('=');
    String argValue;

    if (optionDelimiter != nullptr) {
      size_t equalsLocation = optionDelimiter - argName.Str();

      String originalName = argName;
      optionDelimiter = originalName.Str() + equalsLocation;
      argName = argName.SubStr(0, equalsLocation);

      optionDelimiter++;

      argValue = optionDelimiter;
    }
    else {
      argValue.Append("1");
    }

    // Break if command-specific options clash with global options.
    if (!justOptions) {
      ZAssert(!(commandPtr->mOptions.HasKey(argName) && mGlobalOptions.HasKey(argName)));

      if (commandPtr->mOptions.HasKey(argName)) {
        commandPtr->mOptions[argName] = argValue;
      }
      else if (mGlobalOptions.HasKey(argName)) {
        mGlobalOptions[argName] = argValue;
      }
      else {
        Logger::Log(LogCategory::Warning, String::FromFormat("Unknown option: {0}={1}\n", argName, argValue));
      }
    }
    else {
      if (mGlobalOptions.HasKey(argName)) {
        mGlobalOptions[argName] = argValue;
      }
      else {
        Logger::Log(LogCategory::Warning, String::FromFormat("Unknown option: {0}={1}\n", argName, argValue));
      }
    }
  }

  if (justOptions) {
    return 0;
  }

  bool optionsPopulated = true;

  for (const Pair<String, String>& option : commandPtr->mOptions) {
    if (option.mValue.IsEmpty()) {
      optionsPopulated = false;
      Logger::Log(LogCategory::Warning, String::FromFormat("Option [{0}] was not set.\n", option.mKey.Str()));
    }
  }

  if (optionsPopulated) {
    return commandPtr->mCallback(*commandPtr, mGlobalOptions);
  }
  else {
    Logger::Log(LogCategory::Error, String::FromFormat("Missing required options for command [{0}].\n", commandPtr->mName.Str()));
    return -1;
  }
}

bool CLIParser::WasPassed(const String& option) {
  return mGlobalOptions.HasKey(option) && !mGlobalOptions[option].IsEmpty();
}

String CLIParser::GetValue(const String& option) {
  if (mGlobalOptions.HasKey(option)) {
    return mGlobalOptions[option];
  }
  else {
    return String("");
  }
}

}
