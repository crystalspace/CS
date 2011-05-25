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
#ifndef __THOGGFILE_H__
#define __THOGGFILE_H__

#include "csutil/scf.h"
#include "csutil/ref.h"
#include <iutil/vfs.h>
#include <iutil/comp.h>
#include <videodecode/vpl_structs.h>
#include <csutil/scf_implementation.h>


#include <theora/theoradec.h>
#include <vorbis/codec.h>

#pragma comment (lib,"../libs/libtheora_static.lib")
#pragma comment (lib,"ogg.lib")
#pragma comment (lib,"vorbis.lib")

/**
 * This is an internal representation of theora videos which thoggCodec uses.
 * It's not required for each decoder plugin to have one, thus no interface was created,
 * but it's a nice way to represent data inside the codec
 */


class thoggFile : public scfImplementation2<thoggFile,iFile,iComponent>
{
private:
  iObjectRegistry* object_reg;
  const char *fileName;

  //-------------------------------------------------
  // OGG and Theora stuff
  //-------------------------------------------------
  FILE *infile;

  ogg_sync_state   oy;
  ogg_page         og;
  ogg_stream_state vo;
  ogg_stream_state to;
  th_info      ti;
  th_comment   tc;
  th_dec_ctx       *td;
  th_setup_info    *ts;
  vorbis_info      vi;
  vorbis_dsp_state vd;
  vorbis_block     vb;
  vorbis_comment   vc;
  th_pixel_fmt     px_fmt;

  ogg_packet op;

  int              theora_p;
  int              vorbis_p;
  int              stateflag;
  //-------------------------------------------------

  // Format of the stream
  csVPLvideoFormat *format;

private:
  // helper functions used to decode ogg files
  int buffer_data(ogg_sync_state *oy);
  int queue_page(ogg_page *page);

  /**
   * This checks the file for Theora and Vorbis headers
   * Vorbis headers are not necessarily required, but Theora ones are
   */
  bool checkHeaders();

  /**
   * Read the rest of the headers in the file. 
   * Currently, the possibility of having multiple video/sound streams is ignored
   */
  bool parseSecondaryHeaders();

public:
  thoggFile (iBase* parent);
  virtual ~thoggFile ();

  // From iComponent.
  virtual bool Initialize (iObjectRegistry*);

  /// Initialize everything needed to play a video and also check if it's a theora video
  virtual bool InitFile (FILE *infile);

  /// Set file name
  virtual void SetName (const char *name);

  /// Query file name (in VFS)
  virtual const char *GetName ();

  /// Query file size
  virtual size_t GetSize ();

  /**
   * Check (and clear) file last error status
   * \sa #VFS_STATUS_ACCESSDENIED
   */
  virtual int GetStatus ();

  /**
   * Read DataSize bytes and place them into the buffer at which Data points.
   * \param Data Pointer to the buffer into which the data should be read.  The
   *   buffer should be at least DataSize bytes in size.
   * \param DataSize Number of bytes to read.
   * \return The number of bytes actually read.  If an error occurs, zero is
   *   returned.  Invoke GetStatus() to retrieve the error code.
   */
  virtual size_t Read (char *Data, size_t DataSize);

  /**
   * Write DataSize bytes from the buffer at which Data points.
   * \param Data Pointer to the data to be written.
   * \param DataSize Number of bytes to write.
   * \return The number of bytes actually written.  If an error occurs, zero is
   *   returned.  Invoke GetStatus() to retrieve the error code.
   */
  virtual size_t Write (const char *Data, size_t DataSize);

  /// Flush stream.
  virtual void Flush ();

  /// Returns true if the stream is at end-of-file, else false.
  virtual bool AtEOF ();

  /// Query current file pointer.
  virtual size_t GetPos ();

  /**
   * Set new file pointer.
   * \param newpos New position in file.
   * \return True if the operation succeeded, else false.
   */
  virtual bool SetPos (size_t newpos);

  /**
   * Request whole content of the file as a single data buffer.
   * \param nullterm Set this to true if you want a null char to be appended
   *  to the buffer (e.g. for use with string functions.)
   * \remarks Null-termination might have a performance penalty (depending upon
   *  where the file is stored.) Use only when needed.
   * \return The complete data contained in the file; or an invalidated pointer
   *  if this object does not support this function (e.g. write-only VFS
   *  files).  Check for an invalidated result via csRef<>::IsValid().  Do not
   *  modify the contained data!
   */
  virtual csPtr<iDataBuffer> GetAllData (bool nullterm = false);
};



#endif // __THOGGFILE_H__