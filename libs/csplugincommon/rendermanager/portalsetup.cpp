/*
    Copyright (C) 2007-2008 by Marten Svanfeldt
              (C) 2008 by Frank Richter

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

#include "csplugincommon/rendermanager/portalsetup.h"

namespace CS
{
  namespace RenderManager
  {
#define SPSBPD  StandardPortalSetup_Base::PersistentData
  
    bool SPSBPD::PortalBufferConstraint::IsEqual (
      const PortalBuffers& b1, const PortalBuffers& b2)
    {
      size_t s1 = b1.coordBuf->GetElementCount ();
      size_t s2 = b2.coordBuf->GetElementCount ();

      if (s1 == s2) return true;
      return false;
    }

    bool SPSBPD::PortalBufferConstraint::IsLargerEqual (
      const PortalBuffers& b1, const PortalBuffers& b2)
    {
      size_t s1 = b1.coordBuf->GetElementCount ();
      size_t s2 = b2.coordBuf->GetElementCount ();

      if (s1 >= s2) return true;
      return false;
    }

    bool SPSBPD::PortalBufferConstraint::IsEqual (
      const PortalBuffers& b1, const KeyType& s2)
    {
      size_t s1 = b1.coordBuf->GetElementCount ();

      if (s1 == s2) return true;
      return false;
    }

    bool SPSBPD::PortalBufferConstraint::IsLargerEqual (
      const PortalBuffers& b1, const KeyType& s2)
    {
      size_t s1 = b1.coordBuf->GetElementCount ();

      if (s1 >= s2) return true;
      return false;
    }

    bool SPSBPD::PortalBufferConstraint::IsLargerEqual (
      const KeyType& s1, const PortalBuffers& b2)
    {
      size_t s2 = b2.coordBuf->GetElementCount ();

      if (s1 >= s2) return true;
      return false;
    }
    
    //-----------------------------------------------------------------------

    void SPSBPD::csBoxClipperCached::operator delete (void* p, void* q)
    {
      csBoxClipperCached* bcc = reinterpret_cast<csBoxClipperCached*> (p);
      bcc->owningPersistentData->FreeCachedClipper (bcc);
    }
    void SPSBPD::csBoxClipperCached::operator delete (void* p)
    {
      csBoxClipperCached* bcc = reinterpret_cast<csBoxClipperCached*> (p);
      bcc->owningPersistentData->FreeCachedClipper (bcc);
    }
  
    //-----------------------------------------------------------------------

    void SPSBPD::FreeCachedClipper (csBoxClipperCached* bcc)
    {
      CS::Utility::ResourceCache::ReuseConditionFlagged::StoredAuxiliaryInfo*
	reuseAux = boxClipperCache.GetReuseAuxiliary (
	  reinterpret_cast<csBoxClipperCachedStore*> (bcc));
      reuseAux->reusable = true;
    }
    
    SPSBPD::PersistentData(int textCachOptions) :
      bufCache (CS::Utility::ResourceCache::ReuseConditionAfterTime<uint> (),
	CS::Utility::ResourceCache::PurgeConditionAfterTime<uint> (10000)),
      boxClipperCache (CS::Utility::ResourceCache::ReuseConditionFlagged (),
	CS::Utility::ResourceCache::PurgeConditionAfterTime<uint> (10000)),
      texCache (csimg2D, "rgb8", // @@@ FIXME: Use same format as main view ...
	CS_TEXTURE_3D | CS_TEXTURE_NOMIPMAPS | CS_TEXTURE_CLAMP,
	"target", textCachOptions), 
      fixedTexCacheWidth (0), fixedTexCacheHeight (0)
    {
      bufCache.agedPurgeInterval = 5000;
      boxClipperCache.agedPurgeInterval = 5000;
    }

    void SPSBPD::Initialize (iShaderManager* shmgr, iGraphics3D* g3d,
                             RenderTreeBase::DebugPersistent& dbgPersist)
    {
      svNameTexPortal =
	shmgr->GetSVNameStringset()->Request ("tex portal");
      texCache.SetG3D (g3d);
      
      dbgDrawPortalOutlines =
        dbgPersist.RegisterDebugFlag ("draw.portals.outline");
      dbgDrawPortalPlanes =
        dbgPersist.RegisterDebugFlag ("draw.portals.planes");
      dbgShowPortalTextures =
        dbgPersist.RegisterDebugFlag ("textures.portals");
    }
    
  } // namespace RenderManager
} // namespace CS

