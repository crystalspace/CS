/* ========== HTTP://WWW.SOURCEFORGE.NET/PROJECTS/CRYSTAL ==========
 *
 * FILE: ogl_g3dext.cpp
 *
 * DESCRIPTION:
 *  Base class of the OpenGL EXT renderer.  This implements the interface
 *  "iGraphics3d".
 *
 * LICENSE:
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * AUTHOR:
 *    Thomas H. Hendrick
 *
 * CVS/RCS ID:
 *    $Id$
 *
 * === COPYRIGHT (c)2002 ============== PROJECT CRYSTAL SPACE 3D === */

/* -----------------------------------------------------------------
 * Preprocessor Includes
 * ----------------------------------------------------------------- */
#include "ogl_g3dext.h"


/* -----------------------------------------------------------------
 * Preprocessor Defines
 * ----------------------------------------------------------------- */


/* -----------------------------------------------------------------
 * Method Implementations
 * ----------------------------------------------------------------- */
SCF_IMPLEMENT_IBASE(csGraphics3DGLext)
  SCF_IMPLEMENTS_INTERFACE(iGraphics3D)
SCF_IMPLEMENT_IBASE_END

/* -----------------------------------------------------------------
 * Static Data Declarations
 * ----------------------------------------------------------------- */


/* -----------------------------------------------------------------
 * Public Function Defintions
 * ----------------------------------------------------------------- */

// =================================================================
// CONSTRUCTOR: csGraphics3DGLext
//
// DESCRIPTION:
//  Basic public constructor for this class
//
// PROTECTION:
//  public 
//
// ARGUMENTS:
//  iBase *parent : the iBase parent
//
// RETURN VALUE:
//  N/A
// =================================================================

csGraphics3DGLext::csGraphics3DGLext (iBase *parent ) :
  object_reg (NULL),
  G2D (NULL)
{
  SCF_CONSTRUCT_IBASE (parent);
}


// =================================================================
// DESTRUCTOR : csGraphics3DGLext
//
// DESCRIPTION:
//  Basic public destructor for this class
//
// PROTECTION:
//  public 
//
// ARGUMENTS:
//  None
//
// RETURN VALUE:
//  N/A
// =================================================================

csGraphics3DGLext::~csGraphics3DGLext ( ) {
  // Make sure we close the display
  Close ();

  // G2D - graphics 2D object
  if( G2D ) {
    G2D->DecRef();
  }
}



// =================================================================
// FUNCTION: Initialize 
//
// DESCRIPTION:
//  Initializes this object using the Object Registry Given
//
// PROTECTION:
//  public 
//
// ARGUMENTS:
//  iObjectRegistry *reg: the Object Registry to use
//
// RETURN VALUE:
//  bool: true on success, false on failure
// =================================================================

bool csGraphics3DGLext::Initialize(iObjectRegistry* reg) {
  object_reg = reg;
  CS_ASSERT( NULL != object_reg );
  
  return true;
}

/* -----------------------------------------------------------------
 * Protected Function Definitions
 * ----------------------------------------------------------------- */


/* -----------------------------------------------------------------
 * Private Function Defintions
 * ----------------------------------------------------------------- */


