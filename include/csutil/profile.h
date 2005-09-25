/*
    Copyright (C) 2005 by Jorrit Tyberghein

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

#ifndef __CS_UTIL_PROFILE_H__
#define __CS_UTIL_PROFILE_H__

/**\file
 * Profiling utilities.
 */

#include "csextern.h"
#include "csutil/csstring.h"
#include "csutil/array.h"

#ifdef CS_DO_PROFILING

#include <sys/time.h>

SCF_VERSION (iProfiler, 0, 0, 1);

struct iProfiler : public iBase
{
  virtual void RegisterProfilePoint (const char* token,
  	const char* file, int line,
  	uint32* ptr_count, uint32* ptr_time,
	uint32* ptr_timemin, uint32* ptr_timemax) = 0;
  virtual void Dump () = 0;
  virtual void Reset () = 0;
};

struct csProfileInfo
{
  const char* token;
  const char* file;
  int line;
  uint32* ptr_count;
  uint32* ptr_time;
  uint32* ptr_timemin;
  uint32* ptr_timemax;
};

class csProfiler : public iProfiler
{
public:
  csArray<csProfileInfo> profile_info;

public:
  csProfiler ();
  virtual ~csProfiler ();

  SCF_DECLARE_IBASE;
  virtual void RegisterProfilePoint (const char* token,
  	const char* file, int line,
  	uint32* ptr_count, uint32* ptr_time,
	uint32* ptr_timemin, uint32* ptr_timemax);
  virtual void Dump ();
  virtual void Reset ();
};

#if 1
#define CS_PROFTIME(v) \
v = csGetTicks()
#else
#define CS_PROFTIME(v) \
{\
struct timeval tv;\
gettimeofday(&tv, 0);\
v = tv.tv_sec + tv.tv_usec*1000000;\
}
#endif

#define CS_PROFRESET(obj_reg) \
{ \
csRef<iProfiler> profiler = CS_QUERY_REGISTRY (obj_reg, iProfiler); \
if (profiler) profiler->Reset (); \
}

#define CS_PROFDUMP(obj_reg) \
{ \
csRef<iProfiler> profiler = CS_QUERY_REGISTRY (obj_reg, iProfiler); \
if (profiler) profiler->Dump (); \
}

#define CS_PROFSTART(tok,obj_reg) \
static bool tok##__prof__init = false; \
static uint32 tok##__prof__cnt = 0; \
static uint32 tok##__prof__time = 0; \
static uint32 tok##__prof__timemin = 1000000000; \
static uint32 tok##__prof__timemax = 0; \
if (!tok##__prof__init) \
{ \
  tok##__prof__init = true; \
  csRef<iProfiler> profiler = CS_QUERY_REGISTRY (obj_reg, iProfiler); \
  if (!profiler) \
  { \
    profiler.AttachNew (new csProfiler ()); \
    obj_reg->Register (profiler, "iProfiler"); \
  } \
  if (profiler) \
    profiler->RegisterProfilePoint (#tok,__FILE__, __LINE__, &tok##__prof__cnt, &tok##__prof__time, &tok##__prof__timemin, &tok##__prof__timemax); \
} \
uint32 tok##__prof__starttime; \
CS_PROFTIME(tok##__prof__starttime)

#define CS_PROFSTOP(tok) \
{ \
uint32 prof__endtime; \
CS_PROFTIME(prof__endtime); \
uint32 prof__dt = prof__endtime - tok##__prof__starttime; \
if (prof__dt < tok##__prof__timemin) tok##__prof__timemin = prof__dt; \
if (prof__dt > tok##__prof__timemax) tok##__prof__timemax = prof__dt; \
tok##__prof__time += prof__dt; \
} \
tok##__prof__cnt++

#else

#define CS_PROFRESET(obj_reg)
#define CS_PROFDUMP(obj_reg)
#define CS_PROFSTART(tok,obj_reg)
#define CS_PROFSTOP(tok)

#endif

#endif //__CS_UTIL_PROFILE_H__

