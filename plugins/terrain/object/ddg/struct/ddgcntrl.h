/*
    Copyright (C) 1997, 1998, 1999, 2000 by Alex Pfaffe
	(Digital Dawn Graphics Inc)
  
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
#ifndef _ddgControl_Class
#define _ddgControl_Class

#include "util/ddg.h"
#include "math/ddgvec.h"

/**
 * The context object maintains the position and orientation of an object.
 * Sub classes of this class can be connected to keys/playback routines etc.
 * $TODO create a subclass with path capability, keep core class simple.
 */
class WEXP ddgControl
{
protected:
	/// Flag set to true if orientation has changed from last frame.
    bool				_dirty;
private:
    /// view's rotation angle.
    ddgVector3			_rot;
    /// view's current position.
    ddgVector3			_pos;
public:
	/** 
     *  Constructor.
	 */
	ddgControl(	ddgVector3 *pos = NULL, ddgVector3 *orientation = NULL);
	/**
	 *  Update the current position and orientation.
	 *  Returns true if position or orientation was modified.
	 */
	virtual bool update(void);
	/// Modify the rotation angle of the view's X,Y or Z axis.
	void orientation( float x, float y, float z) { _rot.set(x,y,z); _dirty = true; }
	/// Return the view orientation.
	ddgVector3 *orientation( void ) { return &_rot; }
	/// Modify the world coordinate position of the view along the X,Y or Z axis.
	void position( float x, float y, float z) { _pos.set(x,y,z); _dirty = true; }
	/// Return the view position.
	ddgVector3 *position( void ) { return &_pos; }

    /// Callback which is called to initialize the view.
    bool init(void);
	/// Has context changed since last frame.
	bool dirty( void ) { return _dirty; }
	/// Has context changed since last frame.
	void clean( void ) { _dirty = false; }
};


#endif
