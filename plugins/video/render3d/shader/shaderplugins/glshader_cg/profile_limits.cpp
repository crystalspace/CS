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
/* At least Cg 2.1 seems to store the profile limits as 16-bit integers,
   so use 0x7fff when something is 'unlimited'. */
#define UNLIMITED  0x7fff

// Tabularize what profiles use what limits
#define PROFILES  \
  PROFILE_BEGIN(ARBVP1) \
    LIMIT(MaxAddressRegs, MAX_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB, 1, 8) \
    LIMIT(MaxInstructions, MAX_PROGRAM_NATIVE_INSTRUCTIONS_ARB, 1024, 4096) \
    LIMIT(MaxLocalParams, MAX_PROGRAM_LOCAL_PARAMETERS_ARB, 96, UNLIMITED) \
    LIMIT(NumTemps, MAX_PROGRAM_TEMPORARIES_ARB, 32, 32)  \
    USESEXT(ARB_color_buffer_float, false) \
  PROFILE_END(ARBVP1) \
  \
  PROFILE_BEGIN(ARBFP1) \
    LIMIT(MaxLocalParams, MAX_PROGRAM_LOCAL_PARAMETERS_ARB, 32, UNLIMITED) \
    LIMIT(MaxTexIndirections, MAX_PROGRAM_NATIVE_TEX_INDIRECTIONS_ARB, UNLIMITED, UNLIMITED) \
    LIMIT(NumInstructionSlots, MAX_PROGRAM_NATIVE_INSTRUCTIONS_ARB, 1024, UNLIMITED) \
    LIMIT(NumMathInstructionSlots, MAX_PROGRAM_NATIVE_ALU_INSTRUCTIONS_ARB, 1024, UNLIMITED) \
    LIMIT(NumTemps, MAX_PROGRAM_TEMPORARIES_ARB, 32, UNLIMITED) \
    LIMIT(NumTexInstructionSlots, MAX_PROGRAM_NATIVE_TEX_INSTRUCTIONS_ARB, 1024, UNLIMITED) \
    USESEXT(ARB_color_buffer_float, false) \
  PROFILE_END(ARBFP1) \
  \
  PROFILE_BEGIN(VP40) \
    LIMIT(MaxAddressRegs, MAX_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB, 2, 8) \
    LIMIT(MaxInstructions, MAX_PROGRAM_NATIVE_INSTRUCTIONS_ARB, 2048, 4096) \
    LIMIT(MaxLocalParams, MAX_PROGRAM_LOCAL_PARAMETERS_ARB, 256, UNLIMITED) \
    LIMIT(NumTemps, MAX_PROGRAM_TEMPORARIES_ARB, 32, UNLIMITED) \
  PROFILE_END(VP40) \
  \
  PROFILE_BEGIN(FP30) \
    LIMIT(NumInstructionSlots, MAX_PROGRAM_NATIVE_INSTRUCTIONS_ARB, 256, UNLIMITED) \
    LIMIT(NumTemps, MAX_PROGRAM_TEMPORARIES_ARB, 32, 32) \
    USESEXT(ARB_color_buffer_float, false) \
  PROFILE_END(FP30) \
  \
  PROFILE_BEGIN(FP40) \
    LIMIT(MaxLocalParams, MAX_PROGRAM_LOCAL_PARAMETERS_ARB, 1024, UNLIMITED) \
    LIMIT(NumInstructionSlots, MAX_PROGRAM_NATIVE_INSTRUCTIONS_ARB, 4096, UNLIMITED) \
    LIMIT(NumTemps, MAX_PROGRAM_TEMPORARIES_ARB, 32, UNLIMITED) \
  PROFILE_END(FP40)

  // Extensions that influence profile behaviour - bit mask
  enum
  {
    extARB_color_buffer_float = 1
  };

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
     NumTexInstructionSlots (0),
     extensions (0)
  {
    FixupVendor();
  }
  
  uint ProfileLimits::glGetProgramInteger (csGLExtensionManager* ext,
					   GLenum target, GLenum what)
  {
    GLint v = 0;
    ext->glGetProgramivARB (target, what, &v);
    /* If we happen to get a 0, try the non-native variant of a limit
     * (on some Intel HW native limits are all 0, but non-native ones are
     * useable) */
    if (v == 0)
    {
      switch (what)
      {
      case GL_MAX_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB:
	ext->glGetProgramivARB (target, GL_MAX_PROGRAM_ADDRESS_REGISTERS_ARB, &v);
	break;
      case GL_MAX_PROGRAM_NATIVE_ATTRIBS_ARB:
	ext->glGetProgramivARB (target, GL_MAX_PROGRAM_ATTRIBS_ARB, &v);
	break;
      case GL_MAX_PROGRAM_NATIVE_INSTRUCTIONS_ARB:
	ext->glGetProgramivARB (target, GL_MAX_PROGRAM_INSTRUCTIONS_ARB, &v);
	break;
      case GL_MAX_PROGRAM_NATIVE_PARAMETERS_ARB:
	ext->glGetProgramivARB (target, GL_MAX_PROGRAM_PARAMETERS_ARB, &v);
	break;
      case GL_MAX_PROGRAM_NATIVE_TEMPORARIES_ARB:
	ext->glGetProgramivARB (target, GL_MAX_PROGRAM_TEMPORARIES_ARB, &v);
	break;
      }
    }
    return v;
  }
  
  void ProfileLimits::SetDefaults ()
  {
    extensions = 0;
#define PROFILE_BEGIN(PROFILE)  \
  case CG_PROFILE_ ## PROFILE:  \
    {
#define PROFILE_END(PROFILE)    \
    }                           \
    break;
#define LIMIT(Limit, glLimit, cgDefault, cgMax)   \
      Limit = cgDefault;
#define USESEXT(X, defaultPresent)   \
      if (defaultPresent) extensions |= ext ## X;
  
    switch (profile)
    {
      PROFILES
      default:
        break;
    }
    
#undef PROFILE_BEGIN
#undef PROFILE_END
#undef LIMIT
#undef USESEXT
  }

  const char* ProfileLimits::GetProfileString (CGprofile p)
  {
    switch (p)
    {
      case CG_PROFILE_UNKNOWN: return "(unknown)";
      default: return cgGetProfileString (p);
    }
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
#define LIMIT(Limit, glLimit, cgDefault, cgMax)   \
      Limit = (GL_ ## glLimit != GL_NONE)	\
	? glGetProgramInteger (ext, target, GL_ ## glLimit) : cgDefault; \
      if (Limit > cgMax) Limit = cgMax;
#define USESEXT(X, defaultPresent)   \
      if (ext->CS_GL_##X) extensions |= ext ## X;
  
    switch (profile)
    {
      PROFILES
      default:
        break;
    }
    
#undef PROFILE_BEGIN
#undef PROFILE_END
#undef LIMIT
#undef USESEXT
  }

  void ProfileLimits::ReadFromConfig (iConfigFile* cfg, const char* _prefix)
  {
    csString prefix (_prefix);
    vendor = CS::PluginCommon::ShaderProgramPluginGL::VendorFromString (
      cfg->GetStr (prefix + ".Vendor", "other"));
    FixupVendor();
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
#define READEXT(Ext) \
    {	\
      bool b = cfg->GetBool (prefix + ".Ext." #Ext, false); \
      if (b) extensions |= ext##Ext; else extensions &= ~(ext##Ext); \
    }
    READEXT (ARB_color_buffer_float);
#undef READ
#undef READEXT
  }
  
  void ProfileLimits::GetCgDefaults ()
  {
    extensions = 0;
#define PROFILE_BEGIN(PROFILE)  \
  case CG_PROFILE_ ## PROFILE:  \
    {
#define PROFILE_END(PROFILE)    \
    }                           \
    break;
#define LIMIT(Limit, glLimit, cgDefault, cgMax)   \
      Limit = cgDefault;
#define USESEXT(X, defaultPresent)   \
      if (defaultPresent) extensions |= ext ## X;
  
    switch (profile)
    {
      PROFILES
      default:
        break;
    }
    
#undef PROFILE_BEGIN
#undef PROFILE_END
#undef LIMIT
#undef USESEXT
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
    FixupVendor();
    if (vendor == CS::PluginCommon::ShaderProgramPluginGL::Invalid)
      return false;
    
    uint usedLimits = 0;
    uint usedExts = 0;
  
#define PROFILE_BEGIN(PROFILE)  \
  case CG_PROFILE_ ## PROFILE:  \
    {
#define PROFILE_END(PROFILE)    \
    }                           \
    break;
#define LIMIT(Limit, glLimit, cgDefault, cgMax)   \
      usedLimits |= 1 << lim ## Limit;
#define USESEXT(X, defaultPresent)   \
      usedExts |= ext ## X;
  
    switch (profile)
    {
      PROFILES
      default:
        break;
    }
    
#undef PROFILE_BEGIN
#undef PROFILE_END
#undef LIMIT
#undef USESEXT

    if (usedExts != 0)
    {
      if (i >= components.GetSize()) return false;
      uint v;
      char dummy;
      if (sscanf (components[i++], "%u%c", &v, &dummy) != 1) return false;
      extensions = v;
    }
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
    uint usedExts = 0;
  
#define PROFILE_BEGIN(PROFILE)  \
  case CG_PROFILE_ ## PROFILE:  \
    {
#define PROFILE_END(PROFILE)    \
    }                           \
    break;
#define LIMIT(Limit, glLimit, cgDefault, cgMax)   \
      usedLimits |= 1 << lim ## Limit;
#define USESEXT(X, defaultPresent)   \
      usedExts |= ext ## X;
  
    switch (profile)
    {
      PROFILES
      default:
        break;
    }
    
#undef PROFILE_BEGIN
#undef PROFILE_END
#undef LIMIT
#undef USESEXT

    csString ret (GetProfileString (profile));
    ret.AppendFmt (".%s",
      CS::PluginCommon::ShaderProgramPluginGL::VendorToString (vendor));
    if (usedExts != 0)
    {
      ret.AppendFmt (".%u", extensions);
    }
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
    uint usedExts = 0;
  
#define PROFILE_BEGIN(PROFILE)  \
  case CG_PROFILE_ ## PROFILE:  \
    {
#define PROFILE_END(PROFILE)    \
    }                           \
    break;
#define LIMIT(Limit, glLimit, cgDefault, cgMax)   \
      usedLimits |= 1 << lim ## Limit;
#define USESEXT(X, defaultPresent)   \
      usedExts |= ext ## X;
  
    switch (profile)
    {
      PROFILES
      default:
        break;
    }
    
#undef PROFILE_BEGIN
#undef PROFILE_END
#undef LIMIT
#undef USESEXT

    csString ret (GetProfileString (profile));
    ret.AppendFmt (" %s",
      CS::PluginCommon::ShaderProgramPluginGL::VendorToString (vendor));
      
#define EXT(X)	\
    if (usedExts & ext##X)\
      ret.AppendFmt (" " #X "=%s", extensions & ext##X ? "y" : "n")
    EXT(ARB_color_buffer_float);
#undef EXT
      
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
#define LIMIT(Limit, glLimit, cgDefault, cgMax)   \
      args.Push ("-po"); \
      args.Push (csString().Format (#Limit "=%u", Limit));
#define USESEXT(X, defaultPresent)   \
      if (extensions & ext##X)	\
	args.Push (csString().Format ("-DHAVE_" #X));
  
    switch (profile)
    {
      PROFILES
      default:
        break;
    }
    
#undef PROFILE_BEGIN
#undef PROFILE_END
#undef LIMIT
#undef USESEXT
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
    WRITE (extensions);
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
      FixupVendor ();
    }
#define READ(Limit) \
    { \
      uint32 diskVal; \
      if (file->Read ((char*)&diskVal, sizeof (diskVal)) != sizeof (diskVal)) \
        return false; \
      Limit = csLittleEndian::UInt32 (diskVal); \
    }
    READ (extensions);
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
    COMPARE (extensions); // @@@ Right?
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
    COMPARE (extensions); // @@@ Right?
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
    COMPARE (extensions);
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


