/*
  Copyright (C) 2008 by Frank Richter

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __PROFILE_LIMITS_H__
#define __PROFILE_LIMITS_H__

#include "cg_common.h"

struct iFile;

CS_PLUGIN_NAMESPACE_BEGIN(GLShaderCg)
{

  struct ProfileLimits
  {
    CGprofile profile;
    uint MaxAddressRegs;
    uint MaxInstructions;
    uint MaxLocalParams;
    uint MaxTexIndirections;
    uint NumInstructionSlots;
    uint NumMathInstructionSlots;
    uint NumTemps;
    uint NumTexInstructionSlots;
    
    ProfileLimits (CGprofile profile);
    
    void GetCurrentLimits ();
    void ReadFromConfig (iConfigFile* cfg, const char* prefix);
    void GetCgDefaults ();
    
    csString ToString () const;
    void ToCgOptions (ArgumentArray& args) const;
    
    bool Write (iFile* file) const;
    bool Read (iFile* file);
    
    bool operator< (const ProfileLimits& other) const;
    bool operator>= (const ProfileLimits& other) const
    { return !operator< (other); }
  };

}
CS_PLUGIN_NAMESPACE_END(GLShaderCg)

#endif // __PROFILE_LIMITS_H__
