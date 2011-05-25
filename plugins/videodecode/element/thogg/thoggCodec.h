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
#ifndef __THOGGDATA_H__
#define __THOGGDATA_H__

#include <iutil/comp.h>
#include <videodecode/vpl_codec.h>
#include <videodecode/vpl_structs.h>
#include <csutil/scf_implementation.h>
#include "thoggFile.h"

#define QUALIFIED_PLUGIN_NAME "crystalspace.vpl.element.thogg"

struct iObjectRegistry;
struct csVPLvideoFormat;

/**
* This is the implementation for our API and
* also the implementation of the plugin.
*/
class thoggCodec : public scfImplementation2<thoggCodec,iVPLCodec,iComponent>
{
private:
  const char * pDescription;
  csVPLvideoFormat *format;
  iObjectRegistry* object_reg;
  csRef<thoggFile> oggFile;

public:
  thoggCodec (iBase* parent);
  virtual ~thoggCodec ();

  // From iComponent.
  virtual bool Initialize (iObjectRegistry*);

  // Initialize codec
  virtual bool InitCodec (FILE *infile);

  /// Get the format of the sound data.
  virtual const csVPLvideoFormat *GetFormat();

  virtual void SetFormat(csVPLvideoFormat *vplFormat);

  /// Get size of this sound in frames.
  virtual size_t GetFrameCount();

  /**
   * Return the size of the data stored in bytes
   */
  virtual size_t GetDataSize();

  /// Set an optional description to be associated with this sound data
  //   A filename isn't a bad idea!
  virtual void SetDescription(const char *pDescription);

  /// Retrieve the description associated with this sound data
  //   This may return 0 if no description is set.
  virtual const char *GetDescription();

  /// return the data for the next frame
  virtual void getNextFrame(vidFrameData &data);

  /// run frame-specific updates
  virtual void update();
};

#endif // __THOGGLOADER_H__