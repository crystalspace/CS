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

#include "csplugincommon/opengl/shaderplugin.h"

struct csGLExtensionManager;
struct iFile;

CS_PLUGIN_NAMESPACE_BEGIN(GLShaderCg)
{

  struct ProfileLimits
  {
    CS::PluginCommon::ShaderProgramPluginGL::HardwareVendor vendor;
    CGprofile profile;
    uint MaxAddressRegs;
    uint MaxInstructions;
    uint MaxLocalParams;
    uint MaxTexIndirections;
    uint NumInstructionSlots;
    uint NumMathInstructionSlots;
    uint NumTemps;
    uint NumTexInstructionSlots;
    uint extensions;
    
    ProfileLimits (
      CS::PluginCommon::ShaderProgramPluginGL::HardwareVendor vendor,
      CGprofile profile);
    
    void GetCurrentLimits (csGLExtensionManager* ext);
    void ReadFromConfig (iConfigFile* cfg, const char* prefix);
    void GetCgDefaults ();
    
    bool FromString (const char* str);
    csString ToString () const;
    void ToCgOptions (ArgumentArray& args) const;
    csString ToStringForPunyHumans () const;
    
    bool Write (iFile* file) const;
    bool Read (iFile* file);
    
    bool operator< (const ProfileLimits& other) const;
    bool operator> (const ProfileLimits& other) const;
    bool operator== (const ProfileLimits& other) const;
    bool operator!= (const ProfileLimits& other) const
    { return !operator== (other); }
    bool operator>= (const ProfileLimits& other) const
    { return !operator< (other); }
  private:
    void FixupVendor ()
    {
      /* We only really distinguish the vendor here for purposes of clipping -
         ATI can do a bit 'more' than what the ARBVP spec says. In that respect,
	 NV acts like 'other'. NV-specific clipping requires the NV VP support
	 which isn't available elsewhere anyway.
	 And since the vendor really only matters for ARBVP, and NVIDIA or
	 Other doesn't make a difference, reduce NVIDIA to Other.
       */
      if (vendor == CS::PluginCommon::ShaderProgramPluginGL::NVIDIA)
	vendor = CS::PluginCommon::ShaderProgramPluginGL::Other;
    }
    void SetDefaults ();
    /// Like cgGetProfileString(), but also handles CG_PROFILE_UNKNOWN
    static const char* GetProfileString (CGprofile);
    static uint glGetProgramInteger (csGLExtensionManager* ext,
      GLenum target, GLenum what);
  };
  
  struct ProfileLimitsPair
  {
    ProfileLimits vp;
    ProfileLimits fp;
    
    ProfileLimitsPair() :
      vp (CS::PluginCommon::ShaderProgramPluginGL::Other,
        CG_PROFILE_UNKNOWN),
      fp (CS::PluginCommon::ShaderProgramPluginGL::Other,
        CG_PROFILE_UNKNOWN) {}
    ProfileLimitsPair (
      CS::PluginCommon::ShaderProgramPluginGL::HardwareVendor vendorVP,
      CGprofile profileVP,
      CS::PluginCommon::ShaderProgramPluginGL::HardwareVendor vendorFP,
      CGprofile profileFP) : vp (vendorVP, profileVP),
        fp (vendorFP, profileFP) {}
      
    void GetCurrentLimits (csGLExtensionManager* ext)
    {
      vp.GetCurrentLimits (ext);
      fp.GetCurrentLimits (ext);
    }
    
    bool FromString (const char* str);
    csString ToString () const;
    
    bool operator< (const ProfileLimitsPair& other) const
    {
      if (fp < other.fp) return true;
      if (fp == other.fp) return false;
      return vp < other.vp;
    }
    bool operator> (const ProfileLimitsPair& other) const
    {
      if (fp > other.fp) return true;
      if (fp == other.fp) return false;
      return vp > other.vp;
    }
    bool operator== (const ProfileLimitsPair& other) const
    {
      return (fp == other.fp) && (vp == other.vp);
    }
    bool operator!= (const ProfileLimitsPair& other) const
    { return !operator== (other); }
    bool operator>= (const ProfileLimitsPair& other) const
    { return !operator< (other); }
  };

}
CS_PLUGIN_NAMESPACE_END(GLShaderCg)

#endif // __PROFILE_LIMITS_H__
