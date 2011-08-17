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
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __SELFSHADOW_H__
#define __SELFSHADOW_H__

#include <crystalspace.h>
#include "cstool/demoapplication.h"

class SelfShadowDemo : public CS::Utility::DemoApplication
{
 private:
  bool CreateScene ();
  void LoadKrystal();

 public:
  SelfShadowDemo ();
  csRef<iDebugHelper> rm_dbg;

  //-- CS::Utility::DemoApplication
  void PrintHelp ();
  void Frame ();
  bool OnKeyboard (iEvent &event);

  bool OnInitialize (int argc, char* argv[]);
  //-- csApplicationFramework
  bool Application ();
};

#endif // __SELFSHADOW_H__
