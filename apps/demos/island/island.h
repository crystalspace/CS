/*
  Copyright (C) 2010-11 Christian Van Brussel, Institute of Information
      and Communication Technologies, Electronics and Applied Mathematics
      at Universite catholique de Louvain, Belgium
      http://www.uclouvain.be/en-icteam.html

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

#ifndef __ISLAND_H__
#define __ISLAND_H__

#include "cstool/demoapplication.h"

class IslandDemo : public CS::Utility::DemoApplication
{
 private:
  bool CreateScene ();

  bool inWater;

 public:
  IslandDemo ();

  //-- CS::Utility::DemoApplication
  void PrintHelp ();
  void Frame ();
  const char* GetApplicationConfigFile()
  { return "/config/csisland.cfg"; }

  //-- csApplicationFramework
  bool Application ();
};

#endif // __ISLAND_H__
