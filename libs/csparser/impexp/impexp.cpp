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
#include "csparser/impexp.h"

// converter.cpp: implementation of the converter class.
//
//////////////////////////////////////////////////////////////////////

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdarg.h>

#include "csengine/thing.h"
#include "csengine/cssprite.h"
#include "csutil/inifile.h"
#include "csutil/csstring.h"
#include "cssys/csendian.h"
#include "ivfs.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

converter::converter() : revnorm(FALSE), frame_builder(NULL)
{
}

converter::~converter()
{
  delete frame_builder;
}

void converter::ProcessConfig (iConfigFile* config)
{
  if ( config->SectionExists ("converter"))
  {
  }
}

int converter::set_animation_frame(int frame_number)
{
  // no frame management? then there are no frames!
  if (frame_builder == NULL)
  {
    return 0;
  }

  // use the frame_builder to set a frame, and figure out the
  // biggest frame number
  return frame_builder->SetFrame(frame_number);
}

void converter::set_reverse_normals (int flag)
{
  revnorm = flag;
}


/******************************************************************************/

int converter::ivcon ( const char* input_filename, bool keep_log,
  bool create_output_file, const char* output_filename, iVFS * vfs) {

/******************************************************************************/

  int result = ERROR;

  if (keep_log)
    logfile = fopen("ivcon.log","a");
  else
    logfile = stdout;

  init_program_data ();
   
/*
  If using VFS, then copy file from VFS into a temporary "real" file.
  @@@ FIXME This is an awful hack. Convert entirely to VFS as soon as possible.
*/
  if (!vfs)
    result = comline (input_filename, create_output_file, output_filename);
  else
  {
    iDataBuffer* buf = vfs->ReadFile (input_filename);
    if (buf == NULL)
    {
      fprintf (logfile, "\n");
      fprintf (logfile, "VFS_CONVERSION - Error!\n");
      fprintf (logfile, "  Can't read file %s\n", input_filename);
    }
    else 
    {
      csString tmp_file ("convtemp.");
      tmp_file += file_ext ((char*)input_filename);
      csString tmp_path ("/this/");
      tmp_path += tmp_file;
      if (vfs->WriteFile (tmp_path, **buf, buf->GetSize ()))
      {
        result = comline (tmp_file, create_output_file, output_filename);
        vfs->DeleteFile (tmp_path);
      }
      else
      {
        fprintf (logfile, "\n");
        fprintf (logfile, "VFS_CONVERSION - Error!\n");
        fprintf (logfile, "  Can't write temporary file /this/convtemp.%s\n",
          file_ext ((char*)input_filename));
      }
      buf->DecRef ();
    }
  }

  if (keep_log)
    fclose(logfile);

  return result;
}


/******************************************************************************/

void converter::ave_face_normal ( ) {

/******************************************************************************/

/*
  Purpose:

    AVE_FACE_NORMAL sets face normals as average of face vertex normals.

  Modified:

    09 October 1998

  Author:

    John Burkardt
*/
  int   i;
  int   iface;
  int   ivert;
  int   nfix;
  float norm;
  float x;
  float y;
  float z;

  if ( num_face <= 0 ) {
    return;
  }
  
  nfix = 0;

  for ( iface = 0; iface < num_face; iface++ ) {

/*
  Check the norm of the current normal vector.
*/
    x = face_normal[0][iface];
    y = face_normal[1][iface];
    z = face_normal[2][iface];
    norm = ( float ) sqrt ( x*x + y*y + z*z );

    if ( norm == 0.0 ) {

      nfix = nfix + 1;

      for ( i = 0; i < 3; i++ ) {
        face_normal[i][iface] = 0.0;
      }

      for ( ivert = 0; ivert < face_order[iface]; ivert++ ) {
        for ( i = 0; i < 3; i++ ) {
          face_normal[i][iface] = face_normal[i][iface] +
            vertex_normal[i][ivert][iface];
        }
      }

      x = face_normal[0][iface];
      y = face_normal[1][iface];
      z = face_normal[2][iface];
      norm = ( float ) sqrt ( x*x + y*y + z*z );

      if ( norm == 0.0 ) {
        for ( i = 0; i < 3; i++ ) {
          face_normal[i][iface] = ( float ) 1.0 / sqrt ( 3.0 );
        }
      }
      else {
        for ( i = 0; i < 3; i++ ) {
          face_normal[i][iface] = face_normal[i][iface] / norm;
        }
      }
    }
  }

  if ( nfix > 0 ) {
    fprintf ( logfile,  "\n" );
    fprintf ( logfile,  "AVE_FACE_NORMAL: Recomputed %d face normals\n", nfix );
    fprintf ( logfile,  "  by averaging face vertex normals.\n" );
  }
  return;
}

/******************************************************************************/

int converter::char_index_last ( char* string, char c ) {

/******************************************************************************/

/*
  Purpose:

    CHAR_INDEX_LAST reports the last occurrence of a character in a string.

  Author:
 
    John Burkardt
*/
  int i;
  int j;
  int nchar;

  j = -1;

  nchar = strlen ( string );

  for ( i = 0; i < nchar; i++ ) {
    if ( string[i] == c ) {
      j = i;
    }
  }

  return j;

}
/******************************************************************************/

int converter::char_pad ( int *char_index, int *null_index, char *string, 
  int MAX_STRING ) {

/******************************************************************************/

/*
  Purpose:

    CHAR_PAD "pads" a character in a string with a blank on either side.

  Modified:

    16 October 1998

  Author:

    John Burkardt

  Parameters:

    Input/output, int *CHAR_INDEX, the position of the character to be padded.
    On output, this is increased by 1.

    Input/output, int *NULL_INDEX, the position of the terminating NULL in 
    the string.  On output, this is increased by 2.

    Input/output, char STRING[MAX_STRING], the string to be manipulated.

    Input, int MAX_STRING, the maximum number of characters that can be stored
    in the string.

    Output, int CHAR_PAD, is SUCCESS if the operation worked, and ERROR otherwise.
*/

  int i;

  if ( *char_index < 0 || *char_index >= *null_index || *char_index > MAX_STRING-1 ) {
    return ERROR;
  }

  if ( (*null_index) + 2 > MAX_STRING-1 ) {
    return ERROR;
  }

  for ( i = *null_index + 2; i > *char_index + 2; i-- ) {
    string[i] = string[i-2];
  }
  string[*char_index+2] = ' ';
  string[*char_index+1] = string[*char_index];
  string[*char_index] = ' ';

  *char_index = *char_index + 1;
  *null_index = *null_index + 2;

  return SUCCESS;
}
/******************************************************************************/

int converter::comline ( const char* input_filename, bool create_output_file,
  const char* output_filename ) {

/******************************************************************************/

/*

  Purpose:

    COMLINE carries out a command-line session of file conversion.

  Modified:

    18 November 1998

  Author:
 
    John Burkardt
*/

/* 
  Initialize the data. 
*/
  iarg = 0;
  ierror = 0;

  data_init ( );

  strcpy (filein_name, input_filename);

  if (!create_output_file)
    strcpy(fileout_name,"{none}");
  else if (output_filename)
    strcpy(fileout_name,output_filename);
  else
    strcpy(fileout_name,"iconv.txt");



/*
  Read the input. 
*/
  ierror = data_read ( );

  if ( ierror == ERROR ) {
    fprintf ( logfile,  "\n" );
    fprintf ( logfile,  "COMLINE - Error!\n" );
    fprintf ( logfile,  "  Failure while reading input data.\n" );
    return ERROR;
  }
/*
  Reverse the normal vectors if requested.
*/
  if ( revnorm ) {

    for ( icor3 = 0; icor3 < num_cor3; icor3++ ) {
      for ( i = 0; i < 3; i++ ) {
        cor3_normal[i][icor3] = - cor3_normal[i][icor3];
      }
    }

    for ( iface = 0; iface < num_face; iface++ ) {
      for ( i = 0; i < 3; i++ ) {
        face_normal[i][iface] = - face_normal[i][iface];
      }
    }

    for ( iface = 0; iface < num_face; iface++ ) {
      for ( ivert = 0; ivert < face_order[iface]; ivert++ ) {
        for ( i = 0; i < 3; i++ ) {
          vertex_normal[i][ivert][iface] = 
            - vertex_normal[i][ivert][iface];
        }
      }
    }
    fprintf ( logfile,  "\n" );
    fprintf ( logfile,  "INTERACT - Note:\n" );
    fprintf ( logfile,  "  Reversed node, face, and vertex normals.\n" );
  }
/*
  Write the output. 
*/

  if (create_output_file) {
    ierror = data_write ( );

    if ( ierror == ERROR ) {
      fprintf ( logfile,  "\n" );
      fprintf ( logfile,  "COMLINE - Error!\n" );
      fprintf ( logfile,  "  Failure while writing output data.\n" );
      return ERROR;
    }
  }
  return SUCCESS;
}
/**********************************************************************/

