

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



/******************************************************************************/

int converter::txt_write ( FILE *fileout ) {

/******************************************************************************/

/*
  Purpose:
   
    TXT_WRITE writes the graphics data to a text file.

  Modified:

    02 December 1998

  Author:
 
    John Burkardt
*/
  int i;
  int iface;
  int iline;
  int ivert;
  int nitem;
  int num_text;

  num_text = 0;

  fprintf ( fileout, "%s created by IVCON.\n", fileout_name );
  fprintf ( fileout, "Original data in %s.\n", filein_name );
  fprintf ( fileout, "Object name is %s.\n", object_name );
  fprintf ( fileout, "Object origin at %f %f %f.\n", origin[0], origin[1],
    origin[2] );
  fprintf ( fileout, "Object pivot at %f %f %f.\n", pivot[0], pivot[1],
    pivot[2] );
  num_text = num_text + 5;
/*
  TRANSFORMATION MATRIX.
*/
  fprintf ( fileout, "\n" );
  fprintf ( fileout, "Transformation matrix:\n" );
  fprintf ( fileout, "\n" );
  for ( i = 0; i < 4; i++ ) {
    fprintf ( fileout, "  %f %f %f %f\n", transform_mat[i][0],
      transform_mat[i][1], transform_mat[i][2], transform_mat[i][3] );
  }
  num_text = num_text + 7;
/*
  POINT COORDINATES.
*/
  fprintf ( fileout, "\n" );
  fprintf ( fileout, "  %d points.\n", num_cor3 );
  num_text = num_text + 2;

  if ( num_cor3 > 0 ) {

    fprintf ( fileout, "\n" );
    fprintf ( fileout, "  Point coordinate data:\n" );
    fprintf ( fileout, "\n" );
    num_text = num_text + 3;

    for ( i = 0; i < num_cor3; i++ ) {
      fprintf ( fileout, " %d %f %f %f\n ", i, cor3[0][i], cor3[1][i], 
        cor3[2][i] );
      num_text = num_text + 1;
    }

    fprintf ( fileout, "\n" );
    fprintf ( fileout, "  Point normal vectors:\n" );
    fprintf ( fileout, "\n" );
    num_text = num_text + 3;

    for ( i = 0; i < num_cor3; i++ ) {
      fprintf ( fileout, " %d %f %f %f\n ", i, cor3_normal[0][i], 
        cor3_normal[1][i], cor3_normal[2][i] );
      num_text = num_text + 1;
    }

  }
/*
  LINE INDICES.
*/
  fprintf ( fileout, "\n" );
  fprintf ( fileout, "  %d line data items.\n", num_line );
  num_text = num_text + 2;

  if ( num_line > 0 ) {
    fprintf ( fileout, "\n" );
    fprintf ( fileout, "  Line index data:\n" );
    fprintf ( fileout, "\n" );
    num_text = num_text + 3;

    nitem = 0;

    for ( iline = 0; iline < num_line; iline++ ) {

      fprintf ( fileout, " %d", line_dex[iline] );
      nitem = nitem + 1;

      if ( iline == num_line - 1 || line_dex[iline] == -1 || nitem >= 10 ) {
        nitem = 0;
        fprintf ( fileout, "\n" );
        num_text = num_text + 1;
      }

    }
/*
  LINE MATERIALS.
*/
    fprintf ( fileout, "\n" );
    fprintf ( fileout, "  Line material data:\n" );
    fprintf ( fileout, "\n" );
    num_text = num_text + 3;

    nitem = 0;

    for ( iline = 0; iline < num_line; iline++ ) {

      fprintf ( fileout, " %d", line_mat[iline] );
      nitem = nitem + 1;

      if ( iline == num_line - 1 || line_mat[iline] == -1 || nitem >= 10 ) {
        nitem = 0;
        fprintf ( fileout, "\n" );
        num_text = num_text + 1;
      }
    }

  }
/*
  COLOR DATA
*/
  fprintf ( fileout, "\n" );
  fprintf ( fileout, "  %d colors.\n", num_color );
  num_text = num_text + 2;
/*
  TEXTURE MAPS
*/
  fprintf ( fileout, "\n" );
  fprintf ( fileout, "  %d texture maps.\n", num_texmap );
  num_text = num_text + 2;
/*
  FACE DATA.
*/
  fprintf ( fileout, "\n" );
  fprintf ( fileout, "  %d faces.\n", num_face );
  num_text = num_text + 2;

  if ( num_face > 0 ) {

    fprintf ( fileout, "\n" );
    fprintf ( fileout, "  Face, Number of vertices, Smoothing, Flags:\n" );
    fprintf ( fileout, "\n" );
    num_text = num_text + 3;

    for ( iface = 0; iface < num_face; iface++ ) {
      fprintf ( fileout, " %d %d %d %d\n", iface, face_order[iface], 
        face_smooth[iface], face_flags[iface] );
      num_text = num_text + 1;
    }

    fprintf ( fileout, "\n" );
    fprintf ( fileout, "  Face, Vertices\n" );
    fprintf ( fileout, "\n" );
    num_text = num_text + 3;

    for ( iface = 0; iface < num_face; iface++ ) {

      fprintf ( fileout, "%d   ", iface );
      for ( ivert = 0; ivert < face_order[iface]; ivert++ ) {
        fprintf ( fileout, " %d", face[ivert][iface] );
      }

      fprintf ( fileout, "\n" );
      num_text = num_text + 1;
    }

  }
/*
  FACE NORMAL VECTORS.
*/
  if ( num_face > 0 ) {

    fprintf ( fileout, "\n" );
    fprintf ( fileout, "  Face normal vectors:\n" );
    fprintf ( fileout, "\n" );
    num_text = num_text + 3;

    for ( iface = 0; iface < num_face; iface++ ) {
      fprintf ( fileout, " %d %f %f %f\n", iface, face_normal[0][iface],
        face_normal[1][iface], face_normal[2][iface] );
      num_text = num_text + 1;
    }

  }
/*
  FACE VERTEX NORMAL INDICES.
*/
  if ( num_face > 0 ) {

    fprintf ( fileout, "\n" );
    fprintf ( fileout, "Face vertex-to-normal indices:\n" );
    num_text = num_text + 2;

    for ( iface = 0; iface < num_face; iface++ ) {
      fprintf ( fileout, "\n" );
      num_text = num_text + 1;
      for ( ivert = 0; ivert < face_order[iface]; ivert++ ) {
        fprintf ( fileout, " %d %d %f %f %f\n", iface, ivert, 
          vertex_normal[0][ivert][iface], vertex_normal[1][ivert][iface],
          vertex_normal[2][ivert][iface] );
        num_text = num_text + 1;
      }
    }
  }
/*
  Report.
*/
  fprintf ( logfile,  "\n" );
  fprintf ( logfile,  "TXT_WRITE - Wrote %d text lines.\n", num_text );

  return SUCCESS;
}
