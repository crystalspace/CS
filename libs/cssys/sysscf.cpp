/*
    Interface to SCF kernel for use by plugins.
    Copyright (C) 1998 by Jorrit Tyberghein

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
#include "cssys/system.h"

//------------------------------------------ iSCF interface implementation ---//

IMPLEMENT_EMBEDDED_IBASE (csSystemDriver::csSCF)
  IMPLEMENTS_INTERFACE (iSCF)
IMPLEMENT_EMBEDDED_IBASE_END

bool csSystemDriver::csSCF::scfClassRegistered (const char *iClassID)
{
  return ::scfClassRegistered (iClassID);
}

void *csSystemDriver::csSCF::scfCreateInstance (const char *iClassID,
  const char *iInterfaceID, int iVersion)
{
  return ::scfCreateInstance (iClassID, iInterfaceID, iVersion);
}

const char *csSystemDriver::csSCF::scfGetClassDescription (const char *iClassID)
{
  return ::scfGetClassDescription (iClassID);
}

const char *csSystemDriver::csSCF::scfGetClassDependencies (const char *iClassID)
{
  return ::scfGetClassDependencies (iClassID);
}

bool csSystemDriver::csSCF::scfRegisterClass (const char *iClassID,
  const char *iLibraryName, const char *Dependencies)
{
  return ::scfRegisterClass (iClassID, iLibraryName, Dependencies);
}

bool csSystemDriver::csSCF::scfRegisterStaticClass (scfClassInfo *iClassInfo)
{
  return ::scfRegisterStaticClass (iClassInfo);
}

bool csSystemDriver::csSCF::scfRegisterClassList (scfClassInfo *iClassInfo)
{
  return ::scfRegisterClassList (iClassInfo);
}

bool csSystemDriver::csSCF::scfUnregisterClass (char *iClassID)
{
  return ::scfUnregisterClass (iClassID);
}
