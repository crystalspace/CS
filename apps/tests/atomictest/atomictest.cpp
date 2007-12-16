/*
  Copyright (C) 2007 by Marten Svanfeldt

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
#include "csutil/sysfunc.h"
#include "csutil/threading/atomicops.h"
#include "csutil/threading/barrier.h"
#include "csutil/threading/thread.h"

CS_IMPLEMENT_APPLICATION

// Helper shared between threads
CS::Threading::Barrier syncBarrier2Start (3);
CS::Threading::Barrier syncBarrier4Start (5);

CS::Threading::Barrier syncBarrier2Stop (3);
CS::Threading::Barrier syncBarrier4Stop (5);

size_t dataSize = 0; 
size_t runs = 0;
volatile int32* dataBuffer = 0;


// Helper method to try to make sure the cache is flushed
// Allocate, write & read 4MB of data
static void CacheFlush ()
{
  const size_t size = 4*1024*1024;

  volatile char* data = (volatile char*)cs_malloc (size);

  for (size_t i = 0; i < size; ++i)
  {
    data[i] = 0xAA;
  }

  for (size_t i = 0; i < size; ++i)
  {
    data[i] ^= 0xFF;
  }

  for (size_t i = 0; i < size; ++i)
  {
    data[i] ^= 0x12;
  }

  cs_free ((void*)data);
}


//--- Methods for regular operations
static void DoAddition ()
{
  for (size_t j = 0; j < runs; ++j)
  {
    for (size_t i = 0; i < dataSize; ++i)
    {
      dataBuffer[i] += 1;
    }
  }
}

static void DoSubtraction ()
{
  for (size_t j = 0; j < runs; ++j)
  {
    for (size_t i = 0; i < dataSize; ++i)
    {
      dataBuffer[i] -= 1;
    }
  }
}


class AddRunner2 : public CS::Threading::Runnable
{
  virtual void Run ()
  {
    syncBarrier2Start.Wait ();
    DoAddition ();
    syncBarrier2Stop.Wait ();
  }
};

class AddRunner4 : public CS::Threading::Runnable
{
  virtual void Run ()
  {
    syncBarrier4Start.Wait ();
    DoAddition ();
    syncBarrier4Stop.Wait ();
  }
};

static void TestRegular ()
{
  csPrintf ("Regular operations\n");
  int64 diffTime;
  // Cache flush to start with, and then between each test

  // Non-threaded
  CacheFlush ();
  csPrintf ("Test 1: Addition, single threaded: ");
  diffTime = csGetMicroTicks ();
    DoAddition ();
  diffTime -= csGetMicroTicks ();
  csPrintf ("%" PRId64 "\n", -diffTime);

  CacheFlush ();
  csPrintf ("Test 2: Subtraction, single threaded: ");
  diffTime = csGetMicroTicks ();
    DoSubtraction ();
  diffTime -= csGetMicroTicks ();
  csPrintf ("%"PRId64 "\n", -diffTime);

  // Threaded, 2 threads
  {
    AddRunner2 runner1, runner2;
    CS::Threading::Thread thread1 (&runner1, true);
    CS::Threading::Thread thread2 (&runner2, true);

    CacheFlush ();
    csPrintf ("Test 3: Addition, 2 threads: ");
    diffTime = csGetMicroTicks ();
    syncBarrier2Start.Wait (); // Start the work
    syncBarrier2Stop.Wait (); //Wait for it to finish
    diffTime -= csGetMicroTicks ();
    csPrintf ("%" PRId64 "\n", -diffTime);
  }

  // Threaded, 4 threads
  {
    AddRunner4 runner1, runner2, runner3, runner4;
    CS::Threading::Thread thread1 (&runner1, true);
    CS::Threading::Thread thread2 (&runner2, true);
    CS::Threading::Thread thread3 (&runner3, true);
    CS::Threading::Thread thread4 (&runner4, true);

    CacheFlush ();
    csPrintf ("Test 4: Addition, 4 threads: ");
    diffTime = csGetMicroTicks ();
    syncBarrier4Start.Wait (); // Start the work
    syncBarrier4Stop.Wait (); //Wait for it to finish
    diffTime -= csGetMicroTicks ();
    csPrintf ("%" PRId64 "\n", -diffTime);
  }
};


//-- Methods for atomic operations
static void DoAtomicAddition ()
{
  for (size_t j = 0; j < runs; ++j)
  {
    for (size_t i = 0; i < dataSize; ++i)
    {
      CS::Threading::AtomicOperations::Increment ((int32*)&dataBuffer[i]);
    }
  }
}

static void DoAtomicSubtraction ()
{
  for (size_t j = 0; j < runs; ++j)
  {
    for (size_t i = 0; i < dataSize; ++i)
    {
      CS::Threading::AtomicOperations::Decrement ((int32*)&dataBuffer[i]);
    }
  }
}

class AddAtomicRunner2 : public CS::Threading::Runnable
{
  virtual void Run ()
  {
    syncBarrier2Start.Wait ();
    DoAtomicAddition ();
    syncBarrier2Stop.Wait ();
  }
};

class AddAtomicRunner4 : public CS::Threading::Runnable
{
  virtual void Run ()
  {
    syncBarrier4Start.Wait ();
    DoAtomicSubtraction ();
    syncBarrier4Stop.Wait ();
  }
};

static void TestAtomic ()
{
  csPrintf ("Atomic operations\n");
  int64 diffTime;
  // Cache flush to start with, and then between each test

  // Non-threaded
  CacheFlush ();
  csPrintf ("Test 1: Addition, single threaded: ");
  diffTime = csGetMicroTicks ();
    DoAtomicAddition ();
  diffTime -= csGetMicroTicks ();
  csPrintf ("%" PRId64 "\n", -diffTime);

  CacheFlush ();
  csPrintf ("Test 2: Subtraction, single threaded: ");
  diffTime = csGetMicroTicks ();
    DoAtomicSubtraction ();
  diffTime -= csGetMicroTicks ();
  csPrintf ("%"PRId64 "\n", -diffTime);

  // Threaded, 2 threads
  {
    AddAtomicRunner2 runner1, runner2;
    CS::Threading::Thread thread1 (&runner1, true);
    CS::Threading::Thread thread2 (&runner2, true);

    CacheFlush ();
    csPrintf ("Test 3: Addition, 2 threads: ");
    diffTime = csGetMicroTicks ();
    syncBarrier2Start.Wait (); // Start the work
    syncBarrier2Stop.Wait (); //Wait for it to finish
    diffTime -= csGetMicroTicks ();
    csPrintf ("%" PRId64 "\n", -diffTime);
  }

  // Threaded, 4 threads
  {
    AddAtomicRunner4 runner1, runner2, runner3, runner4;
    CS::Threading::Thread thread1 (&runner1, true);
    CS::Threading::Thread thread2 (&runner2, true);
    CS::Threading::Thread thread3 (&runner3, true);
    CS::Threading::Thread thread4 (&runner4, true);

    CacheFlush ();
    csPrintf ("Test 4: Addition, 4 threads: ");
    diffTime = csGetMicroTicks ();
    syncBarrier4Start.Wait (); // Start the work
    syncBarrier4Stop.Wait (); //Wait for it to finish
    diffTime -= csGetMicroTicks ();
    csPrintf ("%" PRId64 "\n", -diffTime);
  }
  
};

//-- Comparison methods

static void DoMultiplication ()
{
  for (size_t j = 0; j < runs; ++j)
  {
    for (size_t i = 0; i < dataSize; ++i)
    {
      dataBuffer[i] *= 37;
    }
  }
}

static void DoDivision ()
{
  for (size_t j = 0; j < runs; ++j)
  {
    for (size_t i = 0; i < dataSize; ++i)
    {
      dataBuffer[i] /= 83;
    }
  }
}

volatile uint32 global;

static void DoMemoryRead ()
{
  for (size_t j = 0; j < runs; ++j)
  {
    for (size_t i = 0; i < dataSize; ++i)
    {
      global = dataBuffer[i];
    }
  }
}

static void DoFold ()
{  
  for (size_t j = 0; j < runs; ++j)
  {
    for (size_t i = 0; i < dataSize; ++i)
    {
      // Simple hash fold
      uint32 fold;
      uint32 data = dataBuffer[i];
      fold = (data & 0xFF);
      fold ^= ((data >> 8) & 0xFF);
      fold ^= ((data >> 16) & 0xFF);
      fold ^= ((data >> 24) & 0xFF);
      global = fold;
    }
  }
}

static void TestCompare ()
{
  csPrintf ("Test comparison\n");
  int64 diffTime;
  // Cache flush to start with, and then between each test

  // Non-threaded
  CacheFlush ();
  csPrintf ("Test 1: Multiplication, single threaded: ");
  diffTime = csGetMicroTicks ();
    DoMultiplication ();
  diffTime -= csGetMicroTicks ();
  csPrintf ("%" PRId64 "\n", -diffTime);

  CacheFlush ();
  csPrintf ("Test 2: Division, single threaded: ");
  diffTime = csGetMicroTicks ();
    DoDivision ();
  diffTime -= csGetMicroTicks ();
  csPrintf ("%" PRId64 "\n", -diffTime);

  CacheFlush ();
  csPrintf ("Test 3: Single memory read, single threaded: ");
  diffTime = csGetMicroTicks ();
    DoMemoryRead ();
  diffTime -= csGetMicroTicks ();
  csPrintf ("%" PRId64 "\n", -diffTime);

  CacheFlush ();
  csPrintf ("Test 4: XOR fold, single threaded: ");
  diffTime = csGetMicroTicks ();
    DoFold ();
  diffTime -= csGetMicroTicks ();
  csPrintf ("%" PRId64 "\n", -diffTime);
}

// Main runner
static void RunTests(size_t memSize)
{
  dataSize = memSize / sizeof(int32);
  runs = 1024*1024*512/dataSize;

  // Allocate the data buffer to use
  dataBuffer = (volatile int32*)cs_malloc (dataSize*4);

  // Run tests  
  TestRegular ();
  TestAtomic ();
  TestCompare ();
}


/*---------------------------------------------------------------------*
 * Main function
 *---------------------------------------------------------------------*/
int main (int argc, char* argv[])
{
  csPrintf ("ATOMIC OPERATIONS OVERHEAD TESTER\n");
  csPrintf ("Use optimized builds, send report of output to developer@svanfeldt.com\n\n");
  csPrintf ("Config: %s(%d) %s %s \n\n", CS_PROCESSOR_NAME, CS_PROCESSOR_SIZE, 
    CS_PLATFORM_NAME, CS_COMPILER_NAME);

  // Just make sure this is initialized
  csGetMicroTicks ();

  csPrintf ("MemSize: 2kB L1 hits\n");
  RunTests (2*1024);
  csPrintf ("\n");

  csPrintf ("MemSize: 512kB L2 hits\n");
  RunTests (512*1024);
  csPrintf ("\n");

  csPrintf ("MemSize: 32MB Memory hits\n");
  RunTests (32*1024*1024);
  csPrintf ("\n");

  return 0;
}


