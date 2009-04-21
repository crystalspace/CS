/*
    Copyright (C) 2007 by Frank Richter

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

#ifndef __FIXPOLYMESH_H__
#define __FIXPOLYMESH_H__

#include "csutil/csstring.h"
#include "csutil/strhash.h"

struct iDocumentNode;

class FixPolyMesh
{
  csStringHash xmltokens;
#define CS_TOKEN_ITEM_FILE "apps/tools/levtool/fixpolymesh.tok"
#include "cstool/tokenlist.h"
#undef CS_TOKEN_ITEM_FILE 

  void HandlePolyMeshContainer (iDocumentNode* node);
  void HandlePolyMesh (iDocumentNode* node);
public:
  FixPolyMesh ();

  void Fix (iDocumentNode* root);
};

#endif // __FIXPOLYMESH_H__
