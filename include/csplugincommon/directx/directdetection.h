/*
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

#ifndef __CS_DIRECTDETECTION_H__
#define __CS_DIRECTDETECTION_H__

#include "csextern_dx.h"

#include <windows.h>
#include <ddraw.h>

/**\file
 * DirectDraw device enumeration.
 */
/**\addtogroup plugincommon
 * @{ */

/// Description of DirectDraw device
class CS_CSPLUGINCOMMON_DX_EXPORT DirectDetectionDevice
{
public:
  DirectDetectionDevice() : Windowed(false), IsPrimary2D(true), 
    DeviceName2D(0), DeviceDescription2D(0)
  {
    ZeroMemory (&Guid2D, sizeof(GUID));
  }
  DirectDetectionDevice (const DirectDetectionDevice& other) :
    Windowed (other.Windowed), IsPrimary2D (other.IsPrimary2D)
  {
    memcpy (&Guid2D, &other.Guid2D, sizeof (GUID));
    DeviceName2D = csStrNew (other.DeviceName2D);
    DeviceDescription2D = csStrNew (other.DeviceDescription2D);
  }
  ~DirectDetectionDevice()
  {
    delete[] DeviceName2D;
    delete[] DeviceDescription2D;
  }

  /// Guid for DirectDraw device
  GUID Guid2D;
  /// Can enable windowed mode for graphics ?
  bool Windowed;
  /// Is a primary ddraw device ?
  bool IsPrimary2D;
  /// Name of device
  char * DeviceName2D;
  /// Description of device
  char * DeviceDescription2D; 
};

/// Master class of the device detection of direct3d and directdraw
class CS_CSPLUGINCOMMON_DX_EXPORT DirectDetection
{
public:
  bool Have2DDevice();
  DirectDetection();
  virtual ~DirectDetection();
  const DirectDetectionDevice* FindBestDevice (int displayNumber = 0);
  int AddDevice (const DirectDetectionDevice& dd2d);
  bool CheckDevices();
  bool CheckDevices2D();

  void ReportResult (int severity, char *str, HRESULT hRes);
  void SystemFatalError (char *str, HRESULT hRes);

  /// List of devices
  csArray<DirectDetectionDevice> Devices; 
  iObjectRegistry* object_reg;
};

/** @} */

#endif // __CS_DIRECTDETECTION_H__
