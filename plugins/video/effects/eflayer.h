/*
    Copyright (C) 2002 by Anders Stenberg
    Written by Anders Stenberg

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

#ifndef __EFFECTLAYER_H__
#define __EFFECTLAYER_H__

#include "statehdr.h"

class csEffectLayer : public csStateHandler
{
private:
  csRef<iBase> rendererData;

  iBase* GetRendererData()
  {
    return rendererData;
  }

  void SetRendererData(iBase* data)
  {
    rendererData = data;
  }

public:
  SCF_DECLARE_IBASE;

  csEffectLayer()
  {
    SCF_CONSTRUCT_IBASE( NULL );
    SCF_CONSTRUCT_EMBEDDED_IBASE( scfiEffectLayer );
  }

  struct EffectLayer : public iEffectLayer
  {
    SCF_DECLARE_EMBEDDED_IBASE( csEffectLayer );

    void SetStateFloat( csStringID state, float value )
    {
      scfParent->SetStateFloat( state, value );
    }
    void SetStateString( csStringID state, csStringID value )
    {
      scfParent->SetStateString( state, value );
    }
    void SetStateOpaque( csStringID state, void *value )
    {
      scfParent->SetStateOpaque( state, value );
    }
    void SetStateVector4( csStringID state, csEffectVector4 value)
    {
      scfParent->SetStateVector4( state, value);
    }
    
    float GetStateFloat( csStringID state )
    {
      return scfParent->GetStateFloat( state );
    }
    csStringID GetStateString( csStringID state )
    {
      return scfParent->GetStateString( state );
    }
    void *GetStateOpaque( csStringID state )
    {
      return scfParent->GetStateOpaque( state );
    }
    csEffectVector4 GetStateVector4( csStringID state)
    {
      return scfParent->GetStateVector4( state );
    }
    
    csStringID GetFirstState()
    {
      return scfParent->GetFirstState();
    }
    csStringID GetNextState()
    {
      return scfParent->GetNextState();
    }

    iBase* GetRendererData()
    {
      return scfParent->GetRendererData();
    }

    void SetRendererData(iBase* data)
    {
      scfParent->SetRendererData(data);
    }

  } scfiEffectLayer;
  friend struct EffectLayer;
};

#endif // __EFFECTLAYER_H__
