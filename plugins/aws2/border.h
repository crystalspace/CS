#ifndef __AWS2_BORDER_H__
#define __AWS2_BORDER_H__

#include "frame.h"
#include "preferences.h"

namespace aws
{
  enum { AWS_BORDER_RECT, AWS_BORDER_ROUNDED_RECT, AWS_BORDER_MITERED_RECT, AWS_BORDER_CIRCLE };

  enum { AWS_BORDER_FLAT, AWS_BORDER_RIDGED, AWS_BORDER_SUNKEN, AWS_BORDER_BEVELED };

  class border : public frame
  {
    unsigned int border_shape;
    unsigned int border_style;

    csColor4 h1, h2, s1, s2, black;

    float edge;

  protected:
    void draw_shape(iPen *pen, bool swap_colors, int inset)
    {
      csRect r(Bounds());

      r.Inset(inset);

      switch(border_shape)
      {
	case AWS_BORDER_RECT:	    pen->DrawRect(r.xmin, r.ymin, r.xmax, r.ymax, swap_colors); break;
	case AWS_BORDER_ROUNDED_RECT: pen->DrawRoundedRect(r.xmin, r.ymin, r.xmax, r.ymax, edge, swap_colors); break;
	case AWS_BORDER_MITERED_RECT: pen->DrawMiteredRect(r.xmin, r.ymin, r.xmax, r.ymax, edge, swap_colors); break;
	case AWS_BORDER_CIRCLE:	    pen->DrawArc(r.xmin, r.ymin, r.xmax, r.ymax, 0, 2*PI, swap_colors); break;
      }
    }

  public:
    border():border_shape(AWS_BORDER_RECT), border_style(AWS_BORDER_FLAT), edge(0.25) {}

    virtual ~border() {}

    /// Set the style of the border.
    void SetBorderStyle(unsigned int _style) { border_style=_style; }

    /// Set the shape of the border.
    void SetBorderShape(unsigned int _shape) { border_shape=_shape; }

    /// If this border uses ROUND_RECT or MITERED_RECT, then this sets the roundness or miter size.
    void SetEdgeLevel(float e)
    {
      edge = e;
    }

    /// Allows the border to get the current colors.
    void UpdateSkin(preferences &prefs)
    {
      black = prefs.getColor(AC_BLACK); 

      switch(border_style)
      {
	case AWS_BORDER_FLAT: break;
	case AWS_BORDER_RIDGED: h1 = prefs.getColor(AC_HIGHLIGHT); s1 = prefs.getColor(AC_SHADOW); break;
	case AWS_BORDER_SUNKEN: h1 = prefs.getColor(AC_SHADOW); s1 = prefs.getColor(AC_HIGHLIGHT); break;
	case AWS_BORDER_BEVELED: h1 = prefs.getColor(AC_HIGHLIGHT); s1 = prefs.getColor(AC_SHADOW); 
				 h2 = prefs.getColor(AC_HIGHLIGHT2); s2 = prefs.getColor(AC_SHADOW2); break;
      }
    }
  


    /// Draws a border around something
    virtual void OnDraw(iPen *pen)
    { 
      if (border_style!=AWS_BORDER_FLAT)
      {
	pen->SetColor(s1);
	pen->SwapColors();
	pen->SetColor(h1);

	draw_shape(pen, true, 1);
      }

      if (border_style==AWS_BORDER_BEVELED)
      {
	pen->SetColor(s2);
	pen->SwapColors();
	pen->SetColor(h2);

	draw_shape(pen, true, 2);
      }

      pen->SetColor(black);
      draw_shape(pen, false, 0);
    }
  };


}; // end namespace

#endif

