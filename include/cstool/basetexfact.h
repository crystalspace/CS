/*
    Copyright (C) 2003 by Jorrit Tyberghein
	      (C) 2003 by Frank Richter

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

#ifndef __CS_CSTOOL_BASETEXFACT_H__
#define __CS_CSTOOL_BASETEXFACT_H__

/**\file
 * Texture factory base.
 */

#include "csextern.h"

#include "csutil/scf.h"
#include "itexture/itexfact.h"

struct iObjectRegistry;

/**
 * Base class for texture factories, with support for common parameters.
 * \remark Descendants must at least implement Generate(). Adding more 
 * parameters should be done via additional interfaces.
 */
class CS_CRYSTALSPACE_EXPORT csBaseTextureFactory : public iTextureFactory
{
protected:
  /// object registry
  iObjectRegistry* object_reg;
  /// Width parameter
  int width;
  /// Height parameter
  int height;
  /// Texture Type
  iTextureType* texture_type;

public:
  SCF_DECLARE_IBASE;

  csBaseTextureFactory (iTextureType* parent, iObjectRegistry* object_reg);
  virtual ~csBaseTextureFactory();
  
  /**
   * Set the size of the texture to be created.
   */
  virtual void SetSize (int w, int h);
  /**
   * Get the size of the texture to be created.
   */
  virtual void GetSize (int& w, int& h);
  /**
   * Get the TextureType for this texture factory.
   */
  virtual iTextureType* GetTextureType () const;
};

#endif
