
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

int converter::stla_read ( FILE *filein ) {

/******************************************************************************/

/*
  Purpose:
   
    STLA_READ reads an ASCII STL (stereolithography) file.

  Modified:

    20 October 1998

  Author:
 
    John Burkardt
*/
  int   count;
  int   i;
  int   icor3;
  int   ivert;
  char *next;
  float r1;
  float r2;
  float r3;
  float r4;
  float temp[3];
  char  token[MAX_INCHARS];
  int   width;
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
    if ( *next == '\0' || *next == '#' || *next == '!' || *next == '$' ) {
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
  FACET
*/
    if ( leqi ( token, "facet" ) == TRUE ) {
/* 
  Get the XYZ coordinates of the normal vector to the face. 
*/
      sscanf ( next, "%*s %e %e %e", &r1, &r2, &r3 );  

      if ( num_face < MAX_FACE ) {
        face_normal[0][num_face] = r1;
        face_normal[1][num_face] = r2;
        face_normal[2][num_face] = r3;
      }

      fgets ( input, MAX_INCHARS, filein );
      num_text = num_text + 1;

      ivert = 0;

      for ( ;; ) {

        fgets ( input, MAX_INCHARS, filein );
        num_text = num_text + 1;

        count = sscanf ( input, "%*s %e %e %e", &r1, &r2, &r3 );

        if ( count != 3 ) {
          break;
        }

        temp[0] = r1;
        temp[1] = r2;
        temp[2] = r3;

        if ( num_cor3 < 1000 ) {
          icor3 = rcol_find ( cor3, 3, num_cor3, temp );
        }
        else {
          icor3 = -1;
        }

        if ( icor3 == -1 ) {

          icor3 = num_cor3;

          if ( num_cor3 < MAX_COR3 ) {
            for ( i = 0; i < 3; i++ ) {
              cor3[i][num_cor3] = temp[i];
            }
          }
          num_cor3 = num_cor3 + 1;
        }
        else {
          num_dup = num_dup + 1;
        }

        if ( ivert < MAX_ORDER && num_face < MAX_FACE ) {
          face[ivert][num_face] = icor3;
          face_mat[ivert][num_face] = 0;
          for ( i = 0; i < 3; i++ ) {
            vertex_normal[i][ivert][num_face] = face_normal[i][num_face];
          }
        }

        ivert = ivert + 1;
      } 

      fgets ( input, MAX_INCHARS, filein );
      num_text = num_text + 1;

      if ( num_face < MAX_FACE ) {
        face_order[num_face] = ivert;
      } 

      num_face = num_face + 1;

    }
/*
  COLOR 
*/

    else if ( leqi ( token, "color" ) == TRUE ) {
      sscanf ( next, "%*s %f %f %f %f", &r1, &r2, &r3, &r4 );
    }
/* 
 SOLID 
*/
    else if ( leqi ( token, "solid" ) == TRUE ) {
      num_object = num_object + 1;
    }
/* 
 ENDSOLID 
*/
    else if ( leqi ( token, "endsolid" ) == TRUE ) {
    }
/* 
  Unexpected or unrecognized. 
*/
    else {
      fprintf ( logfile,  "\n" );
      fprintf ( logfile,  "STLA_READ - Error!\n" );
      fprintf ( logfile,  "  Unrecognized first word on line.\n" );
      return ERROR;
    }

  }
  return SUCCESS;
}
/******************************************************************************/

int converter::stla_write ( FILE *fileout ) {

/******************************************************************************/

/*
  Purpose:
   
    STLA_WRITE writes an ASCII STL (stereolithography) file.

  Modified:

    01 September 1998

  Author:
 
    John Burkardt
*/
  int   iface;
  int   ivert;
  int   k;
  int   num_text;
/*
  Initialize.
*/
  num_text = 0;
/* 
  SOLID 
*/
  fprintf ( fileout, "solid MYSOLID created by IVCON, original data in %s\n", 
    filein_name );

  num_text = num_text + 1;
/* 
  FACET NORMAL x y z
     OUTER LOOP
       VERTEX x y z
       ...
       VERTEX x y z
     ENDLOOP
   ENDFACET
*/
  for ( iface = 0; iface < num_face; iface++ ) {

    fprintf ( fileout, "  facet normal %f %f %f\n", 
      face_normal[0][iface], face_normal[1][iface], face_normal[2][iface] );

    fprintf ( fileout, "    outer loop\n" );
    num_text = num_text + 2;

    for ( ivert = 0; ivert < face_order[iface]; ivert++ ) {

      k = face[ivert][iface];

      fprintf ( fileout, "      vertex %f %f %f\n", 
        cor3[0][k], cor3[1][k], cor3[2][k] );
      num_text = num_text + 1;
    }

    fprintf ( fileout, "    endloop\n" );
    fprintf ( fileout, "  endfacet\n" );
    num_text = num_text + 2;
  }
/* 
  ENDSOLID 
*/
  fprintf ( fileout, "endsolid MYSOLID\n" );
  num_text = num_text + 1;
/*
  Report.
*/
  fprintf ( logfile,  "\n" );
  fprintf ( logfile,  "STLA_WRITE - Wrote %d text lines.\n", num_text );

  return SUCCESS;
}
