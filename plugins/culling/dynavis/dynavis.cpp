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

#include <string.h>
#include "cssysdef.h"
#include "csver.h"
#include "csutil/scf.h"
#include "csutil/util.h"
#include "csutil/scfstr.h"
#include "dynavis.h"
#include "kdtree.h"

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY (csDynaVis)

SCF_EXPORT_CLASS_TABLE (dynavis)
  SCF_EXPORT_CLASS (csDynaVis, "crystalspace.culling.dynavis",
    "Dynamic Visibility System")
SCF_EXPORT_CLASS_TABLE_END

SCF_IMPLEMENT_IBASE (csDynaVis)
  SCF_IMPLEMENTS_INTERFACE (iVisibilityCuller)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iDebugHelper)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csDynaVis::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csDynaVis::DebugHelper)
  SCF_IMPLEMENTS_INTERFACE (iDebugHelper)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csDynaVis::csDynaVis (iBase *iParent)
{
  SCF_CONSTRUCT_IBASE (iParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiDebugHelper);
  object_reg = NULL;
}

csDynaVis::~csDynaVis ()
{
}

bool csDynaVis::Initialize (iObjectRegistry *object_reg)
{
  csDynaVis::object_reg = object_reg;
  return true;
}

iString* csDynaVis::Debug_UnitTest ()
{
  csKDTree* kdtree = new csKDTree ();
  iDebugHelper* dbghelp = SCF_QUERY_INTERFACE (kdtree, iDebugHelper);
  if (dbghelp)
  {
    iString* rc = dbghelp->UnitTest ();
    dbghelp->DecRef ();
    if (rc)
    {
      delete kdtree;
      return rc;
    }
  }
  delete kdtree;

  return NULL;
}

iString* csDynaVis::Debug_StateTest ()
{
  return NULL;
}

iString* csDynaVis::Debug_Dump ()
{
  return NULL;
}

csTicks csDynaVis::Debug_Benchmark (int num_iterations)
{
  csKDTree* kdtree = new csKDTree ();
  iDebugHelper* dbghelp = SCF_QUERY_INTERFACE (kdtree, iDebugHelper);
  if (dbghelp)
  {
    csTicks rc = dbghelp->Benchmark (num_iterations);
    dbghelp->DecRef ();
    if (rc)
    {
      delete kdtree;
      return rc;
    }
  }
  delete kdtree;

  return 0;
}

