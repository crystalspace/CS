/*
    Copyright (C) 2004 by Eric Sunshine <sunshine@sunshineco.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
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

// Implementation specific to the pure Python "cspace" module (pythmod). Not
// used by the cspython Crystal Space plugin.

#include "cssysdef.h"

CS_IMPLEMENT_FOREIGN_DLL

extern "C" void SWIG_init_cspace();

extern "C" CS_EXPORT_SYM_DLL void init_cspace ()
{
  SWIG_init_cspace();
}
