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

#ifndef __CS_EFFECTPASS_H__
#define __CS_EFFECTPASS_H__

#include "csutil/refarr.h"
#include "csutil/hashmap.h"
#include "statehdr.h"
#include "../ieffects/efpass.h"

struct iEffectLayer;

class csEffectPass : public csStateHandler, public iEffectPass
{
private:
  csRefArray<iEffectLayer> layers;
		
  csRef<iBase> rendererData;

public:
  SCF_DECLARE_IBASE;

  csEffectPass ();
  virtual ~csEffectPass ();

  csPtr<iEffectLayer> CreateLayer();
  int GetLayerCount();
  iEffectLayer* GetLayer (int layer);

  iBase* GetRendererData ();
  void SetRendererData (iBase* data);

  void SetStateFloat( csStringID state, float value )
  {
    csStateHandler::SetStateFloat( state, value );
  }
  void SetStateString( csStringID state, csStringID value )
  {
    csStateHandler::SetStateString( state, value );
  }
  void SetStateOpaque( csStringID state, void *value )
  {
    csStateHandler::SetStateOpaque( state, value );
  }
  void SetStateVector4( csStringID state, csEffectVector4 value)
  {
    csStateHandler::SetStateVector4( state, value);
  }
  
  float GetStateFloat( csStringID state )
  {
    return csStateHandler::GetStateFloat( state );
  }
  csStringID GetStateString( csStringID state )
  {
    return csStateHandler::GetStateString( state );
  }
  void *GetStateOpaque( csStringID state )
  {
    return csStateHandler::GetStateOpaque( state );
  }
  csEffectVector4 GetStateVector4( csStringID state)
  {
    return csStateHandler::GetStateVector4( state );
  }

  csStringID GetFirstState()
  {
    return csStateHandler::GetFirstState();
  }
  csStringID GetNextState()
  {
    return csStateHandler::GetNextState();
  }
};

#endif // __CS_EFFECTPASS_H__


