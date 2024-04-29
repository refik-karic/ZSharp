#pragma once

#include "ZBaseTypes.h"

namespace ZSharp {

typedef int32 (*PlatformThreadFunction)(void* arg);

struct PlatformThread;

struct PlatformMonitor;

PlatformThread* PlatformCreateThread(PlatformThreadFunction threadFunction, void* threadData);

void PlatformPinThreadsToProcessors(PlatformThread** threads, size_t numThreads, bool logical);

void PlatformJoinThread(PlatformThread* thread);

void PlatformJoinThreadPool(PlatformThread** threads, size_t numThreads);

PlatformMonitor* PlatformCreateMonitor();

void PlatformWaitMonitor(PlatformMonitor* monitor);

void PlatformSignalMonitor(PlatformMonitor* monitor);

void PlatformClearMonitor(PlatformMonitor* monitor);

void PlatformDestroyMonitor(PlatformMonitor* monitor);

void PlatformYieldThread();

void PlatformBusySpin();

}
