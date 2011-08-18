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

#ifndef __CS_THOGGMEDIACONT_H__
#define __CS_THOGGMEDIACONT_H__

/**\file
* Video Player: media stream 
*/

#include <iutil/comp.h>
#include <ivideodecode/mediacontainer.h>
#include <csutil/scf_implementation.h>
#include <csutil/refarr.h>
#include <csutil/array.h>

#include "vorbis/codec.h"
#include "thoggVideoMedia.h"
#include "thoggAudioMedia.h"

#define QUALIFIED_PLUGIN_NAME "crystalspace.vpl.element.thogg"

using namespace CS::Threading;

/**
  * Container for the different streams inside a video file
  */
class TheoraMediaContainer : public scfImplementation2<TheoraMediaContainer, 
  iMediaContainer,
  iComponent>
{
private:
  iObjectRegistry* object_reg;
  csRefArray<iMedia> media;
  csArray<size_t> activeStreams;
  const char* pDescription;
  bool endOfFile;
  unsigned long mSize;
  float timeToSeek;
  bool    _waitToFillCache;

  csRef<TheoraVideoMedia> _activeTheoraStream;
  csRef<TheoraAudioMedia> _activeVorbisStream;

  size_t cacheSize;

  csTicks timeSinceStart;

  /// audio languages
  
  csArray<Language> languages;

  /// audio stream
  csRef<iSndSysStream> sndstream;

private:
  Mutex swapMutex;
  Condition isSeeking;

public:
  ogg_sync_state        _syncState;
  ogg_page              _oggPage;
  FILE                  *infile;
  csRef<iTextureHandle> _target;

private:
  bool canSwap;
  bool canWrite;
  int hasDataToBuffer;
  int ok;

  //helper for buffering data
  int BufferData (ogg_sync_state *oy);

public:
  TheoraMediaContainer (iBase* parent);
  virtual ~TheoraMediaContainer ();

  // From iComponent.
  virtual bool Initialize (iObjectRegistry*);

  virtual size_t GetMediaCount () const;
  virtual csRef<iMedia> GetMedia (size_t index);
  virtual const char* GetDescription () const;
  inline virtual void SetDescription (const char* pDescription)
    { this->pDescription=pDescription; }
  void AddMedia (csRef<iMedia> media);
  virtual void GetTargetTexture (csRef<iTextureHandle> &target) ;
  virtual void GetTargetAudio (csRef<iSndSysStream> &target) ;
  virtual void SetActiveStream (size_t index);
  virtual bool RemoveActiveStream (size_t index);
  virtual void Update ();
  virtual bool Eof () const;
  virtual void Seek (float time) ;
  virtual void AutoActivateStreams () ;
  virtual float GetPosition () const;
  virtual float GetLength () const;
  virtual void SwapBuffers() ;

  virtual void WriteData () ;


  virtual void SetCacheSize(size_t size) ;

  virtual float GetAspectRatio () ;

  virtual void DropFrame ();

  virtual void SelectLanguage (const char* identifier);

  /// Does a seek on the active media
  void DoSeek ();
  void QueuePage (ogg_page *page);
  void SetLanguages (csArray<Language> languages);
  inline void ClearMedia()  { media.Empty (); }
  inline unsigned long GetFileSize () const { return mSize; }
  inline void SetFileSize (unsigned long size)  { mSize=size; }
};

/** @} */

#endif // __CS_THOGGMEDIACONT_H__
