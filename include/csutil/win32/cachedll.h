/*
    Copyright (C) 2004 by Jorrit Tyberghein
	      (C) 2004 by Frank Richter

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

#ifndef __CS_UTIL_WIN32_CACHEDLL_H__
#define __CS_UTIL_WIN32_CACHEDLL_H__

/**\file
 * Helper to avoid repeated loading/unloading of DLLs
 */

namespace CS
{
namespace Platform
{
namespace Win32
{

/**
 * Small helper to avoid repeated loading/unloading of DLLs loaded at 
 * runtime. Loads a DLL when needed and unloads it on destruction.
 * Usage example:
 * \code
 * static CS::Platform::Win32::CacheDLL hKernel32 ("kernel32.dll");
 * if (hKernel32 != 0)
 * {
 *   ...
 * }
 * \endcode
 */
class CacheDLL
{
  const char* dllName;
  HMODULE dllHandle;
public:
  CacheDLL (const char* dll) : dllName(dll), dllHandle (0)
  {
  }
  
  ~CacheDLL ()
  {
    if (dllHandle != 0)
      FreeLibrary (dllHandle);
  }
  
  HMODULE GetHandle ()
  {
    if (dllHandle == 0)
      dllHandle = LoadLibraryA (dllName);
    return dllHandle;
  }
  
  operator HMODULE ()
  {
    return GetHandle ();
  }
};

} // namespace CS
} // namespace Platform
} // namespace Win32

#endif // __CS_UTIL_WIN32_CACHEDLL_H__
