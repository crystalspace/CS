/*
    Copyright (C) 1998 by Ivan Avramovic <ivan@avramovic.com>
  
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

#ifndef __CS_LOADINFO_H__
#define __CS_LOADINFO_H__

#include "csgeom/transfrm.h"

class csMaterialWrapper;
class csTextureList;

class PSLoadInfo
{
public:
  csMaterialWrapper* default_material;
  float default_texlen;
  bool use_mat_set;
  char* mat_set_name;
  csReversibleTransform hard_trans;
  bool do_hard_trans;
  bool is_convex;
  
  ///
  PSLoadInfo () : default_material (NULL),
   default_texlen (1),
   use_mat_set (false), mat_set_name (NULL),
   do_hard_trans (false), is_convex (false) {}

  void SetTextureSet (const char* name)
  {
    if (mat_set_name) delete [] mat_set_name;
    mat_set_name = new char [strlen (name) + 1];
    strcpy (mat_set_name, name);
  }   
};

#endif // __CS_LOADINFO_H__