void converter::cor3_2_vertex_rgb ( void ) {

/**********************************************************************/

/*
  Purpose:

    COR3_RGB converts node RGB data to vertex RGB data.

  Modified:

    19 November 1998

  Author:

    John Burkardt
*/
  int icor3;
  int iface;
  int ivert;
  int j;

  for ( iface = 0; iface < num_face; iface ++ ) {
    for ( ivert = 0; ivert < face_order[iface]; ivert++ ) {
      icor3 = face[ivert][iface];
      for ( j = 0; j < 3; j++ ) {
        vertex_rgb[j][ivert][iface] = cor3_rgb[j][icor3];
      }
    }
  }

  return;
}
/******************************************************************************/

void converter::data_check ( void ) {

/******************************************************************************/

/*
  Purpose:
   
    DATA_CHECK checks the input data.

  Modified:

    09 October 1998

  Author:
 
    John Burkardt
*/
  int iface;
  int nfix;

  if ( num_color > MAX_COLOR ) {
    fprintf ( logfile,  "\n" );
    fprintf ( logfile,  "DATA_CHECK: Warning!\n" );
    fprintf ( logfile,  "  The input data requires %d colors.\n", num_color );
    fprintf ( logfile,  "  There was only room for %d\n", MAX_COLOR );
    num_color = MAX_COLOR;
  }
  
  if ( num_cor3 > MAX_COR3 ) {
    fprintf ( logfile,  "\n" );
    fprintf ( logfile,  "DATA_CHECK: Warning!\n" );
    fprintf ( logfile,  "  The input data requires %d points.\n", num_cor3 );
    fprintf ( logfile,  "  There was only room for %d\n", MAX_COR3 );
    num_cor3 = MAX_COR3;
  }

  if ( num_face > MAX_FACE ) {
    fprintf ( logfile,  "\n" );
    fprintf ( logfile,  "DATA_CHECK: Warning!\n" );
    fprintf ( logfile,  "  The input data requires %d faces.\n", num_face );
    fprintf ( logfile,  "  There was only room for %d\n", MAX_FACE );
    num_face = MAX_FACE;
  }

  if ( num_line > MAX_LINE ) {
    fprintf ( logfile,  "\n" );
    fprintf ( logfile,  "DATA_CHECK - Warning!\n" );
    fprintf ( logfile,  "  The input data requires %d line items.\n", num_line );
    fprintf ( logfile,  "  There was only room for %d.\n", MAX_LINE );
    num_line = MAX_LINE;
  }

  nfix = 0;

  for ( iface = 0; iface < num_face; iface++ ) {

    if ( face_order[iface] > MAX_ORDER ) {
      face_order[iface] = MAX_ORDER;
      nfix = nfix + 1;
    }

  }

  if ( nfix > 0 ) {
    fprintf ( logfile,  "\n" );
    fprintf ( logfile,  "DATA_CHECK - Warning!\n" );
    fprintf ( logfile,  "  Corrected %d faces using more than %d vertices per face.\n",
      nfix, MAX_ORDER );
  }
  return;
}
/******************************************************************************/

void converter::data_init ( ) {

/******************************************************************************/

/*
  Purpose:

    DATA_INIT initializes the internal graphics data.

  Modified:

    02 December 1998

  Author:

    John Burkardt
*/

  int i;
  int iface;
  int ivert;
  int j;
  int k;

  for ( i = 0; i < 3; i++ ) {
    for ( j = 0; j < MAX_COR3; j++ ) {
      cor3[i][j] = 0.0;
      temp_cor3[i][j] = 0.0;
    }
  }

  for ( i = 0; i < 2; i++ ) {
    for ( j = 0; j < MAX_COR3; j++ ) {
      cor3_uv[i][j] = 0.0;
      temp_cor3_uv[i][j] = 0.0;
    }
  }

  for ( i = 0; i < 3; i++ ) {
    for ( j = 0; j < MAX_COR3; j++ ) {
      cor3_normal[i][j] = 0.0;
    }
  }

  for ( j = 0; j < MAX_COR3; j++ ) {
    cor3_rgb[0][j] = 0.299;
    cor3_rgb[1][j] = 0.587;
    cor3_rgb[2][j] = 0.114;
  }

  for ( iface = 0; iface < MAX_FACE; iface++ ) {
    for ( ivert = 0; ivert < MAX_ORDER; ivert++ ) {
      face[ivert][iface] = 0;
    }
  }

  for ( iface = 0; iface < MAX_FACE; iface++ ) {
    face_flags[iface] = 6;
  }

  for ( iface = 0; iface < MAX_FACE; iface++ ) {
    for ( ivert = 0; ivert < MAX_ORDER; ivert++ ) {
      face_mat[ivert][iface] = 0;
      face_texnode[ivert][iface] = -1;
    }
  }

  for ( iface = 0; iface < MAX_FACE; iface++ ) {
    for ( i = 0; i < 3; i++ ) {
      face_normal[i][iface] = 0;
    }
  }

  for ( iface = 0; iface < MAX_FACE; iface++ ) {
    face_object[iface] = -1;
  }

  for ( iface = 0; iface < MAX_FACE; iface++ ) {
    face_order[iface] = 0;
  }

  for ( iface = 0; iface < MAX_FACE; iface++ ) {
    face_smooth[iface] = 1;
  }

  for ( i = 0; i < MAX_LINE; i++ ) {
    line_dex[i] = -1;
  }

  for ( i = 0; i < MAX_LINE; i++ ) {
    line_mat[i] = 0;
  }

  for ( j = 0; j < MAX_ORDER*MAX_FACE; j++ ) {
    for ( i = 0; i < 3; i++ ) {
      normal_temp[i][j] = 0;
    }
  }

  num_color = 0;
  num_cor3 = 0;
  num_cor3_uv = 0;
  temp_num_cor3 = 0;
  temp_num_cor3_uv = 0;
  num_face = 0;
  num_line = 0;
  num_texmap = 0;

  strcpy ( object_name, "IVCON" );

  for ( i = 0; i < 3; i++ ) {
    origin[i] = 0.0;
  }

  for ( i = 0; i < 3; i++ ) {
    pivot[i] = 0.0;
  }

  for ( j = 0; j < MAX_COLOR; j++ ) {
    rgbcolor[0][j] = 0.299;
    rgbcolor[1][j] = 0.587;
    rgbcolor[2][j] = 0.114;
  }

  for ( j = 0; j < MAX_TEXMAP; j++ ) {
    strcpy ( texmap_name[j], "null" );
  }

  for ( i = 0; i < 4; i++ ) {
    for ( j = 0; j < 4; j++ ) {
      if ( i == j ) {
        transform_mat[i][j] = 1.0;
      }
      else {
        transform_mat[i][j] = 0.0;
      }
    }
  }

  for ( iface = 0; iface < MAX_FACE; iface++ ) {
    for ( ivert = 0; ivert < MAX_ORDER; ivert++ ) {
      for ( i = 0; i < 3; i++ ) {
        vertex_normal[i][ivert][iface] = 0.0;
      }
    }
  }

  for ( iface = 0; iface < MAX_FACE; iface++ ) {
    for ( ivert = 0; ivert < MAX_ORDER; ivert++ ) {
      for ( i = 0; i < 2; i++ ) {
        vertex_uv[i][ivert][iface] = 0.0;
      }
    }
  }

  for ( j = 0; j < 3; j++ ) {
    for ( k = 0; k < MAX_FACE; k++ ) {
      vertex_rgb[0][j][k] = 0.299;
      vertex_rgb[1][j][k] = 0.587;
      vertex_rgb[2][j][k] = 0.114;
    }
  }

  if ( debug == TRUE ) {
    fprintf ( logfile,  "\n" );
    fprintf ( logfile,  "DATA_INIT: Graphics data initialized.\n" );
  }

  return;
}
/******************************************************************************/

