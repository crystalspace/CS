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
#ifdef DDG
#include "util/ddgpath.h"
#include "util/ddgclock.h"
#endif
#include "struct/ddgcntxt.h"

ddgContext::ddgContext(Mode m, Quality q, ddgControl *ctrl, ddgClock *cl)
{
	_mode = m;
	_quality = q;
	_clock = cl;
	_clipbox.set(0,100,0,100,0.6,5100);  // 5 Km visibility.
	_fov = 90;
	_tanHalfFOV = tan(ddgAngle::degtorad(_fov/2.0));
	_dirty = true;
	_control = ctrl;
	_aspect = 1.0;
	_path = NULL;
	_frame = 0;
	_record = false;
	_frustrum3d = new ddgPlane3[6];
	_levelView = false;
	ddgMemorySet(ddgPlane3,6);
	_rootHull.planes = _frustrum3d;
	_rootHull.noPlanes = 6;

	_frustrum2d = new ddgPlane2[4];
	ddgMemorySet(ddgPlane2,4);
	_topDownWedge.lines = _frustrum2d;
	_topDownWedge.noLines = 4;
}

ddgContext::~ddgContext(void)
{
	delete [] _frustrum3d;
	ddgMemoryFree(ddgPlane3,6);
	_frustrum3d = NULL;
	delete [] _frustrum2d;
	ddgMemoryFree(ddgPlane2,4);
	_frustrum2d = NULL;
}

bool ddgContext::update()
{
	ddgAssert(_control)
	_dirty = false;
#ifdef DDG
	// Advance time.
	if (_clock)
		_clock->update();
	// If we are playing back from a script load the next frame.
	if (_path && !_record && _frame < _path->frames())
	{
		_path->get(_control->position(), _control->orientation(), _frame);
		_dirty = true;
		_frame++;
	}
	// Let the controller move the camera.
	else
#endif
	{
		_dirty = _control->update();
		_control->clean();
	}
	// Calculate a vector in the current direction and an up vector.
	if (_dirty)
		_control->orientation()->angletovector(&_forward,&_right,&_up);
#ifdef DDG
	// If we are recording add the current position.
	if (_path && _record)
	{
		_path->add(_control->position(), _control->orientation());
	}
#endif
	_tanHalfFOV = tan(ddgAngle::degtorad(_fov/2.0));
		// Generally we want to clip agains the left/right top/bottom and near planes.
	// Far is handled by the _farClipSQ and provides a curved clip plane.
	return _dirty;
}


bool ddgContext::visible ( ddgBBox3 *bbox )
{
	float xmin, xmax, ymin, ymax, zmin, zmax;

	// Get each bounding box corner point in screen coordinates.
	xmin = ymin = zmin =  MAXFLOAT;
	xmax = ymax = zmax = -MAXFLOAT;

	float _res[4], _res2[4];
	for (unsigned int i=0; i<8; i++)
	{
        // Convert the world coordinates to screen coordinates.
        // Note this should be done at the same resolution at
        // the final rendering or errors will occur along the edges.
        // Perform matrix multiplication.
        _res[0] = _mm[0]*bbox->cornerx(i) + _mm[4]*bbox->cornery(i) + _mm[8]*bbox->cornerz(i) + _mm[12];
        _res[1] = _mm[1]*bbox->cornerx(i) + _mm[5]*bbox->cornery(i) + _mm[9]*bbox->cornerz(i) + _mm[13];
        _res[2] = _mm[2]*bbox->cornerx(i) + _mm[6]*bbox->cornery(i) + _mm[10]*bbox->cornerz(i) + _mm[14];
        // Multiply by the projection matrix.
        _res2[0] = _pm[0] * _res[0] + _pm[4]*_res[1] + _pm[8]*_res[2] + _pm[12];
        _res2[1] = _pm[1] * _res[0] + _pm[5]*_res[1] + _pm[9]*_res[2] + _pm[13];
        _res2[2] = _pm[2] * _res[0] + _pm[6]*_res[1] + _pm[10]*_res[2] + _pm[14];
        // See if z is still useful.
        if (_res2[2] < _clipbox.cornerz(0) || _res2[2] > _clipbox.cornerz(7))
            continue;
        // Perform perspective scaling.
        _res2[0] = _clipbox.cornerx(0)+(1+(_res2[0]/_res2[2]))*_clipbox.cornerx(7)/2;
        _res2[1] = _clipbox.cornery(0)+(1+(_res2[1]/_res2[2]))*_clipbox.cornery(7)/2;
		
        // Find bounding box.
        if (xmin > _res2[0]) xmin = _res2[0];
        if (xmax < _res2[0]) xmax = _res2[0];
        if (ymin > _res2[1]) ymin = _res2[1];
        if (ymax < _res2[1]) ymax = _res2[1];
        if (zmin > _res2[2]) zmin = _res2[2];
        if (zmax < _res2[2]) zmax = _res2[2];
	}

    // If the bounding box does not overlap the view volume clip.
	if (xmax < _clipbox.cornerx(0) || xmin > _clipbox.cornerx(7) ||
		ymax < _clipbox.cornery(0) || ymin > _clipbox.cornery(7) ||
		zmax < _clipbox.cornerz(0) || zmin > _clipbox.cornerz(7))
	{
        // Indicate that this box is clipped.
        return false;
	}
	return true;
}

