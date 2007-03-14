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

#ifndef __CS_CSPLUGINCOMMON_DIRECTX_ERROR_H__
#define __CS_CSPLUGINCOMMON_DIRECTX_ERROR_H__

#include "csextern_dx.h"

/**\file
 * DirectX error strings.
 */
/**\addtogroup plugincommon
 * @{ */

/**
 * Obtain symbols and descriptions for DirectX error codes.
 * \remarks Provides a similar service like the "dxerr" library. However,
 *  since that library is static, compiler-dependent and not necessarily
 *  available, it's safer to just roll a custom version.
 */
class CS_CSPLUGINCOMMON_DX_EXPORT csDirectXError
{
public:
  /**
   * Get the symbol for an error code (like <tt>"DXERROR_SOMETHING"</tt>).
   * \remarks Return value only guaranteed to be valid until next call.
   */
  static const char* GetErrorSymbol (HRESULT hr);
  /** 
   * Get the description for an error code (like <tt>"Something has 
   * happened"</tt>).
   * \remarks Return value only guaranteed to be valid until next call.
   */
  static const char* GetErrorDescription (HRESULT hr);
};

/** @} */

#endif // __CS_CSPLUGINCOMMON_DIRECTX_ERROR_H__