int converter::data_read ( void ) {

/******************************************************************************/

/*
  Purpose:

    DATA_READ reads a file into internal graphics data.

  Modified:

    19 November 1998

  Author:
 
    John Burkardt
*/
  FILE *filein;
  char *filein_type;
  int   ierror;
  int   ntemp;
/* 
  Retrieve the input file type. 
*/
  filein_type = file_ext ( filein_name );

  if ( filein_type == NULL ) {
    fprintf ( logfile,  "\n" );
    fprintf ( logfile,  "DATA_READ - Error!\n" );
    fprintf ( logfile,  "  Could not determine the type of '%s'.\n", filein_name );
    return ERROR;
  }
  else if ( debug ) {
    fprintf ( logfile,  "\n" );
    fprintf ( logfile,  "DATA_READ: Input file has type %s.\n", filein_type );  
  }

/* 
  Open the file. 
*/
  if ( (leqi ( filein_type, "3DS" ) == TRUE ) ||
       (leqi ( filein_type, "MD2" ) == TRUE ) ||
       (leqi ( filein_type, "MDL" ) == TRUE ) )
  {
    filein = fopen ( filein_name, "rb" );
  }
  else {
    filein = fopen ( filein_name, "r" );
  }

  if ( filein == NULL ) {
    fprintf ( logfile,  "\n" );
    fprintf ( logfile,  "DATA_READ - Error!\n" );
    fprintf ( logfile,  "  Could not open the input file '%s'!\n", filein_name );
    return ERROR;
  }
/*
  Initialize some data.
*/
  max_order2 = 0;
  num_bad = 0;
  num_color = 0;
  num_comment = 0;
  num_cor3 = 0;
  num_dup = 0;
  num_face = 0;
  num_group = 0;
  num_line = 0;
  num_object = 0;
  num_text = 0;
/*
  Read the input file. 
*/
  if ( leqi ( filein_type, "3DS" ) == TRUE ) {
    ierror = tds_read ( filein );
  }
  else if ( leqi ( filein_type, "ASE" ) == TRUE ) {
    ierror = ase_read ( filein );
    cor3_2_vertex_rgb ( );
  }
  else if ( leqi ( filein_type, "DXF" ) == TRUE ) {
    ierror = dxf_read ( filein );
  }
  else if ( leqi ( filein_type, "HRC" ) == TRUE ) {
    ierror = hrc_read ( filein );
  }
  else if ( leqi ( filein_type, "IV" ) == TRUE ) {
    ierror = iv_read ( filein );
  }
  else if ( leqi ( filein_type, "OBJ" ) == TRUE ) {
    ierror = obj_read ( filein );
  }
  else if ( leqi ( filein_type, "SMF" ) == TRUE ) {
    ierror = smf_read ( filein );
  }
  else if ( 
    leqi ( filein_type, "STL" ) == TRUE ||
    leqi ( filein_type, "STLA") == TRUE ) {
    ierror = stla_read ( filein );
  }
  else if ( leqi ( filein_type, "VLA" ) == TRUE ) {
    ierror = vla_read ( filein );
  }
  else if ( leqi ( filein_type, "MD2" ) == TRUE) {
    // make sure libs/csutil/md2/md2.cpp is linked in or this
    // will give the linker problems!
    ierror = md2_read ( filein );
  }
  else if ( leqi ( filein_type, "MDL" ) == TRUE) {
    ierror = mdl_read ( filein );
  }
  else {
    fprintf ( logfile,  "\n" );
    fprintf ( logfile,  "DATA_READ - Error!\n" );
    fprintf ( logfile,  "  Unacceptable input file type.\n" );
    return ERROR;
  }

  fclose ( filein );
/*
  Report on what we read.
*/
  ntemp = MIN ( num_face, MAX_FACE );
  max_order2 = ivec_max (ntemp, face_order );

  data_report ( );
/*
  Warn about any errors that occurred during reading.
*/
  if ( ierror == ERROR ) {
    fprintf ( logfile,  "\n" );
    fprintf ( logfile,  "DATA_READ - Error!\n" );
    fprintf ( logfile,  "  An error occurred while reading the input file.\n" );
    return ERROR;
  }
/*
  Check the data immediately.
*/
  data_check ( );
/*
  Recompute zero face-vertex normals from vertex positions.
*/
  set_vertex_normal ( );
/*
  Compute the node normals from the vertex normals.
*/
  set_cor3_normal ( );
/*
  Recompute zero face normals by averaging face-vertex normals.
*/
  ave_face_normal ( );
/*
  Report on the data.
*/
  minmax ( );

  return SUCCESS;
}
/**********************************************************************/

void converter::data_report ( void ) {

/**********************************************************************/

/*
  Purpose:

    DATA_REPORT gives a summary of the contents of the data file.

  Modified:

    20 October 1998

  Author:

    John Burkardt
*/

  fprintf ( logfile,  "\n" );
  fprintf ( logfile,  "CrystalSpace IVCON Import/Export Log\n\n" );
  fprintf ( logfile,  "Input file: %s\n",filein_name);
  fprintf ( logfile,  "Output file: %s\n",fileout_name);
  fprintf ( logfile,  "DATA_REPORT - The input file contains:\n" );
  fprintf ( logfile,  "\n" );
  fprintf ( logfile,  "  Bad data items             %d\n", num_bad );
  fprintf ( logfile,  "  Text lines                 %d\n", num_text );
  fprintf ( logfile,  "  Colors                     %d\n", num_color );
  fprintf ( logfile,  "  Comments                   %d\n", num_comment );
  fprintf ( logfile,  "  Duplicate points           %d\n", num_dup );
  fprintf ( logfile,  "  Faces                      %d\n", num_face );
  fprintf ( logfile,  "  Groups                     %d\n", num_group );
  fprintf ( logfile,  "  Vertices per face, maximum %d\n", max_order2 );
  fprintf ( logfile,  "  Line items                 %d\n", num_line );
  fprintf ( logfile,  "  Points                     %d\n", num_cor3 );
  fprintf ( logfile,  "  Objects                    %d\n", num_object );

  return;
}
/******************************************************************************/

int converter::data_write ( void ) {

/******************************************************************************/

/*
  Purpose:

    DATA_WRITE writes the graphics data to the output file.

  Modified:

    09 October 1998

  Author:
 
    John Burkardt
*/
  FILE *fileout;
  char *fileout_type;
  int   result;

  result = SUCCESS;
/* 
  Retrieve the output file type. 
*/
  fileout_type = file_ext ( fileout_name );

  if ( fileout_type == NULL ) {
    fprintf ( logfile,  "\n" );
    fprintf ( logfile,  "DATA_WRITE - Error!\n" );
    fprintf ( logfile,  "  Could not determine the output file type.\n" );
    return ERROR;
  }
/* 
  Open the output file. 
*/
  if ( leqi ( fileout_type, "3DS" ) == TRUE ) {
    fileout = fopen ( fileout_name, "wb" );
  }
  else {
    fileout = fopen ( fileout_name, "w" );
  }

  if ( fileout == NULL ) {
    fprintf ( logfile,  "\n" );
    fprintf ( logfile,  "DATA_WRITE - Error!\n" );
    fprintf ( logfile,  "  Could not open the output file!\n" );
    return ERROR;
  }
/* 
  Write the output file. 
*/
  if ( leqi ( fileout_type, "3DS" ) == TRUE ) {
    tds_pre_process();
    result = tds_write ( fileout );
  }
  else if ( leqi ( fileout_type, "ASE" ) == TRUE ) {
    result = ase_write ( fileout );
  }
  else if ( leqi ( fileout_type, "DXF" ) == TRUE ) {
    result = dxf_write ( fileout );
  }
  else if ( leqi ( fileout_type, "HRC" ) == TRUE ) {
    result = hrc_write ( fileout );
  }
  else if ( leqi ( fileout_type, "IV" ) == TRUE ) {
    result = iv_write ( fileout );
  }
  else if ( leqi ( fileout_type, "OBJ" ) == TRUE ) {
    result = obj_write ( fileout );
  }
  else if ( leqi ( fileout_type, "POV" ) == TRUE ) {
    result = pov_write ( fileout );
  }
  else if ( leqi ( fileout_type, "SMF" ) == TRUE ) {
    result = smf_write ( fileout );
  }
  else if ( 
    leqi ( fileout_type, "STL" ) == TRUE ||
    leqi ( fileout_type, "STLA" ) == TRUE ) {
    result = stla_write ( fileout );
  }
  else if ( leqi ( fileout_type, "TXT" ) == TRUE ) {
    result = txt_write ( fileout );
  }
  else if ( leqi ( fileout_type, "VLA" ) == TRUE ) {
    result = vla_write ( fileout );
  }
  else {
    result = ERROR;
    fprintf ( logfile,  "\n" );
    fprintf ( logfile,  "DATA_WRITE - Error!\n" );
    fprintf ( logfile,  "  Unacceptable output file type \"%s\".\n", fileout_type );
  }
/*  
  Close the output file. 
*/
  fclose ( fileout );

  if ( result == ERROR ) {
    return ERROR;
  }
  else {
    return SUCCESS;
  }
}

