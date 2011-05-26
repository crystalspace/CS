/*
  Copyright (C) 2011 by Matthieu Kraus

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

// type specific data
#define element_t float
#define compare(a,b) (a > b)

// check whether async_strided_copy is available
#ifdef CL_VERSION_1_1
#  define USE_STRIDED_COPY
#endif

// helper macro to obtain left/right id for a pass
#define GetIds(dist,id)\
uint leftId;\
uint rightId;\
{\
  const uint mask = ~0 << dist;\
  leftId = (id & ~mask)\
         | ((id & mask) << 1);\
\
  rightId = leftId | (1 << dist);\
}

// helper function to obtain the mask a work group will use
uint GetIdMask(const uint id,
               const uint endDist,
               const uint passes)
{
  const uint startDist = endDist+passes;
  const uint mask = ~(~0 << endDist);
  const uint invMask = ~0 << startDist;

  uint result = (id & mask)
              | ((id & invMask) << passes);
  return result;
}

// performs multiple passes at once
// fetches 2^passes elements into local memory
// instead of only using private and global memory
// startPass+passes must be <= stage
// work group size must be 2^(passes-1)
// lArray size must be 2^passes
__kernel
void bitonicSortBatch(__global element_t* gArray,
                      __local element_t* lArray, // must be 2*local size
                      const uint stage,
                      const uint startPass,
                      const uint passes,
                      const uint direction)
{
  const uint lId = get_local_id(0);
  const uint gId = get_global_id(0);
  const int dist = stage - startPass - passes;
  const uint mask = GetIdMask(stage, dist, passes);

#ifndef USE_STRIDED_COPY
  const uint gIdLeft = mask | (lId << (dist+1));
  const uint gIdRight = gIdLeft | (1 << dist);
  const uint lIdLeft = lId << 1;
  const uint lIdRight = lIdLeft|1;
#else
  const uint width = get_local_size(0) << 1;
#endif

#ifdef USE_STRIDED_COPY
  event_t event = async_work_group_strided_copy(
                         lArray,
                         gArray+mask,
                         width,
                         1 << dist,
                         0);
  wait_group_events(1, &event);
#else
  lArray[lIdLeft] = gArray[gIdLeft];
  lArray[lIdRight] = gArray[gIdRight];

  barrier(CLK_LOCAL_MEM_FENCE);
#endif

  uint inc = direction ^ ((gId >> stage) & 1);

  for(int passDist = passes; passDist >= 0; --passDist)
  {
    GetIds(passDist,lId);

    const element_t left = lArray[leftId];
    const element_t right = lArray[rightId];

    if(inc == compare(left,right))
    {
      lArray[rightId] = left;
      lArray[leftId] = right;
    }

    barrier(CLK_LOCAL_MEM_FENCE);
  }

#ifdef USE_STRIDED_COPY
  async_work_group_strided_copy(gArray+mask,
                                lArray,
                                width,
                                1 << dist,
                                0);
#else
  gArray[gIdLeft] = lArray[lIdLeft];
  gArray[gIdRight] = lArray[lIdRight];
#endif
}

// performs a single sort pass
__kernel 
void bitonicSort(__global element_t* array,
                 const uint stage,
                 const uint dist, // stage-pass
                 const uint direction)
{
  const uint id = get_global_id(0);
  const uint inc = direction ^ ((id >> stage) & 1);

  GetIds(dist,id);

  const element_t leftElement = array[leftId];
  const element_t rightElement = array[rightId];

  if(inc == compare(leftElement,rightElement))
  {
    array[rightId] = leftElement;
    array[leftId]  = rightElement;
  }
}
