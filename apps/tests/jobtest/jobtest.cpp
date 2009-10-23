/*
  Copyright (C) 2009 by Marten Svanfeldt

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "cssysdef.h"
#include "cstool/initapp.h"

#include "csutil/threadjobqueue.h"

using namespace CS::Threading;

CS_IMPLEMENT_APPLICATION

enum
{
  NUM_TIMES = 4,
  MAX_WORKER_THREADS = 8,
  WORK_UNIT_STEPS = 8
};

int64 BenchResult[MAX_WORKER_THREADS][WORK_UNIT_STEPS] = {{0}};

template<bool UseMemory>
void PerformSomeWork (void* membuff, size_t iterations = (1<<16))
{
  float* inbuff = (float*)membuff;
  float* outbuff = inbuff + iterations;

  volatile float tmpResult = 0.1423272f;
  float inData;

  for (size_t i = 0; i < iterations; ++i)
  {
    float result;

    if(UseMemory)
    {
      inData = inbuff[i];
    }
    else
    {
      inData = 0.8473f;
    }

    result = cosf (iterations * 0.01231f + inData);
    result *= iterations / (i+1);
    
    if(UseMemory)
    {
      outbuff[i] = result;
    }
    else
    {
      tmpResult += result;
    }

  }
}

template<bool useMemory>
class WorkerJob : public scfImplementation1<WorkerJob<useMemory>, iJob>
{
  typedef scfImplementation1<WorkerJob<useMemory>, iJob> Superclass;
public:
  WorkerJob(size_t it)
    : Superclass (this), iterations(it)
  { 
  }
  
  virtual void Run ()
  {
    for(size_t i = 0; i < iterations; ++i)
      PerformSomeWork<useMemory> (0, 1<<8);
  }

  size_t iterations;
};


unsigned int GetWorkUnits (unsigned int step)
{
  return 128 << step;
}

void RunBenchmark (unsigned int numThreads)
{
  csRef<iJob> job;

  // Setup a job queue
  csRef<iJobQueue> jobQueue;
  jobQueue.AttachNew (new ThreadedJobQueue(numThreads, THREAD_PRIO_NORMAL));

  // For each number of work units
  for (unsigned int i = 0; i < WORK_UNIT_STEPS; ++i)
  {
    unsigned int numWork = GetWorkUnits (i);
    int64 startTick = csGetMicroTicks ();

    // Input some jobs
    for (size_t k = 0; k < numWork/4; ++k)
    {
      for (size_t j = 1; j < 4; ++j)
      {
        int s = (int) (((double)rand() / (double)RAND_MAX)*6);
        job.AttachNew (new WorkerJob<false> (1 << s));
        jobQueue->Enqueue (job);
      }    
    }

    jobQueue->WaitAll ();

    int64 endTick = csGetMicroTicks ();
    BenchResult[numThreads - 1][i] += endTick - startTick;

    csPrintf(".");
  }  
}

void PrintResult ()
{
  csPrintf("\n");

  // Header
  csPrintf ("%6s", "WU");
  
  for (unsigned int numThreads = 1; numThreads <= MAX_WORKER_THREADS; ++numThreads)
  {
    csPrintf("%8d", numThreads);
  }
  csPrintf("\n");

  // For each number of WU
  for (unsigned int WU = 0; WU < WORK_UNIT_STEPS; ++WU)
  {
    csPrintf("%6d", GetWorkUnits(WU));

    // For each number of threads
    for (unsigned int t = 0; t < MAX_WORKER_THREADS; ++t)
    {
      csPrintf("%8" PRId64, BenchResult[t][WU] / NUM_TIMES);
    }

    csPrintf("\n");
  }
}

int main(int argc, char* argv[])
{
  csInitializer::InitializeSCF(argc, argv);

  srand(12341);

  for (unsigned int iter = 0; iter < NUM_TIMES; ++iter)
  {
    for (unsigned int i = 0; i < MAX_WORKER_THREADS; ++i)
    {
      RunBenchmark (i+1);
    }
  }
  
  PrintResult ();

  return 0;
}