/******************************************************************************/

int converter::face_print ( int iface ) {

/******************************************************************************/

/*
  Purpose:

    FACE_PRINT prints out information about a face.

  Modified:

    31 August 1998

  Author:

    John Burkardt
*/
  int ivert;
  int j;
  int k;

  if ( iface < 0 || iface > num_face-1 ) {
    fprintf ( logfile,  "\n" );
    fprintf ( logfile,  "FACE_PRINT - Error!\n" );
    fprintf ( logfile,  "  Face indices must be between 1 and %d\n", num_face );
    fprintf ( logfile,  "  But your requested value was %d\n", iface );
    return ERROR;
  }

  fprintf ( logfile,  "\n" );
  fprintf ( logfile,  "FACE_PRINT\n" );
  fprintf ( logfile,  "  Information about face %d\n", iface );
  fprintf ( logfile,  "\n" );
  fprintf ( logfile,  "  Number of vertices is %d\n", face_order[iface] );
  fprintf ( logfile,  "\n" );
  fprintf ( logfile,  "  Vertex list:\n" );
  fprintf ( logfile,  "    Vertex #, Node #, Material #, X, Y, Z:\n" );
  fprintf ( logfile,  "\n" );
  for ( ivert = 0; ivert < face_order[iface]; ivert++ ) {
    j = face[ivert][iface];
    k = face_mat[ivert][iface];
    fprintf ( logfile,  " %d %d %d %f %f %f\n", ivert, j, k, cor3[0][j], cor3[1][j], 
     cor3[2][j] );
  }

  fprintf ( logfile,  "\n" );
  fprintf ( logfile,  "  Face normal vector:\n" );
  fprintf ( logfile,  "\n" );
  fprintf ( logfile,  " %f %f %f\n", face_normal[0][iface], face_normal[1][iface],
    face_normal[2][iface] );

  fprintf ( logfile,  "\n" );
  fprintf ( logfile,  "  Vertex face normals:\n" );
  fprintf ( logfile,  "\n" );
  for ( ivert = 0; ivert < face_order[iface]; ivert++ ) {
    fprintf ( logfile,  " %d %f %f %f\n", ivert, vertex_normal[0][ivert][iface],
      vertex_normal[1][ivert][iface], vertex_normal[2][ivert][iface] );
  }

  return SUCCESS;

}
/******************************************************************************/

int converter::face_subset ( void ) {

/******************************************************************************/


/*
  Purpose:

    FACE_SUBSET selects a subset of the current faces as the converternew object.

  Warning:

    The original graphic object is overwritten by the converternew one.

  Modified:

    12 October 1998

  Author:

    John Burkardt

*/
  int i;
  int iface;
  int iface1;
  int iface2;
  int inc;
  int ivert;
  int j;
  int k;
  int num_cor32;

  num_line = 0;
/*
  Get the first and last faces to save, IFACE1 and IFACE2.
*/
  fprintf ( logfile,  "\n" );
  fprintf ( logfile,  "Enter lowest face number to save between 0 and %d:\n", num_face-1 );
  scanf ( "%d", &iface1 );
  if ( iface1 < 0 || iface1 > num_face - 1 ) {
    fprintf ( logfile,  "Illegal choice!\n" );
    return ERROR;
  }

  fprintf ( logfile,  "\n" );
  fprintf ( logfile,  "Enter highest face number to save between %d and %d:\n", 
    iface1, num_face-1 );
  scanf ( "%d", &iface2 );
  if ( iface2 < iface1 || iface2 > num_face - 1 ) {
    fprintf ( logfile,  "Illegal choice!\n" );
    return ERROR;
  }

  inc = iface1;
/*
  "Slide" the data for the saved faces down the face arrays.
*/
  for ( iface = 0; iface < iface2 + 1 - iface1; iface++ ) {
    face_order[iface] = face_order[iface+inc];
    for ( ivert = 0; ivert < MAX_ORDER; ivert++ ) {
      face[ivert][iface] = face[ivert][iface+inc];
      face_mat[ivert][iface] = face_mat[ivert][iface+inc];
      for ( i = 0; i < 3; i++ ) {
        vertex_normal[i][ivert][iface] =
          vertex_normal[i][ivert][iface+inc];
        vertex_rgb[i][ivert][iface] = vertex_rgb[i][ivert][iface+inc];
      }
    }
    for ( i = 0; i < 3; i++ ) {
      face_normal[i][iface] = face_normal[i][iface+inc];
    }
  }
/*
  Now reset the number of faces.
*/
  num_face = iface2 + 1 - iface1;
/*
  Now, for each point I, set LIST(I) = J if point I is the J-th
  point we are going to save, and 0 otherwise.  Then J will be
  the converternew label of point I.
*/
  for ( i = 0; i < num_cor3; i++ ) {
    list[i] = -1;
  }

  num_cor32 = 0;

  for ( iface = 0; iface < num_face; iface++ ){
    for ( ivert = 0; ivert < face_order[iface]; ivert++ ){
      j = face[ivert][iface];
      if ( list[j] == -1 ) {
        num_cor32 = num_cor32 + 1;
        list[j] = num_cor32;
      }
    }
  }
/*
  Now make the nonzero list entries rise in order, so that
  we can compress the COR3 data in a minute.
*/
  num_cor32 = 0;

  for ( i = 0; i < num_cor3; i++ ) {
    if ( list[i] != -1 ) {
      list[i] = num_cor32;
      num_cor32 = num_cor32 + 1;
    }
  }
/*
  Relabel the FACE array with the converternew node indices.
*/
  for ( iface = 0; iface < num_face; iface++ ){
    for ( ivert = 0; ivert < face_order[iface]; ivert++ ){
      j = face[ivert][iface];
      face[ivert][iface] = list[j];
    }
  }
/*
  Rebuild the COR3 array by sliding data down.
*/
  for ( i = 0; i < num_cor3; i++ ){
    k = list[i];
    if ( k != -1 ) {
      for ( j = 0; j < 3; j++ ) {
        cor3[j][k] = cor3[j][i];
      }
    }
  }

  num_cor3 = num_cor32;

  return SUCCESS;
}

/******************************************************************************/

char * converter::file_ext ( char *file_name ) {

/******************************************************************************/

/*
  Purpose:
   
    FILE_EXT picks out the extension in a file name.

  Modified:

    21 July 1998

  Author:
 
    John Burkardt
*/
  int i;

  i = char_index_last ( file_name, '.' );

  if ( i == -1 ) {
    return NULL;
  }
  else {
    return file_name+i+1;
  }
}
/******************************************************************************/

