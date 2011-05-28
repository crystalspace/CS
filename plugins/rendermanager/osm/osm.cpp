/*
  Copyright (C) 2011 Alexandru - Teodor Voicu
      Imperial College London
      http://www3.imperial.ac.uk/

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

#include "osm.h"
#include "iengine.h"
#include "ivideo.h"
#include "ivaria/reporter.h"
#include "csplugincommon/rendermanager/render.h"

CS_PLUGIN_NAMESPACE_BEGIN(RMOSM)
{

SCF_IMPLEMENT_FACTORY(RMOSM);

//----------------------------------------------------------------------
RMOSM::RMOSM(iBase *parent) : scfImplementationType(this, parent)
{}

//----------------------------------------------------------------------
bool RMOSM::Initialize(iObjectRegistry *registry)
{
  const char *messageID = "crystalspace.rendermanager.osm";

  objRegistry = registry;
  csPrintf("Loading %s\n", messageID);
  
  return true;
}

//----------------------------------------------------------------------
bool RMOSM::RenderView(iView *view)
{
  csPrintf("Render Loop ... \n");
  return true;
}

//----------------------------------------------------------------------
bool RMOSM::PrecacheView(iView *view)
{
  return RenderView (view);
}

}
CS_PLUGIN_NAMESPACE_END(RMOSM)