// Convert the world coordinates to camera coordinates.
// Don't apply perspective scaling.
// Return true if point is visible from camera.
void ddgContext::transform( ddgVector3 vin, ddgVector3 *vout )
{
    // Convert the world coordinates to camera coordinates.
    // Perform matrix multiplication.
    vout->v[0] = _mm[0]*vin[0]+_mm[4]*vin[1]+_mm[8]*vin[2]+_mm[12];
    vout->v[1] = _mm[1]*vin[0]+_mm[5]*vin[1]+_mm[9]*vin[2]+_mm[13];
    vout->v[2] = _mm[2]*vin[0]+_mm[6]*vin[1]+_mm[10]*vin[2]+_mm[14];
}

// Test camera space coordinate agains frustrum.
bool ddgContext::visible( ddgVector3 vin )
{
	// Test against near, and far plane and test viewing
	// frustrum.
	vin.v[2] *= -1;
	if (_tanHalfFOV != 1.0)		// Not 90 degree case
		vin.multiply(1.0/_tanHalfFOV);

	// Test against near, and far plane and test viewing frustrum.
	if (vin.v[2] < _clipbox.cornerz(0)
		|| vin.v[2] > _clipbox.cornerz(7)
		|| vin.v[2] < fabs(_tanHalfFOV * vin.v[0])
		|| vin.v[2] < fabs(_tanHalfFOV * vin.v[1])
		)
		return false;
	else
		return true;
}

void ddgContext::itransformation( ddgMatrix4 *matinv)
{
	//invert matrix
	matinv->m[0].v[0] = _mm(0,0);
	matinv->m[1].v[1] = _mm(1,1);
	matinv->m[2].v[2] = _mm(2,2);
	matinv->m[0].v[1] = _mm(1,0);
	matinv->m[0].v[2] = _mm(2,0);
	matinv->m[1].v[2] = _mm(2,1);
	matinv->m[1].v[0] = _mm(0,1);
	matinv->m[2].v[0] = _mm(0,2);
	matinv->m[2].v[1] = _mm(1,2);
	matinv->m[3].v[0] = matinv->m[3].v[1] = matinv->m[3].v[2] = 0;
	matinv->m[0].v[3] = matinv->m[1].v[3] = matinv->m[2].v[3] = 0;
	matinv->m[3].v[3] = 1;

}

void ddgContext::updateClippingInfo(void)
{
#ifdef DDG
	extractPlanesFromMatrix(_frustrum3d);
#else
	extractPlanes(_frustrum3d);
#endif
	// Generally we want to clip against the left/right top/bottom and near planes.
	// Far is handled by the _farClipSQ and provides a curved clip plane.
	// When our up vector is near to parallel with the y/height axis
	// We only clip agains the left and right plane since the others are
	// pretty much irrelevant.
	if (_forward[1] == 0.0 && _up[1] == 1.0)
		{
		_levelView = true;
		ddgVector2 pt;
		// Left and right.
		pt.set(_frustrum3d[0].n[0],_frustrum3d[0].n[2]);
		_topDownWedge.lines[0].set(&pt,_frustrum3d[0].d);
		pt.set(_frustrum3d[1].n[0],_frustrum3d[1].n[2]);
		_topDownWedge.lines[1].set(&pt,_frustrum3d[1].d);
		// Near and far.
		pt.set(_frustrum3d[4].n[0],_frustrum3d[4].n[2]);
		_topDownWedge.lines[2].set(&pt,_frustrum3d[4].d);
		pt.set(_frustrum3d[5].n[0],_frustrum3d[5].n[2]);
		_topDownWedge.lines[3].set(&pt,_frustrum3d[5].d);
		}
	else
		{
		_levelView = false;
		}
}


//
// Calculate the approximate screen space distance between to points.
// Assumes normal is a unit vector.
// Assumes p1 and p2 have the same normal.
// Returns   true if estimated screen space distance is larger than tolerance.

bool ddgContext::ssdistance( ddgVector3 *p1, ddgVector3 *p2, ddgVector3 *normal, bool  )
{
	float _kk = 1.0;  // scale factor.
    ddgVector3 v = (*p1) + (*p2);
    v.divide(2.0);          // Middle point between p1 and p2.
    ddgVector3 dv(v- *(_control->position()));		// Translate to camera space.

    float ds = dv.sizesq();	// Squared size of middle point.
    float dd = ds - sq(dv.dot(normal)); // Angular difference.
    return (dd < (_kk*sq(ds)));
}

