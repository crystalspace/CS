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
#include <limits.h>

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
        SCF_INTERFACE (iCombiner, 0, 0, 2);
        
        /// Start addition of a new snippet.
        virtual void BeginSnippet () = 0;
        /// Add an input of the snippet.
        virtual void AddInput (const char* name, const char* type) = 0;
        /// Add an output of the snippet.
        virtual void AddOutput (const char* name, const char* type) = 0;
        /// Specify a renaming for an input.
	virtual void InputRename (const char* fromName, const char* toName) = 0;
        /// Specify a renaming for an output.
	virtual void OutputRename (const char* fromName, const char* toName) = 0;
        /**
         * Query a chain of coercions from a type to another.
         * Nodes are atom-snippet-esque.
         */
        virtual csPtr<iDocumentNodeIterator> QueryCoerceChain (const char* fromType,
          const char* toType) = 0;
        /**
         * Add a link from a variable (usually an output of the snippet) to
         * another variable (usually an input to some other snippet).
         */
	virtual void Link (const char* fromName, const char* toName) = 0;
        /// Add contents of a &lt;block&gt; node.
        virtual void WriteBlock (const char* location, 
          iDocumentNode* blockNodes) = 0;
        /// Finish snippet.
        virtual bool EndSnippet () = 0;
        
        /// Add a global variable.
        virtual void AddGlobal (const char* name, const char* type) = 0;
        /// Set output variable.
        virtual void SetOutput (const char* name) = 0;
        
        /**
         * Compute a cost for a coercion from one type to another.
         * Metric is arbitrary; however, when comparing two costs, the lower
         * cost should indicate that the coercion is "cheaper" (ie faster) at
         * runtime than the other. A cost of 0 means that a coercion is free.
         */
        virtual uint CoerceCost (const char* fromType, const char* toType) = 0;
        
        /// Write accumulated snippets to pass node.
        virtual void WriteToPass (iDocumentNode* pass) = 0;
        
        /**
         * Check if the combiner options in node 'params' are compatible to
         * this combiner instance.
         */
        virtual bool CompatibleParams (iDocumentNode* params) = 0;

        /**
         * Query an identifying "tag" for input coming from the specified 
         * block. The same tag for multiple inputs should essentially
         * indicate that the same input resource is referenced.
         * Can return 0 if no tag can be determined unambigously.
         */
        virtual csRef<iString> QueryInputTag (const char* location, 
          iDocumentNode* blockNodes) = 0;
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

