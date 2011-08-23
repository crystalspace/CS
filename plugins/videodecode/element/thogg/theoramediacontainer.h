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
#include "theoravideomedia.h"
#include "theoraaudiomedia.h"

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
  iObjectRegistry*    _object_reg;
  csRefArray<iMedia>  _media;
  csArray<size_t>     _activeStreams;
  const char*         _pDescription;
  bool                _endOfFile;
  unsigned long       _fileSize;
  float               _timeToSeekTo;
  bool                _waitToFillCache;

  csRef<csTheoraVideoMedia> _activeTheoraStream;
  csRef<csTheoraAudioMedia> _activeVorbisStream;

  size_t  _cacheSize;

  // audio languages
  
  csArray<Language> _languages;

  // audio stream
  csRef<iSndSysStream> _sndstream;

  // audio stream length in seconds
  size_t _audioStreamLength;

private:
  Mutex     _swapMutex;
  Condition _isSeeking;

public:
  ogg_sync_state          _syncState;
  ogg_page                _oggPage;
  FILE*                   _infile;
  csRef<iTextureHandle>   _target;

private:
  bool  _canSwap;
  bool  _canWrite;
  int   _hasDataToBuffer;
  int   _updateState;

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
    { this->_pDescription=pDescription; }

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
  virtual void SwapBuffers () ;

  virtual void WriteData () ;


  virtual void SetCacheSize(size_t size) ;

  virtual float GetAspectRatio () ;

  virtual void DropFrame ();

  virtual void SelectLanguage (const char* identifier);

  virtual void OnPause ();

  virtual void OnPlay ();

  virtual void OnStop ();

  // Execute a seek on the active media
  void DoSeek ();

  // Queue a page to the appropriate stream
  void QueuePage (ogg_page *page);
  void SetLanguages (csArray<Language> languages);

  inline void ClearMedia ()  
  { _media.Empty (); }

  inline unsigned long GetFileSize () const 
  { return _fileSize; }

  inline void SetFileSize (unsigned long size)  
  { _fileSize=size; }
};

/** @} */

#endif // __CS_THOGGMEDIACONT_H__
