

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

int converter::smf_read ( FILE *filein ) {

/******************************************************************************/

/*
  Purpose:
   
    SMF_READ reads an SMF file.

  Example:

    #  magnolia.obj

    v -3.269770 -39.572201 0.876128
    v -3.263720 -39.507999 2.160890
    ...
    v 0.000000 -9.988540 0.000000
    f 8 9 11 10
    f 12 13 15 14
    ...
    f 788 806 774

  Modified:

    22 October 1998

  Author:
 
    John Burkardt
*/
  int   count;
  int   ivert;
  char *next;
  int   node;
  float r1;
  float r2;
  float r3;
  char  token[MAX_INCHARS];
  char  token2[MAX_INCHARS];
  int   vert_base;
  int   width;

  vert_base = 0;
/* 
  Read the next line of the file into INPUT. 
*/
  while ( fgets ( input, MAX_INCHARS, filein ) != NULL ) {

    num_text = num_text + 1;
/* 
  Advance to the first nonspace character in INPUT. 
*/
    for ( next = input; *next != '\0' && isspace(*next); next++ ) {
    }
/* 
  Skip blank lines and comments. 
*/

    if ( *next == '\0' ) {
      continue;
    }

    if ( *next == '#' || *next == '$' ) {
      num_comment = num_comment + 1;
      continue;
    }
/* 
  Extract the first word in this line. 
*/
    sscanf ( next, "%s%n", token, &width );
/* 
  Set NEXT to point to just after this token. 
*/
    next = next + width;
/*
  BEGIN
*/
    if ( leqi ( token, "BEGIN" ) == TRUE ) {

      vert_base = num_cor3;
      num_group = num_group + 1;

    }
/*
  END
*/
    else if ( leqi ( token, "END" ) == TRUE ) {
    }
/*  
  F V1 V2 V3

  Face.
  A face is defined by the vertices.
  Node indices are 1 based rather than 0 based.
  So we have to decrement them before loading them into FACE.
  Note that vertex indices start back at 0 each time a BEGIN is entered.
  The strategy here won't handle nested BEGIN's, just one at a time.
*/

    else if ( leqi ( token, "F" ) == TRUE ) {

      ivert = 0;
      face_order[num_face] = 0;
/*
  Read each item in the F definition as a token, and then
  take it apart.
*/
      for ( ;; ) {

        count = sscanf ( next, "%s%n", token2, &width );
        next = next + width;
 
        if ( count != 1 ) {
          break;
        }
 
        count = sscanf ( token2, "%d%n", &node, &width );

        if ( count != 1 ) {
          break;
        }

        if ( ivert < MAX_ORDER && num_face < MAX_FACE ) {
          face[ivert][num_face] = node - 1 + vert_base;
          face_mat[ivert][num_face] = 0;
          face_order[num_face] = face_order[num_face] + 1;
        } 
        ivert = ivert + 1;
      } 
      num_face = num_face + 1;
    }
/*
  ROT
*/
    else if ( leqi ( token, "ROT" ) == TRUE ) {
    }
/*
  TRANS
*/
    else if ( leqi ( token, "TRANS" ) == TRUE ) {
    }
/*
  V X Y Z
  Geometric vertex.
*/

    else if ( leqi ( token, "V" ) == TRUE ) {

      sscanf ( next, "%e %e %e", &r1, &r2, &r3 );

      if ( num_cor3 < MAX_COR3 ) {
        cor3[0][num_cor3] = r1;
        cor3[1][num_cor3] = r2;
        cor3[2][num_cor3] = r3;
      }

      num_cor3 = num_cor3 + 1;

    }
/*
  Unrecognized  
*/
    else {
      num_bad = num_bad + 1;
    }

  }
  return SUCCESS;
}
/******************************************************************************/

int converter::smf_write ( FILE *fileout ) {

/******************************************************************************/

/*
  Purpose:
   
    SMF_WRITE writes an SMF file.

  Example:

    #  magnolia.obj

    v -3.269770 -39.572201 0.876128
    v -3.263720 -39.507999 2.160890
    ...
    v 0.000000 -9.988540 0.000000
    f 8 9 11 10
    f 12 13 15 14
    ...
    f 788 806 774

  Modified:

    19 October 1998

  Author:
 
    John Burkardt
*/
  int   i;
  int   iface;
  int   ivert;
  int   num_text;
/* 
  Initialize. 
*/
  num_text = 0;

  fprintf ( fileout, "#$SMF 1.0\n" );
  fprintf ( fileout, "#$vertices %d\n", num_cor3 );
  fprintf ( fileout, "#$faces %d\n", num_face );
  fprintf ( fileout, "#\n" );
  fprintf ( fileout, "# %s created by IVCON.\n", fileout_name );
  fprintf ( fileout, "# Original data in %s.\n", filein_name );
  fprintf ( fileout, "#\n" );

  num_text = num_text + 7;
/* 
  V: vertex coordinates. 
*/
  for ( i = 0; i < num_cor3; i++ ) {
    fprintf ( fileout, "v %f %f %f\n", 
      cor3[0][i], cor3[1][i], cor3[2][i] );
    num_text = num_text + 1;
  }
/* 
  F: faces. 
*/
  if ( num_face > 0 ) {
    fprintf ( fileout, "\n" );
    num_text = num_text + 1;
  }

  for ( iface = 0; iface < num_face; iface++ ) {

    fprintf ( fileout, "f" );
    for ( ivert = 0; ivert < face_order[iface]; ivert++ ) {
      fprintf ( fileout, " %d", face[ivert][iface]+1 );
    }
    fprintf ( fileout, "\n" );
    num_text = num_text + 1;
  }
/*
  Report.
*/
  fprintf ( logfile,  "\n" );
  fprintf ( logfile,  "SMF_WRITE - Wrote %d text lines.\n", num_text );

  return SUCCESS;
}
