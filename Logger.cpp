#include "Logger.h"

#include "ZAssert.h"
#include "PlatformDebug.h"
#include "PlatformFile.h"
#include "PlatformHAL.h"
#include "PlatformIntrinsics.h"
#include "PlatformMisc.h"
#include "PlatformTime.h"
#include "Span.h"

namespace ZSharp {

Logger::Logger() : mLog(LogFilePath(), 0) {
  LogPrologue();
}

Logger::~Logger() {
}

void Logger::Log(LogCategory category, const String& message) {
  Logger& logger = GetInstance();
  logger.InternalLog(category, message);
}

void Logger::InternalLog(LogCategory category, const String& message) {
  String logMessage;

  bool logStdOutput = true;
  const char* logCategory = "";
  size_t categoryLength = 0;

  switch (category) {
    case LogCategory::Warning:
    {
      logCategory = "[Warning]";
      categoryLength = sizeof("[Warning]") - 1;
    }
      break;
    case LogCategory::Error:
    {
      logCategory = "[Error]";
      categoryLength = sizeof("[Error]") - 1;
    }
      break;
    case LogCategory::Info:
    {
      logCategory = "[Info]";
      categoryLength = sizeof("[Info]") - 1;
    }
      break;
    case LogCategory::Debug:
    {
      logCategory = "[Debug]";
      categoryLength = sizeof("[Debug]") - 1;
    }
      break;
    case LogCategory::Perf:
    {
      logCategory = "[Perf]";
      categoryLength = sizeof("[Perf]") - 1;
    }
      break;
    case LogCategory::System:
    {
      logCategory = "[System]";
      categoryLength = sizeof("[System]") - 1;
      logStdOutput = false;
    }
      break;
  }

  Span<const char> categoryString(logCategory, categoryLength);
  logMessage.Appendf("{0} [{1}] {2}", categoryString, PlatformSystemTimeFormat(), message);

  // Log to 3 locations:
  //  1) Console (if process contains one)
  //  2) Debugger window
  //  3) Log file

  if (logStdOutput && PlatformHasConsole()) {
    PlatformWriteConsole(message);
  }

  PlatformDebugPrint(message.Str());

  const size_t logLength = logMessage.Length();
  if (!IsExcessiveSize(logLength)) {
    mLog.Write(logMessage.Str(), logLength);
    mLogSize += logLength;
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

void Logger::LogPrologue() {
  String prologue;
  prologue.Append("\n\n==========Begin Sys Info==========\n");
  prologue.Appendf("CPU: ID={0}, Brand={1}\n",
    PlatformCPUVendor(), 
    PlatformCPUBrand());
  prologue.Appendf("Cores: Physical={0}, Logical={1}\n",
    PlatformGetNumPhysicalCores(), 
    PlatformGetNumLogicalCores());
  prologue.Appendf("SIMD: 4-Wide={0}, 8-Wide={1}, 16-Wide={2}\n",
    PlatformSupportsSIMDLanes(SIMDLaneWidth::Four),
    PlatformSupportsSIMDLanes(SIMDLaneWidth::Eight),
    PlatformSupportsSIMDLanes(SIMDLaneWidth::Sixteen));
  prologue.Appendf("OS: Username={0}, Machine={1}\n",
    PlatformGetUsername(), 
    PlatformGetMachineName());
  prologue.Appendf("Build: {0}\n",
    PlatformGetBuildType());

  Array<String> displays(PlatformEnumDisplayInfo());
  for (size_t i = 0; i < displays.Size(); ++i) {
    prologue.Append(displays[i]);
  }

  prologue.Append("==========End Sys Info==========\n\n");

  InternalLog(LogCategory::System, prologue);
}

}