void converter::hello ( void ) {

/******************************************************************************/
/*
  Purpose:

    HELLO prints an explanatory header message.

  Modified:

    19 October 1998

  Author:
 
    John Burkardt
*/
  fprintf ( logfile,  "\n" );
  fprintf ( logfile,  "Hello:  This is IVCON,\n" );
  fprintf ( logfile,  "  for 3D graphics file conversion.\n" );
  fprintf ( logfile,  "\n" );
  fprintf ( logfile,  "    \".3ds\"   3D Studio Max binary;\n" );
  fprintf ( logfile,  "    \".ase\"   3D Studio Max ASCII export;\n" );
  fprintf ( logfile,  "    \".dxf\"   DXF;\n" );
  fprintf ( logfile,  "    \".hrc\"   SoftImage hierarchy;\n" );
  fprintf ( logfile,  "    \".iv\"    SGI Open Inventor;\n" );
  fprintf ( logfile,  "    \".obj\"   WaveFront Advanced Visualizer;\n" );
  fprintf ( logfile,  "    \".pov\"   Persistence of Vision (output only);\n" );
  fprintf ( logfile,  "    \".smf\"   Michael Garland's format;\n" );
  fprintf ( logfile,  "    \".stl\"   ASCII StereoLithography;\n" );
  fprintf ( logfile,  "    \".stla\"  ASCII StereoLithography;\n" );
  fprintf ( logfile,  "    \".txt\"   Text (output only);\n" );
  fprintf ( logfile,  "    \".vla\"   VLA.\n" );
  fprintf ( logfile,  "\n" );
  fprintf ( logfile,  "  Current limits include:\n" );
  fprintf ( logfile,  "    %d faces;\n", MAX_FACE );
  fprintf ( logfile,  "    %d line items;\n", MAX_LINE );
  fprintf ( logfile,  "    %d points.\n", MAX_COR3 );
  fprintf ( logfile,  "\n" );
  fprintf ( logfile,  "  Last modification: 18 November 1998.\n" );
  fprintf ( logfile,  "\n" );
  fprintf ( logfile,  "  Send problem reports to burkardt@psc.edu.\n" );
 
  return;
}
/******************************************************************************/

void converter::help ( void ) {

/******************************************************************************/

/*
  Purpose:

    HELP prints a list of the interactive commands.

  Modified:

    17 November 1998

  Author:
 
    John Burkardt
*/
  fprintf ( logfile,  "\n" );
  fprintf ( logfile,  "Commands:\n" );
  fprintf ( logfile,  "\n" );
  fprintf ( logfile,  "< file  Read input file;\n" );
  fprintf ( logfile,  "> file  Write output file;\n" );
  fprintf ( logfile,  "B       Switch the binary file byte-swapping mode;\n" );
  fprintf ( logfile,  "D       Switch the debugging mode;\n" );
  fprintf ( logfile,  "F       Print information about one face;\n" );
  fprintf ( logfile,  "H       Print this help list;\n" );
  fprintf ( logfile,  "I       Info, print out recent changes;\n" );
  fprintf ( logfile,  "N       Recompute normal vectors;\n" );
  fprintf ( logfile,  "Q       Quit;\n" );
  fprintf ( logfile,  "R       Reverse the normal vectors.\n" );
  fprintf ( logfile,  "S       Select face subset (NOT WORKING).\n" );
  fprintf ( logfile,  "T       Transform the data.\n" );
  fprintf ( logfile,  "W       Reverse the face node ordering.\n" );

  return;
}

/******************************************************************************/

void converter::init_program_data ( ) {

/******************************************************************************/

/*
  Purpose:

    INIT_PROGRAM_DATA initializes the internal program data.

  Modified:

    18 November 1998

  Author:

    John Burkardt
*/
  byte_swap = (convert_endian (0x12345678UL) != 0x12345678UL);
  debug = FALSE;
  num_color = 0;
  num_cor3 = 0;
  num_face = 0;
  num_line = 0;

  if ( debug == TRUE ) {
    fprintf ( logfile,  "\n" );
    fprintf ( logfile,  "INIT_PROGRAM_DATA: Program data initialized.\n" );
  }

  return;

}
/******************************************************************************/

int converter::interact ( ) {

/******************************************************************************/

/*
  Purpose:
   
    INTERACT carries on an interactive session with the user.

  Modified:

    18 November 1998

  Author:
 
    John Burkardt
*/
  int    i;
  int    icor3;
  int    ierror;
  int    iface;
  int    itemp;
  int    ivert;
  int    j;
  int    jvert;
  int    m;
  char  *next;
  float  temp;
  float  x;
  float  y;
  float  z;

  strcpy ( filein_name, "NO_IN_NAME" );
  strcpy ( fileout_name, "NO_OUT_NAME" );

/*  
  Say hello. 
*/
  hello ( );
/*  
  Get the next user command. 
*/
  fprintf ( logfile,  "\n" );
  fprintf ( logfile,  "Enter command (H for help)\n" );

  while ( fgets ( input, MAX_INCHARS, stdin ) != NULL ) {
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
/*  
  Command: < FILENAME 
*/
    if ( *next == '<' ) {

      next = next + 1;
      sscanf ( next, "%s", filein_name );

      data_init ( );

      ierror = data_read ( );

      if ( ierror == ERROR ) {
        fprintf ( logfile,  "\n" );
        fprintf ( logfile,  "INTERACT - Error!\n" );
        fprintf ( logfile,  "  DATA_READ failed to read input data.\n" );
      }
    }
/*  
  Command: > FILENAME 
*/
    else if ( *next == '>' ) {

      next = next + 1;
      sscanf ( next, "%s", fileout_name ); 

      ierror = data_write ( );
 
      if ( ierror == ERROR ) {
        fprintf ( logfile,  "\n" );
        fprintf ( logfile,  "INTERACT - Error!\n" );
        fprintf ( logfile,  "  OUTPUT_DATA failed to write output data.\n" );
      }

    }
/*
  B: Switch byte swapping option.
*/
    else if ( *next == 'B' || *next == 'b' ) {

      if ( byte_swap == TRUE ) {
        byte_swap = FALSE;
        fprintf ( logfile,  "Byte_swapping reset to FALSE.\n" );
      }
      else {
        byte_swap = TRUE;
        fprintf ( logfile,  "Byte_swapping reset to TRUE.\n" );
      }

    }
/*
  D: Switch debug option.
*/
    else if ( *next == 'D' || *next == 'd' ) {
      if ( debug == TRUE ) {
        debug = FALSE;
        fprintf ( logfile,  "Debug reset to FALSE.\n" );
      }
      else {
        debug = TRUE;
        fprintf ( logfile,  "Debug reset to TRUE.\n" );
      }
    }
/*  
  F: Check a face. 
*/
    else if ( *next == 'f' || *next == 'F' ) {
      fprintf ( logfile,  "\n" );
      fprintf ( logfile,  "  Enter a face index between 0 and %d:", num_face-1 );
      scanf ( "%d", &iface );
      face_print ( iface );
    }
/*  
  H: Help
*/
    else if ( *next == 'h' || *next == 'H' ) {
      help ( );
    }
/*
  I: Print change information. 
*/
    else if ( *next == 'i' || *next == 'I') {
      news ( );
    }
/*
  N: Recompute normal vectors.
*/
    else if ( *next == 'n' || *next == 'N') {

      for ( iface = 0; iface < num_face; iface++ ) {
        for ( i = 0; i < 3; i++ ) {
          face_normal[i][iface] = 0.0;
        }
      }

      for ( iface = 0; iface < num_face; iface++ ) {
         for ( ivert = 0; ivert < face_order[iface]; ivert++ ) {
          for ( i = 0; i < 3; i++ ) {
            vertex_normal[i][ivert][iface] = 0.0;
          }
        }
      }

      set_vertex_normal ( );

      set_cor3_normal ( );

      ave_face_normal ( );
    }
/*
  Q: Quit
*/
    else if ( *next == 'q' || *next == 'Q' ) {
      fprintf ( logfile,  "\n" );
      fprintf ( logfile,  "INTERACT - Normal end of execution.\n" );
      return SUCCESS;
    }
/*
  R: Reverse normal vectors. 
*/
    else if ( *next == 'r' || *next == 'R' ) {

      for ( icor3 = 0; icor3 < num_cor3; icor3++ ) {
        for ( i = 0; i < 3; i++ ) {
          cor3_normal[i][icor3] = - cor3_normal[i][icor3];
        }
      }

      for ( iface = 0; iface < num_face; iface++ ) {
        for ( i = 0; i < 3; i++ ) {
          face_normal[i][iface] = - face_normal[i][iface];
        }
      }

      for ( iface = 0; iface < num_face; iface++ ) {
        for ( ivert = 0; ivert < face_order[iface]; ivert++ ) {
          for ( i = 0; i < 3; i++ ) {
            vertex_normal[i][ivert][iface] = 
              - vertex_normal[i][ivert][iface];
          }
        }
      }
      fprintf ( logfile,  "\n" );
      fprintf ( logfile,  "INTERACT - Note:\n" );
      fprintf ( logfile,  "  Reversed node, face and vertex normals.\n" );
    }
/*  
  S: Select a few faces, discard the rest.
*/
    else if ( *next == 's' || *next == 'S' ) {
      face_subset ( );
    }
/*  
  T: Transform the data.
*/
    else if ( *next == 't' || *next == 'T' ) {

      fprintf ( logfile,  "\n" );
      fprintf ( logfile,  "For now, we only offer point scaling.\n" );
      fprintf ( logfile,  "Enter X, Y, Z scale factors:\n" );

      scanf ( "%f %f %f", &x, &y, &z );

      for ( j = 0; j < num_cor3; j++ ) {
        cor3[0][j] = x * cor3[0][j];
        cor3[1][j] = y * cor3[1][j];
        cor3[2][j] = z * cor3[2][j];
      }

      for ( iface = 0; iface < num_face; iface++ ) {
        for ( i = 0; i < 3; i++ ) {
          face_normal[i][iface] = 0.0;
        }
      }

      for ( iface = 0; iface < num_face; iface++ ) {
         for ( ivert = 0; ivert < face_order[iface]; ivert++ ) {
          for ( i = 0; i < 3; i++ ) {
            vertex_normal[i][ivert][iface] = 0.0;
          }
        }
      }

      set_vertex_normal ( );

      set_cor3_normal ( );

      ave_face_normal ( );
    }
/*
  U: Renumber faces, count objects:
*/
    else if ( *next == 'u' || *next == 'U' ) {
    }
/*
  V: Convert polygons to triangles:
*/
    else if ( *next == 'v' || *next == 'V' ) {
    }
/*
  W: Reverse the face node ordering. 
*/
    else if ( *next == 'w' || *next == 'W' ) {

      if ( num_face > 0 ) {

        for ( iface = 0; iface < num_face; iface++ ) {

          m = face_order[iface];

          for ( ivert = 0; ivert < m/2; ivert++ ) {

            jvert = m - ivert - 1;

            itemp = face[ivert][iface];
            face[ivert][iface] = face[jvert][iface];
            face[jvert][iface] = itemp;

            itemp = face_mat[ivert][iface];
            face_mat[ivert][iface] = face_mat[jvert][iface];
            face_mat[jvert][iface] = itemp;

            for ( i = 0; i < 3; i++ ) {
              temp = vertex_normal[i][ivert][iface];
              vertex_normal[i][ivert][iface] = 
                vertex_normal[i][jvert][iface];
              vertex_normal[i][jvert][iface] = temp;
            }
          }
        }
        fprintf ( logfile,  "\n" );
        fprintf ( logfile,  "INTERACT - Note:\n" );
        fprintf ( logfile,  "  Reversed face node ordering.\n" );
      }
    }
/*
  Command: ???  
*/
    else {
      fprintf ( logfile,  "\n" );
      fprintf ( logfile,  "INTERACT: Warning!\n" );
      fprintf ( logfile,  "  Your command was not recognized.\n" );
    }

    fprintf ( logfile,  "\n" );
    fprintf ( logfile,  "Enter command (H for help)\n" );

  }
  return SUCCESS;
}

