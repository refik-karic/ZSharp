#pragma once

#include "ZBaseTypes.h"

namespace ZSharp {

typedef int32 (*PlatformThreadFunction)(void* arg);

struct PlatformThread;

PlatformThread* PlatformCreateThread(PlatformThreadFunction threadFunction, void* threadData);

void PlatformPinThreadsToProcessors(PlatformThread** threads, size_t numThreads, bool logical);

void PlatformJoinThread(PlatformThread* thread);

void PlatformJoinThreadPool(PlatformThread** threads, size_t numThreads);

void PlatformYieldThread();

void PlatformBusySpin();

}
