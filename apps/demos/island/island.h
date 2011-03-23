/*
  Copyright (C) 2010 Christian Van Brussel, Communications and Remote
      Sensing Laboratory of the School of Engineering at the 
      Universite catholique de Louvain, Belgium
      http://www.tele.ucl.ac.be

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

#include "cstool/csdemoapplication.h"

class IslandDemo : public CS::Demo::DemoApplication
{
 private:
  bool CreateScene ();

  bool inWater;

 public:
  IslandDemo ();

  //-- CS::Demo::DemoApplication
  void PrintHelp ();
  bool OnInitialize (int argc, char* argv[]);
  void Frame ();

  //-- csApplicationFramework
  bool Application ();
};

#endif // __ISLAND_H__
