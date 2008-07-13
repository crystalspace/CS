/*
    Copyright (C) 2008 by Frank Richter

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

#ifndef __CS_TERRAIN_SAVER_H__
#define __CS_TERRAIN_SAVER_H__

#include "imap/services.h"
#include "imap/writer.h"
#include "iutil/comp.h"

#include "csutil/scf_implementation.h"

CS_PLUGIN_NAMESPACE_BEGIN(Terrain2Loader)
{
  class Terrain2SaverCommon
  {
  protected:
    csRef<iSyntaxService> synldr;

    bool Initialize (iObjectRegistry *objreg);

    template<typename IProp>
    bool SaveProperties (iDocumentNode* node, IProp* props,
      IProp* dfltProp = 0);

    bool SaveRenderProperties (iDocumentNode* node,
      iTerrainCellRenderProperties* props,
      iTerrainCellRenderProperties* dfltProp = 0);

    bool SaveFeederProperties (iDocumentNode* node,
      iTerrainCellFeederProperties* props,
      iTerrainCellFeederProperties* dfltProp = 0);
  };

  class Terrain2FactorySaver :
    public scfImplementation2<Terrain2FactorySaver,
			      iSaverPlugin,
			      iComponent>,
    public Terrain2SaverCommon
  {
  public:
    Terrain2FactorySaver (iBase*);
    virtual ~Terrain2FactorySaver ();

    bool Initialize (iObjectRegistry *objreg);

    bool WriteDown (iBase *obj, iDocumentNode *parent,
      iStreamSource *ssource);
  private:
  };

  class Terrain2ObjectSaver :
    public scfImplementation2<Terrain2ObjectSaver,
			      iSaverPlugin,
			      iComponent>,
    public Terrain2SaverCommon
  {
  public:
    Terrain2ObjectSaver (iBase*);
    virtual ~Terrain2ObjectSaver ();

    bool Initialize (iObjectRegistry *objreg);

    bool WriteDown (iBase *obj, iDocumentNode *parent,
      iStreamSource *ssource);
  private:
  };

}
CS_PLUGIN_NAMESPACE_END(Terrain2Loader)

#endif // __CS_TERRAIN_SAVER_H__
