/*
	Copyright (C) 1998 by Jorrit Tyberghein

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
#ifndef AMIGA_H
#define AMIGA_H

#include "system/system.h"
#include "render/graph2d.h"
#include "util/input/csinput.h"

/// Amiga/Warp3D version
class SysSystemDriver : public csSystemDriver
{
public:
	SysSystemDriver () : csSystemDriver() {};
	~SysSystemDriver ();

	virtual void Loop ();
	virtual bool ParseArg(int argc, char* argv[], int& i);
	virtual void Help ();
};

/// Amiga/Warp3D version
class SysGraphics2D : public csGraphics2D
{
public:
	SysGraphics2D(int argc, char* argv[]);
	virtual ~SysGraphics2D(void);

	virtual bool Open(const char *Title);
	virtual void Close(void);

	virtual void Print(csRect *area = NULL);
	virtual void SetRGB(int i, int r, int g, int b);

	virtual bool BeginDraw();
	virtual void FinishDraw();
	virtual bool SetMouseCursor(int iShape, TextureMM *iBitmap);
};

/// Amiga/Warp3D version
class SysKeyboardDriver : public csKeyboardDriver
{
public:
	SysKeyboardDriver();
	~SysKeyboardDriver(void);
	bool Open(void);
	void Close(void);
};

/// Amiga/Warp3D version
class SysMouseDriver : public csMouseDriver
{
public:
	SysMouseDriver();
	~SysMouseDriver(void);

	bool Open(void);
	void Close(void);
};

#endif
