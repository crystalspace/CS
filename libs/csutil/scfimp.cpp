/*
    Copyright (C) 2001 by Andrew Zabolotny
    Crystal Space Shared Class Facility (SCF) Shared Global

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
#include "csutil/scf.h"

// This file defines the global iSCF::SCF variable which is needed by the
// application and all plugin modules.  This variable is defined here rather
// than in scf.cpp in order to reduce the amount of code which is pulled into
// plugin modules at link time.  Plugin modules need only link with the object
// file from this implementation in order to satisfy SCF requirements.  If, on
// the other hand, this variable had been declared in scf.cpp, then plugin
// modules would unnecessarily link with the object file generated from
// scf.cpp, as well as all other object files referenced by that one, such as
// object files for locating and loading plugin modules.  Such object files
// are really only needed by the application, thus by defining this variable
// here, we avoid having to link plugin modules with other unnecessary goop.

iSCF *iSCF::SCF = 0;
