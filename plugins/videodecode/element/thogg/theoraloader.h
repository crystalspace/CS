/*
Copyright (C) 2011 by Alin Baciu

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

#ifndef __THOGGLOADER_H__
#define __THOGGLOADER_H__

#include <iutil/comp.h>
#include <ivideodecode/medialoader.h>
#include <ivideodecode/mediacontainer.h>
#include <ivideodecode/media.h>
//#include <ivideodecode/mediaparser.h>
#include <csutil/scf_implementation.h>
#include "theoramediacontainer.h"
#include <csutil/nobjvec.h>

#include "vorbis/codec.h"


#define QUALIFIED_PLUGIN_NAME "crystalspace.vpl.element.thogg"

struct iObjectRegistry;

#ifdef WIN32
  #pragma comment (lib,"libtheora_static.lib")
  #pragma comment (lib,"ogg.lib")
  #pragma comment (lib,"vorbis.lib")
#endif

/**
  * This is the implementation for our API and
  * also the implementation of the plugin.
  */
class csThOggLoader : public scfImplementation2<csThOggLoader,iMediaLoader,iComponent>
{
private:

  iObjectRegistry* _object_reg;

  csRef<iTextureManager> _texManager;

  //ogg stuff
  ogg_sync_state   _oy;
  ogg_page         _og;

  //theora stuff
  th_info      ti;
  th_comment   tc;
  //------------------

  //vorbis stuff
  vorbis_info      _vi;
  vorbis_comment   _vc;
  //------------------

  FILE *              _infile;
  csRef<iGraphics3D>  _g3d;

  csString            _path;
  csArray<Language>   _languages;

private:

  /* Helper; just grab some more compressed bitstream and sync it for
  page extraction */
  int BufferData (ogg_sync_state *oy);

  bool StartParsing (csRef<TheoraMediaContainer> container);
  bool ParseHeaders (csRef<TheoraMediaContainer> container);
  void ComputeStreamLength (csRef<TheoraMediaContainer> container);

public:
  csThOggLoader (iBase* parent);
  virtual ~csThOggLoader ();

  // From iComponent.
  virtual bool Initialize (iObjectRegistry*);


  virtual csRef<iMediaContainer> LoadMedia (const char * pFileName, const char *pDescription=0);

  virtual void Create (csString path,csArray<Language> languages) ;
};

#endif // __THOGGLOADER_H__
