/*
    Copyright (C) 2006 by Jorrit Tyberghein
	                  Frank Richter

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

#define WIN32_LEAN_AND_MEAN
#define DXERROR9(v,n,d) {v, n, d},
#define DXERROR9LAST(v,n,d) {v, n, d}
#include "dxerr.inc"

#include "csutil/csstring.h"
#include "csutil/win32/wintools.h"
#include "csplugincommon/directx/error.h"

CS_IMPLEMENT_STATIC_VAR(GetErrorSymbolStr, csString, ())

const char* csDirectXError::GetErrorSymbol (HRESULT hr)
{
  const Error* err = FindError (hr);

  if (err->Name != 0) return err->Name;

  GetErrorSymbolStr()->Format ("0x%.8lx", (long)hr);
  return GetErrorSymbolStr()->GetData();
}

CS_IMPLEMENT_STATIC_VAR(GetErrorDescrStr, csString, ())

const char* csDirectXError::GetErrorDescription (HRESULT hr)
{
  const Error* err = FindError (hr);

  if (err->Description != 0) return err->Description;
  char* msg = cswinGetErrorMessage (hr);
  GetErrorDescrStr()->Replace (msg);
  delete[] msg;
  return GetErrorDescrStr()->GetData();
}
