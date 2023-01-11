#include "Logger.h"

#include "ZAssert.h"
#include "PlatformDebug.h"
#include "PlatformFile.h"
#include "PlatformHAL.h"
#include "PlatformIntrinsics.h"
#include "PlatformMisc.h"
#include "PlatformTime.h"

namespace ZSharp {

Logger::Logger() : mLog(LogFilePath(), 0) {
  LogPreamble();
}

Logger::~Logger() {
}

void Logger::Log(LogCategory category, const String& message) {
  Logger& logger = GetInstance();
  logger.InternalLog(category, message);
}

void Logger::InternalLog(LogCategory category, const String& message) {
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

  logMessage.Appendf("[{0}] ", PlatformSystemTimeFormat());
  logMessage.Append(message);

  // Log to 3 locations:
  //  1) Console (if process contains one)
  //  2) Debugger window
  //  3) Log file

  if (PlatformHasConsole()) {
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

void Logger::LogPreamble() {
  String preamble;
  preamble.Append("\n==========Begin Sys Info==========\n");
  preamble.Appendf("CPU: ID={0}, Brand={1}\n", PlatformCPUVendor(), PlatformCPUBrand());
  preamble.Appendf("Cores: Physical={0}, Logical={1}\n", PlatformGetNumPhysicalCores(), PlatformGetNumLogicalCores());
  preamble.Appendf("SIMD: SSE2={0}, SSE4={1}, AVX2={2}, AVX512={3}\n", PlatformSupportsSIMDMode(SIMDMode::SSE2),
    PlatformSupportsSIMDMode(SIMDMode::SSE4),
    PlatformSupportsSIMDMode(SIMDMode::AVX2),
    PlatformSupportsSIMDMode(SIMDMode::AVX512));
  preamble.Appendf("OS: Username={0}, Machine={1}\n", PlatformGetUsername(), PlatformGetMachineName());
  preamble.Append("==========End Sys Info==========\n");

  InternalLog(LogCategory::Info, preamble);
}

}
