
/*
    Crystal Space 3d format converter 


    Based on IVCON - converts various 3D graphics file
    Author: John Burkardt - used with permission
    CS adaption and conversion to C++ classes  Bruce Williams

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

#include "cssysdef.h"
#include "cstool/impexp.h"

// converter.cpp: implementation of the converter class.
//
//////////////////////////////////////////////////////////////////////

/*
  Purpose:
   
    SPR_WRITE writes a standard CS SPR file.

  Modified:

    12 April 2001

  Author:

    Luca Pancallo
*/

/******************************************************************************/

int converter::spr_write ( FILE *fileout ) {

/******************************************************************************/


  int i1;
  int i2;
  int i3;
//  int i4;
  int iface;
//  int ivert;
  int j;
  int line;

  line = 0;
/*
  Write the header.
*/
  char *name = "obj";
  fprintf ( fileout, "MESHOBJ '%s' (\n", name );
  fprintf ( fileout, "  PLUGIN ('crystalspace.mesh.loader.factory.sprite.3d')\n" );
  fprintf ( fileout, "  PARAMS (\n" );
  fprintf ( fileout, "      MATERIAL ('%sskin')\n", name );

  line = line + 4;
/*
  Write the FRAME block.
*/
  //fprintf ( fileout, ";    Number of Vertexes %d\n", num_cor3 );
  //fprintf ( fileout, ";    Number of Faces %d\n", num_face );

  fprintf ( fileout, "\n	FRAME 'stand01' ( \n\n" );

  line = line + 3;
/*
  Write the FRAME vertexes.
*/
  for ( j = 0; j < num_cor3; j++ ) {
    fprintf ( fileout, "      V (%f,%f,%f:%f,%f)\n", cor3[0][j],
      cor3[1][j], cor3[2][j], vertex_uv[0][0][j], vertex_uv[1][0][j]);
    line = line + 1;
  }

  fprintf ( fileout, "	)\n" );

/*
  Write the ACTION block.
*/
  fprintf ( fileout, "\n  ACTION 'stand' ( F ('stand01', 100) ) \n\n");

/*
  Write the TRIANGLE block.
*/

  for ( iface = 0; iface < num_face; iface++ ) {

    i1 = face[0][iface];
    i2 = face[1][iface];
    i3 = face[2][iface];

    if ( face_order[iface] == 3 ) {
      fprintf ( fileout, "      TRIANGLE (%d,%d,%d)\n", i1, i2, i3 ); 
      line = line + 1;
    }
    else if ( face_order[iface] == 4 ) {
      // 4 vertexes! How do we manage this?
      fprintf ( fileout, "\n4 vertexes! How do we manage this?\n\n");
	}
  }

/*
  Write the close block.
*/

    fprintf ( fileout, "  )\n");
	fprintf ( fileout, ")\n");

  return SUCCESS;
}
