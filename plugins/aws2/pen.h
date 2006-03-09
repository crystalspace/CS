/*
    Copyright (C) 2005 by Christopher Nelson

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

#ifndef __AWS_PEN_H__
#define __AWS_PEN_H__

#include "cstool/pen.h"
#include "csutil/memfile.h"

#include "javascript.h"

/** @file Defines the pen object
 * 
 *   The pen object is used as a base for all user-interface
 * objects.  
 */

/** Creates an initializes the automation objects for a pen. */
void Pen_SetupAutomation();
 
namespace aws
{
  /** @brief The pen class is the native base for all visible pens.
   * It also is the automation base for all visible pens, and creates
   * several automation properties.
   *
   * It makes good sense to keep all the properties in the automation engine,
   * and provide native methods to access them, considering that most of the
   * manipulation of those properties will be done from scripts. */
  class pen : public iPen
  {    
	/** The pen object. */
    JSObject  *p_object;   
    
    /** The buffer that contains the shtuff we want. */
    csMemFile *buf;
    
       
  public:         
  	 /// Sets the object for this pen
     void SetPenObject(JSObject *o) { p_object=o; }
    
     /// Gets the object for this pen.
     JSObject *PenObject() { return p_object; }     
         
  public:
  	 pen():buf(0) { Clear(); }
  	 virtual ~pen() {}
  	   	  	  	   
  	 /** Used by the callback to set the value of a property. */
  	 bool SetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp);
  	 
  	 /** Used by the callback to get the value of a property. */
  	 bool GetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp);  
  	 
  	   	 
  	 //////////////// Drawing ///////////////////////////////////////////
  	 
  	  /** Clears the draw buffer out. */
  	  virtual void Clear() 
  	  {
	 	if (buf) delete buf;
	 	buf = new csMemFile();
	 	buf->SetPos(0);	 
  	  }
  	 
  	  /** Draws the cached contents of this buffer into the pen. */
  	  void Draw(iPen *_pen_);
  	 
  	  /** 
	   * Sets the current color. 
	   */
	  virtual void SetColor (float r, float g, float b, float a);
	
	  /** 
	   * Sets the current color. 
	   */
	  virtual void SetColor(const csColor4 &color);
	
	  /**
	   * Swaps the current color and the alternate color. 
	   */
	  virtual void SwapColors();
	
	  /**    
	   * Clears the current transform, resets to identity.
	   */
	  virtual void ClearTransform();
	
	  /** 
	   * Pushes the current transform onto the stack. *
	   */
	  virtual void PushTransform();
	
	  /**
	   * Pops the transform stack. The top of the stack becomes the current
	   * transform. 
	   */
	  virtual void PopTransform();
	
	  /** 
	   * Sets the origin of the coordinate system. 
	   */
	  virtual void SetOrigin(const csVector3 &o);
	
	  /** 
	   * Translates by the given vector
	   */
	  virtual void Translate(const csVector3 &t);
	
	  /**
	   * Rotates by the given angle.
	   */
	  virtual void Rotate(const float &a);
	
	  /** 
	   * Draws a single line. 
	   */
	  virtual void DrawLine (uint x1, uint y1, uint x2, uint y2);
	
	  /** 
	   * Draws a single point. 
	   */
	  virtual void DrawPoint (uint x1, uint y2);
	
	  /** 
	   * Draws a rectangle. 
	   */
	  virtual void DrawRect (uint x1, uint y1, uint x2, uint y2,
	  	bool swap_colors = false, bool fill = false);
	  
	  /** 
	   * Draws a mitered rectangle. The miter value should be between 0.0 and 1.0, 
	   * and determines how much of the corner is mitered off and beveled. 
	   */
	  virtual void DrawMiteredRect (uint x1, uint y1, uint x2, uint y2, 
	    float miter, bool swap_colors = false, bool fill = false);
	
	  /** 
	   * Draws a rounded rectangle. The roundness value should be between
	   * 0.0 and 1.0, and determines how much of the corner is rounded off. 
	   */
	  virtual void DrawRoundedRect (uint x1, uint y1, uint x2, uint y2, 
	    float roundness, bool swap_colors = false, bool fill = false); 
	
	  /** 
	   * Draws an elliptical arc from start angle to end angle.  Angle must be
	   * specified in radians. The arc will be made to fit in the given box.
	   * If you want a circular arc, make sure the box is a square.  If you want
	   * a full circle or ellipse, specify 0 as the start angle and 2*PI as the end
	   * angle.
	   */
	  virtual void DrawArc(uint x1, uint y1, uint x2, uint y2, float start_angle,
	  	float end_angle, bool swap_colors = false, bool fill=false);
	
	  /**
	   * Draws a triangle around the given vertices. 
	   */
	  virtual void DrawTriangle(uint x1, uint y1, uint x2, uint y2, uint x3, uint y3, bool fill=false);
	
	  /**
	   * Writes text in the given font at the given location.
	   */
	  virtual void Write(iFont *font, uint x1, uint y1, char *text);
	
	  /**
	   * Writes text in the given font, in the given box.  The alignment
	   * specified in h_align and v_align determine how it should be aligned.  
	   */
	  virtual void WriteBoxed(iFont *font, uint x1, uint y1, uint x2, uint y2, 
	    uint h_align, uint v_align, char *text);
  	       
  };  
  
  
}

/** Returns true if the object is a pen object. */
  bool IsPenObject(JSObject *o);
  

#endif
