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
#ifndef __CS_EFFECTLAYER_H__
#define __CS_EFFECTLAYER_H__

#include "../ieffects/eflayer.h"
#include "statehdr.h"

class csEffectLayer : public csStateHandler, public iEffectLayer
{
private:
  csRef<iBase> rendererData;

public:
  SCF_DECLARE_IBASE;

  csEffectLayer ();
  virtual ~csEffectLayer ();

  void SetRendererData (iBase* data);
  iBase* GetRendererData ();

  /// Set a state float.
  virtual void SetStateFloat( csStringID state, float value )
  { csStateHandler::SetStateFloat (state, value); }
  /// Set a state string.
  virtual void SetStateString( csStringID state, csStringID value )
  { csStateHandler::SetStateString (state, value); }
  /// Set a state opaque data.
  virtual void SetStateOpaque( csStringID state, void *value )
  { csStateHandler::SetStateOpaque (state, value); }
  /// Set a state vector4.
  virtual void SetStateVector4( csStringID state, csEffectVector4 value)
  { csStateHandler::SetStateVector4 (state, value); }
  /// Get a state float.
  virtual float GetStateFloat( csStringID state )
  { return csStateHandler::GetStateFloat (state); }
  /// Get a state string.
  virtual csStringID GetStateString( csStringID state )
  { return csStateHandler::GetStateString (state); }
  /// Get a state opaque data.
  virtual void *GetStateOpaque( csStringID state )
  { return csStateHandler::GetStateOpaque (state); }
  /// Get a state vector4. 
  virtual csEffectVector4 GetStateVector4(csStringID state)
  { return csStateHandler::GetStateVector4 (state); }

  /// Get the id of the first state.
  virtual csStringID GetFirstState()
  { return csStateHandler::GetFirstState (); }
  /// Get the id of the next state.
  virtual csStringID GetNextState()
  { return csStateHandler::GetNextState (); }
};

#endif // __CS_EFFECTLAYER_H__
