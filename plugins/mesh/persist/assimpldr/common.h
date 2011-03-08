/*
  Copyright (C) 2011 Christian Van Brussel, Communications and Remote
      Sensing Laboratory of the School of Engineering at the 
      Universite catholique de Louvain, Belgium
      http://www.tele.ucl.ac.be

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
#ifndef __CS_ASSIMPLOADER_COMMON_H__
#define __CS_ASSIMPLOADER_COMMON_H__

#include "cssysdef.h"

#include "csgeom/tri.h"
#include "csgfx/renderbuffer.h"
#include "cstool/rbuflock.h"
#include "csutil/cscolor.h"
#include "csutil/csstring.h"
#include "csutil/databuf.h"
#include "csutil/scfstr.h"

#include "iutil/objreg.h"
#include "ivaria/reporter.h"

#include "assimpldr.h"

CS_PLUGIN_NAMESPACE_BEGIN(AssimpLoader)
{

  /**
   * Type conversion
   */
  inline csVector3 Assimp2CS (aiVector3D& v)
  {
    return csVector3 (v.x, v.y, v.z);
  }

  inline csColor4 Assimp2CS (aiColor4D& c)
  {
    return csColor4 (c.r, c.g, c.b, c.a);
  }

  inline csTriangle Assimp2CS (unsigned int* t)
  {
    return csTriangle (t[0], t[1], t[2]);
  }

  inline csQuaternion Assimp2CS (aiQuaternion& q)
  {
    return csQuaternion (q.x, q.y, q.z, q.w);
  }

  /**
   * Render buffer creation
   */
  template<typename T>
  static csRef<iRenderBuffer> FillBuffer
    (csDirtyAccessArray<T>& buf, csRenderBufferComponentType compType,
     int componentNum, bool indexBuf)
  {
    csRef<iRenderBuffer> buffer;
    size_t bufElems = buf.GetSize() / componentNum;
    if (indexBuf)
    {
      T min;
      T max;
      size_t i = 0;
      size_t n = buf.GetSize(); 
      if (n & 1)
      {
	min = max = csMax (buf[0], T (0));
	i++;
      }
      else
      {
	min = T (INT_MAX);
	max = 0;
      }
      for (; i < n; i += 2)
      {
	T a = buf[i]; T b = buf[i+1];
	if (a < b)
	{
	  min = csMin (min, a);
	  max = csMax (max, b);
	}
	else
	{
	  min = csMin (min, b);
	  max = csMax (max, a);
	}
      }
      buffer = csRenderBuffer::CreateIndexRenderBuffer
	(bufElems, CS_BUF_STATIC, compType,
	 size_t (min), size_t (max));
    }
    else
    {
      buffer = csRenderBuffer::CreateRenderBuffer
	(bufElems, CS_BUF_STATIC, compType, (uint)componentNum);
    }
    buffer->CopyInto (buf.GetArray(), bufElems);

    return buffer;
  }
 

  /**
   * IO stream handling
   */

  class csIOStream : public Assimp::IOStream
  {
  public:
    csIOStream (iFile* file)
      : file (file) {}
    ~csIOStream () {}

    size_t Read (void* pvBuffer, size_t pSize, size_t pCount)
    { return file ? file->Read ((char*) pvBuffer, pSize * pCount) : 0; }

    size_t Write (const void* pvBuffer, size_t pSize, size_t pCount)
    { return file ? file->Write ((char*) pvBuffer, pSize * pCount) : 0; }

    aiReturn Seek (size_t pOffset, aiOrigin pOrigin)
    {
      if (!file)
	return aiReturn_FAILURE;

      switch (pOrigin)
	{
	case aiOrigin_SET:
	  return file->SetPos (pOffset) ? aiReturn_SUCCESS : aiReturn_FAILURE;

	case aiOrigin_CUR:
	  return file->SetPos (file->GetPos () + pOffset)
	    ? aiReturn_SUCCESS : aiReturn_FAILURE;

	case aiOrigin_END:
	  return file->SetPos (file->GetSize () + pOffset)
	    ? aiReturn_SUCCESS : aiReturn_FAILURE;

	default:
	  break;
	}

      return aiReturn_FAILURE;
    }

    size_t Tell () const
    { return file ? file->GetPos () : 0; }

    size_t FileSize () const
    { return file ? file->GetSize () : 0; }

    void Flush ()
    { if (file) file->Flush (); }

  private:
    csRef<iFile> file;
  };

  class csIOSystem : public Assimp::IOSystem
  {
  public:
    csIOSystem (iVFS* vfs, const char* filename)
      : vfs (vfs), changer (vfs)
    {
      csString file = filename;
      if (filename && file.FindFirst ('/') != (size_t) -1)
	changer.ChangeTo (filename);
    }

    ~csIOSystem () {}

    bool Exists (const char *pFile) const
    {
      return vfs->Exists (pFile);
    }

    char getOsSeparator () const
    { return '/'; }

    Assimp::IOStream* Open (const char *pFile, const char *pMode="rb")
    {
      printf ("Opening file [%s]...", pFile); 
      csRef<iFile> file = vfs->Open (pFile,
				     *pMode == 'w' ? VFS_FILE_WRITE : VFS_FILE_READ);
      printf (file ? "Success\n" : "Failed!\n");
      return new csIOStream (file);
    }

    void Close (Assimp::IOStream* pFile)
    { delete pFile; }

  private:
    csRef<iVFS> vfs;
    csVfsDirectoryChanger changer;
  };

}
CS_PLUGIN_NAMESPACE_END(AssimpLoader)

#endif // __CS_ASSIMPLOADER_COMMON_H__
