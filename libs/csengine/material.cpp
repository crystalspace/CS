/*
    Copyright (C) 2000 by W.C.A. Wijngaards
  
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


#include "cssysdef.h"
#include "csengine/material.h"
#include "csengine/texture.h"

csMaterial :: csMaterial() 
{
  // set defaults
  // black flat shaded.
  flat_color.Set(0.f,0.f,0.f);
  texture = 0;
  diffuse = 0.7;
  ambient = 0.0;
  reflection = 0.0;
}

csMaterial :: csMaterial(csTextureHandle *txt) 
{
  csMaterial();
  texture = txt;
}

csMaterial :: ~csMaterial() 
{
  delete texture;
}
