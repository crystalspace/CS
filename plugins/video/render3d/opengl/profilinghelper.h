/*
  Copyright (C) 2010 by Frank Richter

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

#ifndef __PROFILINGHELPER_H__
#define __PROFILINGHELPER_H__

#include "csutil/fifo.h"
#include "csutil/ref.h"

struct iFile;

CS_PLUGIN_NAMESPACE_BEGIN(gl3d)
{
  class QueryPool;
  
  class ProfilingHelper
  {
    csRef<iFile> outputFile;
    
    int64 stampOffset;
    int64 stampOffsetWall;
    struct ProfileEvent
    {
      uint frame;
      const char* descr;
      bool isSingular;
      uint64 issued;
      GLuint query1;
      GLuint query2;
      
      ProfileEvent (uint frame, const char* descr, uint64 issued, GLuint event)
       : frame (frame), descr (descr), isSingular (true), issued (issued),
         query1 (event), query2 (0) {}
      ProfileEvent (uint frame, const char* descr, uint64 issued,
		    GLuint begin, GLuint end)
       : frame (frame), descr (descr), isSingular (false), issued (issued),
         query1 (begin), query2 (end) {}
    };
    csFIFO<ProfileEvent,
	   csArrayElementHandler<ProfileEvent>,
	   CS::Container::ArrayAllocDefault,
	   csArrayCapacityFixedGrow<512> > events;
  public:
    ProfilingHelper ();
    
    void SetFile (iFile* file);
    
    void ResetStampOffset ();
    void RecordEvent (uint frameNum, uint64 issuedAtWallTime, GLuint event,
		      const char* descr);
    void RecordTimeSpan (uint frameNum, uint64 issuedAtWallTime,
			 GLuint begin, GLuint end, const char* descr);
    
    void FlushEvents (uint newFrame, QueryPool& queries, uint maxDelta);
  };
}
CS_PLUGIN_NAMESPACE_END(gl3d)

#endif // __PROFILINGHELPER_H__
