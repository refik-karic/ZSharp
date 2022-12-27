#include "Logger.h"

#include "ZAssert.h"
#include "PlatformDebug.h"
#include "PlatformFile.h"
#include "PlatformTime.h"

namespace ZSharp {

Logger::Logger() : mLog(LogFilePath(), 0) {
}

Logger::~Logger() {
}

void Logger::Log(LogCategory category, const String& message) {
  Logger& logger = GetInstance();

  String logMessage;

  switch (category) {
    case LogCategory::Warning:
      logMessage.Append("[Warning] ");
      break;
    case LogCategory::Error:
      logMessage.Append("[Error] ");
      break;
    case LogCategory::Info:
      logMessage.Append("[Info] ");
      break;
    case LogCategory::Debug:
      logMessage.Append("[Debug] ");
      break;
    case LogCategory::Perf:
      logMessage.Append("[Perf] ");
      break;
  }

  logMessage.Appendf("[{0}] ", PlatformSystemTimeFormat().Str());
  logMessage.Append(message);

  PlatformDebugPrint(message.Str());

  const size_t logLength = logMessage.Length();
  if (!logger.IsExcessiveSize(logLength)) {
    logger.mLog.Write(logMessage.Str(), logLength);
    logger.mLogSize += logLength;
  }
}

Logger& Logger::GetInstance() {
  static Logger logger;
  return logger;
}

FileString Logger::LogFilePath() {
  FileString logPath(PlatformGetWorkingDirectory());
  
  String logFilename(PlatformGetExecutableName());
  logFilename.Append("_log.txt");

  logPath.SetFilename(logFilename);
  return logPath;
}

bool Logger::IsExcessiveSize(const size_t nextMessageLength) const {
  const size_t MaxLogSize = 10000000000;
  if (mLogSize + nextMessageLength > MaxLogSize) {
    ZAssert(false);
    PlatformDebugPrint("Exceeded max log size!\n");
    return true;
  }
  else {
    return false;
  }
}

}
