

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

int converter::vla_read ( FILE *filein ) {

/******************************************************************************/

/*
  Purpose:
   
    VLA_READ reads a VLA file.

  Modified:

    24 July 1998

  Author:
 
    John Burkardt
*/
  int   i;
  int   icor3;
  int   num_dup;
  char *next;
  int   num_text;
  float r1;
  float r2;
  float r3;
  float temp[3];
  char  token[MAX_INCHARS];
  int   width;
/*
  Initialize. 
*/
  num_cor3 = 0;
  num_dup = 0;
  num_face = 0;
  num_line = 0;
  num_text = 0;
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
    if ( *next == '\0' || *next == ';' ) {
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
  SET (ignore) 
*/
    if ( leqi ( token, "set" ) == TRUE ) {
    }
/* 
  P (begin a line)
  L (continue a line) 
*/
    else if ( leqi ( token, "P" ) == TRUE || leqi ( token, "L") == TRUE ) {

      if ( leqi ( token, "P" ) == TRUE ) {
        if ( num_line > 0 ) {
          if ( num_line < MAX_LINE ) {
            line_dex[num_line] = -1;
            line_mat[num_line] = -1;
            num_line = num_line + 1;
          }
        }
      }

      sscanf ( next, "%e %e %e", &r1, &r2, &r3 );

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

      if ( num_line < MAX_LINE ) {
        line_dex[num_line] = icor3;
        line_mat[num_line] = 0;
        num_line = num_line + 1;
      }
    }
/* 
  Unexpected or unrecognized. 
*/
    else {
      fprintf ( logfile,  "\n" );
      fprintf ( logfile,  "VLA_READ - Error!\n" );
      fprintf ( logfile,  "  Unrecognized first word on line.\n" );
      return ERROR;
    }

  }

  if ( num_line > 0 ) {
    if ( num_line < MAX_LINE ) {
      line_dex[num_line] = -1;
      line_mat[num_line] = -1;
      num_line = num_line + 1;
    }
  }
/*
  Report.
*/
  fprintf ( logfile,  "\n" );
  fprintf ( logfile,  "VLA_READ - Input file statistics:\n" );
  fprintf ( logfile,  "  %d lines of text;\n", num_text );
  fprintf ( logfile,  "  %d faces;\n", num_face );
  fprintf ( logfile,  "  %d lines;\n", num_line );
  fprintf ( logfile,  "  %d points;\n", num_cor3 );
  fprintf ( logfile,  "  %d duplicate points.\n", num_dup );

  return SUCCESS;
}
/******************************************************************************/

int converter::vla_write ( FILE *fileout ) {

/******************************************************************************/

/*
  Purpose:
   
    VLA_WRITE writes a VLA file.

  Modified:

    24 July 1998

  Author:
 
    John Burkardt
*/
  char  c;
  int   iline;
  int   k;
  int   num_text;
/* 
  Initialize. 
*/
  num_text = 0;

  fprintf ( fileout, "set comment %s created by IVCON.\n", fileout_name );
  fprintf ( fileout, "set comment Original data in %s.\n", filein_name );
  fprintf ( fileout, "set comment\n" );
  fprintf ( fileout, "set intensity EXPLICIT\n" );
  fprintf ( fileout, "set parametric NON_PARAMETRIC\n" );
  fprintf ( fileout, "set filecontent LINES\n" );
  fprintf ( fileout, "set filetype converternew\n" );
  fprintf ( fileout, "set depthcue 0\n" );
  fprintf ( fileout, "set defaultdraw stellar\n" );
  fprintf ( fileout, "set coordsys RIGHT\n" );
  fprintf ( fileout, "set author IVCON\n" );
  fprintf ( fileout, "set site Buhl Planetarium\n" );
  fprintf ( fileout, "set library_id UNKNOWN\n" );

  num_text = num_text + 13;

  c = 'P';

  for ( iline = 0; iline < num_line; iline++ ) {
    
    k = line_dex[iline];

    if ( k == -1 ) {

      c = 'P';
    }
    else {

      fprintf ( fileout, "%c %f %f %f 1.00\n", 
        c, cor3[0][k], cor3[1][k], cor3[2][k] );

      num_text = num_text + 1;

      c = 'L';
    }
  }
/*
  Report.
*/
  fprintf ( logfile,  "\n" );
  fprintf ( logfile,  "VLA_WRITE - Wrote %d text lines.\n", num_text );


  return SUCCESS;
}
