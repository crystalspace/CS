/*
    Copyright (C) 2000 by Norman Krämer

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

#ifndef __FUNCON_H__
#define __FUNCON_H__

#include "cscon.h"
#include "igraph3d.h"
#include "ivfs.h"

typedef struct{
  iTextureHandle *txt;
  bool do_keycolor;
  UByte kr, kg, kb;
  bool do_stretch;
  int offx, offy;
  bool do_alpha;
  float alpha;
} ConDecoBorder;

typedef struct{
  ConDecoBorder border[8];
  ConDecoBorder bgnd;
  int lx, rx, ty, by;
  int p2lx, p2rx, p2ty, p2by;
} ConsoleDecoration;

class funConsole : public csConsole
{
public:
  DECLARE_IBASE;
  funConsole(iBase *base);
  virtual ~funConsole();
  virtual bool Initialize(iSystem *);
  virtual void Draw3D(csRect *rect);
  virtual void SetTransparency(bool){ csConsole::SetTransparency ( true ); }
  virtual void GetPosition(int &x, int &y, int &width, int &height) const;
  virtual void SetPosition(int x, int y, int width = -1, int height = -1);

protected:
  iGraphics3D *piG3D;
  iVFS *piVFS;
  ConsoleDecoration deco;
  csRect outersize, bordersize, p2size;
  bool border_computed;


  void LoadPix();
  void PrepPix( csIniFile *ini, const char *sect, ConDecoBorder &border, bool bgnd );
  void DrawBorder ( int x, int y, int width, int height, ConDecoBorder &border, int align );

};

#endif // ! __CS_FUNCON_H__
