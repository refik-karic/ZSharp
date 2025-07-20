#pragma once

#include "ZBaseTypes.h"
#include "ZFile.h"
#include "ZString.h"

namespace ZSharp {

enum class LogCategory {
  Warning,
  Error,
  Info,
  Debug,
  Perf,
  System
};

class Logger {
  public:

  static void Log(LogCategory category, const String& message);

  Logger();

  ~Logger();

  private:

  static FileString LogFilePath();

  bool IsExcessiveSize(const size_t nextMessageLength) const;

  void InternalLog(LogCategory category, const String& message);

  void LogPrologue();

  SystemBufferedFileWriter mLog;
  size_t mLogSize = 0;
};

extern Logger* GlobalLog;

}
