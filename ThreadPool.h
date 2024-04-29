#pragma once

#include "ZBaseTypes.h"

#include "Array.h"
#include "Delegate.h"
#include "PlatformThread.h"
#include "PlatformAtomic.h"

namespace ZSharp {

typedef Delegate<size_t, size_t, void*> ParallelRange;

struct ThreadControl {
  enum class RunStatus {
    SLEEP,
    RUNNING,
    END
  };

  RunStatus status = RunStatus::SLEEP;
  PlatformMutex lock;
  PlatformMonitor* monitor;
  size_t remainingJobs = 0;
  ParallelRange func;
  void* data;
};

struct WorkerThreadControl {
  ThreadControl* masterControl = nullptr;
  size_t id = 0;
  size_t begin = 0;
  size_t end = 0;
};

class ThreadPool final {
  public:

  ThreadPool();
  ~ThreadPool();

  ThreadPool(const ThreadPool& rhs) = delete;
  void operator=(const ThreadPool& rhs) = delete;

  void Wake();

  void Sleep();

  void Execute(ParallelRange& range, void* data, size_t length, size_t chunkMultiple, bool async);

  private:
  Array<PlatformThread*> mPool;
  Array<WorkerThreadControl> mWorkerControl;
  ThreadControl mControl;
};

}
