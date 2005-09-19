/*
    Copyright (C) 2003 by Jorrit Tyberghein
	      (C) 2003 by Frank Richter

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

#ifndef __CS_SKY_H__
#define __CS_SKY_H__

#include "cstool/basetexfact.h"
#include "csutil/strhash.h"
#include "csutil/csstring.h"

class csPtSkyType : public csBaseProctexType
{
public:
  csPtSkyType (iBase *p);
  virtual csPtr<iTextureFactory> NewFactory();
};

class csPtSkyFactory : public csBaseTextureFactory
{
public:
  csPtSkyFactory (iTextureType* p, iObjectRegistry* object_reg);
  virtual csPtr<iTextureWrapper> Generate ();
};

class csPtSkyLoader : public csBaseProctexLoader
{
  csStringHash tokens;
//#define CS_TOKEN_ITEM_FILE "plugins/proctex/standard/sky.tok"
//#include "cstool/tokenlist.h"
public:
  csPtSkyLoader(iBase *p);

  virtual csPtr<iBase> Parse (iDocumentNode* node, 
    iStreamSource*, iLoaderContext* ldr_context, iBase* context);
};

class csPtSkySaver : public csBaseProctexSaver
{
public:
  csPtSkySaver(iBase *p);

  virtual bool WriteDown (iBase *obj, iDocumentNode* parent,
  	iStreamSource*);
};

#endif 