/******************************************************************************/

int converter::ivec_max ( int n, int *a ) {

/******************************************************************************/

/*
  Purpose:

    IVEC_MAX returns the maximum element in an integer array.

  Modified:

    09 October 1998

  Author:

    John Burkardt
*/
  int  i;
  int *ia;
  int  imax;

  if ( n <= 0 ) {
    imax = 0;
  }
  else {
    ia = a;
    imax = *ia;
    for ( i = 1; i < n; i++ ) {
      ia = ia + 1;
      if ( imax < *ia ) {
        imax = *ia;
      }
    }
  }
  return imax;
}
/******************************************************************************/

int eqi ( char* string1, char* string2 ) {

/******************************************************************************/

/*
  Purpose:

    LEQI compares two strings for equality, disregarding case.

  Modified:

    15 September 1998

  Author:
 
    John Burkardt
*/
  int i;
  int nchar;
  int nchar1;
  int nchar2;

  nchar1 = strlen ( string1 );
  nchar2 = strlen ( string2 );
  nchar = MIN ( nchar1, nchar2 );

/*
  The strings are not equal if they differ over their common length.
*/
  for ( i = 0; i < nchar; i++ ) {

    if ( toupper ( string1[i] ) != toupper ( string2[i] ) ) {
      return FALSE;
    }
  }
/*
  The strings are not equal if the longer one includes nonblanks
  in the tail.
*/
  if ( nchar1 > nchar ) {
    for ( i = nchar; i < nchar1; i++ ) {
      if ( string1[i] != ' ' ) {
        return FALSE;
      }
    } 
  }
  else if ( nchar2 > nchar ) {
    for ( i = nchar; i < nchar2; i++ ) {
      if ( string2[i] != ' ' ) {
        return FALSE;
      }
    } 
  }
  return TRUE;
}

/******************************************************************************/

void converter::minmax ( void ) {

/******************************************************************************/

/*
  Purpose:

    MINMAX computes the coordinate minima and maxima.

  Modified:

    31 August 1998

  Author:

    John Burkardt
*/
  int   i;
  float xave;
  float xmax;
  float xmin;
  float yave;
  float ymax;
  float ymin;
  float zave;
  float zmax;
  float zmin;

  xave = cor3[0][0];
  xmax = cor3[0][0];
  xmin = cor3[0][0];

  yave = cor3[1][0];
  ymax = cor3[1][0];
  ymin = cor3[1][0];

  zave = cor3[2][0];
  zmax = cor3[2][0];
  zmin = cor3[2][0];

  for ( i = 1; i < num_cor3; i++ ) {

    xave = xave + cor3[0][i];
    xmin = MIN ( xmin, cor3[0][i] );
    xmax = MAX ( xmax, cor3[0][i] );

    yave = yave + cor3[1][i];
    ymin = MIN ( ymin, cor3[1][i] );
    ymax = MAX ( ymax, cor3[1][i] );

    zave = zave + cor3[2][i];
    zmin = MIN ( zmin, cor3[2][i] );
    zmax = MAX ( zmax, cor3[2][i] );
  }

  xave = xave / num_cor3;
  yave = yave / num_cor3;
  zave = zave / num_cor3;

  fprintf ( logfile,  "\n" );
  fprintf ( logfile,  "MINMAX - Data range:\n" );
  fprintf ( logfile,  "\n" );
  fprintf ( logfile,  "   Minimum   Average   Maximum  Range\n" );
  fprintf ( logfile,  "\n" );
  fprintf ( logfile,  "X  %f %f %f %f\n", xmin, xave, xmax, xmax-xmin );
  fprintf ( logfile,  "Y  %f %f %f %f\n", ymin, yave, ymax, ymax-ymin );
  fprintf ( logfile,  "Z  %f %f %f %f\n", zmin, zave, zmax, zmax-zmin );

}
/******************************************************************************/

