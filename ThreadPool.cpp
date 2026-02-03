#include "ThreadPool.h"

#include "CommonMath.h"
#include "PlatformHAL.h"
#include "PlatformMemory.h"

namespace ZSharp {

int32 BackgroundWorker(void* data) {
  WorkerThreadControl& workerControl = *((WorkerThreadControl*)data);
  // TODO: We expose the other worker queue's here so that in the future we can steal work if some threads finish before others.
  //ThreadControl& control = *(workerControl.masterControl);

  while (true) {
    if (workerControl.status == WorkerThreadControl::RunStatus::RUNNING) {
      // Don't let anyone modify the job queue while we're inspecting it
      workerControl.jobLock.Aquire();

      if (!workerControl.jobs.IsEmpty()) {
        ThreadJob job(*(workerControl.jobs.begin()));
        workerControl.jobs.RemoveFront();
        workerControl.jobLock.Release();

        job.func(job.data);
      }
      else {
        workerControl.jobLock.Release();
      }

      if (workerControl.jobs.IsEmpty()) {
        workerControl.status = WorkerThreadControl::RunStatus::SLEEP;
        PlatformClearMonitor(workerControl.runningMonitor);
      }
    }
    else if (workerControl.status == WorkerThreadControl::RunStatus::SLEEP) {
      PlatformSignalMonitor(workerControl.waitingMonitor);
      PlatformWaitMonitor(workerControl.runningMonitor);
      PlatformClearMonitor(workerControl.waitingMonitor);
    }
    else if (workerControl.status == WorkerThreadControl::RunStatus::END) {
      break;
    }
  }

  PlatformSignalMonitor(workerControl.waitingMonitor);

  return 0;
}

ThreadPool::ThreadPool() {
  size_t numCores = PlatformGetNumPhysicalCores();
  mPool.Resize(numCores);

  mControl.workers.Resize(numCores);

  for (size_t i = 0; i < numCores; ++i) {
    WorkerThreadControl& control = mControl.workers[i];
    control.masterControl = &mControl;
    control.id = i;
    control.runningMonitor = PlatformCreateMonitor(false);
    control.waitingMonitor = PlatformCreateMonitor(true);
  }

  for (size_t i = 0; i < numCores; ++i) {
    mPool[i] = PlatformCreateThread(&BackgroundWorker, &(mControl.workers[i]));
  }

  PlatformPinThreadsToProcessors(mPool.GetData(), mPool.Size(), false);
}

ThreadPool::~ThreadPool() {
  for (WorkerThreadControl& worker : mControl.workers) {
    PlatformWaitMonitor(worker.waitingMonitor);
    worker.status = WorkerThreadControl::RunStatus::END;
    PlatformSignalMonitor(worker.runningMonitor);
  }

  PlatformJoinThreadPool(mPool.GetData(), mPool.Size());

  for (WorkerThreadControl& control : mControl.workers) {
    PlatformDestroyMonitor(control.waitingMonitor);
    PlatformDestroyMonitor(control.runningMonitor);
  }
}

void ThreadPool::Wake() {
  for (WorkerThreadControl& worker : mControl.workers) {
    worker.status = WorkerThreadControl::RunStatus::RUNNING;
    PlatformSignalMonitor(worker.runningMonitor);
  }
}

void ThreadPool::Sleep() {
  for (WorkerThreadControl& worker : mControl.workers) {
    worker.status = WorkerThreadControl::RunStatus::SLEEP;
    PlatformClearMonitor(worker.runningMonitor);
  }
}

void ThreadPool::WaitForJobs() {
  Array<PlatformMonitor*> monitors(mControl.workers.Size());

  size_t numWaiting = 0;
  for (size_t i = 0; i < monitors.Size(); ++i) {
    WorkerThreadControl& worker = mControl.workers[i];
    if (worker.status == WorkerThreadControl::RunStatus::RUNNING) {
      monitors[numWaiting] = worker.waitingMonitor;
      ++numWaiting;
    }
  }

  // Only issue a true wait if we know there are still some threads running.
  // Most of the time the worker threads should be idle unless we're backed up.
  // Waiting for all the handles can be expensive, up to around 500us, so avoid it if we can.
  if (numWaiting > 0) {
    PlatformWaitMonitors(monitors.GetData(), numWaiting);
  }
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
