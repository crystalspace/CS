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
#include <videodecode/medialoader.h>
#include <videodecode/mediacontainer.h>
#include <videodecode/media.h>
#include <videodecode/vpl_structs.h>
#include <csutil/scf_implementation.h>
#include "thoggMediaContainer.h"
#include "thoggVideoMedia.h"
#include "thoggAudioMedia.h"
#include <csutil/nobjvec.h>


#include "theora/theora.h"
#include "vorbis/codec.h"


#define QUALIFIED_PLUGIN_NAME "crystalspace.vpl.element.thogg"

struct iObjectRegistry;

/**
* This is the implementation for our API and
* also the implementation of the plugin.
*/
class thoggLoader : public scfImplementation2<thoggLoader,iMediaLoader,iComponent>
{
private:
  iObjectRegistry* object_reg;

  //ogg stuff
  ogg_sync_state   oy;
  ogg_page         og;

  //theora stuff
  th_info      ti;
  th_comment   tc;
  //------------------

  //vorbis stuff
  vorbis_info      vi;
  vorbis_comment   vc;
  //------------------

  FILE *infile;

private:
	
/* Helper; just grab some more compressed bitstream and sync it for
   page extraction */
  int BufferData(ogg_sync_state *oy);

  bool StartParsing(csRef<TheoraMediaContainer> container);
  bool ParseHeaders(csRef<TheoraMediaContainer> container);
	void ComputeStreamLength(csRef<TheoraMediaContainer> container);

public:
  thoggLoader (iBase* parent);
  virtual ~thoggLoader ();

  // From iComponent.
  virtual bool Initialize (iObjectRegistry*);

  
  virtual csRef<iMediaContainer> LoadMedia (const char * pFileName, const char *pDescription=0, const char* pMediaType = "AutoDetect");
};

#endif // __THOGGLOADER_H__