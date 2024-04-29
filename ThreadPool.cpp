#include "ThreadPool.h"

#include "CommonMath.h"
#include "PlatformHAL.h"
#include "PlatformMemory.h"

namespace ZSharp {

int32 BackgroundWorker(void* data) {
  WorkerThreadControl& workerControl = *((WorkerThreadControl*)data);
  ThreadControl& control = *(workerControl.masterControl);

  while (true) {
    if (control.status == ThreadControl::RunStatus::RUNNING) {
      if (control.func.IsBound() && (workerControl.begin != workerControl.end)) {
        control.func(workerControl.begin, workerControl.end, control.data);

        workerControl.begin = 0;
        workerControl.end = 0;

        control.lock.Aquire();

        control.remainingJobs--;

        if (control.remainingJobs == 0) {
          PlatformClearMonitor(control.monitor);
          control.status = ThreadControl::RunStatus::SLEEP;
        }

        control.lock.Release();
      }
      else {
        PlatformBusySpin();
      }
    }
    else if (control.status == ThreadControl::RunStatus::SLEEP) {
      PlatformWaitMonitor(control.monitor);
    }
    else if (control.status == ThreadControl::RunStatus::END) {
      break;
    }
  }

  return 0;
}

ThreadPool::ThreadPool() {
  // Note: We don't want too many worker threads competing with the main thread.
  //  Therefore we only allocate half the CPU to worker threads.
  size_t numCores = PlatformGetNumPhysicalCores() / 2;
  mPool.Resize(numCores);
  mWorkerControl.Resize(numCores);

  mControl.monitor = PlatformCreateMonitor();

  for (size_t i = 0; i < mWorkerControl.Size(); ++i) {
    WorkerThreadControl& control = mWorkerControl[i];
    control.masterControl = &mControl;
    control.id = i;
  }

  for (size_t i = 0; i < numCores; ++i) {
    mPool[i] = PlatformCreateThread(&BackgroundWorker, &mWorkerControl[i]);
  }

  PlatformPinThreadsToProcessors(mPool.GetData(), mPool.Size(), false);
}

ThreadPool::~ThreadPool() {
  mControl.status = ThreadControl::RunStatus::END;
  PlatformSignalMonitor(mControl.monitor);

  PlatformJoinThreadPool(mPool.GetData(), mPool.Size());

  PlatformDestroyMonitor(mControl.monitor);
}

void ThreadPool::Wake() {
  mControl.data = nullptr;
  mControl.func.Unbind();
  mControl.remainingJobs = 0;

  mControl.status = ThreadControl::RunStatus::RUNNING;

  PlatformSignalMonitor(mControl.monitor);
}

void ThreadPool::Sleep() {
  mControl.status = ThreadControl::RunStatus::SLEEP;
}

void ThreadPool::Execute(ParallelRange& range, void* data, size_t length, size_t chunkMultiple, bool async) {
  size_t numThreads = mPool.Size();
  
  if (length == 0) {
    return;
  }

  if (length < numThreads) {
    mWorkerControl[0].begin = 0;
    mWorkerControl[0].end = length;
    mControl.remainingJobs++;
  }
  else {
    size_t chunkSize = length / numThreads;
    if ((chunkSize % chunkMultiple) == 0) {
      for (size_t i = 0, j = 0; i < length; i += chunkSize, ++j) {
        mWorkerControl[j].begin = i;
        mWorkerControl[j].end = i + chunkSize;
        mControl.remainingJobs++;
      }
    }
    else {
      size_t multiple = RoundDownNearestMultiple(chunkSize, chunkMultiple);
      size_t i = 0;
      for (size_t j = 0; j < numThreads; i += multiple, ++j) {
        mWorkerControl[j].begin = i;

        if (j == numThreads - 1) {
          mWorkerControl[j].end = length;
        }
        else {
          mWorkerControl[j].end = i + multiple;
        }

        mControl.remainingJobs++;
      }
    }
  }

  mControl.data = data;
  mControl.func = range;

  if (!async) {
    while (mControl.remainingJobs > 0) {
      PlatformBusySpin();
    }
  }
}

}
