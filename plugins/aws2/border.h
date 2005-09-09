#ifndef __AWS2_BORDER_H__
#define __AWS2_BORDER_H__

#include "frame.h"
#include "preferences.h"

namespace aws
{
  enum { AWS_BORDER_RECT, AWS_BORDER_ROUND_RECT, AWS_BORDER_MITERED_RECT, AWS_BORDER_CIRCLE };

  enum { AWS_BORDER_FLAT, AWS_BORDER_RIDGED, AWS_BORDER_SUNKEN, AWS_BORDER_BEVELED }

  class border : public frame
  {
    unsigned int border_shape;
    unsigned int border_style;

    csPenColor h1, h2, s1, s2, black;

    float edge;

  public:
    border():border_shape(AWS_BORDER_RECT), border_style(AWS_BORDER_PLAIN), edge(0.25) {}

    virtual ~border() {}

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
      switch(border_style)
      {	
	case AWS_BORDER_RIDGED: 
	case AWS_BORDER_SUNKEN:  
	case AWS_BORDER_BEVELED: 

      }

      pen->setColor(black);
      switch(border_shape)
      {
	case AWS_BORDER_RECT:	    pen->DrawRect(Bounds().x1, Bounds().y1, Bounds().x2, Bounds().y2); break;
	case AWS_BORDER_ROUND_RECT: pen->DrawRoundRect(Bounds().x1, Bounds().y1, Bounds().x2, Bounds().y2, edge); break;
	case AWS_BORDER_MITER_RECT: pen->DrawMiteredRect(Bounds().x1, Bounds().y1, Bounds().x2, Bounds().y2, edge); break;
	case AWS_BORDER_CIRCLE:	    pen->DrawArc(Bounds().x1, Bounds().y1, Bounds().x2, Bounds().y2); break;
      }
    }
  };


}; // end namespace

#endif

