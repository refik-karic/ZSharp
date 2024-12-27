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
      // Don't let anyone modify the job queue while we're inspecting it
      // Use a separate monitor for testing when the job is complete or not
      // This lets us add/remove jobs to the queue but also know whether a thread is still executing a final job before sleeping
      PlatformClearMonitor(workerControl.jobMonitor);
      workerControl.jobLock.Aquire();

      if (!workerControl.jobs.IsEmpty()) {
        ThreadJob job(*(workerControl.jobs.begin()));
        workerControl.jobs.RemoveFront();
        workerControl.jobLock.Release();

        job.func(job.data);
        PlatformSignalMonitor(workerControl.jobMonitor);
      }
      else {
        workerControl.jobLock.Release();
        PlatformSignalMonitor(workerControl.jobMonitor);
      }

      control.lock.Aquire();

      if (control.status == ThreadControl::RunStatus::SLEEP) {
        control.lock.Release();
        continue;
      }

      size_t remainingJobs = 0;

      for (WorkerThreadControl& worker : control.workers) {
        remainingJobs += worker.jobs.Size();
      }

      if (remainingJobs == 0) {
        // Jobs that have been pop'd off the queue may still be running, wait until they're complete.
        for (WorkerThreadControl& worker : control.workers) {
          PlatformWaitMonitor(worker.jobMonitor);
        }

        PlatformClearMonitor(control.monitor);
        control.status = ThreadControl::RunStatus::SLEEP;
        PlatformSignalMonitor(control.asyncMonitor);
      }

      control.lock.Release();

      if (remainingJobs > 0) {
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
  size_t numCores = PlatformGetNumPhysicalCores();
  mPool.Resize(numCores);

  mControl.monitor = PlatformCreateMonitor();
  mControl.asyncMonitor = PlatformCreateMonitor();
  mControl.workers.Resize(numCores);

  for (size_t i = 0; i < numCores; ++i) {
    WorkerThreadControl& control = mControl.workers[i];
    control.masterControl = &mControl;
    control.id = i;
    control.jobMonitor = PlatformCreateMonitor();
  }

  for (size_t i = 0; i < numCores; ++i) {
    mPool[i] = PlatformCreateThread(&BackgroundWorker, &(mControl.workers[i]));
  }

  PlatformPinThreadsToProcessors(mPool.GetData(), mPool.Size(), false);
  PlatformSignalMonitor(mControl.asyncMonitor);
}

ThreadPool::~ThreadPool() {
  mControl.status = ThreadControl::RunStatus::END;
  PlatformSignalMonitor(mControl.monitor);

  PlatformJoinThreadPool(mPool.GetData(), mPool.Size());

  for (WorkerThreadControl& control : mControl.workers) {
    PlatformDestroyMonitor(control.jobMonitor);
  }

  PlatformDestroyMonitor(mControl.monitor);
  PlatformDestroyMonitor(mControl.asyncMonitor);
}

void ThreadPool::Wake() {
  mControl.status = ThreadControl::RunStatus::RUNNING;

  PlatformSignalMonitor(mControl.monitor);
  PlatformClearMonitor(mControl.asyncMonitor);
}

void ThreadPool::Sleep() {
  mControl.status = ThreadControl::RunStatus::SLEEP;
}

void ThreadPool::WaitForJobs() {
  PlatformWaitMonitor(mControl.asyncMonitor);
}

void ThreadPool::Execute(ParallelRange& range, void* data, size_t length) {
  size_t numThreads = mPool.Size();
  
  if (length == 0) {
    return;
  }

  // If we can't evenly distribute the work among threads, throw everything on one worker thread.
  if (length < numThreads) {
    Span<uint8> threadData((uint8*)data, length);
    ThreadJob job;
    job.data = threadData;
    job.func = range;

    WorkerThreadControl& worker = mControl.workers[0];
    worker.jobLock.Aquire();
    worker.jobs.Add(job);
    worker.jobLock.Release();
  }
  else {
    size_t remainder = length % numThreads;
    size_t chunkSize = (size_t)floorf((float)length / (float)numThreads);

    for (size_t i = 0, j = 0; j < numThreads; ++j) {
      size_t nextChunk = chunkSize;

      if (j < remainder) {
        ++nextChunk;
      }

      Span<uint8> threadData(((uint8*)data) + i, nextChunk);
      ThreadJob job;
      job.data = threadData;
      job.func = range;
      WorkerThreadControl& worker = mControl.workers[j];
      worker.jobLock.Aquire();
      worker.jobs.Add(job);
      worker.jobLock.Release();

      i += nextChunk;
    }
  }

  Wake();
}

}
