/*
  Copyright (C) 2008 by Michael Gist

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
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
#include "csutil/platform.h"
#include "csutil/threadmanager.h"

ThreadManager* ThreadManager::singletonPtr = NULL;

ThreadManager::ThreadManager()
{
  CS_ASSERT(singletonPtr == NULL);

  uint count = CS::Platform::GetProcessorCount();

  // If we can't detect, assume we have one.
  if(count == 0)
  {
    count = 1;
  }

  // Have 'processor count' extra threads.
  queue.AttachNew(new ThreadedJobQueue(count));
}

ThreadManager* ThreadManager::GetThreadManager()
{
  if(singletonPtr == 0)
  {
    singletonPtr = new ThreadManager();
  }
  return singletonPtr;
}
