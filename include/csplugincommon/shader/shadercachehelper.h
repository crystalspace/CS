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

#ifndef __CS_CSPLUGINCOMMON_SHADER_SHADERCACHEHELPER_H__
#define __CS_CSPLUGINCOMMON_SHADER_SHADERCACHEHELPER_H__

/**\file
 */

#include "csutil/csmd5.h"
#include "csutil/fifo.h"
#include "csutil/memfile.h"
#include "csutil/ref.h"

struct iDataBuffer;
struct iDocumentNode;
struct iDocumentSystem;
struct iFile;
struct iObjectRegistry;

/**\addtogroup plugincommon
 * @{ */

namespace CS
{
  namespace PluginCommon
  {
    namespace ShaderCacheHelper
    {
      /**
       * Computes a hash for a shader document, including all referenced
       * documents (ie external files pulled in via "file" attributes or
       * ?Include? PIs).
       */
      class CS_CRYSTALSPACE_EXPORT ShaderDocHasher
      {
        struct DocStackEntry
        {
          csRef<iDocumentNode> docNode;
          csRef<iDataBuffer> sourceData;
          
          csMD5::Digest ComputeHash ();
        };
        csFIFO<DocStackEntry> scanStack;
        void PushReferencedFiles (DocStackEntry& entry);
        void PushReferencedFiles (iDocumentNode* node);
        bool AddFile (const char* filename);
      
        csRef<iDocumentSystem> docSys;
        csRef<iVFS> vfs;
        csMemFile actualHashes;
      public:
        ShaderDocHasher (iObjectRegistry* objReg, iDocumentNode* doc);
        
        csPtr<iDataBuffer> GetHashStream ();
        bool ValidateHashStream (iDataBuffer* stream);
      };
    } // namespace ShaderCacheHelper
  } // namespace PluginCommon
} // namespace CS

/** @} */

#endif // __CS_CSPLUGINCOMMON_SHADER_SHADERCACHEHELPER_H__

