/*
    Copyright (C) 1998, 1999 by Nathaniel 'NooTe' Saint Martin
    Copyright (C) 1998, 1999 by Jorrit Tyberghein
    Written by Nathaniel 'NooTe' Saint Martin

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

#if !defined(__CSSOUNDSOURCEA3D_H__)
#define __CSSOUNDSOURCEA3D_H__

#include "isndrdr.h"
#include "isndsrc.h"
#include "cssndldr/common/sndbuf.h"

class csSoundSourceA3D : public ISoundSource
{
public:
	csSoundSourceA3D();
	virtual ~csSoundSourceA3D();

	STDMETHODIMP StopSource();
	STDMETHODIMP PlaySource(bool inLoop);

	STDMETHODIMP SetPosition(float x, float y, float z);
  STDMETHODIMP SetVelocity(float x, float y, float z);

  STDMETHODIMP GetInfoSource(csSoundSourceInfo *info);

 	DECLARE_IUNKNOWN()
	DECLARE_INTERFACE_TABLE(csSoundSourceA3D)

public:
  csSoundSourceInfo info;

	int DestroySource();
	int CreateSource(ISoundRender *render, csSoundBuffer * sound);

  IA3dSource *m_p3DSource;
  IA3d4 *m_p3DAudioRenderer;
};

#endif // __CSSOUNDSOURCEA3D_H__
