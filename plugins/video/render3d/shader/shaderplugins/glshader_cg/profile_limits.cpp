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

#include "cssysdef.h"

#include "iutil/vfs.h"

#include "csutil/csendian.h"

#include "profile_limits.h"

CS_PLUGIN_NAMESPACE_BEGIN(GLShaderCg)
{
  ProfileLimits::ProfileLimits (CGprofile profile)
   : profile (profile), 
     MaxAddressRegs (0),
     MaxInstructions (0),
     MaxLocalParams (0),
     MaxTexIndirections (0),
     NumInstructionSlots (0),
     NumMathInstructionSlots (0),
     NumTemps (0),
     NumTexInstructionSlots (0)
  {
  }
  
  static uint glGetInteger (GLenum what)
  {
    GLint v;
    glGetIntegerv (what, &v);
    return v;
  }

  void ProfileLimits::GetCurrentLimits ()
  {
#define PROFILE_BEGIN(PROFILE)  \
  case CG_PROFILE_ ## PROFILE:  \
    {
#define PROFILE_END(PROFILE)    \
    }                           \
    break;
#define LIMIT(Limit, glLimit)   \
      Limit = glGetInteger (GL_ ## glLimit);
  
    switch (profile)
    {
#include "profile_limits.inc"
      default:
        break;
    }
    
#undef PROFILE_BEGIN
#undef PROFILE_END
#undef LIMIT
  }

  enum
  {
    limMaxAddressRegs,
    limMaxInstructions,
    limMaxLocalParams,
    limMaxTexIndirections,
    limNumInstructionSlots,
    limNumMathInstructionSlots,
    limNumTemps,
    limNumTexInstructionSlots
  };

  csString ProfileLimits::ToString ()
  {
    uint usedLimits = 0;
  
#define PROFILE_BEGIN(PROFILE)  \
  case CG_PROFILE_ ## PROFILE:  \
    {
#define PROFILE_END(PROFILE)    \
    }                           \
    break;
#define LIMIT(Limit, glLimit)   \
      usedLimits |= 1 << lim ## Limit;
  
    switch (profile)
    {
#include "profile_limits.inc"
      default:
        break;
    }
    
#undef PROFILE_BEGIN
#undef PROFILE_END
#undef LIMIT

    csString ret (cgGetProfileString (profile));
#define EMIT(Limit) if (usedLimits & (1 << lim ## Limit)) ret.AppendFmt (".%u", Limit);
    EMIT (MaxAddressRegs);
    EMIT (MaxInstructions);
    EMIT (MaxLocalParams);
    EMIT (MaxTexIndirections);
    EMIT (NumInstructionSlots);
    EMIT (NumMathInstructionSlots);
    EMIT (NumTemps);
    EMIT (NumTexInstructionSlots);
#undef EMIT
    return ret;
  };
    
  bool ProfileLimits::Write (iFile* file)
  {
#define WRITE(Limit) \
    { \
      uint32 diskVal = csLittleEndian::UInt32 (Limit); \
      if (file->Write ((char*)&diskVal, sizeof (diskVal)) != sizeof (diskVal)) \
        return false; \
    }
    WRITE (MaxAddressRegs);
    WRITE (MaxInstructions);
    WRITE (MaxLocalParams);
    WRITE (MaxTexIndirections);
    WRITE (NumInstructionSlots);
    WRITE (NumMathInstructionSlots);
    WRITE (NumTemps);
    WRITE (NumTexInstructionSlots);
#undef WRITE

    return true;
  }
    
  bool ProfileLimits::Read (iFile* file)
  {
#define READ(Limit) \
    { \
      uint32 diskVal; \
      if (file->Read ((char*)&diskVal, sizeof (diskVal)) != sizeof (diskVal)) \
        return false; \
      Limit = csLittleEndian::UInt32 (diskVal); \
    }
    READ (MaxAddressRegs);
    READ (MaxInstructions);
    READ (MaxLocalParams);
    READ (MaxTexIndirections);
    READ (NumInstructionSlots);
    READ (NumMathInstructionSlots);
    READ (NumTemps);
    READ (NumTexInstructionSlots);
#undef READ

    return true;
  }
  
  static int GetProfileOrdering (CGprofile profile)
  {
    switch (profile)
    {
      case CG_PROFILE_PS_1_1:   return 0;
      case CG_PROFILE_PS_1_2:   return 1;
      case CG_PROFILE_PS_1_3:   return 2;
      case CG_PROFILE_VP20:     return 3;
      case CG_PROFILE_FP20:     return 4;
      case CG_PROFILE_ARBVP1:   return 5;
      case CG_PROFILE_ARBFP1:   return 6;
      case CG_PROFILE_VP30:     return 7;
      case CG_PROFILE_FP30:     return 8;
      case CG_PROFILE_VP40:     return 9;
      case CG_PROFILE_FP40:     return 10;
      case CG_PROFILE_GPU_VP:   return 11;
      case CG_PROFILE_GPU_FP:   return 12;
      default:
        return -1;
    }
  }

  bool ProfileLimits::operator< (const ProfileLimits& other) const
  {
    int p1 = GetProfileOrdering (profile);
    int p2 = GetProfileOrdering (other.profile);
    if (p1 < p2) return true;
    if (p1 > p2) return false;
  
#define COMPARE(Limit) \
    if (Limit < other.Limit) return true; \
    if (Limit > other.Limit) return false;
    COMPARE (MaxInstructions);
    COMPARE (NumInstructionSlots);
    COMPARE (NumMathInstructionSlots);
    COMPARE (NumTexInstructionSlots);
    COMPARE (NumTemps);
    COMPARE (MaxLocalParams);
    COMPARE (MaxTexIndirections);
    COMPARE (MaxAddressRegs);
#undef READ
    return false;
  }
}
CS_PLUGIN_NAMESPACE_END(GLShaderCg)


