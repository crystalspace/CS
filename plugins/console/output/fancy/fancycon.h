/*
    Copyright (C) 2000 by Norman Kramer

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

#ifndef __FANCYCON_H__
#define __FANCYCON_H__

#include "ivaria/conout.h"
#include "isys/plugin.h"
#include "csgeom/csrect.h"

struct iGraphics2D;
struct iGraphics3D;
struct iMaterialHandle;
struct iVFS;
struct iImageIO;

struct ConDecoBorder
{
  iMaterialHandle *mat;
  bool do_keycolor;
  UByte kr, kg, kb;
  bool do_stretch;
  int offx, offy;
  bool do_alpha;
  float alpha;
};

struct ConsoleDecoration
{
  ConDecoBorder border[8];
  ConDecoBorder bgnd;
  int lx, rx, ty, by;
  int p2lx, p2rx, p2ty, p2by;
};

class csFancyConsole : public iConsoleOutput
{
private:
  iSystem *System;
  iVFS *VFS;
  iConsoleOutput* base;
  iGraphics2D *G2D;
  iGraphics3D *G3D;
  iImageIO *ImageLoader;
  ConsoleDecoration deco;
  csRect outersize, bordersize, p2size;
  bool border_computed;
  bool pix_loaded;
  bool system_ready;
  bool auto_update;
  bool visible;

  void LoadPix();
  void PrepPix(iConfigFile *ini, const char *sect, ConDecoBorder &border,
    bool bgnd );
  void DrawBorder(int x, int y, int width, int height, ConDecoBorder &border,
    int align );
  void GetPosition(int &x, int &y, int &width, int &height) const;
  void SetPosition(int x, int y, int width = -1, int height = -1);

public:
  SCF_DECLARE_IBASE;
  csFancyConsole (iBase *);
  virtual ~csFancyConsole ();
  virtual bool Initialize (iSystem *);
  virtual bool HandleEvent (iEvent &Event);
  virtual void PutText (int iMode, const char *iText);
  virtual const char *GetLine (int iLine = -1) const
    { return base->GetLine(iLine); }
  virtual void Draw2D (csRect *oRect = 0) { base->Draw2D(oRect); }
  virtual void Draw3D (csRect *rect);
  virtual void Clear (bool iWipe = false) { base->Clear(iWipe); }
  virtual void SetBufferSize (int n) { base->SetBufferSize(n); }
  virtual bool GetTransparency () const { return base->GetTransparency(); }
  virtual void SetTransparency (bool) { base->SetTransparency (true); }
  virtual iFont *GetFont () const { return base->GetFont(); }
  virtual void SetFont (iFont *Font) { base->SetFont(Font); }
  virtual int GetTopLine () const { return base->GetTopLine(); }
  virtual void ScrollTo (int iTopLine, bool iSnap = true)
    { base->ScrollTo(iTopLine, iSnap); }
  virtual int GetCursorStyle () const { return base->GetCursorStyle(); }
  virtual void SetCursorStyle (int iStyle) { base->SetCursorStyle(iStyle); }
  virtual void SetVisible (bool b) { visible = b; base->SetVisible(b); }
  virtual bool GetVisible () { return visible; }
  virtual void AutoUpdate (bool b) { auto_update = b; base->AutoUpdate(b); }
  virtual void SetCursorPos (int p) { base->SetCursorPos(p); }
  virtual int GetMaxLineWidth () { return base->GetMaxLineWidth(); }
  virtual void RegisterWatcher (iConsoleWatcher *p)
    { base->RegisterWatcher(p); }
  virtual bool ConsoleExtension (const char *command, ...);
  virtual bool ConsoleExtensionV (const char *iCommand, va_list);

  // Implement iPlugIn interface.
  struct eiPlugIn : public iPlugIn
  {
    SCF_DECLARE_EMBEDDED_IBASE(csFancyConsole);
    virtual bool Initialize (iSystem* p) { return scfParent->Initialize(p); }
    virtual bool HandleEvent (iEvent& e) { return scfParent->HandleEvent(e); }
  } scfiPlugIn;
};

#endif // __FANCYCON_H__
