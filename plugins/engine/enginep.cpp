/*
    Copyright (C) 2000 by Jorrit Tyberghein

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
#include "plugins/engine/enginep.h"
#include "csengine/world.h"

IMPLEMENT_IBASE (csEngine)
  IMPLEMENTS_INTERFACE (iEngine)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_FACTORY (csEngine)

EXPORT_CLASS_TABLE (enginep)
  EXPORT_CLASS (csEngine, "crystalspace.engine.plugin", 
		"CrystalSpace Engine Plugin" )
EXPORT_CLASS_TABLE_END

csEngine::csEngine (iBase* parent)
{
  CONSTRUCT_IBASE (parent);
  world = NULL;
}

csEngine::~csEngine ()
{
  delete world;
}

bool csEngine::Initialize (iSystem* sys)
{
  time_t t = sys->GetTime ();
  world = new csWorld (NULL);
  world->Initialize (sys);
  return true;
}

