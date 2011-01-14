/*
    Copyright (C) 2011 by Frank Richter

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

#ifndef __TEXTUREINFO_H__
#define __TEXTUREINFO_H__

#include "csutil/csstring.h"
#include "csutil/refarr.h"
#include "csutil/refcount.h"

struct iDocumentNode;
struct iImage;

class TextureInfo : public CS::Utility::FastRefCount<TextureInfo>
{
  csRef<iDocumentNode> node;
  csString file;
  csString texClass;
  
  csRefArray<iImage> mips;
public:
  TextureInfo (iDocumentNode* node, const char* file, const char* texClass);
  
  iDocumentNode* GetDocumentNode() const { return node; }
  const csString& GetFileName() const
  { return file; }
  iImage* GetMip (uint mip);
};


#endif // __TEXTUREINFO_H__
