/*
    Copyright (C) 2003 by Jorrit Tyberghein
	      (C) 2003 by Frank Richter

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

#ifndef __DOCCONV_H__
#define __DOCCONV_H__

#include <stdarg.h>
#include "igeom/polymesh.h"
#include "csgeom/vector3.h"
#include "csgeom/box.h"
#include "csutil/parray.h"

struct iVFS;
struct iCommandLineParser;
struct iObjectRegistry;
struct iFile;
struct iDocument;
struct iDocumentNode;
class csVector3;

/**
 * iDocumentNode wrapper.
 */
/*class dcDocNodeWrap
{
public:
  csRef<iDocumentNode> node;
};*/

class csString;

/**
 * Main class.
 */
class DocConv
{
public:
  iObjectRegistry* object_reg;
  csRef<iVFS> vfs;
  csRef<iCommandLineParser> cmdline;

  void ReportError (const char* description, ...);
  void Report (int severity, const char* description, ...);

  /**
   * Clone a node and children.
   */
  void CloneNode (iDocumentNode* from, iDocumentNode* to);

  //-----------------------------------------------------------------------

public:
  DocConv ();
  ~DocConv ();

  void Main ();
};

#endif // __DOCCONV_H__

