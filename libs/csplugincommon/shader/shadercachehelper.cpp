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
#include "csutil/csendian.h"
#include "csutil/databuf.h"
#include "csutil/documenthelper.h"
#include "csutil/parasiticdatabuffer.h"
#include "csutil/sysfunc.h"
#include "csutil/xmltiny.h"
#include "cstool/vfsdirchange.h"

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
	  PushReferencedFiles (doc);
	}
      }
      
      void ShaderDocHasher::PushReferencedFiles (DocStackEntry& entry)
      {
        CS_ASSERT(entry.docNode || entry.sourceData);
        if (!entry.docNode.IsValid())
        {
          csRef<iDocument> doc = docSys->CreateDocument();
          const char* err = doc->Parse (entry.sourceData);
          if (err != 0)
          {
	  #ifdef CS_DEBUG
	    csPrintf ("%s: parse error '%s'\n", CS_FUNCTION_NAME,
	      err);
	  #endif
            return;
          }
          entry.docNode = doc->GetRoot();
        }
        
        csVfsDirectoryChanger changer (vfs);
        if (!entry.fullPath.IsEmpty()) changer.ChangeTo (entry.fullPath);
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
              AddFile (fileAttr);
            }
	  }
        
          csRef<iDocumentNodeIterator> nodes (node->GetNodes());
          while (nodes->HasNext())
          {
            csRef<iDocumentNode> child = nodes->Next();
            PushReferencedFiles (child);
          }
        }
        else if (nodeType == CS_NODE_DOCUMENT)
        {
          csRef<iDocumentNodeIterator> nodes (node->GetNodes());
          while (nodes->HasNext())
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
        csRef<iDataBuffer> fullPath (vfs->ExpandPath (filename));
        if (seenFiles.Contains (fullPath->GetData())) return true;
      
	csRef<iFile> file = vfs->Open (fullPath->GetData(), VFS_FILE_READ);
	if (file.IsValid())
	{
	  DocStackEntry newEntry;
	  newEntry.sourceData = file->GetAllData();
	  newEntry.fullPath = fullPath->GetData();
	  scanStack.Push (newEntry);
          seenFiles.AddNoTest (fullPath->GetData());
	  return true;
	}
	else
	{
	#ifdef CS_DEBUG
	  csPrintf ("%s: failed to open '%s'\n", CS_FUNCTION_NAME,
	    fullPath->GetData());
	#endif
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
      
      //---------------------------------------------------------------------
      
      bool WriteDataBuffer (iFile* file, iDataBuffer* buf)
      {
        size_t bufSize = buf ? buf->GetSize() : 0;
	uint32 sizeLE = csLittleEndian::UInt32 (bufSize);
	if (file->Write ((char*)&sizeLE, sizeof (sizeLE)) != sizeof (sizeLE))
	  return false;
	if (buf && (file->Write (buf->GetData(), bufSize) != bufSize))
	  return false;
	  
	size_t pad = 4 - (bufSize & 3);
	if (pad < 4)
	{
          static const char null[] = {0, 0, 0};
	  if (file->Write (null, pad) != pad)
	    return false;
        }
	return true;
      }
      
      csPtr<iDataBuffer> ReadDataBuffer (iFile* file)
      {
	uint32 bufSize;
	size_t read = file->Read ((char*)&bufSize, sizeof (bufSize));
	if (read != sizeof (bufSize)) return 0;
	bufSize = csLittleEndian::UInt32 (bufSize);
	csRef<iDataBuffer> allFileData (file->GetAllData());
	csRef<iDataBuffer> buf;
	buf.AttachNew (new csParasiticDataBuffer (allFileData,
	  file->GetPos(), bufSize));
	if (buf->GetSize() != bufSize) return 0;
	size_t pad = 4 - (bufSize & 3);
	if (pad < 4) bufSize += pad;
	file->SetPos (file->GetPos() + bufSize);
	return csPtr<iDataBuffer> (buf);
      }
      
      //---------------------------------------------------------------------
      
      bool WriteString (iFile* file, const char* str)
      {
        CS::DataBuffer<> dbuf (const_cast<char*> (str),
          str ? strlen (str)+1 : 0, false);
        return WriteDataBuffer (file, &dbuf);
      }
      
      csString ReadString (iFile* file)
      {
        csString ret;
        
        csRef<iDataBuffer> buf (ReadDataBuffer (file));
        if (buf.IsValid() && (buf->GetSize() > 0))
          ret.Replace (buf->GetData(), buf->GetSize()-1);
        
        return ret;
      }
      
      //---------------------------------------------------------------------
      
      bool StringStoreWriter::StartUse (iFile* file)
      {
        CS_ASSERT(!this->file.IsValid());
        this->file = file;
        headPos = file->GetPos();
        strings.Empty();
        stringPositions.DeleteAll();
        
        uint32 dummy = (uint32)~0;
        if (file->Write ((char*)&dummy, sizeof (dummy)) != sizeof (dummy))
        {
          this->file.Invalidate();
          return false;
        }
        return true;
      }
      
      bool StringStoreWriter::EndUse ()
      {
        CS_ASSERT(this->file.IsValid());
        
        size_t curFilePos = file->GetPos();
        
        csRef<iDataBuffer> stringsBuf (strings.GetAllData());
        if (!stringsBuf.IsValid())
          stringsBuf.AttachNew (new CS::DataBuffer<> ((size_t)0));
        if (!WriteDataBuffer (file, stringsBuf)) return false;
        
        uint32 ofsLE = curFilePos - headPos;
        curFilePos = file->GetPos();
        bool ret = false;
        file->SetPos (headPos);
        ofsLE = csLittleEndian::UInt32 (ofsLE);
        ret = (file->Write ((char*)&ofsLE, sizeof (ofsLE)) == sizeof (ofsLE));
        
        file->SetPos (curFilePos);
        file.Invalidate();
        return ret;
      }
      
      uint32 StringStoreWriter::GetID (const char* string)
      {
        CS_ASSERT(this->file.IsValid());
        
        uint32 pos = stringPositions.Get (string, (uint32)~0);
        if (pos == (uint32)~0)
        {
          pos = strings.GetPos();
          strings.Write (string, strlen (string)+1);
          stringPositions.Put (string, pos);
        }
        return pos;
      }
      
      //---------------------------------------------------------------------
      
      bool StringStoreReader::StartUse (iFile* file)
      {
        CS_ASSERT(!this->file.IsValid());
        this->file = file;
        
        size_t curFilePos = file->GetPos();
        
        uint32 ofsLE;
        if (file->Read ((char*)&ofsLE, sizeof (ofsLE)) != sizeof (ofsLE))
        {
          this->file.Invalidate();
          return false;
        }
        ofsLE = csLittleEndian::UInt32 (ofsLE);
        file->SetPos (curFilePos + ofsLE);
        blockBuf = ReadDataBuffer (file);
        endPos = file->GetPos();;
        if (!blockBuf.IsValid()) return false;
        stringBlock = blockBuf->GetData();
        file->SetPos (curFilePos + sizeof (ofsLE));
        return true;
      }
      
      bool StringStoreReader::EndUse ()
      {
        CS_ASSERT(this->file.IsValid());
        
        file->SetPos (endPos);
        file.Invalidate();
        return true;
      }
      
      const char* StringStoreReader::GetString (uint32 id) const
      {
        CS_ASSERT(this->file.IsValid());
        CS_ASSERT(id < blockBuf->GetSize());
        return stringBlock + id;
      }
      
    } // namespace ShaderCacheHelper
  } // namespace PluginCommon
} // namespace CS
