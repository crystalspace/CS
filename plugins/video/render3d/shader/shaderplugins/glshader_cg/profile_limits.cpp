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

#include "iutil/cfgfile.h"
#include "iutil/vfs.h"

#include "csutil/csendian.h"

#include "profile_limits.h"

#include "csplugincommon/opengl/glextmanager.h"

CS_PLUGIN_NAMESPACE_BEGIN(GLShaderCg)
{
// Tabularize what profiles use what limits
#define PROFILES  \
  PROFILE_BEGIN(ARBVP1) \
    LIMIT(MaxAddressRegs, MAX_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB, 1) \
    LIMIT(MaxInstructions, MAX_PROGRAM_NATIVE_INSTRUCTIONS_ARB, 1024) \
    LIMIT(MaxLocalParams, MAX_PROGRAM_LOCAL_PARAMETERS_ARB, 96) \
    LIMIT(NumTemps, MAX_PROGRAM_TEMPORARIES_ARB, 32)  \
  PROFILE_END(ARBVP1) \
  \
  PROFILE_BEGIN(ARBFP1) \
    LIMIT(MaxLocalParams, MAX_PROGRAM_LOCAL_PARAMETERS_ARB, 32) \
    LIMIT(MaxTexIndirections, MAX_PROGRAM_NATIVE_TEX_INDIRECTIONS_ARB, ~(1 << 31)) \
    LIMIT(NumInstructionSlots, MAX_PROGRAM_NATIVE_INSTRUCTIONS_ARB, 1024) \
    LIMIT(NumMathInstructionSlots, MAX_PROGRAM_NATIVE_ALU_INSTRUCTIONS_ARB, 1024) \
    LIMIT(NumTemps, MAX_PROGRAM_TEMPORARIES_ARB, 32) \
    LIMIT(NumTexInstructionSlots, MAX_PROGRAM_NATIVE_TEX_INSTRUCTIONS_ARB, 1024) \
  PROFILE_END(ARBFP1) \
  \
  PROFILE_BEGIN(VP40) \
    LIMIT(MaxAddressRegs, MAX_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB, 2) \
    LIMIT(MaxInstructions, MAX_PROGRAM_NATIVE_INSTRUCTIONS_ARB, 2048) \
    LIMIT(MaxLocalParams, MAX_PROGRAM_LOCAL_PARAMETERS_ARB, 256) \
    LIMIT(NumTemps, MAX_PROGRAM_TEMPORARIES_ARB, 32) \
  PROFILE_END(VP40) \
  \
  PROFILE_BEGIN(FP30) \
    LIMIT(NumInstructionSlots, MAX_PROGRAM_NATIVE_INSTRUCTIONS_ARB, 256) \
    LIMIT(NumTemps, MAX_PROGRAM_TEMPORARIES_ARB, 32) \
  PROFILE_END(FP30) \
  \
  PROFILE_BEGIN(FP40) \
    LIMIT(MaxLocalParams, MAX_PROGRAM_LOCAL_PARAMETERS_ARB, 1024) \
    LIMIT(NumInstructionSlots, MAX_PROGRAM_NATIVE_INSTRUCTIONS_ARB, 4096) \
    LIMIT(NumTemps, MAX_PROGRAM_TEMPORARIES_ARB, 32) \
  PROFILE_END(FP40)

  ProfileLimits::ProfileLimits (
    CS::PluginCommon::ShaderProgramPluginGL::HardwareVendor vendor,
    CGprofile profile)
   : vendor (vendor), profile (profile), 
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
  
  uint ProfileLimits::glGetProgramInteger (csGLExtensionManager* ext,
								 GLenum target, GLenum what)
  {
    GLint v;
    ext->glGetProgramivARB (target, what, &v);
    return v;
  }
  
  void ProfileLimits::SetDefaults ()
  {
#define PROFILE_BEGIN(PROFILE)  \
  case CG_PROFILE_ ## PROFILE:  \
    {
#define PROFILE_END(PROFILE)    \
    }                           \
    break;
#define LIMIT(Limit, glLimit, cgDefault)   \
      Limit = cgDefault;
  
    switch (profile)
    {
      PROFILES
      default:
        break;
    }
    
#undef PROFILE_BEGIN
#undef PROFILE_END
#undef LIMIT
  }
  
  static GLenum GetProgramIntegerTarget (CGprofile profile)
  {
    switch (profile)
    {
      case CG_PROFILE_VP30:
      case CG_PROFILE_VP40:
      case CG_PROFILE_ARBVP1:
	return GL_VERTEX_PROGRAM_ARB;
      case CG_PROFILE_FP30:
      case CG_PROFILE_FP40:
      case CG_PROFILE_ARBFP1:
	return GL_FRAGMENT_PROGRAM_ARB;
      default:
	return GL_NONE;
    }
  }

  void ProfileLimits::GetCurrentLimits (csGLExtensionManager* ext)
  {
#define PROFILE_BEGIN(PROFILE)  \
  case CG_PROFILE_ ## PROFILE:  \
    {						    \
      const GLenum target = GetProgramIntegerTarget (CG_PROFILE_ ## PROFILE);
#define PROFILE_END(PROFILE)    \
    }                           \
    break;
#define LIMIT(Limit, glLimit, cgDefault)   \
      Limit = (GL_ ## glLimit != GL_NONE)	\
	? glGetProgramInteger (ext, target, GL_ ## glLimit) : cgDefault;
  
    switch (profile)
    {
      PROFILES
      default:
        break;
    }
    
#undef PROFILE_BEGIN
#undef PROFILE_END
#undef LIMIT
  }

  void ProfileLimits::ReadFromConfig (iConfigFile* cfg, const char* _prefix)
  {
    csString prefix (_prefix);
    vendor = CS::PluginCommon::ShaderProgramPluginGL::VendorFromString (
      cfg->GetStr (prefix + ".Vendor", "other"));
    // Set defaults
    SetDefaults ();
#define READ(Limit) \
    { int x = cfg->GetInt (prefix + "." #Limit, -1); if (x >= 0) Limit = x; }
    READ (MaxAddressRegs);
    READ (MaxInstructions);
    READ (MaxLocalParams);
    READ (MaxTexIndirections);
    READ (NumInstructionSlots);
    READ (NumMathInstructionSlots);
    READ (NumTemps);
    READ (NumTexInstructionSlots);
#undef READ
  }
  
  void ProfileLimits::GetCgDefaults ()
  {
#define PROFILE_BEGIN(PROFILE)  \
  case CG_PROFILE_ ## PROFILE:  \
    {
#define PROFILE_END(PROFILE)    \
    }                           \
    break;
#define LIMIT(Limit, glLimit, cgDefault)   \
      Limit = cgDefault;
  
    switch (profile)
    {
      PROFILES
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

  bool ProfileLimits::FromString (const char* str)
  {
    csStringArray components;
    components.SplitString (str, ".");
    
    if (components.GetSize() == 0) return false;
    size_t i = 0;
    profile = cgGetProfile (components[i++]);
    if (profile == CG_PROFILE_UNKNOWN) return false;
  
    if (i >= components.GetSize()) return false;
    vendor = CS::PluginCommon::ShaderProgramPluginGL::VendorFromString (
      components[i++]);
    if (vendor == CS::PluginCommon::ShaderProgramPluginGL::Invalid)
      return false;
    
    uint usedLimits = 0;
  
#define PROFILE_BEGIN(PROFILE)  \
  case CG_PROFILE_ ## PROFILE:  \
    {
#define PROFILE_END(PROFILE)    \
    }                           \
    break;
#define LIMIT(Limit, glLimit, cgDefault)   \
      usedLimits |= 1 << lim ## Limit;
  
    switch (profile)
    {
      PROFILES
      default:
        break;
    }
    
#undef PROFILE_BEGIN
#undef PROFILE_END
#undef LIMIT

#define EMIT(Limit) \
  if (usedLimits & (1 << lim ## Limit)) \
  { \
    if (i >= components.GetSize()) return false; \
    int v; \
    char dummy; \
    if (sscanf (components[i++], "%d%c", &v, &dummy) != 1) return false; \
    Limit = v; \
  }
    EMIT (MaxInstructions);
    EMIT (NumInstructionSlots);
    EMIT (NumMathInstructionSlots);
    EMIT (NumTexInstructionSlots);
    EMIT (NumTemps);
    EMIT (MaxLocalParams);
    EMIT (MaxTexIndirections);
    EMIT (MaxAddressRegs);
#undef EMIT
    return i == components.GetSize();
  };
  
  csString ProfileLimits::ToString () const
  {
    uint usedLimits = 0;
  
#define PROFILE_BEGIN(PROFILE)  \
  case CG_PROFILE_ ## PROFILE:  \
    {
#define PROFILE_END(PROFILE)    \
    }                           \
    break;
#define LIMIT(Limit, glLimit, cgDefault)   \
      usedLimits |= 1 << lim ## Limit;
  
    switch (profile)
    {
      PROFILES
      default:
        break;
    }
    
#undef PROFILE_BEGIN
#undef PROFILE_END
#undef LIMIT

    csString ret (cgGetProfileString (profile));
    ret.AppendFmt (".%s",
      CS::PluginCommon::ShaderProgramPluginGL::VendorToString (vendor));
#define EMIT(Limit) if (usedLimits & (1 << lim ## Limit)) ret.AppendFmt (".%u", Limit);
    EMIT (MaxInstructions);
    EMIT (NumInstructionSlots);
    EMIT (NumMathInstructionSlots);
    EMIT (NumTexInstructionSlots);
    EMIT (NumTemps);
    EMIT (MaxLocalParams);
    EMIT (MaxTexIndirections);
    EMIT (MaxAddressRegs);
#undef EMIT
    return ret;
  };
  
  csString ProfileLimits::ToStringForPunyHumans () const
  {
    uint usedLimits = 0;
  
#define PROFILE_BEGIN(PROFILE)  \
  case CG_PROFILE_ ## PROFILE:  \
    {
#define PROFILE_END(PROFILE)    \
    }                           \
    break;
#define LIMIT(Limit, glLimit, cgDefault)   \
      usedLimits |= 1 << lim ## Limit;
  
    switch (profile)
    {
      PROFILES
      default:
        break;
    }
    
#undef PROFILE_BEGIN
#undef PROFILE_END
#undef LIMIT

    csString ret (cgGetProfileString (profile));
    ret.AppendFmt (" %s",
      CS::PluginCommon::ShaderProgramPluginGL::VendorToString (vendor));
#define EMIT(Limit) if (usedLimits & (1 << lim ## Limit)) \
      ret.AppendFmt (" " #Limit "=%u", Limit);
    EMIT (MaxInstructions);
    EMIT (NumInstructionSlots);
    EMIT (NumMathInstructionSlots);
    EMIT (NumTexInstructionSlots);
    EMIT (NumTemps);
    EMIT (MaxLocalParams);
    EMIT (MaxTexIndirections);
    EMIT (MaxAddressRegs);
#undef EMIT
    return ret;
  };
  
  void ProfileLimits::ToCgOptions (ArgumentArray& args) const
  {
#define PROFILE_BEGIN(PROFILE)  \
  case CG_PROFILE_ ## PROFILE:  \
    {
#define PROFILE_END(PROFILE)    \
    }                           \
    break;
#define LIMIT(Limit, glLimit, cgDefault)   \
      args.Push ("-po"); \
      args.Push (csString().Format (#Limit "=%u", Limit));
  
    switch (profile)
    {
      PROFILES
      default:
        break;
    }
    
#undef PROFILE_BEGIN
#undef PROFILE_END
#undef LIMIT
  }
    
  bool ProfileLimits::Write (iFile* file) const
  {
    {
      int32 diskVal = csLittleEndian::Int32 (vendor);
      if (file->Write ((char*)&diskVal, sizeof (diskVal)) != sizeof (diskVal))
        return false;
    }
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
    {
      int32 diskVal;
      if (file->Read ((char*)&diskVal, sizeof (diskVal)) != sizeof (diskVal))
        return false;
      vendor = (CS::PluginCommon::ShaderProgramPluginGL::HardwareVendor)
        csLittleEndian::Int32 (diskVal);
    }
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
    
    if (vendor < other.vendor) return true;
    if (vendor > other.vendor) return false;
  
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
#undef COMPARE
    return false;
  }
  
  bool ProfileLimits::operator> (const ProfileLimits& other) const
  {
    int p1 = GetProfileOrdering (profile);
    int p2 = GetProfileOrdering (other.profile);
    if (p1 > p2) return true;
    if (p1 < p2) return false;
  
    if (vendor > other.vendor) return true;
    if (vendor < other.vendor) return false;
  
#define COMPARE(Limit) \
    if (Limit > other.Limit) return true; \
    if (Limit < other.Limit) return false;
    COMPARE (MaxInstructions);
    COMPARE (NumInstructionSlots);
    COMPARE (NumMathInstructionSlots);
    COMPARE (NumTexInstructionSlots);
    COMPARE (NumTemps);
    COMPARE (MaxLocalParams);
    COMPARE (MaxTexIndirections);
    COMPARE (MaxAddressRegs);
#undef COMPARE
    return false;
  }
  
  bool ProfileLimits::operator== (const ProfileLimits& other) const
  {
    int p1 = GetProfileOrdering (profile);
    int p2 = GetProfileOrdering (other.profile);
    if (p1 != p2) return false;
    
    if (vendor != other.vendor) return false;
  
#define COMPARE(Limit) \
    if (Limit != other.Limit) return false;
    COMPARE (MaxInstructions);
    COMPARE (NumInstructionSlots);
    COMPARE (NumMathInstructionSlots);
    COMPARE (NumTexInstructionSlots);
    COMPARE (NumTemps);
    COMPARE (MaxLocalParams);
    COMPARE (MaxTexIndirections);
    COMPARE (MaxAddressRegs);
#undef COMPARE
    return true;
  }

  //-------------------------------------------------------------------------

  bool ProfileLimitsPair::FromString (const char* str)
  {
    csString tagFP (str);
    csString tagVP;
    {
      size_t semicolon = tagFP.FindFirst (';');
      if (semicolon == (size_t)-1) return false;
      tagFP.SubString (tagVP, semicolon+1, tagFP.Length() - (semicolon+1));
      tagFP.Truncate (semicolon);
    }
    
    if (!vp.FromString (tagVP))
      return false;
    
    if (!fp.FromString (tagFP))
      return false;
      
    return true;
  }
  
  csString ProfileLimitsPair::ToString () const
  {
    return csString().Format ("%s;%s",
      fp.ToString().GetData(), vp.ToString().GetData());
  }

}
CS_PLUGIN_NAMESPACE_END(GLShaderCg)


