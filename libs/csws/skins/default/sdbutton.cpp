/*
    Crystal Space Windowing System: Default button skin
    Copyright (C) 1998,1999 by Andrew Zabolotny <bit@eltech.ru>

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

#include "cssysdef.h"
#include "csws/csbutton.h"
#include "csws/sdefault.h"

void csDefaultButtonSkin::Draw (csComponent &This)
{
#define This ((csButton &)This)
  int li, di;                   // light and dark colors
  int areaw = 0, areah = 0;             // drawing area width and height

  csPixmap *ImageNormal, *ImagePressed;
  This.GetBitmap (&ImageNormal, &ImagePressed);

  int ButtonStyle = This.GetButtonStyle ();

  if (This.Pressed)
  {
    di = CSPAL_BUTTON_LIGHT3D;
    li = CSPAL_BUTTON_DARK3D;
  }
  else
  {
    di = CSPAL_BUTTON_DARK3D;
    li = CSPAL_BUTTON_LIGHT3D;
  } /* endif */

  bool DefaultBorder = ((ButtonStyle & CSBS_NODEFAULTBORDER) == 0)
    && ((This.parent->GetDefault () == &This));

  // Draw the frame
  switch (This.GetFrameStyle ())
  {
    case csbfsNone:
      if (!This.GetState (CSS_TRANSPARENT))
        This.Clear (CSPAL_BUTTON_BACKGROUND);
      areaw = This.bound.Width (); areah = This.bound.Height ();
      break;
    case csbfsOblique:
      if (This.bound.Height () >= 6)
      {
        int aw = This.bound.Height () / 3;
        if (aw > This.bound.Width () - 4)
          aw = This.bound.Width () - 4;
        if (DefaultBorder)
          This.ObliqueRect3D (0, 0, This.bound.Width (), This.bound.Height (), aw,
            CSPAL_BUTTON_DEFFRAME, CSPAL_BUTTON_DEFFRAME);
        else
          This.ObliqueRect3D (0, 0, This.bound.Width (), This.bound.Height (), aw,
            CSPAL_BUTTON_LIGHT3D, CSPAL_BUTTON_DARK3D);
        This.ObliqueRect3D (1, 1, This.bound.Width () - 1, This.bound.Height () - 1, aw - 1, di, li);
        This.ObliqueRect3D (2, 2, This.bound.Width () - 2, This.bound.Height () - 2, aw - 2, di, li);
        int rvy = This.bound.Height ();
        // Fill the button background
        int dx = aw - 1;
        rvy = This.bound.Height () - 1;
		int i;
        for (i = 3; i < aw; i++, dx--)
        {
          This.Line (dx, i, This.bound.Width () - 3, i, CSPAL_BUTTON_BACKGROUND);
          This.Line (3, rvy - i, This.bound.Width () - dx, rvy - i, CSPAL_BUTTON_BACKGROUND);
        } /* endfor */
        This.Box (3, aw, This.bound.Width () - 3, This.bound.Height () - aw, CSPAL_BUTTON_BACKGROUND);
        areaw = This.bound.Width () - 6; areah = This.bound.Height () - 6;
        break;
      } // otherwise fallback to rectangular frame
    case csbfsThickRect:
      if (DefaultBorder)
        This.Rect3D (0, 0, This.bound.Width (), This.bound.Height (),
          CSPAL_BUTTON_DEFFRAME, CSPAL_BUTTON_DEFFRAME);
      else
        This.Rect3D (0, 0, This.bound.Width (), This.bound.Height (),
          CSPAL_BUTTON_LIGHT3D, CSPAL_BUTTON_DARK3D);
      This.Rect3D (1, 1, This.bound.Width () - 1, This.bound.Height () - 1, di, li);
      This.Rect3D (2, 2, This.bound.Width () - 2, This.bound.Height () - 2, di, li);
      This.Box (3, 3, This.bound.Width () - 3, This.bound.Height () - 3, CSPAL_BUTTON_BACKGROUND);
      areaw = This.bound.Width () - 6; areah = This.bound.Height () - 6;
      break;
    case csbfsThinRect:
      if (DefaultBorder)
        This.Rect3D (0, 0, This.bound.Width (), This.bound.Height (),
          CSPAL_BUTTON_DEFFRAME, CSPAL_BUTTON_DEFFRAME);
      else
        This.Rect3D (0, 0, This.bound.Width (), This.bound.Height (),
          CSPAL_BUTTON_LIGHT3D, CSPAL_BUTTON_DARK3D);
      This.Rect3D (1, 1, This.bound.Width () - 1, This.bound.Height () - 1, di, li);
      This.Box (2, 2, This.bound.Width () - 2, This.bound.Height () - 2, CSPAL_BUTTON_BACKGROUND);
      areaw = This.bound.Width () - 4; areah = This.bound.Height () - 4;
      break;
    case csbfsVeryThinRect:
      if (This.Pressed)
      {
        This.Rect3D (0, 0, This.bound.Width (), This.bound.Height (), di, li);
        areaw = This.bound.Width () - 2; areah = This.bound.Height () - 2;
        This.Box (1, 1, This.bound.Width () - 1, This.bound.Height () - 1, CSPAL_BUTTON_BACKGROUND);
      }
      else
      {
        areaw = This.bound.Width (); areah = This.bound.Height ();
        This.Clear (CSPAL_BUTTON_BACKGROUND);
      } /* endif */
      break;

    case csbfsTextured: {
    	csPixmap *FrameNormal, *FramePressed;
    	int			  OrgX, OrgY;

    	// Retrieve info needed for texture operations
    	This.GetFrameBitmaps(&FrameNormal, &FramePressed, 0);
    	This.GetTextureOrigin(&OrgX, &OrgY);

			// Tile texture using user origins and user alpha
    	if (This.Pressed)
    		This.Pixmap(FramePressed,
    							  1, 1, This.bound.Width ()-1, This.bound.Height ()-1,
    							  OrgX+1, OrgY+1, This.GetAlpha());

	   	else
    		This.Pixmap(FrameNormal,
    							  1, 1, This.bound.Width ()-1, This.bound.Height ()-1,
    							  OrgX, OrgY, This.GetAlpha());


    	// Draw a simple 3D border
      if (DefaultBorder)
        This.Rect3D (0, 0, This.bound.Width (), This.bound.Height (),
          CSPAL_BUTTON_DEFFRAME, CSPAL_BUTTON_DEFFRAME);
      else
        This.Rect3D (0, 0, This.bound.Width (), This.bound.Height (),
          CSPAL_BUTTON_LIGHT3D, CSPAL_BUTTON_DARK3D);


    	This.Rect3D (1, 1, This.bound.Width () - 1, This.bound.Height () - 1, di, li);
      //This.Rect3D (2, 2, This.bound.Width () - 2, This.bound.Height () - 2, di, li);


    } /* end case csbfsTextured */
    break;

    case csbfsBitmap: {
    	csPixmap *FrameNormal, *FramePressed, *FrameHighlighted;

    	// Get the bitmaps we can use
    	This.GetFrameBitmaps(&FrameNormal, &FramePressed, &FrameHighlighted);

    	int x = (This.bound.Width() - FrameNormal->Width()) / 2;

			// Draw bitmaps (not tiled)
			if (This.Pressed)
    		This.Pixmap(FramePressed, x, 0, This.GetAlpha());

    	else if (This.Highlighted)
    		This.Pixmap(FrameHighlighted, x, 0, This.GetAlpha());

	   	else
	   		This.Pixmap(FrameNormal, x, 0, This.GetAlpha());

    } /* end case csbfsBitmap */
    break;

    default:
      return;
  } /* endswitch */

  // Calculate image position
  int imgx = 0, imgy = 0, imgw = 0, imgh = 0;
  csPixmap *img = This.Pressed ? ImagePressed : ImageNormal;
  if (img)
  {
    imgw = img->Width (); imgh = img->Height ();
    if (imgw > areaw) imgw = areaw;
    if (imgh > areah) imgh = areah;
    imgx = (This.bound.Width () - imgw) / 2;
    imgy = (This.bound.Height () - imgh) / 2;
    if ((ButtonStyle & CSBS_SHIFT) && This.Pressed) { imgx++; imgy++; }
  } /* endif */

  // Calculate text position
  int txtx = 0, txty = 0;
  const char *text = This.GetText ();
  if (text)
  {
    int fh, fw = This.GetTextSize (text, &fh);
    txtx = (This.bound.Width () - fw) / 2;
    if (img)
      switch (ButtonStyle & CSBS_TEXTPLACEMENT)
      {
        case CSBS_TEXTABOVE:
          imgy += fh / 2;
          txty = imgy - fh - 1;
          break;
        case CSBS_TEXTBELOW:
          imgy -= fh / 2;
          txty = imgy + img->Height () + 1;
          break;
        case CSBS_TEXTONTOP:
          txty = (This.bound.Height () - fh) / 2;
          break;
      } /* endswitch */
    else if (This.GetFrameStyle() == csbfsBitmap)
    {
    	txty = This.bound.Height() - fh;

    }
    else
      txty = (This.bound.Height () - fh) / 2;

    if ((ButtonStyle & CSBS_SHIFT) && This.Pressed) { txtx++; txty++; }
  }

  // Draw image
  if (img)
    This.Pixmap (img, imgx, imgy, imgw, imgh, This.GetState (CSS_DISABLED) ? 192 : 0);
  // Draw text
  if (text)
  {
    if (!This.GetDrawTextOnHighlightOnly() ||
            (This.GetDrawTextOnHighlightOnly() && (This.Highlighted || This.Pressed)))
   {
      This.Text (txtx, txty, This.GetState (CSS_DISABLED) ? CSPAL_BUTTON_DTEXT :
      	(This.Highlighted ?  CSPAL_BUTTON_LIGHT3D : CSPAL_BUTTON_TEXT), -1, text);
   }

    if (!This.GetState (CSS_DISABLED))
      This.DrawUnderline (txtx, txty, text, This.GetUnderlinePos (), CSPAL_BUTTON_TEXT);
  }
#undef This
}

void csDefaultButtonSkin::SuggestSize (csButton &This, int &w, int &h)
{
  w = 0; h = 0;

  csPixmap *ImageNormal, *ImagePressed;
  This.GetBitmap (&ImageNormal, &ImagePressed);

  if (ImagePressed)
  {
    w = ImagePressed->Width ();
    h = ImagePressed->Height ();
  } /* endif */
  if (ImageNormal)
  {
    if (w < ImageNormal->Width ())
      w = ImageNormal->Width ();
    if (h < ImageNormal->Height ())
      h = ImageNormal->Height ();
  } /* endif */

  const char *text = This.GetText ();
  if (text)
  {
    int th, tw = This.GetTextSize (text, &th);
    tw += 8; th += 4;
    if (tw > w) w = tw;
    h += th;
  } /* endif */

  switch (This.GetFrameStyle ())
  {
    case csbfsOblique:
      h += 6;
      w += (h / 3) * 2;
      break;
    case csbfsThickRect:
      w += 6;
      h += 6;
      break;
    case csbfsThinRect:
      w += 4;
      h += 4;
      break;
    case csbfsVeryThinRect:
      w += 1;
      h += 1;
      break;
    case csbfsNone:
    default:
      break;
  } /* endswitch */
}