float ddgContext::ssdistance2( ddgVector3 *p, ddgVector3 *d)
{
	ddgVector3 csp1, csp2;
	/*
	transform( *p, csp1);
	transform( *p + *d, csp2);
	// Check for boundary case.
    if (csp1[2] < clipbox.cornerz(0) || (csp2[2] < clipbox.cornerz(0)))
		return MAXFLOAT;
	// Map to screen coords.
	Vector3 cspp1(csp1), cspp2(csp2);
	cspp1.divide(cspp1.v[2]); cspp1.v[2] = 0;
	cspp2.divide(cspp2.v[2]); cspp2.v[2] = 0;
	Vector3 diff = cspp1 - cspp2;
	*/
	transform(*p - *d, &csp1);
	transform(*p + *d, &csp2);
	csp1.v[2] = 0; csp2.v[2] = 0;
	ddgVector3 diff = csp1 - csp2;
	float l = diff.size();
	return l;
}

ddgInside ddgContext::clip( ddgBBox3 *bbox)
{
	if (_levelView)
		{
		static ddgRect2	rect;
		rect.min.v[0] = bbox->min.v[0];
		rect.max.v[0] = bbox->max.v[0];
		rect.min.v[1] = bbox->min.v[2];
		rect.max.v[1] = bbox->max.v[2];
		return _topDownWedge.clip(&rect);
		}
	else
		return _rootHull.clip(bbox);
}


void ddgContext::extractPlanes( ddgPlane3 planes[6])
{
	ddgVector3 *p = _control->position();

	float snear = _clipbox.cornerz(0), sfar = _clipbox.cornerz(7);
	// Derive 8 world space coords which represent the viewing frustrum.
	// visible from the camera.
	// Right
	ddgVector3 r = _right;
	r.multiply(_tanHalfFOV*_aspect);
	// Up
	ddgVector3 u = _up;
	u.multiply(_tanHalfFOV);
	// Near
	ddgVector3 n = _forward;
	n.multiply(snear);
	// Far
	ddgVector3 f = _forward;
	f.multiply(sfar);

	ddgVector3 t, pn, pf;
	pn = p+n;
	pf = p+f;
	t = u - r; t.multiply(snear);
	fc[0] = pn + t;	// Near Top Left
	fc[3] = pn - t;	// Near Bottom Right
	t = u + r; t.multiply(snear);
	fc[1] = pn + t;	// Near Top Right
	fc[2] = pn - t;	// Near Bottom Left
	
	t = u - r; t.multiply(sfar);
	fc[4] = pf + t;	// Far Top Left
	fc[7] = pf - t;	// Far Bottom Right

	t = u + r; t.multiply(sfar);
	fc[5] = pf + t;	// Far Top Right
	fc[6] = pf - t;	// Far Bottom Left
	// Find the plane vectors for each side of the frustrum.

	ddgVector3 pnormal, v1, v2;
	int i;

	// Points which can be used to create plane equations.
	int pv[6][3] = { {4,0,2}, {1,5,7}, {2,3,7}, {4,5,1}, {0,1,3}, {5,4,6} };
	for (i = 0; i < 6; i++)
	{
		v1 = fc[pv[i][1]] - fc[pv[i][0]];
		v2 = fc[pv[i][2]] - fc[pv[i][0]];
		pnormal.cross(&v1 , &v2);
		planes[i].set(pnormal,-1 * pnormal.dot(p));
		planes[i].normalize();
	}

}

// Extract 6 clipping planes in world space coordinates from the matrices.
void ddgContext::extractPlanesFromMatrix(ddgPlane3 planes[6])
{
	ddgVector4 tmpVec;
	ddgMatrix4 comboMat;

	comboMat = _pm * _mm;
	comboMat.transpose();
	// Left
	tmpVec = comboMat.m[3] - comboMat.m[0];
	planes[0].set(ddgVector3(-tmpVec[0], -tmpVec[1], -tmpVec[2]), -tmpVec[3]);
	planes[0].normalize();
	// Right
	tmpVec = comboMat.m[3] + comboMat.m[0];
	planes[1].set(ddgVector3(-tmpVec[0], -tmpVec[1], -tmpVec[2]), -tmpVec[3]);
	planes[1].normalize();
	// Bottom
	tmpVec = comboMat.m[3] - comboMat.m[1];
	planes[2].set(ddgVector3(-tmpVec[0], -tmpVec[1], -tmpVec[2]), -tmpVec[3]);
	planes[2].normalize();
	// Top
	tmpVec = comboMat.m[3] + comboMat.m[1];
	planes[3].set(ddgVector3(-tmpVec[0], -tmpVec[1], -tmpVec[2]), -tmpVec[3]);
	planes[3].normalize();
	// Near
	tmpVec = comboMat.m[3] - comboMat.m[2];
	planes[4].set(ddgVector3(-tmpVec[0], -tmpVec[1], -tmpVec[2]), -tmpVec[3]);
	planes[4].normalize();
	// Far
	tmpVec = comboMat.m[3] + comboMat.m[2];
	planes[5].set(ddgVector3(-tmpVec[0], -tmpVec[1], -tmpVec[2]), -tmpVec[3]);
	planes[5].normalize();
}
