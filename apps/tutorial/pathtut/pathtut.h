/*
    Copyright (C) 2002 by Bhasker Hariharan

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

#ifndef __PATHTUT_H__
#define __PATHTUT_H__

#include <stdarg.h>
#include <crystalspace.h>

class PathTut
{
private:
  iObjectRegistry* object_reg;
  csRef<iEngine> engine;
  csRef<iLoader> loader;
  csRef<iGraphics3D> g3d;
  csRef<iKeyboardDriver> kbd;
  csRef<iVirtualClock> vc;
  iSector* room;
  csRef<iView> view;
  csRef<iMeshWrapper> sprite;
  csPath*   m_Path;
  csTicks   m_Duration; //Time for 1 Complete Circle
  csTicks   m_CurrentTime;
  static bool PathTutEventHandler (iEvent& ev);
  bool HandleEvent (iEvent& ev);
  void SetupFrame ();
  void FinishFrame ();
  void InitializePath();
  void Animate(csTicks elapsedTime);
public:
  PathTut (iObjectRegistry* object_reg);
  ~PathTut ();

  bool Initialize (int argc, const char* const argv[]);
  void Start ();
};

#endif // __PATHTUT_H__

