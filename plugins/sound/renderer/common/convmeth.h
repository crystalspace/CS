/*
    Copyright (C) 1998, 1999 by Jorrit Tyberghein

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

#ifndef __SOUND_CONVMETH_H__
#define __SOUND_CONVMETH_H__

// convenience methods used in all sound renderers

#define IMPLEMENT_SOUNDRENDER_CONVENIENCE_METHODS(classname)            \
  void classname::PlaySound(iSoundData *snd, bool Loop) {               \
    iSoundSource *src = CreateSource(snd,false);                        \
    if (src) {src->Play(Loop?SOUND_LOOP:0); src->DecRef();}             \
  }                                                                     \
  void classname::PlaySound(iSoundStream *snd, bool Loop) {             \
    iSoundSource *src = CreateSource(snd,false);                        \
    if (src) {src->Play(Loop?SOUND_LOOP:0); src->DecRef();}             \
  }                                                                     \
  iSoundSource *classname::CreateSource(iSoundData *snd, bool is3d) {   \
    if (!snd) return NULL;                                              \
    iSoundStream *str = snd->CreateStream();                            \
    iSoundSource *src = CreateSource(str, is3d);                        \
    str->DecRef();                                                      \
    return src;                                                         \
  }

#endif

