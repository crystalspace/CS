/*
    Copyright (C) 2004 by Jorrit Tyberghein
	      (C) 2004 by Frank Richter

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

#ifndef __CS_CSTOOL_DEBUGIMAGEWRITER_H__
#define __CS_CSTOOL_DEBUGIMAGEWRITER_H__

/**\file
 * Image object debugging helper
 */

#include "csutil/csstring.h"
#include "csutil/scf.h"
#include "igraphic/imageio.h"
#include "iutil/databuff.h"
#include "iutil/vfs.h"
#include "ivaria/reporter.h"

/**
 * Helper class allowing in debug builds for to quickly dump an iImage 
 * object onto disk for visual inspection.
 */
class CS_CRYSTALSPACE_EXPORT csDebugImageWriter
{
  static void Report (int severity, const char* msg, ...)
  {
#ifdef CS_DEBUG
    va_list arg;
    va_start (arg, msg);
    csReportV (iSCF::SCF->object_reg, severity, "crystalspace.debugimagewriter",
      msg, arg);
    va_end (arg);
#endif
  }
public:
  /**
   * Write an image onto disk.
   * \param image The image to be written.
   * \param filename The VFS name of the file to be written.
   * \remarks \a filename is a string with printf() style format specifiers.
   * \remarks If \a filename does not contain an absolute VFS path, it will
   *  be treated as relative to "/tmp/".
   * \sa \ref FormatterNotes
   */
  static void DebugImageWrite (iImage* image, const char* filename, ...)
  {
  #ifdef CS_DEBUG
    CS_ASSERT(iSCF::SCF->object_reg);
    csRef<iImageIO> imgsaver =
      CS_QUERY_REGISTRY (iSCF::SCF->object_reg, iImageIO);
    if (!imgsaver) 
    {
      Report (CS_REPORTER_SEVERITY_ERROR, "No iImageIO");
      return;
    }
    csRef<iVFS> vfs =
      CS_QUERY_REGISTRY (iSCF::SCF->object_reg, iVFS);
    if (!vfs) 
    {
      Report (CS_REPORTER_SEVERITY_ERROR, "No iVFS");
      return;
    }
    
    csString finalFilename;
    va_list arg;
    va_start (arg, filename);
    finalFilename.FormatV (filename, arg);
    va_end (arg);
    
    csRef<iDataBuffer> buf = imgsaver->Save (image, "image/png");
    if (!buf) 
    {
      Report (CS_REPORTER_SEVERITY_ERROR, "Error saving image");
      return;
    }
    vfs->PushDir ();
    vfs->ChDir ("/tmp");
    bool written = vfs->WriteFile (finalFilename, (char*)buf->GetInt8 (), 
      buf->GetSize ());
    vfs->PopDir ();
    if (!written)
    {
      Report (CS_REPORTER_SEVERITY_ERROR, "Could not write to %s", 
	finalFilename.GetData ());
    }
  #endif
  }
};

#endif // __CS_CSTOOL_DEBUGIMAGEWRITER_H__
