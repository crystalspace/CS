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
#ifndef _ddgContext_Class
#define _ddgContext_Class

#include "util/ddg.h"
#include "math/ddgmatrx.h"
#include "math/ddgbbox.h"
#include "math/ddgchull.h"
#include "struct/ddgcntrl.h"

class ddgClock;
class ddgPath;
/**
 * The context object maintains the state of the current view.
 * It defines a location/orientation and various view parameters.
 */
class WEXP ddgContext
{
public:
	/// The rendering mode to use.
	enum Mode{ ALL=0, CLIPPING, NOTEXT, FLAT, MINIMAL };
	/// The quality at which the rendering is to be done.
	enum Quality{ NONE, DEBUG, LOW, MEDIUM, HIGH };
private:
    /// The rendering mode to use for this context.
    Mode				_mode;
    /// The quality mode to use.
    Quality				_quality;
	/// Clock associated with this context.
	ddgClock			*_clock;
	/// Constant used for transform, calculate once per frame.
	float				_tanHalfFOV;
	/// modelview matrix.
    ddgMatrix4			_mm;
	/// projection matrix.
	ddgMatrix4			_pm;
	/// Flag set to true if orientation has changed from last frame.
    bool				_dirty:1;
	/// The object which is controlling the context.
	ddgControl			*_control;
    /// Field of view (wideangle/zoom).
	float				_fov;
	/// Aspect ratio of the view, width/height.
    float				_aspect;
	/// Box representing the view's viewing frustrum.
    ddgBBox3				_clipbox;
	///	A set of frustrum planes.
	ddgPlane3			*_frustrum3d;
	/// Frame we are on in the path.
	unsigned int		_frame;
	/// Path that we are playing back or recording to.
	ddgPath				*_path;
	/// Should goto root object perhaps.
	bool				_record;
    /// view's normalized forward vector (calculated).
    ddgVector3			_forward;
    /// view's up vector (calculated).
	ddgVector3			_up;
    /// view's normalized right vector (calculated).
	ddgVector3			_right;
	/// A root hull against which to clip.
	ddgCHull3			_rootHull;
	/** A pair of lines defining the top down view frustrum
	 *  This is used to perform clipping in the case where the
	 *  camera orientation is level with the XZ plane.
	 */
	ddgPlane2			*_frustrum2d;
	/// A 2D topdown view of the visible area of the view frustrum.
	ddgCHull2			_topDownWedge;
	/// Is the view nearly level.
	bool				_levelView;
public:
	/// Frustrum corner points.
	ddgVector3			fc[8];
	/** 
     *  Constructor.
	 */
	ddgContext(Mode m = ALL, Quality q = MEDIUM,
		ddgControl *ctrl = NULL, ddgClock *cl = NULL);

	/// Destructor.
	~ddgContext(void);

	/// Return the clock.
	ddgClock *clock(void) { return _clock; }
	/// Return the mode.
	Mode	mode(void) { return _mode; }
	/// Return the quality.
	Quality quality(void) { return _quality; }
    /// Set the rendering mode to use for this view.
    void mode (Mode m) { _mode = m; }
    /// Set the quality mode to use.
    void quality(Quality q) { _quality = q; }
    /// Set the clock to use.
    void clock(ddgClock *c) { _clock = c; }
	/// Return the hullset.
	ddgCHull3		*rootHull(void) { return &_rootHull; }
	/// Return the hullset.
	ddgCHull2		*topDownWedge(void) { return &_topDownWedge; }

	/**
	 * Update the clock, control object, matrices and anything else.
	 * Return true if something changed.
	 */
	bool update(void);
	/// Set the near clipping plane.
	void nearfar (float n, float f) { _clipbox.setz(n,f); _dirty = true; }
	/// Modify the field of vision angle of the view.
	void fov( float f) { _fov = f; _tanHalfFOV = tan(ddgAngle::degtorad(_fov/2.0)); _dirty = true; }
	/// Return the field of vision angle of the view.
	float fov( void ) { return _fov; }
    /// Set aspect ratio of the view, width/height.
    void aspect( float a) { _aspect = a; _dirty = true; }
    /// Get aspect ratio of the view, width/height.
    float aspect( void ) { return _aspect; }
	/// Return the tangent of half the field of vision angle of the view.
	float tanHalfFOV( void ) { return _tanHalfFOV; }
	/// Return the control object.
	ddgControl *control(void) { return _control; }
	/// Set the control object.
	void control( ddgControl *c ) { _control = c; }
	/// Modify the bounding box of the view's viewing frustrum.
	void clipbox( ddgBBox3 *b) { _clipbox.set( b->min, b->max); _dirty = true; }
	/// Return the bounding box of the view's viewing frustrum.
	ddgBBox3 *clipbox( void ) { return &_clipbox; }
	/// Clip against the convex hull set.
	ddgInside clip( ddgBBox3 *bbox);

	/// Return the up vector of the view.
	ddgVector3 *up( void) { return &_up; }
	/// Modify the up vector of the view.
	void up( ddgVector3* u) { _up.set( u); _dirty = true; }
	/// Modify the forward vector of the view.
	void forward( ddgVector3* f) { _forward.set( f); _dirty = true; }
	/// Return the forward vector of the view.
	ddgVector3 *forward( void) { return &_forward; }
	/// Modify the right vector of the view.
	void right( ddgVector3* r) { _right.set( r); _dirty = true; }
	/// Return the right vector of the view.
	ddgVector3 *right( void) { return &_right; }

    /// Callback which is called to initialize the view.
    bool init(void);
    /// Returns true if bbox is visible from this view.
    bool visible( ddgBBox3 * bbox );
	/// Convert the world coordinates to view coordinates.
	void transform( ddgVector3 vin, ddgVector3 *vout );
	/// Returns true if world coordinates are visible from view.
	bool visible( ddgVector3 vin );
	/// Has context changed since last frame.
	bool dirty( void ) { return _dirty; }
	/// Set the context to clean.
	void clean( void ) { _dirty = false; }
	/**
	 * Get frustrum clipping planes in world space coordinates.
	 */
	void extractPlanesFromMatrix(ddgPlane3 planes[6]);
	/// Update the clipping information based on the current viewpoint.
	void updateClippingInfo(void);
	/** Return true if the view is practically level.
	 *  That means up vector near [0,1,0] and forward vector near [x,0,y]
	 */
	bool levelView( void ) { return _levelView; }


	/// Return the frustrum planes.
	ddgPlane3 *frustrum(void)	{ return _frustrum3d; }
	/// World to view transformation matrix.
	ddgMatrix4 *transformation(void) { return &_mm; }
	/// View to world transformation matrix. [Inverse]
	void itransformation(ddgMatrix4 *it);
	/// Projection matrix.
	ddgMatrix4 *projection(void) { return &_pm; }

	/// Set the path and whether it is a playback or recording session.
	void path( ddgPath* p, bool r = false, unsigned int f = 0)
	{ _path = p; _record = r; _frame = f; }
	/// Return the current frame
	unsigned int frame(void) { return _frame; }
    /// Calculate the screen disance between two points.
    /// returns True if screen distance is greater than tolerance.
    bool ssdistance( ddgVector3 *p1, ddgVector3 *p2, ddgVector3 *normal, bool pixels = false);
    /// returns screen space distance between two points.
    float ssdistance2( ddgVector3 *p1, ddgVector3 *p2);
	/// Compute frustrum from 5 points in world space coordinates.
	void extractPlanes( ddgPlane3 planes[6]);
};

#endif
