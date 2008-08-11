/*
Copyright (C) 2008 by Julian Mautner

This application is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This application is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this application; if not, write to the Free
Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __CSCLOUDSYSTEM_PLUGIN_H__
#define __CSCLOUDSYSTEM_PLUGIN_H__

#include <iutil/objreg.h>
#include <iutil/comp.h>
#include "imesh/clouds.h"

//Supervisor-class implementation
class csCloudSystem : public scfImplementation1<csCloudSystem, iCloudSystem>
{
private:
  UINT                              m_iCloudCount;
  csArray<csRef<iClouds>>           m_Clouds;

public:
  csCloudSystem(iBase* pParent) : scfImplementationType(this, pParent)
  {

  }
  ~csCloudSystem()
  {

  }

  virtual inline const UINT GetCloudCount() const {return m_iCloudCount;}
  virtual inline csRef<iClouds> GetCloud(const UINT i) const {return m_Clouds[i];}

  virtual csRef<iClouds> AddCloud()
  {
    return m_Clouds[m_iCloudCount++];
  }
  virtual const bool RemoveCloud(csRef<iClouds> pCloud)
  {
    return true;
  }
};

#endif // __CSCLOUDSYSTEM_PLUGIN_H__