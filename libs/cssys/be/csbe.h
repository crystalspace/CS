/*
    Copyright (C) 1998,1999 by Jorrit Tyberghein
    Written by Xavier Planet.
    Overhauled and re-engineered by Eric Sunshine <sunshine@sunshineco.com>
  
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

#ifndef CSBE_H
#define CSBE_H

#include "cscom/com.h"
#include "def.h"
#include "csinput/csinput.h"
#include "cssys/common/system.h"
#include "cssys/be/beitf.h"
#include "igraph2d.h"
class CrystApp;

class SysKeyboardDriver : public csKeyboardDriver {};
class SysMouseDriver : public csMouseDriver {};

class SysSystemDriver : public csSystemDriver
{
protected:
  bool running;
  CrystApp* app;
  void ProcessUserEvent(BMessage*);
  bool SetMouseCursor(int shape);
public:
  SysSystemDriver();

  // Main event loop
  virtual void Loop();
  long LoopThread();

  /// Implementation of IBeLibSystemDriver
  class XBeLibSystemDriver : public IBeLibSystemDriver
  {
    DECLARE_IUNKNOWN()
    STDMETHOD(ProcessUserEvent)(BMessage*);
    STDMETHOD(SetMouseCursor)(int shape, ITextureHandle*);
  };
  DECLARE_IUNKNOWN()
  DECLARE_INTERFACE_TABLE(SysSystemDriver)
  DECLARE_COMPOSITE_INTERFACE_EMBEDDED(BeLibSystemDriver);
};

#endif // CSBE_H
