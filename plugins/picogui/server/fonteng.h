/*
    Crystal Space Font Engine Interface for PicoGUI
    (C) 2003 Mat Sutcliffe <oktal@gmx.co.uk>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
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

#ifndef __CS_PICOGUI_SERVER_FONTENG_H__
#define __CS_PICOGUI_SERVER_FONTENG_H__

#include "csutil/ref.h"
#include "ivideo/fontserv.h"
#include "csutil/csstring.h"
#include "hbitmap.h"

class csPicoGUIServer;

/**
 * The static methods of this class make up a PicoGUI font engine.
 * PicoGUI will call them, and they access the CS font server.
 * They may be used as C-style function pointers, for passing to PicoGUI.
 */
class csPGFontEngine
{
 private:
  static csRef<iFontServer> Serv;

 protected:
  static bool Construct (iFontServer *);
  friend class csPicoGUIServer;

 public:
  static g_error Init ();
  static void Shutdown ();
  static void DrawChar (font_descriptor *, stdbitmap *, pgpair *pos,
    uint32 color, int ch, pgquad *clip, int16 lgop, int16 angle);
  static void MeasureChar (font_descriptor *, pgpair *pos, int ch, int16 angle);
  static void DrawString (font_descriptor *, stdbitmap *, pgpair *pos,
    uint32 color, const pgstring *ch, pgquad *clip, int16 lgop, int16 angle);
  static void MeasureString (font_descriptor *, const pgstring *ch,
    int16 angle, int16 *x, int16 *y);
  static g_error Create (font_descriptor *, const font_style *);
  static void Destroy (font_descriptor *);
  static void GetStyle (int i, font_style *);
  static void GetMetrics (font_descriptor *, font_metrics *);
};

/// Cast the (void*) data pointer in a font_descriptor to an iFont
#define GETFONT(PG) ((iFont *) (PG)->data)

/// Set the (void*) data pointer in a font_descriptor to an iFont
#define SETFONT(PG, CS) ((PG)->data = (void *) (CS))

/// Get the stored data buffer from a PicoGUI string (warning: temporary!)
#define GETSTR(PG) (csString ().Append ((char *) (PG)->buffer, (PG)->num_chars).GetData ())

/// Set a PicoGUI string using a (const char*) (warning: temporary!)
#define SETSTR(PG, CS) (* (PG) = * pgstring_tmpwrap (CS))

#endif
