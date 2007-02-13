/*
  Copyright (C) 2003-2007 by Marten Svanfeldt
		2004-2007 by Frank Richter

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

#ifndef __CS_CSPLUGINCOMMON_SHADER_WEAVERCOMBINER_H__
#define __CS_CSPLUGINCOMMON_SHADER_WEAVERCOMBINER_H__

#include "csutil/scf.h"

/**\file
 */

/**\addtogroup plugincommon
 * @{ */

struct iDocumentNode;
struct iDocumentNodeIterator;

namespace CS
{
  namespace PluginCommon
  {
    namespace ShaderWeaver
    {
      static const uint NoCoercion = UINT_MAX;
    
      struct iCombiner : public virtual iBase
      {
        SCF_INTERFACE (iCombiner, 0, 0, 1);
        
        virtual void BeginSnippet () = 0;
        virtual void AddInput (const char* name, const char* type) = 0;
        virtual void AddOutput (const char* name, const char* type) = 0;
	virtual void InputRename (const char* fromName, const char* toName) = 0;
	virtual void OutputRename (const char* fromName, const char* toName) = 0;
        virtual csPtr<iDocumentNodeIterator> QueryCoerceChain (const char* fromType,
          const char* toType) = 0;
	virtual void Link (const char* fromName, const char* toName) = 0;
        virtual void WriteBlock (const char* location, 
          iDocumentNode* blockNodes) = 0;
        virtual bool EndSnippet () = 0;
        
        virtual void AddGlobal (const char* name, const char* type) = 0;
        virtual void SetOutput (const char* name) = 0;
        
        virtual uint CoerceCost (const char* fromType, const char* toType) = 0;
        
        virtual void WriteToPass (iDocumentNode* pass) = 0;
        
        virtual bool CompatibleParams (iDocumentNode* params) = 0;
      };
      
      struct iCombinerLoader : public virtual iBase
      {
        SCF_INTERFACE (iCombinerLoader, 0, 0, 1);
        
        virtual csPtr<iCombiner> GetCombiner (iDocumentNode* params) = 0;
      };
    } // namespace ShaderWeaver
  } // namespace PluginCommon
} // namespace CS

/** @} */

#endif // __CS_CSPLUGINCOMMON_SHADER_WEAVERCOMBINER_H__

