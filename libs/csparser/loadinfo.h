/*
    Copyright (C) 1998 by Jorrit Tyberghein
    Written by Ivan Avramovic <ivan@avramovic.com>
  
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

#ifndef _LOADINFO_H_
#define _LOADINFO_H_

class csTextureHandle;
class CLights;
class csWorld;
class csTextureList;

class PSLoadInfo
{
public:
  csTextureHandle* default_texture;
  float default_texlen;
  CLights* default_lightx;
  csWorld* w;
  csTextureList* textures;
  bool use_bsp;
  ///
  PSLoadInfo(csWorld* world, csTextureList* t) : default_texture(NULL), 
   default_texlen(1), default_lightx(NULL), w(world), textures(t),
   use_bsp(false) {}
};

#endif
