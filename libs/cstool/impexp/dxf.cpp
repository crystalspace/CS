
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

int converter::dxf_read ( FILE *filein ) {

/******************************************************************************/

/*
  Purpose:
   
    DXF_READ reads an AutoCAD DXF file.

  Examples:

      0
    SECTION
      2
    ENTITIES
      0
    LINE
      8
    0
     10
      (X coordinate of beginning of first line.)
     20
      (Y coordinate of beginning of first line.)
     30
      (Z coordinate of beginning of first line.)
     11
      (X coordinate of end of first line.)
     21
      (Y coordinate of end of first line.)
     31
      (Z coordinate of end of first line.)
      0
    LINE
      8
    0
     10
      (X coordinate of beginning of second line.)
     20
      ...etc...
     31
     (Z coordinate of end of last line.)
      0
    ENDSEC
      0
    EOF

  Modified:

    27 April 1999

  Author:
 
    John Burkardt
*/
  int   code;
  int   count;
  float cvec[3];
  int   icor3;
  char  input1[MAX_INCHARS];
  char  input2[MAX_INCHARS];
  float rval;
  int   width;
/* 
  Read the next two lines of the file into INPUT1 and INPUT2. 
*/

  while ( TRUE ) {

/* 
  INPUT1 should contain a single integer, which tells what INPUT2
  will contain.
*/
    if ( fgets ( input1, MAX_INCHARS, filein ) == NULL ) {
      break;
    }

    num_text = num_text + 1;

    count = sscanf ( input1, "%d%n", &code, &width );
    if ( count <= 0 ) {
      break;
    }
/*
  Read the second line, and interpret it according to the code.
*/
    if ( fgets ( input2, MAX_INCHARS, filein ) == NULL ) {
      break;
    }

    num_text = num_text + 1;

    if ( code == 10 || code == 20 || code == 30 ||
         code == 11 || code == 21 || code == 31 ) {

      count = sscanf ( input2, "%e%n", &rval, &width );

      if ( code == 10 && num_line > 0 ) {
        line_dex[num_line] = - 1;
        line_mat[num_line] = - 1;
        num_line = num_line + 1;
      }

      if ( code == 10 || code == 11 ) {
        cvec[0] = rval;
      }
      else if ( code == 20 || code == 21 ) {
        cvec[1] = rval;
      }
      else if ( code == 30 || code == 31 ) {

        cvec[2] = rval;

        if ( num_cor3 < 1000 ) {
          icor3 = rcol_find ( cor3, 3, num_cor3, cvec );
        }
        else {
          icor3 = -1;
        }

        if ( icor3 == -1 ) {
          icor3 = num_cor3;
          cor3[0][num_cor3] = cvec[0];
          cor3[1][num_cor3] = cvec[1];
          cor3[2][num_cor3] = cvec[2];
          num_cor3 = num_cor3 + 1;
        }
        else {
          num_dup = num_dup + 1;
        }

        line_dex[num_line] = icor3;
        line_mat[num_line] = 0;
        num_line = num_line + 1;
      }

    }

  }

  if ( num_line > 0 ) {
    line_dex[num_line] = - 1;
    line_mat[num_line] = - 1;
    num_line = num_line + 1;
  }
  return SUCCESS;
}
/******************************************************************************/

int converter::dxf_write ( FILE *fileout ) {

/******************************************************************************/

/*
  Purpose:
   
    DXF_WRITE writes graphics information to an AutoCAD DXF file.

  Modified:

    18 August 1998

  Author:
 
    John Burkardt
*/
  int   iline;
  int   k;
  int   kold;
  int   converternewline;
  int   num_text;

/* 
  Initialize. 
*/
  num_text = 0;

  fprintf ( fileout, "  0\n" );
  fprintf ( fileout, "SECTION\n" );
  fprintf ( fileout, "  2\n" );
  fprintf ( fileout, "HEADER\n" );
  fprintf ( fileout, "999\n" );
  fprintf ( fileout, "%s created by IVCON.\n", fileout_name );
  fprintf ( fileout, "999\n" );
  fprintf ( fileout, "Original data in %s.\n", filein_name );
  fprintf ( fileout, "  0\n" );
  fprintf ( fileout, "ENDSEC\n" );
  num_text = num_text + 8;

  fprintf ( fileout, "  0\n" );
  fprintf ( fileout, "SECTION\n" );
  fprintf ( fileout, "  2\n" );
  fprintf ( fileout, "TABLES\n" );
  fprintf ( fileout, "  0\n" );
  fprintf ( fileout, "ENDSEC\n" );
  num_text = num_text + 4;

  fprintf ( fileout, "  0\n" );
  fprintf ( fileout, "SECTION\n" );
  fprintf ( fileout, "  2\n" );
  fprintf ( fileout, "BLOCKS\n" );
  fprintf ( fileout, "  0\n" );
  fprintf ( fileout, "ENDSEC\n" );
  num_text = num_text + 8;

  fprintf ( fileout, "  0\n" );
  fprintf ( fileout, "SECTION\n" );
  fprintf ( fileout, "  2\n" );
  fprintf ( fileout, "ENTITIES\n" );
  num_text = num_text + 8;

  kold = 0;
  converternewline = TRUE;

  for ( iline = 0; iline < num_line; iline++ ) {
    
    k = line_dex[iline];

    if ( k == -1 ) {

      converternewline = TRUE;
    }
    else {

      if ( converternewline == FALSE ) {

        fprintf ( fileout, "  0\n" );
        fprintf ( fileout, "LINE\n" );
        fprintf ( fileout, "  8\n" );
        fprintf ( fileout, "  0\n" );
        fprintf ( fileout, " 10\n" );
        fprintf ( fileout, "%f\n", cor3[0][kold] );
        fprintf ( fileout, " 20\n" );
        fprintf ( fileout, "%f\n", cor3[1][kold] );
        fprintf ( fileout, " 30\n" );
        fprintf ( fileout, "%f\n", cor3[2][kold] );
        fprintf ( fileout, " 11\n" );
        fprintf ( fileout, "%f\n", cor3[0][k] );
        fprintf ( fileout, " 21\n" );
        fprintf ( fileout, "%f\n", cor3[1][k] );
        fprintf ( fileout, " 31\n" );
        fprintf ( fileout, "%f\n", cor3[2][k] );

        num_text = num_text + 16;

      }

      kold = k;
      converternewline = FALSE;

    }
  }

  fprintf ( fileout, "  0\n" );
  fprintf ( fileout, "ENDSEC\n" );
  fprintf ( fileout, "  0\n" );
  fprintf ( fileout, "EOF\n" );
  num_text = num_text + 4;
/*
  Report.
*/
  fprintf ( logfile,  "\n" );
  fprintf ( logfile,  "DXF_WRITE - Wrote %d text lines.\n", num_text );

  return SUCCESS;
}

