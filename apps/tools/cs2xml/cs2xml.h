/*
    Copyright (C) 2002 by Jorrit Tyberghein

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

#ifndef __CS2XML_H
#define __CS2XML_H

#include <stdarg.h>
#include "csutil/ref.h"

struct iVFS;
struct iCommandLineParser;
struct iObjectRegistry;
struct iFile;
struct iDocumentNode;
class csParser;

/**
 * Main class.
 */
class Cs2Xml
{
public:
  iObjectRegistry* object_reg;
  csRef<iVFS> vfs;
  csRef<iCommandLineParser> cmdline;

  void ReportError (const char* description, ...);

  void ParseGeneral (const char* parent_token,
  	csParser* parser, csRef<iDocumentNode>& parent, char* buf);

  bool IsEmpty (const char* in);
  bool IsNumeric (const char* in);
  bool IsString (const char* in);
  bool IsBoolean (const char* in, bool& val);
  int IsNumberList (const char* in);
  // If remove_under is true, underscores are removed.
  char* ToLower (const char* in, bool remove_under);

  csRef<iDocumentNode> CreateValueNode (csRef<iDocumentNode>& parent,
  	const char* name, const char* value);
  csRef<iDocumentNode> CreateValueNodeAsInt (csRef<iDocumentNode>& parent,
  	const char* name, int value);
  csRef<iDocumentNode> CreateValueNodeAsFloat (csRef<iDocumentNode>& parent,
  	const char* name, float value);

  void ParseMatrix (csParser *parser, csRef<iDocumentNode>& parent,
  	char* buf);

public:
  Cs2Xml (iObjectRegistry* object_reg);
  ~Cs2Xml ();

  void Main ();
};

#endif // __CS2XML_H

