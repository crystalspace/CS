/*
    Copyright (C) 1997, 1998, 1999 by Alex Pfaffe
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
#ifndef _ddgCamera_Class
#define _ddgCamera_Class

#include "ddgbbox.h"
#include "ddgscene.h"
#include "ddgparam.h"
#include "ddgmatrx.h"
#include "ddggeom.h"

class ddgPath;
class ddgContext;

/**
 * Camera object to manage a scene and connect to a viewport.
 *
 * Class which supports OpenGL camera capabilities.
 * Contains parameters to control various settings
 * Also registers keys for camera motion control.
 *
 * Should define a set of activities which camera can perform.
 * These activities can then be mapped to events.  ddgVector of
 * events is then mapped to activities to allow alternate key mappings.
 */
class WEXP ddgCamera
{
    /// Internal variable for camera position.
    float   _beginX;
    /// Internal variable for camera position.
    float   _beginY;
    /// Internal variable to indicate if camera is moving.
    bool    _moving;
    /// Distance a single step forward should be.
    ddgParam   _stepForward;
    /// Distance a single step to the right should be.
    ddgParam   _stepRight;
    /// Constant used for ssdistance, calculate once per frame.
    float   _kk;
	/// Constant used for transform, calculate once per frame.
	float   _tanHalfFov;
	/// modelview matrix.
    ddgMatrix4		_mm;
	/// projection matrix.
	ddgMatrix4		_mp;
	/// Screen space 2D clipping rectangle.
	int		 _vp[4];
	/// Flag set to true if orientation has changed from last frame.
    bool    _dirty:1;
	/// Flag set to true if we are recording a path.
	bool	_record:1;
	/// Frame we are on in the path.
	unsigned int	_frame;
	/// Path that we are playing back or recording to.
	ddgPath	*_path;
	/// Flag set to true if camera in orthographic mode.
	bool			_ortho;
	/// Size of orthographic window.
	unsigned int	_width,
					_height;
    /// Aspect ratio of the camera, width/height.
    float			_aspect;
    /// The scene which this camera will render.
    ddgScene*			_scene;
    /// Camera's rotation angle.
    ddgVector3			_rot;
    /// Camera's current position.
    ddgVector3			_pos;
    /// The movement of the camera.
    ddgVector3			_delta;
    /// Camera's forward vector (calculated).
    ddgVector3			_forward;
    /// Camera's up vector (calculated).
	ddgVector3			_up;
	/// Box representing the camera's viewing frustrum.
    ddgBBox			_clipbox;
    /// Field of view (wideangle/zoom).
	float			_fov;
	/// The context for this camera.
	ddgContext			*_context;
	/// Parameter to control field of view.
    ddgParam			_fovP;
    /// Error tolerance as percentage of screen width.
    float _tolerance; 
    /// The set of parameters controlled from the key board.
    ddgParamSet		_paramset;

  public:

    /**
	 *  Create a camera object.
     *  pos  Camera location.
     *  orientation  Camera about X (heading), Y (pitch) and Z (roll).
     */
    ddgCamera(ddgContext *ctx, ddgVector3 *pos = NULL, ddgVector3 *orientation = NULL);
    /// Set aspect ratio of the camera, width/height.
    void aspect( float a) { _aspect = a; _dirty = true; }
    /// Get aspect ratio of the camera, width/height.
    float aspect( void ) { return _aspect; }
    /// Set the scene which this camera will render.
    void scene( ddgScene* s ) { _scene = s; }
	/// Set the near clipping plane.
	void nearfar (float n, float f) { _clipbox.setz(n,f); _dirty = true; }
	/// Modify the rotation angle of the camera's X,Y or Z axis.
	void orientation( float x, float y, float z) { _rot.set(x,y,z); _dirty = true; }
	/// Modify the world coordinate position of the camera along the X,Y or Z axis.
	void position( float x, float y, float z) { _pos.set(x,y,z); _dirty = true; }
	/// Return the camera position.
	ddgVector3 *position( void ) { return _pos; }
	/// Return the camera orientation.
	ddgVector3 *orientation( void ) { return _rot; }
	/// Modify the field of vision angle of the camera.
	void fov( float f) { _fov = f; _dirty = true; }
	/// Return the field of vision angle of the camera.
	float fov( void ) { return _fov; }
	/// Modify the bounding box of the camera's viewing frustrum.
	void clipbox( ddgBBox *b) { _clipbox.set( b); _dirty = true; }
	/// Return the bounding box of the camera's viewing frustrum.
	ddgBBox *clipbox( void ) { return &_clipbox; }
	/// Modify the external force moving the camera.
	void delta( ddgVector3* d) { _delta.set( d); _dirty = true; }
	/// Modify the up vector of the camera.
	void up( ddgVector3* u) { _up.set( u); _dirty = true; }
	/// Modify the forward vector of the camera.
	void forward( ddgVector3* f) { _forward.set( f); _dirty = true; }
	/// Return the forward vector of the camera.
	ddgVector3 *forward( void) { return _forward; }

