/*
    Copyright (C) 2001 by Jorrit Tyberghein
  
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

#ifndef __IENGINE_CURVE_H__
#define __IENGINE_CURVE_H__

#include "csutil/scf.h"

struct iCurveTemplate;
struct iMaterialWrapper;
class csCurve;

SCF_VERSION (iCurve, 0, 0, 1);

/**
 * This is the interface for a curve.
 */
struct iCurve : public iBase
{
  /// Get the original curve (@@@UGLY).
  virtual csCurve* GetOriginalObject () = 0;
  /// Get the parent curve template.
  virtual iCurveTemplate* GetParentTemplate () = 0;
  /// Set the material wrapper.
  virtual void SetMaterial (iMaterialWrapper* mat) = 0;
  /// Get the material wrapper.
  virtual iMaterialWrapper* GetMaterial () = 0;
  /// Set the name of this curve.
  virtual void SetName (const char* name) = 0;
  /// Get the name of this curve.
  virtual const char* GetName () const = 0;
  /// Set a control point.
  virtual void SetControlPoint (int idx, int control_id) = 0;
};

SCF_VERSION (iCurveTemplate, 0, 0, 1);

/**
 * This is a interface for a curve template.
 */
struct iCurveTemplate : public iBase
{
  /// Set the name of this curve template.
  virtual void SetName (const char* name) = 0;
  /// Get the name of the curve template.
  virtual const char* GetName () const = 0;
  /// Set the material wrapper.
  virtual void SetMaterial (iMaterialWrapper* mat) = 0;
  /// Get the material wrapper.
  virtual iMaterialWrapper* GetMaterial () = 0;
  /// Make a curve from this template.
  virtual iCurve* MakeCurve () = 0;
  /// Get the number of vertices.
  virtual int GetNumVertices () const = 0;
  /// Get a vertex.
  virtual int GetVertex (int idx) const = 0;
};

#endif // __IENGINE_CURVE_H__

