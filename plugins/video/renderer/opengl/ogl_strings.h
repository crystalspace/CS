/*
    Copyright (C) 2002 by Anders Stenberg

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

#ifndef STRINGS_OPENGL_H
#define STRINGS_OPENGL_H

#include "ivideo/effects/efserver.h"

class csEffectStrings
{
public:
  #define REGISTER_STRING( name, string ) \
  static csStringID name;
  #include "effectstrings.h"
  #undef REGISTER_STRING

  static void InitStrings (iEffectServer* server);
};

#endif // STRINGS_OPENGL_H
