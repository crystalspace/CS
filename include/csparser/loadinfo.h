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
  bool use_tex_set;
  char* tex_set_name;
  
  ///
  PSLoadInfo () : default_texture(NULL),
   default_texlen(1), default_lightx(NULL),
   use_tex_set(false), tex_set_name(NULL) {}

  void SetTextureSet (const char* name)
  {
    if (tex_set_name) delete [] tex_set_name;
    tex_set_name = new char [strlen (name) + 1];
    strcpy (tex_set_name, name);
  }   
};

#endif
