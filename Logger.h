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
  Perf
};

class Logger {
  public:

  static void Log(LogCategory category, const String& message);

  private:
  Logger();

  ~Logger();

  static Logger& GetInstance();

  static FileString LogFilePath();

  bool IsExcessiveSize(const size_t nextMessageLength) const;

  void InternalLog(LogCategory category, const String& message);

  void LogPreamble();

  BufferedFileWriter mLog;
  size_t mLogSize = 0;
};

}
