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

#include "csplugincommon/rendermanager/posteffects.h"
#include "csplugincommon/rendermanager/shadow_common.h"

#include "imap/loader.h"
#include "iutil/objreg.h"

#include "csutil/cfgacc.h"

namespace CS
{
  namespace RenderManager
  {
    void ShadowSettings::ReadSettings (iObjectRegistry* objReg, 
                                       const char* shadowType)
    {
      csConfigAccess cfg (objReg, "/config/shadows.cfg");
      csRef<iStringSet> strings = csQueryRegistryTagInterface<iStringSet> (objReg,
	"crystalspace.shared.stringset");
      
      const char* shaderType = cfg->GetStr (
        csString().Format ("RenderManager.Shadows.%s.Shader.Type", shadowType),
        "shadow");
      this->shadowShaderType = strings->Request (shaderType);
      
      const char* defaultShader = cfg->GetStr (
	  csString().Format ("RenderManager.Shadows.%s.Shader.Default", shadowType));
      if (defaultShader != 0)
      {
	csRef<iLoader> loader (csQueryRegistry<iLoader> (objReg));
  shadowDefaultShader = loader->LoadShader (defaultShader);
      }
      
      const char* postEffectsLayers = cfg->GetStr (
        csString().Format ("RenderManager.Shadows.%s.PostProcess", shadowType),
        0);
      if (postEffectsLayers != 0)
      {
	postEffects.AttachNew (new PostEffectManager);
	postEffects->Initialize (objReg);
	PostEffectLayersParser layerParser (objReg);
	layerParser.AddLayersFromFile (postEffectsLayers, *postEffects);
      }
            
      csRef<iShaderManager> shaderManager =
	csQueryRegistry<iShaderManager> (objReg);
      iShaderVarStringSet* svStrings = shaderManager->GetSVNameStringset();
      
      ReadTargets (targets, cfg,
        csString().Format ("RenderManager.Shadows.%s.Texture.", shadowType),
        svStrings, objReg);
	
      if (provideIDs)
      {
	svMeshIDName = svStrings->Request ("shadowmap mesh id");
      }
      else
        svMeshIDName = CS::InvalidShaderVarStringID;
        
      this->shadowShaderType = strings->Request (shaderType);
    }
    
    void ShadowSettings::AdvanceFrame (csTicks time)
    {
      for (size_t t = 0; t < targets.GetSize(); t++)
        targets[t]->texCache.AdvanceFrame (time);
    }
    
    bool ShadowSettings::ReadTargets (TargetArray& targets, iConfigFile* cfg,
                                      const char* prefix,
                                      iShaderVarStringSet* svStrings,
                                      iObjectRegistry* objReg)
    {
      csRef<iGraphics3D> g3d = csQueryRegistry<iGraphics3D> (objReg);
      
      csSet<csString> seenTextures;
      csRef<iConfigIterator> texKeys = cfg->Enumerate (prefix);
      
      while (texKeys->HasNext())
      {
        texKeys->Next();
        csString currentKey (texKeys->GetKey (true));
        size_t dotPos = currentKey.FindFirst ('.');
        if (dotPos != (size_t)-1)
        {
          csString texID;
          currentKey.SubString (texID, 0, dotPos);
          if (!seenTextures.Contains (texID))
          {
            const char* attachmentStr = cfg->GetStr (
              csString().Format ("%s%s.Attachment", prefix, texID.GetData()),
              "depth");
            const char* svNameStr = cfg->GetStr (
              csString().Format ("%s%s.ShaderVar", prefix, texID.GetData()),
              "light shadow map");
            const char* formatStr = cfg->GetStr (
              csString().Format ("%s%s.Format", prefix, texID.GetData()),
              "d32");
            
            csRenderTargetAttachment attachment;
            if (strcmp (attachmentStr, "depth") == 0)
              attachment = rtaDepth;
            else if (strcmp (attachmentStr, "color0") == 0)
              attachment = rtaColor0;
            else
              return false;
              
            if (postEffects.IsValid())
            {
              if (attachment == rtaColor0)
              {
                postEffects->SetIntermediateTargetFormat (formatStr);
              }
            }
              
            CS::ShaderVarStringID svName = svStrings->Request (svNameStr);
              
            uint texFlags = 0;
            if (cfg->GetBool (
		csString().Format ("%s%s.NoMipMap", prefix, texID.GetData()),
		true))
	     texFlags |= CS_TEXTURE_NOMIPMAPS;
            if (cfg->GetBool (
		csString().Format ("%s%s.NoFilter", prefix, texID.GetData()),
		false))
	     texFlags |= CS_TEXTURE_NOFILTER;
	     
            Target* newTarget = new Target (attachment, svName, formatStr,
              texFlags);
            newTarget->texCache.SetG3D (g3d);
            targets.Push (newTarget);
            
            seenTextures.AddNoTest (texID);
          }
        }
      }
      return true;
    }
    
  } // namespace RenderManager
} // namespace CS

