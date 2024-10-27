#pragma once

#include "ZBaseTypes.h"

#include "Array.h"
#include "Delegate.h"
#include "List.h"
#include "PlatformThread.h"
#include "PlatformAtomic.h"
#include "Span.h"

namespace ZSharp {

typedef Delegate<Span<uint8>> ParallelRange;

struct ThreadJob {
  ParallelRange func;
  Span<uint8> data;
};

struct WorkerThreadControl;

struct ThreadControl {
  enum class RunStatus {
    SLEEP,
    RUNNING,
    END
  };

  RunStatus status = RunStatus::SLEEP;
  PlatformMutex lock;
  PlatformMonitor* monitor;
  PlatformMonitor* asyncMonitor;
  Array<WorkerThreadControl> workers;
};

struct WorkerThreadControl {
  ThreadControl* masterControl = nullptr;
  size_t id = 0;
  PlatformMutex jobLock;
  PlatformMonitor* jobMonitor;
  List<ThreadJob> jobs;
};

class ThreadPool final {
  public:

  ThreadPool();
  ~ThreadPool();

  ThreadPool(const ThreadPool& rhs) = delete;
  void operator=(const ThreadPool& rhs) = delete;

  void Wake();

  void Sleep();

  void Execute(ParallelRange& range, void* data, size_t length, size_t chunkMultiple);

  void WaitForJobs();

  private:
  Array<PlatformThread*> mPool;
  ThreadControl mControl;
};

}
