/*
    Copyright (C) 2005 by Frank Richter

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

#ifndef __CS_CSUTIL_WIN32_PSDK_COMPAT_H__
#define __CS_CSUTIL_WIN32_PSDK_COMPAT_H__

/**\file
 * Header to smooth out differences between Platform SDK versions.
 * Between versions of Win32 Platform SDKs, symbols are added and sometimes
 * dropped. For easier code maintenance across PSDK versions, select symbols 
 * that are notavailable in all versions are provided here.
 */

#ifndef CDS_UPDATEREGISTRY
#define CDS_UPDATEREGISTRY  0x00000001
#endif
#ifndef CDS_TEST
#define CDS_TEST            0x00000002
#endif
#ifndef CDS_FULLSCREEN
#define CDS_FULLSCREEN      0x00000004
#endif
#ifndef CDS_GLOBAL
#define CDS_GLOBAL          0x00000008
#endif
#ifndef CDS_SET_PRIMARY
#define CDS_SET_PRIMARY     0x00000010
#endif
#ifndef CDS_RESET
#define CDS_RESET           0x40000000
#endif
#ifndef CDS_SETRECT
#define CDS_SETRECT         0x20000000
#endif
#ifndef CDS_NORESET
#define CDS_NORESET         0x10000000
#endif

/* Return values for ChangeDisplaySettings */
#ifndef DISP_CHANGE_SUCCESSFUL
#define DISP_CHANGE_SUCCESSFUL       0
#endif
#ifndef DISP_CHANGE_RESTART
#define DISP_CHANGE_RESTART          1
#endif
#ifndef DISP_CHANGE_FAILED
#define DISP_CHANGE_FAILED          -1
#endif
#ifndef DISP_CHANGE_BADMODE
#define DISP_CHANGE_BADMODE         -2
#endif
#ifndef DISP_CHANGE_NOTUPDATED
#define DISP_CHANGE_NOTUPDATED      -3
#endif
#ifndef DISP_CHANGE_BADFLAGS
#define DISP_CHANGE_BADFLAGS        -4
#endif
#ifndef DISP_CHANGE_BADPARAM
#define DISP_CHANGE_BADPARAM        -5
#endif

#ifndef ENUM_CURRENT_SETTINGS
#define ENUM_CURRENT_SETTINGS       ((DWORD)-1)
#endif

// These don't exist on some older PSDKs
#ifndef _WIN64
  #ifndef SetWindowLongPtrA
    #define SetWindowLongPtrA SetWindowLongA
  #endif
  #ifndef SetWindowLongPtrW
    #define SetWindowLongPtrW SetWindowLongW
  #endif
  
  #ifndef GetWindowLongPtrA
    #define GetWindowLongPtrA GetWindowLongA
  #endif
  #ifndef GetWindowLongPtrW
    #define GetWindowLongPtrW GetWindowLongW
  #endif
  
  #ifndef GWLP_WNDPROC
    #define GWLP_WNDPROC GWL_WNDPROC
  #endif
  
  #ifndef GWLP_USERDATA
    #define GWLP_USERDATA GWL_USERDATA
  #endif
#endif

#endif // __CS_CSUTIL_WIN32_PSDK_COMPAT_H__
