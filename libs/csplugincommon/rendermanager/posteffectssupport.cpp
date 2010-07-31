/*
    Copyright (C) 2010 by Frank Richter

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

#include "csplugincommon/rendermanager/posteffectssupport.h"

#include "csutil/cfgacc.h"
#include "imap/loader.h"

namespace CS
{
  namespace RenderManager
  {
    PostEffectsSupport::PostEffectsSupport() : postEffectsParser (nullptr)
    {
    }
    
    PostEffectsSupport::~PostEffectsSupport()
    {
      delete postEffectsParser;
    }
    
    void PostEffectsSupport::Initialize (iObjectRegistry* objectReg, const char* configKey)
    {
      CS_ASSERT(!postEffectsParser);
      postEffectsParser = new PostEffectLayersParser (objectReg);
      
      postEffects.Initialize (objectReg);
      
      if (configKey)
      {
	csString realConfigKey (configKey);
	realConfigKey.Append (".Effects");
	csConfigAccess cfg (objectReg);
 	const char* effectsFile = cfg->GetStr (realConfigKey, nullptr);
	if (effectsFile)
	{
	  postEffectsParser->AddLayersFromFile (effectsFile, postEffects);
	}
      }
    }
    
    bool PostEffectsSupport::AddLayersFromDocument (iDocumentNode* node)
    {
      CS_ASSERT(postEffectsParser);
      return postEffectsParser->AddLayersFromDocument (node, postEffects);
    }
    
    bool PostEffectsSupport::AddLayersFromFile (const char* filename)
    {
      CS_ASSERT(postEffectsParser);
      return postEffectsParser->AddLayersFromFile (filename, postEffects);
    }
  } // namespace RenderManager
} // namespace CS
