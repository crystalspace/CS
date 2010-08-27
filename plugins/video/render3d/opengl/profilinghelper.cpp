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

#include "cssysdef.h"

#if defined(CS_OPENGL_PATH)
#include CS_HEADER_GLOBAL(CS_OPENGL_PATH,gl.h)
#else
#include <GL/gl.h>
#endif

#include "profilinghelper.h"

#include "iutil/vfs.h"

#include "csutil/csstring.h"

#include "gl_render3d.h"
#include "querypool.h"

CS_PLUGIN_NAMESPACE_BEGIN(gl3d)
{
  ProfilingHelper::ProfilingHelper () : stampOffset (0) {}
    
  void ProfilingHelper::SetFile (iFile* file)
  {
    outputFile = file;
    if (outputFile)
    {
      static const char header[] = "Frame,Issued,Latency,Duration,Description\n";
      outputFile->Write (header, strlen (header));
    }
  }

  void ProfilingHelper::ResetStampOffset ()
  {
    csGLGraphics3D::ext->glGetInteger64v (GL_TIMESTAMP, &stampOffset);
    int64 wallTime = csGetMicroTicks();
    stampOffsetWall = wallTime;
  }
  
  void ProfilingHelper::RecordEvent (uint frameNum, uint64 issued, GLuint event,
				    const char* descr)
  {
    events.Push (ProfileEvent (frameNum, descr, issued, event));
  }

  void ProfilingHelper::RecordTimeSpan (uint frameNum, uint64 issued,
					GLuint begin, GLuint end, const char* descr)
  {
    events.Push (ProfileEvent (frameNum, descr, issued, begin, end));
  }

  void ProfilingHelper::FlushEvents (uint newFrame, QueryPool& queries, uint maxDelta)
  {
    CS_ASSERT(outputFile);
    
    while (events.GetSize() > 0)
    {
      if ((newFrame - events.Top().frame) < maxDelta) break;
      
      ProfileEvent event (events.PopTop());
      
      uint64 issueTime = (event.issued - stampOffsetWall)*1000;
      uint64 startTime, endTime;
      csGLGraphics3D::ext->glGetQueryObjectui64v (event.query1, GL_QUERY_RESULT, &startTime);
      queries.RecycleQuery (event.query1);
      startTime -= stampOffset;
      if (event.isSingular)
      {
	endTime = startTime;
      }
      else
      {
	csGLGraphics3D::ext->glGetQueryObjectui64v (event.query2, GL_QUERY_RESULT, &endTime);
	endTime -= stampOffset;
	queries.RecycleQuery (event.query2);
      }

      csString line;
      line.Format ("%u,%" CS_PRIu64 ",%" CS_PRIu64 ",%" CS_PRIu64 ",%s\n",
		   event.frame, issueTime,
		   startTime - issueTime, endTime - startTime, event.descr);
      outputFile->Write (line.GetData(), line.Length());
    }
  }

}
CS_PLUGIN_NAMESPACE_END(gl3d)