void converter::news ( void ) {

/******************************************************************************/

/*
  Purpose:

    converternewS reports the program change history.

  Modified:
 
    03 December 1998

  Author:
 
    John Burkardt
*/
  fprintf ( logfile,  "\n" );
  fprintf ( logfile,  "Recent changes:\n" );
  fprintf ( logfile,  "\n" );
  fprintf ( logfile,  "  03 December 1998\n" );
  fprintf ( logfile,  "    Set up simple hooks in TDS_READ_MATERIAL_SECTION.\n" );
  fprintf ( logfile,  "  02 December 1998\n" );
  fprintf ( logfile,  "    Set up simple hooks for texture map names.\n" );
  fprintf ( logfile,  "  19 November 1998\n" );
  fprintf ( logfile,  "    IV_WRITE uses PER_VERTEX normal binding.\n" );
  fprintf ( logfile,  "  18 November 1998\n" );
  fprintf ( logfile,  "    Added node normals.\n" );
  fprintf ( logfile,  "    Finally added the -RN option.\n" );
  fprintf ( logfile,  "  17 November 1998\n" );
  fprintf ( logfile,  "    Added face node ordering reversal option.\n" );
  fprintf ( logfile,  "  20 October 1998\n" );
  fprintf ( logfile,  "    Added DATA_REPORT.\n" );
  fprintf ( logfile,  "  19 October 1998\n" );
  fprintf ( logfile,  "    SMF_READ and SMF_WRITE added.\n" );
  fprintf ( logfile,  "  16 October 1998\n" );
  fprintf ( logfile,  "    Fixing a bug in IV_READ that chokes on ]} and other\n" );
  fprintf ( logfile,  "    cases where brackets aren't properly spaced.\n" );
  fprintf ( logfile,  "  11 October 1998\n" );
  fprintf ( logfile,  "    Added face subset selection option S.\n" );
  fprintf ( logfile,  "  09 October 1998\n" );
  fprintf ( logfile,  "    Reworking normal vector treatments.\n" );
  fprintf ( logfile,  "    Synchronizing IVREAD and IVCON.\n" );
  fprintf ( logfile,  "    POV_WRITE added.\n" );
  fprintf ( logfile,  "  02 October 1998\n" );
  fprintf ( logfile,  "    IVCON reproduces BOX.3DS and CONE.3DS exactly.\n" );
  fprintf ( logfile,  "  30 September 1998\n" );
  fprintf ( logfile,  "    IVCON compiled on the PC.\n" );
  fprintf ( logfile,  "    Interactive BYTE_SWAP option added for binary files.\n" );
  fprintf ( logfile,  "  25 September 1998\n" );
  fprintf ( logfile,  "    OBJECT_NAME made available to store object name.\n" );
  fprintf ( logfile,  "  23 September 1998\n" );
  fprintf ( logfile,  "    3DS binary files can be written.\n" );
  fprintf ( logfile,  "  15 September 1998\n" );
  fprintf ( logfile,  "    3DS binary files can be read.\n" );
  fprintf ( logfile,  "  01 September 1998\n" );
  fprintf ( logfile,  "    MINMAX, AVE_FACE_NORMAL added.\n" );
  fprintf ( logfile,  "    Major modifications to normal vectors.\n" );
  fprintf ( logfile,  "  24 August 1998\n" );
  fprintf ( logfile,  "    HRC_READ added.\n" );
  fprintf ( logfile,  "  21 August 1998\n" );
  fprintf ( logfile,  "    TXT_WRITE improved.\n" );
  fprintf ( logfile,  "  20 August 1998\n" );
  fprintf ( logfile,  "    HRC_WRITE can output lines as linear splines.\n" );
  fprintf ( logfile,  "  19 August 1998\n" );
  fprintf ( logfile,  "    Automatic normal computation for OBJ files.\n" );
  fprintf ( logfile,  "    Added normal vector computation.\n" );
  fprintf ( logfile,  "    HRC_WRITE is working.\n" );
  fprintf ( logfile,  "  18 August 1998\n" );
  fprintf ( logfile,  "    IV read/write handles BASECOLOR RGB properly now.\n" );
  fprintf ( logfile,  "    Improved treatment of face materials and normals.\n" );
  fprintf ( logfile,  "  17 August 1998\n" );
  fprintf ( logfile,  "    MAX_ORDER increased to 35.\n" );
  fprintf ( logfile,  "    FACE_PRINT routine added.\n" );
  fprintf ( logfile,  "    INIT_DATA routine added.\n" );
  fprintf ( logfile,  "  14 August 1998\n" );
  fprintf ( logfile,  "    IV_Read is working.\n" );
  fprintf ( logfile,  "  13 August 1998\n" );
  fprintf ( logfile,  "    ASE_Write is working.\n" );
  fprintf ( logfile,  "    IV_Write is working.\n" );
  fprintf ( logfile,  "  12 August 1998\n" );
  fprintf ( logfile,  "    ASE_Read is working.\n" );
  fprintf ( logfile,  "  10 August 1998\n" );
  fprintf ( logfile,  "    DXF_Write is working.\n" );
  fprintf ( logfile,  "    DXF_Read is working.\n" );
  fprintf ( logfile,  "  27 July 1998\n" );
  fprintf ( logfile,  "    Interactive mode is working.\n" );
  fprintf ( logfile,  "    OBJ_Read is working.\n" );
  fprintf ( logfile,  "  25 July 1998\n" );
  fprintf ( logfile,  "    OBJ_Write is working.\n" );
  fprintf ( logfile,  "  24 July 1998\n" );
  fprintf ( logfile,  "    InCheck checks the input data.\n" );
  fprintf ( logfile,  "    VLA_Read is working.\n" );
  fprintf ( logfile,  "    VLA_Write is working.\n" );
  fprintf ( logfile,  "  23 July 1998\n" );
  fprintf ( logfile,  "    STL_Write is working.\n" );
  fprintf ( logfile,  "  22 July 1998\n" );
  fprintf ( logfile,  "    STL_Read is working.\n" );
  fprintf ( logfile,  "    TXT_Write is working.\n" );
}


/******************************************************************************/

void converter::print_sizes ( ) {

/******************************************************************************/

/*
  Purpose:

    PRINT_SIZES reports the size in bytes of various data types.

  Modified:

    14 October 1998

  Author:

    John Burkardt
*/
  fprintf ( logfile,  "\n" );
  fprintf ( logfile,  "PRINT_SIZES: Report data type sizes.\n" );
  fprintf ( logfile,  "\n" );
  fprintf ( logfile,  "  Type                Size     Min     Max\n" );
  fprintf ( logfile,  "\n" );
  fprintf ( logfile,  "  char                %d       %d      %d\n", 
    (int)sizeof ( char ), CHAR_MIN, CHAR_MAX );
  fprintf ( logfile,  "  unsigned char       %d       0       %d\n", 
    (int)sizeof ( char ),           UCHAR_MAX );
  fprintf ( logfile,  "  short int           %d       %d      %d\n", 
    (int)sizeof ( short int ), SHRT_MIN, SHRT_MAX );
  fprintf ( logfile,  "  unsigned short int  %d       0       %u\n", 
    (int)sizeof ( unsigned short int ),           USHRT_MAX );
  fprintf ( logfile,  "  int                 %d       %d      %d\n", 
    (int)sizeof ( int ), INT_MIN, INT_MAX );
  fprintf ( logfile,  "  unsigned int        %d       0       %u\n", 
    (int)sizeof ( unsigned int ), UINT_MAX );
  fprintf ( logfile,  "  long int            %d       %ld      %ld\n", 
    (int)sizeof ( long int ), (long int)LONG_MIN, (long int)LONG_MAX );
  fprintf ( logfile,  "  unsigned long int   %d       0       %lu\n", 
    (int)sizeof ( unsigned long int ), (unsigned long int)ULONG_MAX );
/*
  FLT_MIN, FLT_MAX, DBL_MIN, DBL_MAX not defined on Microsoft C.
*/

/*
  fprintf ( logfile,  "  float               %d       %e      %e\n", 
    (int)sizeof ( float ), FLT_MIN, FLT_MAX );
  fprintf ( logfile,  "  double              %d       %e      %e\n", 
    (int)sizeof ( double ), DBL_MIN, DBL_MAX );
*/

  return;
}

/******************************************************************************/

int converter::rcol_find ( float a[][MAX_COR3], int m, int n, float r[] ) { 

/******************************************************************************/

/*
  Purpose:
   
    RCOL_FIND finds if a vector occurs in a table.

  Comment:

    Explicitly forcing the second dimension to be MAX_COR3 is a kludge.
    I have to figure out how to do this as pointer references.

    Also, since the array is not sorted, this routine should not be carelessly
    called repeatedly for really big values of N, because you'll waste a
    lot of time.

  Modified:

    27 April 1999

  Author:
 
    John Burkardt
*/
  int i;
  int icol;
  int j;

  icol = -1;

  for ( j = 0; j < n; j++ ) {
    for ( i = 0; i < m; i++ ) {
      if ( a[i][j] != r[i] ) {
        break;
      }
      if ( i == m-1 ) {
        return j;
      }
    }
  }

  return icol;
}
/******************************************************************************/