    /// Callback which is called to initialize the camera.
    bool init(void);
    /// Print keyboard control help to stderr.
    void help(void);
    /// Render the scene maintained by this camera.
    void draw(void);
    /// Update the camera for the next frame.
    void update(void);
    /// Event handler, returns true if a redisplay is needed.
    bool mouse(int button, int state, int x, int y);
    /// Event handler, returns true if a redisplay is needed.
    bool mouseMotion(int x, int y);    
    /// Event handler, returns true if a redisplay is needed.
    bool specialKey(int key, int x, int y);
    /// Event handler, returns true if a redisplay is needed.
    bool key(unsigned char key, int x, int y);
    /// Returns true if bbox is visible from this camera.
    bool visible( ddgBBox * bbox );
    /**
	 * Re-calculate frame variables only if camera moved.
     * Cache the the world to camera transformation matrix.
	 */
    void updateFrameInfo(void);

    /// Calculate the screen disance between two points.
    /// returns True if screen distance is greater than tolerance.
    bool ssdistance( ddgVector3 *p1, ddgVector3 *p2, ddgVector3 *normal, bool pixels = false);
    /// returns screen space distance between two points.
    float ssdistance2( ddgVector3 *p1, ddgVector3 *p2);
	/// Set up an ortho graphic camera.
	void ortho( unsigned int w, unsigned int h);
	/**
	 *  Map a 3D world coordinate to screen space.
	 *  vin is in world space, vout is in screen space,
	 *  Returns false if point is not on screen and
	 *  vout is left unchanged.
     */
	bool map(ddgVector3 vin, ddgVector3 *vout);
	/**
	 *  Map a screen coordinate to world space.
	 *  vin is in screen scape, vout is in world space,
	 *  Returns false if point is not on screen and
	 *  vout is left unchanged.
     */
	bool unmap(ddgVector3 vin, ddgVector3 *vout);
	/// Convert the world coordinates to camera coordinates.
	void transform( ddgVector3 vin, ddgVector3 *vout );
	/// Returns true if camera coordinates are visible from camera.
	bool visible( ddgVector3 vin );
	/// Has camera changed since last frame.
	bool dirty( void ) { return _dirty; }
	/// Return the window size.	vp returns the viewport dimensions in pixels.
	void viewport( int vp[4] ) { vp[0] = _vp[0]; vp[1] = _vp[1]; vp[2] = _vp[2]; vp[3] = _vp[3]; }
	/// Set the path and whether it is a playback or recording session.
	void path( ddgPath* p, bool r = false, unsigned int f = 0) { _path = p; _record = r; _frame = f; }
	/// Return the current frame
	unsigned int frame(void) { return _frame; }
	/// Return the context for this camera.
	ddgContext *context(void) { return _context; }
	/// Return the parameter set for this camera.
	ddgParamSet *paramset(void) { return &_paramset; }
    /// Return the world to camera transformation matrix.
    ddgMatrix4 *wtoc(void) { return &_mm; }
	/// Set inverse matrix for objects which should always face the camera.
	void uploadInvertRot();
	/// Get frustrum clipping planes in world space coordinates.
	void extractPlanes(ddgPlane Planes[6]);

}; 


#endif
