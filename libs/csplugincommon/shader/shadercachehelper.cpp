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

#include "csplugincommon/shader/shadercachehelper.h"

#include "iutil/objreg.h"
#include "csutil/databuf.h"
#include "csutil/documenthelper.h"
#include "csutil/xmltiny.h"

namespace CS
{
  namespace PluginCommon
  {
    namespace ShaderCacheHelper
    {
      csMD5::Digest ShaderDocHasher::DocStackEntry::ComputeHash ()
      {
        CS_ASSERT(docNode || sourceData);
        if (!sourceData.IsValid())
        {
          csString nodeFlat (CS::DocSystem::FlattenNode (docNode));
          size_t nodeFlatLen = nodeFlat.Length();
          sourceData.AttachNew (new csDataBuffer (nodeFlat.Detach(), nodeFlatLen));
        }
        return csMD5::Encode (sourceData->GetData(), sourceData->GetSize());
      }
      
      //---------------------------------------------------------------------
      
      ShaderDocHasher::ShaderDocHasher (iObjectRegistry* objReg,
                                        iDocumentNode* doc)
      {
        vfs = csQueryRegistry<iVFS> (objReg);
        docSys = csQueryRegistry<iDocumentSystem> (objReg);
        if (!docSys.IsValid())
          docSys.AttachNew (new csTinyDocumentSystem);
          
	if (doc != 0)
	{
	  DocStackEntry newEntry;
	  newEntry.docNode = doc;
	  scanStack.Push (newEntry);
	}
      }
      
      void ShaderDocHasher::PushReferencedFiles (DocStackEntry& entry)
      {
        CS_ASSERT(entry.docNode || entry.sourceData);
        if (!entry.docNode.IsValid())
        {
          csRef<iDocument> doc = docSys->CreateDocument();
          if (doc->Parse (entry.sourceData) != 0) return;
          entry.docNode = doc->GetRoot();
        }
        PushReferencedFiles (entry.docNode);
      }
      
      struct ReplacedEntity
      {
	const char* entity;
	char replacement;
      };
      
      static const ReplacedEntity entities[] = {
	{"&lt;", '<'},
	{"&gt;", '>'},
	{0, 0}
      };
      
      static const char* ReplaceEntities (const char* str, csString& scratch)
      {
	const ReplacedEntity* entity = entities;
	while (entity->entity != 0)
	{
	  const char* entPos;
	  if ((entPos = strstr (str, entity->entity)) != 0)
	  {
	    size_t offset = entPos - str;
	    if (scratch.GetData() == 0)
	    {
	      scratch.Replace (str);
	      str = scratch.GetData ();
	    }
	    scratch.DeleteAt (offset, strlen (entity->entity));
	    scratch.Insert (offset, entity->replacement);
	  }
	  else
	    entity++;
	}
	return str;
      }
      
      static bool SplitNodeValue (const char* nodeStr, csString& command, 
				  csString& args)
      {
	csString replaceScratch;
	const char* nodeValue = ReplaceEntities (nodeStr, replaceScratch);
	if ((nodeValue != 0) && (*nodeValue == '?') && 
	  (*(nodeValue + strlen (nodeValue) - 1) == '?'))
	{
	  const char* valStart = nodeValue + 1;
      
	  while (*valStart == ' ') valStart++;
	  CS_ASSERT (*valStart != 0);
	  size_t valLen = strlen (valStart) - 1;
	  if (valLen != 0)
	  {
	    while (*(valStart + valLen - 1) == ' ') valLen--;
	    const char* space = strchr (valStart, ' ');
	    /* The rightmost spaces were skipped and don't interest us
	      any more. */
	    if (space >= valStart + valLen) space = 0;
	    size_t cmdLen;
	    if (space != 0)
	    {
	      cmdLen = space - valStart;
	    }
	    else
	    {
	      cmdLen = valLen;
	    }
	    command.Replace (valStart, cmdLen);
	    args.Replace (valStart + cmdLen, valLen - cmdLen);
	    args.LTrim();
	    return true;
	  }
	}
	return false;
      }

      void ShaderDocHasher::PushReferencedFiles (iDocumentNode* node)
      {
        csDocumentNodeType nodeType = node->GetType();
        if (nodeType == CS_NODE_ELEMENT)
        {
          const char* nodeName = node->GetValue();
          /* @@@ Hacky: Blacklist fallbackshader nodes as they aren't really 
             a part of the shader to scan */
          if (strcmp (nodeName, "fallbackshader") != 0)
          {
            const char* fileAttr = node->GetAttributeValue ("file");
            if (fileAttr && *fileAttr)
            {
              if (!AddFile (fileAttr))
              {
                // @@@ Some sort of error handling?
              }
            }
	  }
        
          csRef<iDocumentNodeIterator> nodes (node->GetNodes());
          while (nodes->HasNext());
          {
            csRef<iDocumentNode> child = nodes->Next();
            PushReferencedFiles (child);
          }
        }
        else if (nodeType == CS_NODE_UNKNOWN)
        {
          csString cmd, args;
          if (SplitNodeValue (node->GetValue(), cmd, args)
            && (cmd == "Include"))
	  {
	    args.Trim ();
	    AddFile (args);
	  }
        }
      }
        
      bool ShaderDocHasher::AddFile (const char* filename)
      {
	csRef<iFile> file = vfs->Open (filename, VFS_FILE_READ);
	if (file.IsValid())
	{
	  DocStackEntry newEntry;
	  newEntry.docNode = doc;
	  scanStack.Push (newEntry);
	  return true;
	}
	else
	{
	  return false;
	}
      }
      
      csPtr<iDataBuffer> ShaderDocHasher::GetHashStream ()
      {
        while (scanStack.GetSize() > 0)
        {
          DocStackEntry scanEntry = scanStack.PopTop();
          csMD5::Digest entryDigest (scanEntry.ComputeHash ());
          actualHashes.Write ((char*)&entryDigest, sizeof(csMD5::Digest));
	  PushReferencedFiles (scanEntry);
        }
        return actualHashes.GetAllData();
      }
      
      bool ShaderDocHasher::ValidateHashStream (iDataBuffer* stream)
      {
        const uint8* dataBytes = stream->GetUint8();
        size_t bytesRemaining = stream->GetSize();
        
        while (scanStack.GetSize() > 0)
        {
          if (bytesRemaining < sizeof(csMD5::Digest)) return false;
          csMD5::Digest diskDigest;
          memcpy (&diskDigest, dataBytes, sizeof(csMD5::Digest));
          dataBytes += sizeof(csMD5::Digest);
          bytesRemaining -= sizeof(csMD5::Digest);
        
          DocStackEntry scanEntry = scanStack.PopTop();
          csMD5::Digest entryDigest (scanEntry.ComputeHash ());
          actualHashes.Write ((char*)&entryDigest, sizeof(csMD5::Digest));
          if (memcmp (&diskDigest, &entryDigest, sizeof(csMD5::Digest)) != 0)
            return false;
	  PushReferencedFiles (scanEntry);
        }
        
        if (bytesRemaining > 0) return false;
        return true;
      }
    } // namespace ShaderCacheHelper
  } // namespace PluginCommon
} // namespace CS
