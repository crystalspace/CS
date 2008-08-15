/*
    Copyright (C) 2008 by Frank Richter

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

#include "csplugincommon/rendermanager/hdrhelper.h"

#include "iutil/cfgfile.h"

namespace CS
{
  namespace RenderManager
  {
    HDRSettings::HDRSettings (iConfigFile* config, const char* prefix)
      : config (config), prefix (prefix) {}
    
    bool HDRSettings::IsEnabled()
    {
      return config->GetBool (csString ().Format ("%s.HDR.Enabled",
        prefix.GetData()), false);
    }
    
    HDRHelper::Quality HDRSettings::GetQuality()
    {
      HDRHelper::Quality qual = HDRHelper::qualInt8;
      
      const char* qualStr = config->GetStr (
        csString ().Format ("%s.HDR.Quality", prefix.GetData()), 0);
      if (qualStr)
      {
        if (strcmp (qualStr, "int8") == 0)
          qual = HDRHelper::qualInt8;
        else if (strcmp (qualStr, "int10") == 0)
          qual = HDRHelper::qualInt10;
        else if (strcmp (qualStr, "int16") == 0)
          qual = HDRHelper::qualInt16;
        else if (strcmp (qualStr, "float16") == 0)
          qual = HDRHelper::qualFloat16;
        else if (strcmp (qualStr, "float32") == 0)
          qual = HDRHelper::qualFloat32;
      }
      
      return qual;
    }
    
    int HDRSettings::GetColorRange()
    {
      return config->GetInt (csString ().Format ("%s.HDR.ColorRange",
        prefix.GetData()), 4);
    }
  } // namespace RenderManager
} // namespace CS