float converter::reverse_bytes_float ( float x ) {

/******************************************************************************/

/*
  Purpose:

    REVERSE_BYTES_FLOAT reverses the four bytes in a float.

  Modified:

    11 September 1998

  Author:

    John Burkardt

  Parameters:

    X, a float whose bytes are to be reversed.

    REVERSE_BYTES_FLOAT, a float with bytes in reverse order from those in X.

*/
  char c;
  union {
    float yfloat;
    char ychar[4];
  } y;

  y.yfloat = x;
  
  c = y.ychar[0];
  y.ychar[0] = y.ychar[3];
  y.ychar[3] = c;

  c = y.ychar[1];
  y.ychar[1] = y.ychar[2];
  y.ychar[2] = c;

  return ( y.yfloat );
}
/******************************************************************************/

void converter::set_cor3_normal (  ) {

/******************************************************************************/

/*
  Purpose:

    SET_COR3_NORMAL computes node normal vectors.

  Modified:

    18 November 1998

  Author:

    John Burkardt
*/
  int   icor3;
  int   iface;
  int   ivert;
  int   j;
  float norm;
  float temp;

  for ( icor3 = 0; icor3 < num_cor3; icor3++ ) {
    for ( j = 0; j < 3; j++ ) {
      cor3_normal[j][icor3] = 0.0;
    }
  }
/*
  Add up the normals at all the faces to which the node belongs.
*/
  for ( iface = 0; iface < num_face; iface++ ) {

    for ( ivert = 0; ivert < face_order[iface]; ivert++ ) {

      icor3 = face[ivert][iface];

      for ( j = 0; j < 3; j++ ) {
        cor3_normal[j][icor3] = cor3_normal[j][icor3]
          + vertex_normal[j][ivert][iface];
      }
    }
  }
/*
  Renormalize.
*/
  for ( icor3 = 0; icor3 < num_cor3; icor3++ ) {

    norm = 0.0;
    for ( j = 0; j < 3; j++ ) {
      temp = cor3_normal[j][icor3];
      norm = norm + temp * temp;
    }

    if ( norm == 0.0 ) {
      norm = 3.0;
      for ( j = 0; j < 3; j++ ) {
        cor3_normal[j][icor3] = 1.0;
      }
    }

    norm = ( float ) sqrt ( norm );

    for ( j = 0; j < 3; j++ ) {
      cor3_normal[j][icor3] = cor3_normal[j][icor3] / norm;
    }
  }

  return;
}
/******************************************************************************/

void converter::set_vertex_normal (  ) {

/******************************************************************************/

/*
  Purpose:

    SET_VERTEX_NORMAL recomputes the face vertex normal vectors.

  Modified:

    12 October 1998

  Author:

    John Burkardt
*/
  int   i;
  int   i0;
  int   i1;
  int   i2;
  int   iface;
  int   ivert;
  int   jp1;
  int   jp2;
  int   nfix;
  float norm;
  float temp;
  float x0;
  float x1;
  float x2;
  float xc;
  float y0;
  float y1;
  float y2;
  float yc;
  float z0;
  float z1;
  float z2;
  float zc;

  if ( num_face <= 0 ) {
    return;
  }

  nfix = 0;
/*
  Consider each face.
*/
  for ( iface = 0; iface < num_face; iface++ ) {

    for ( ivert = 0; ivert < face_order[iface]; ivert++ ) {

      norm = 0.0;
      for ( i = 0; i < 3; i++ ) {
        temp = vertex_normal[i][ivert][iface];
        norm = norm + temp * temp;
      }
      norm = ( float ) sqrt ( norm );

      if ( norm == 0.0 ) {

        nfix = nfix + 1;

        i0 = face[ivert][iface];
        x0 = cor3[0][i0];
        y0 = cor3[1][i0];
        z0 = cor3[2][i0];

        jp1 = ivert + 1;
        if ( jp1 >= face_order[iface] ) {
          jp1 = jp1 - face_order[iface];
        }
        i1 = face[jp1][iface];
        x1 = cor3[0][i1];
        y1 = cor3[1][i1];
        z1 = cor3[2][i1];

        jp2 = ivert + 2;
        if ( jp2 >= face_order[iface] ) {
          jp2 = jp2 - face_order[iface];
        }
        i2 = face[jp2][iface];
        x2 = cor3[0][i2];
        y2 = cor3[1][i2];
        z2 = cor3[2][i2];

        xc = ( y1 - y0 ) * ( z2 - z0 ) - ( z1 - z0 ) * ( y2 - y0 );
        yc = ( z1 - z0 ) * ( x2 - x0 ) - ( x1 - x0 ) * ( z2 - z0 );
        zc = ( x1 - x0 ) * ( y2 - y0 ) - ( y1 - y0 ) * ( x2 - x0 );

        norm = ( float ) sqrt ( xc * xc + yc * yc + zc * zc );

        if ( norm == 0.0 ) {
          xc = ( float ) 1.0 / sqrt ( 3.0 );
          yc = ( float ) 1.0 / sqrt ( 3.0 );
          zc = ( float ) 1.0 / sqrt ( 3.0 );
        }
        else {
          xc = xc / norm;
          yc = yc / norm;
          zc = zc / norm;
        }

        vertex_normal[0][ivert][iface] = xc;
        vertex_normal[1][ivert][iface] = yc;
        vertex_normal[2][ivert][iface] = zc;

      }
    }
  }

  if ( nfix > 0 ) {
    fprintf ( logfile,  "\n" );
    fprintf ( logfile,  "SET_VERTEX_NORMAL: Recomputed %d face vertex normals.\n", nfix );
  }

  return;
}




/******************************************************************************/

int converter::leqi ( char* string1, char* string2 ) {

/******************************************************************************/

/*
  Purpose:

    LEQI compares two strings for equality, disregarding case.

  Modified:

    15 September 1998

  Author:
 
    John Burkardt
*/
  int i;
  int nchar;
  int nchar1;
  int nchar2;

  nchar1 = strlen ( string1 );
  nchar2 = strlen ( string2 );
  nchar = MIN ( nchar1, nchar2 );

/*
  The strings are not equal if they differ over their common length.
*/
  for ( i = 0; i < nchar; i++ ) {

    if ( toupper ( string1[i] ) != toupper ( string2[i] ) ) {
      return FALSE;
    }
  }
/*
  The strings are not equal if the longer one includes nonblanks
  in the tail.
*/
  if ( nchar1 > nchar ) {
    for ( i = nchar; i < nchar1; i++ ) {
      if ( string1[i] != ' ' ) {
        return FALSE;
      }
    } 
  }
  else if ( nchar2 > nchar ) {
    for ( i = nchar; i < nchar2; i++ ) {
      if ( string2[i] != ' ' ) {
        return FALSE;
      }
    } 
  }
  return TRUE;
}

/**
 * If several polygons share a vertex that doesnt mean this vertex maps to
 * the same texture coordinate for every polygon. Nontheless various 3d formats
 * just store the vertex once and then combine it with different texture points.
 * For CS sprite format we need to create an instance of such a vertex for every
 * texturepoint.
 * So we store the texpoints and geom. points in the temp_* variables and then
 * duplicate the shared vertices.
 */
int converter::makeunique (int idx, int vtidx)
{
  int i=0;
  float x, y, z, u, v, x1, y1, z1, u1, v1;
  //DEBUG_BREAK;
  x = temp_cor3[0][idx];
  y = temp_cor3[1][idx];
  z = temp_cor3[2][idx];
  u = temp_cor3_uv[0][vtidx];
  v = temp_cor3_uv[1][vtidx];

  while (i<num_cor3)
  {
    x1 = cor3[0][i];
    y1 = cor3[1][i];
    z1 = cor3[2][i];
    u1 = cor3_uv[0][i];
    v1 = cor3_uv[1][i];
    if (x==x1 && y==y1 && z==z1 && u==u1 && v==v1)
      break;
    i++;
  }

  if (i==num_cor3)
  {
    cor3[0][i] = x;
    cor3[1][i] = y;
    cor3[2][i] = z;
    cor3_uv[0][i] = u;
    cor3_uv[1][i] = v;
    num_cor3++;
    num_cor3_uv++;
  }
  return i;
}

//
// frame manipulator methods
//

csConverter_FrameManipulator::csConverter_FrameManipulator(converter *target)
 : m_data_target(target)
{
}

csConverter_FrameManipulator::~csConverter_FrameManipulator()
{
}




