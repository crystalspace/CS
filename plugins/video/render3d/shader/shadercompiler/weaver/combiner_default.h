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

#ifndef __CS_COMBINER_DEFAULT_H__
#define __CS_COMBINER_DEFAULT_H__

#include "csplugincommon/shader/weavercombiner.h"
#include "csutil/refarr.h"
#include "csutil/scf_implementation.h"

CS_PLUGIN_NAMESPACE_BEGIN(ShaderWeaver)
{

  class CombinerDefault : 
    public scfImplementation1<CombinerDefault,
			      CS::PluginCommon::ShaderWeaver::iCombiner>
  {
    csRefArray<iDocumentNode> passNodes;
  public:
    CombinerDefault ();
    
    void BeginSnippet ();
    void AddInput (const char* name, const char* type) {}
    void AddOutput (const char* name, const char* type) {}
    void InputRename (const char* fromName, const char* toName);
    void OutputRename (const char* fromName, const char* toName);
    csPtr<iDocumentNodeIterator> QueryCoerceChain (const char* fromType,
      const char* toType) { return 0; }
    void Link (const char* fromName, const char* toName) {}
    void WriteBlock (const char* location, iDocumentNode* blockNodes);
    bool EndSnippet ();
    void AddGlobal (const char* name, const char* type) {}
    void SetOutput (const char* name) {}
    uint CoerceCost (const char* fromType, const char* toType);
    void WriteToPass (iDocumentNode* pass);
    bool CompatibleParams (iDocumentNode* params);
  };

}
CS_PLUGIN_NAMESPACE_END(ShaderWeaver)

#endif // __CS_COMBINER_DEFAULT_H__
