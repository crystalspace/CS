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

#ifndef DEMOLDR_H
#define DEMOLDR_H

#include <stdarg.h>
#include "cssys/sysdriv.h"
#include "csgeom/math2d.h"
#include "csgeom/math3d.h"
#include "csgeom/path.h"
#include "csutil/csvector.h"

struct iSequence;
struct iSequenceManager;
class Demo;
class DemoSequenceManager;
class csNamedPath;

struct NamedSequence
{
  char* name;
  iSequence* sequence;
};

/**
 * The sequence loader.
 */
class DemoSequenceLoader
{
private:
  csVector sequences;
  iSequenceManager* seqmgr;
  Demo* demo;
  DemoSequenceManager* demoseq;

  csNamedPath* LoadPath (char* buf, const char* name);
  void LoadSequence (char* buf, iSequence* seq);
  void LoadSequences (char* buf);
  void LoadSequencesMain (char* buf);

public:
  DemoSequenceLoader (Demo* demo, DemoSequenceManager* demoseq,
  	iSequenceManager* seqmgr, const char* fileName);
  ~DemoSequenceLoader ();
  iSequence* GetSequence (const char* name);
};

#endif // DEMOLDR_H

