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
#include <csutil/refarr.h>

#include "csClouds.h"
#include "imesh/clouds.h"

//Supervisor-class implementation
class csCloudSystem : public scfImplementation2<csCloudSystem, iCloudSystem, iComponent>
{
private:
  csRefArray<iClouds>               m_ppClouds;
  iObjectRegistry*                  m_pObjectReg;

public:
  csCloudSystem(iBase* pParent) : scfImplementationType(this, pParent)
  {
  }
  ~csCloudSystem()
  {
    RemoveAllClouds();
  }

  //Initialisation method from iComponent
  virtual bool Initialize(iObjectRegistry* pObjectReg)
  {
    m_pObjectReg = pObjectReg;
    return true;
  }

  virtual inline const UINT GetCloudCount() const {return static_cast<UINT>(m_ppClouds.GetSize());}
  virtual inline const iClouds* GetCloud(const UINT i) const {return m_ppClouds.Get(i);}

  virtual inline iClouds* AddCloud()
  {
    csRef<csClouds> pNewCloud;
    pNewCloud.AttachNew(new csClouds(this));
    pNewCloud->Init(m_pObjectReg);
    m_ppClouds.Push(pNewCloud);
    return pNewCloud;
  }
  virtual inline const bool RemoveCloud(iClouds* pCloud)
  {
    static_cast<csClouds*>(pCloud)->Exit();
    return m_ppClouds.Delete(pCloud);
  }
  virtual inline const bool RemoveCloud(const UINT iIndex)
  {
    return RemoveCloud(m_ppClouds.Get(iIndex));
  }

  virtual inline const bool RemoveAllClouds()
  {
    m_ppClouds.DeleteAll();
    return true;
  }
};

#endif // __CSCLOUDSYSTEM_PLUGIN_H__