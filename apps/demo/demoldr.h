/*
    Copyright (C) 2001 by Jorrit Tyberghein

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

#ifndef __DEMOLDR_H__
#define __DEMOLDR_H__

#include <stdarg.h>
#include "csgeom/math2d.h"
#include "csgeom/math3d.h"
#include "csgeom/path.h"
#include "csutil/parray.h"
#include "csutil/strhash.h"
#include "iutil/document.h"
#include "ivaria/sequence.h"

class Demo;
class DemoSequenceManager;
class csNamedPath;

struct NamedSequence
{
  char* name;
  csRef<iSequence> sequence;
  NamedSequence () : name (0) { }
  ~NamedSequence () { delete[] name; }
};

/**
 * The sequence loader.
 */
class DemoSequenceLoader
{
private:
  csPDelArray<NamedSequence> sequences;
  iSequenceManager* seqmgr;
  Demo* demo;
  DemoSequenceManager* demoseq;
  csStringHash xmltokens;

  csNamedPath* LoadPath (iDocumentNode* node, const char* name);
  void LoadSequence (iDocumentNode* node, iSequence* seq);
  void LoadSequences (iDocumentNode* node);
  void LoadSequencesMain (iDocumentNode* node);

  bool ParseVectorList (iDocumentNode* node, csVector3* list, int num);

public:
  DemoSequenceLoader (Demo* demo, DemoSequenceManager* demoseq,
  	iSequenceManager* seqmgr, const char* fileName);
  ~DemoSequenceLoader ();
  iSequence* GetSequence (const char* name);
};

#endif // __DEMOLDR_H__

