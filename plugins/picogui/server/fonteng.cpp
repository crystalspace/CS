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

#include "cssysdef.h"

extern "C"
{
  #include <picogui/types.h>
  #include <pgserver/common.h>
  #include <pgserver/types.h>
  #include <pgserver/font.h>
}

#include "fonteng.h"

csRef<iFontServer> csPGFontEngine::Serv;

bool csPGFontEngine::Construct (iFontServer *s)
{
#if 0
  Serv = s;
  static fontlib lib;
  lib.engine_init	= Init;
  lib.engine_shutdown	= Shutdown;
  lib.draw_char		= DrawChar;
  lib.measure_char	= MeasureChar;
  lib.draw_string	= DrawString;
  lib.measure_string	= MeasureString;
  lib.create		= Create;
  lib.destroy		= Destroy;
  lib.getstyle		= GetStyle;
  lib.getmetrics	= GetMetrics;
  g_error err = bdf_regfunc (& lib);
  return err == 0;
#endif
  return true;
}

g_error csPGFontEngine::Init ()
{
  return 0;
}

void csPGFontEngine::Shutdown ()
{
  Serv = 0;
}

void csPGFontEngine::DrawChar (font_descriptor *f, stdbitmap *b,
  pgpair *pos, uint32 color, int ch, pgquad *clip, int16 lgop, int16 angle)
{
  char str[2];
  str[0] = ch;
  str[1] = '\0';

  //GETBMP(b)->G2D ()->Write (GETFONT(f), pos->x, pos->y, color, -1, str);
}

void csPGFontEngine::MeasureChar (font_descriptor *f, pgpair *pos,
  int ch, int16 angle)
{
  int x32, y32;
  static char str[2] = " ";
  str[0] = ch;
  GETFONT(f)->GetDimensions (str, x32, y32);
  pos->x = x32;
  pos->y = y32;
}

void csPGFontEngine::DrawString (font_descriptor *f, stdbitmap *b,
  pgpair *pos, uint32 color, const pgstring *str,
  pgquad *clip, int16 lgop, int16 angle)
{
  //GETBMP(b)->G2D ()->Write (GETFONT(f), pos->x, pos->y, color, -1, GETSTR(str));
}

void csPGFontEngine::MeasureString (font_descriptor *f, const pgstring *str,
  int16 angle, int16 *x, int16 *y)
{
  int x32, y32;
  GETFONT(f)->GetDimensions (GETSTR(str), x32, y32);
  *x = x32;
  *y = y32;
}

g_error csPGFontEngine::Create (font_descriptor *f, const font_style *s)
{
  csRef<iFont> font = Serv->LoadFont (s->name);
  if (font)
  {
    font->SetSize (s->size);
    font->IncRef ();
    SETFONT(f, font);
    return 0;
  }
  else
    return 1;
}

void csPGFontEngine::Destroy (font_descriptor *f)
{
  GETFONT(f)->DecRef ();
}

void csPGFontEngine::GetStyle (int n, font_style *s)
{
  csRef<iFont> font = Serv->GetFont (n);
  if (font)
  {
    s->name = "";
    s->size = font->GetSize ();
  }
}

void csPGFontEngine::GetMetrics (font_descriptor *f, font_metrics *m)
{
  int w32, h32;
  GETFONT(f)->GetMaxSize (w32, h32);
  m->charcell.w = w32;
  m->charcell.h = h32;
  m->ascent = GETFONT(f)->GetAscent ();
  m->descent = GETFONT(f)->GetDescent ();
  m->linegap = 0;
  m->lineheight = m->ascent + m->descent;
}
