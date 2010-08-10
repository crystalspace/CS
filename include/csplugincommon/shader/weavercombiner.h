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

#include "ivideo/graph3d.h"

#include "csutil/scf.h"
#include "csgeom/vector4.h"
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

      struct iCoerceChainIterator : public iDocumentNodeIterator
      {
        SCF_INTERFACE (iCoerceChainIterator, 0, 1, 0);
        
        virtual csRef<iDocumentNode> Next () = 0;
        virtual csRef<iDocumentNode> Next (const char*& fromType, 
          const char*& toType) = 0;
      };
    
      struct iCombiner : public virtual iBase
      {
        SCF_INTERFACE (iCombiner, 1, 0, 1);
        
        /// Start addition of a new snippet.
        virtual void BeginSnippet (const char* annotation = 0) = 0;
        /// Add an input of the snippet.
        virtual void AddInput (const char* name, const char* type) = 0;
        /// Add an input of the snippet with a given value.
        virtual void AddInputValue (const char* name, const char* type,
          const char* value) = 0;
        /// Add an output of the snippet.
        virtual void AddOutput (const char* name, const char* type) = 0;
        /// Specify a renaming for an input.
	virtual void InputRename (const char* fromName, const char* toName) = 0;
        /// Specify a renaming for an output.
	virtual void OutputRename (const char* fromName, const char* toName) = 0;
        /**
         * Instruct combiner to propagate all attributes from an input (name
         * is mapped) to an output (name is unmapped).
         */
        virtual void PropagateAttributes (const char* fromInput,
          const char* toOutput) = 0;
        /// Add an attribute to an output (name is unmapped)
        virtual void AddOutputAttribute (const char* outputName, 
          const char* name, const char* type) = 0;
        /// Add an attribute to an input (name is mapped)
        virtual void AddInputAttribute (const char* inputName,
          const char* name, const char* type, const char* defVal) = 0;
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
        virtual void AddGlobal (const char* name, const char* type,
          const char* annotation = 0) = 0;
        /// Set output variable.
        virtual void SetOutput (csRenderTargetAttachment target,
          const char* name, const char* annotation = 0) = 0;
        
        /**
         * Query a chain of coercions from a type to another.
         * Nodes are atom-snippet-esque.
         */
        /* @@@ FIXME: A bit of a design issue that weaver nodes are returned,
                      since that means combiners have to "know" the syntax of
                      weaver input docs! */
        virtual csPtr<iCoerceChainIterator> QueryCoerceChain (const char* fromType,
          const char* toType) = 0;

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
	  
	/**
	 * Set a descriptive name of the program to be combined.
	 * Emitted to the output, can be used to identify generated programs.
	 */
	virtual void SetDescription (const char* descr) = 0;
      };
      
      struct iCombinerLoader : public virtual iBase
      {
        SCF_INTERFACE (iCombinerLoader, 0, 0, 4);
        
        virtual csPtr<iCombiner> GetCombiner (iDocumentNode* params) = 0;

        virtual void GenerateConstantInputBlocks (iDocumentNode* node,
          const char* locationPrefix, const csVector4& value,
          int usedComponents, const char* outputName) = 0;
        virtual void GenerateSVInputBlocks (iDocumentNode* node,
          const char* locationPrefix, const char* svName, 
          const char* outputType, const char* outputName, 
          const char* uniqueTag) = 0;
        virtual void GenerateBufferInputBlocks (iDocumentNode* node,
          const char* locationPrefix, const char* bufName, 
          const char* outputType, const char* outputName, 
          const char* uniqueTag) = 0;

	/**
	 * A short string identifying the revision or some such of the combiner.
	 * It is used to trigger regeneration of shaders if changed. Thus, a fix
	 * or such that affects existing, cached shaders should result in a change
	 * of this code in order to trigger the regeneration of the previously
	 * cached shaders.
	 */
	virtual const char* GetCodeString() = 0;
      };
    } // namespace ShaderWeaver
  } // namespace PluginCommon
} // namespace CS

/** @} */

#endif // __CS_CSPLUGINCOMMON_SHADER_WEAVERCOMBINER_H__

