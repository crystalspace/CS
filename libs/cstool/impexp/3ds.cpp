
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

int converter::tds_read ( FILE *filein ) {

/******************************************************************************/

/*
  Purpose:
   
    TDS_READ reads a 3D Studio MAX binary 3DS file.

  Modified:

    20 October 1998

  Author:
 
    John Burkardt
*/
  unsigned long int   chunk_begin;
  unsigned long int   chunk_end;
  unsigned long int   chunk_length;
  unsigned long int   chunk_length2;
  unsigned long int   position;
  unsigned short int  temp_int;
  int                 version;
  int                 views_read;
/* 
  Initialize.
*/
  views_read = 0;

  temp_int = tds_read_u_short_int ( filein );

  if ( temp_int == 0x4d4d ) {

    if ( debug == TRUE ) {
      fprintf ( logfile,  "TDS_READ: DEBUG: Read magic number.\n" );
    }
/* 
  Move to 28 bytes from the beginning of the file. 
*/
    position = 28;
    fseek ( filein, position, SEEK_SET );
    version = fgetc ( filein );

    if ( version < 3 ) {
      fprintf ( logfile,  "\n" );
      fprintf ( logfile,  "TDS_READ - Fatal error!\n" );
      fprintf ( logfile,  "  This routine can only read 3DS version 3 or later.\n" );
      fprintf ( logfile,  "  The input file is version %d.\n" ,version );
      return ERROR;
    }

    if ( debug == TRUE ) {
      fprintf ( logfile,  "TDS_READ: DEBUG: Read version number.\n" );
    }
/* 
  Move to 2 bytes from the beginning of the file. 
  Set CURRENT_POINTER to the first byte of the chunk.
  Set CHUNK_LENGTH to the number of bytes in the chunk.
*/
    chunk_begin = 0;
    position = 2;
    fseek ( filein, position, SEEK_SET );

    chunk_length = tds_read_u_long_int ( filein );
    position = 6;

    chunk_end = chunk_begin + chunk_length;

    if ( debug == TRUE ) {
      fprintf ( logfile,  "TDS_READ: Chunk length = %lu.\n", chunk_length );
    }

    while ( position + 2 < chunk_end ) {

      temp_int = tds_read_u_short_int ( filein );
      position = position + 2;

      if ( debug == TRUE ) {
        fprintf ( logfile,  "TDS_READ: Short int = %hu, position = %lu.\n", temp_int, position );
      }

      if ( temp_int == 0x0002 ) {
        chunk_length2 = tds_read_u_long_int ( filein );
        position = position + 4;
        position = position - 6 + chunk_length2;
        fseek ( filein, position, SEEK_SET );
      }
      else if ( temp_int == 0x3d3d ) {
        position = position - 2;
        position = position + tds_read_edit_section ( filein, &views_read );
      }
      else if ( temp_int == 0xb000 ) {
        position = position - 2;
        position = position + tds_read_keyframe_section ( filein, &views_read );
      }
      else {
        fprintf ( logfile,  "\n" );
        fprintf ( logfile,  "TDS_READ: Error!\n" );
        fprintf ( logfile,  "  Unexpected input, position = %lu.\n", position );
        fprintf ( logfile,  "  TEMP_INT = %hux\n", temp_int );
        return ERROR;
      }
    }
    position = chunk_begin + chunk_length;
    fseek ( filein, position, SEEK_SET );
  }
  else {
    fprintf ( logfile,  "\n" );
    fprintf ( logfile,  "TDS_READ - Fatal error!\n" );
    fprintf ( logfile,  "  Could not find the main section tag.\n" );
    return ERROR;
  }

  return SUCCESS;
}
/******************************************************************************/

unsigned long converter::tds_read_ambient_section ( FILE *filein ) {

/******************************************************************************/

  unsigned long int   current_pointer;
  unsigned char       end_found = FALSE;
  int                 i;
  float               rgb_val[3];
  unsigned short int  temp_int;
  unsigned long int   temp_pointer;
  unsigned long int   teller; 
  unsigned char       true_c_val[3];
 
  current_pointer = ftell ( filein ) - 2;
  temp_pointer = tds_read_u_long_int ( filein );
  teller = 6;
 
  while ( end_found == FALSE ) {

    temp_int = tds_read_u_short_int ( filein );
    teller = teller + 2;
 
    switch ( temp_int ) {
      case 0x0010:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "     COLOR_F color definition section tag of %0X\n", 
            temp_int );
        }
        for ( i = 0; i < 3; i++ ) {
          rgb_val[i] = tds_read_float ( filein );
        }
        teller = teller + 3 * sizeof ( float );
        break;
      case 0x0011:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "     COLOR_24 24 bit color definition section tag of %0X\n",
            temp_int );
        }

        for ( i = 0; i < 3; i++ ) {
          true_c_val[i] = fgetc ( filein );
        }
        teller = teller + 3;
        break;
      default:
        break;
    }
 
    if ( teller >= temp_pointer ) {
      end_found = TRUE;
    }

  }
 
  fseek ( filein, current_pointer+temp_pointer, SEEK_SET );

  return ( temp_pointer );
}
/******************************************************************************/

unsigned long converter::tds_read_background_section ( FILE *filein ) {

/******************************************************************************/

  unsigned long int   current_pointer;
  unsigned char       end_found = FALSE;
  int                 i;
  float               rgb_val[3];
  unsigned short int  temp_int;
  unsigned long int   temp_pointer;
  unsigned long int   teller; 
  unsigned char       true_c_val[3];
 
  current_pointer = ftell ( filein ) - 2;
  temp_pointer = tds_read_u_long_int ( filein );
  teller = 6;

  while ( end_found == FALSE ) {

    temp_int = tds_read_u_short_int ( filein );
    teller = teller + 2;
 
    switch ( temp_int ) {
      case 0x0010:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "   COLOR_F RGB color definition section tag of %0X\n", 
            temp_int );
        }
        for ( i = 0; i < 3; i++ ) {
          rgb_val[i] = tds_read_float ( filein );
        }
        teller = teller + 3 * sizeof ( float );
        break;
      case 0x0011:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "   COLOR_24 24 bit color definition section tag of %0X\n", 
            temp_int );
        }
 
        for ( i = 0; i < 3; i++ ) {
          true_c_val[i] = fgetc ( filein );
        }
        teller = teller + 3;
        break;
      default:
        break;
    }

    if ( teller >= temp_pointer ) {
      end_found = TRUE;
    }

  }
 
  fseek ( filein, current_pointer+temp_pointer, SEEK_SET );

  return ( temp_pointer );
}
/******************************************************************************/

unsigned long converter::tds_read_boolean ( unsigned char *boolean, FILE *filein ) {

/******************************************************************************/

  unsigned long current_pointer;
  unsigned long temp_pointer;
 
  current_pointer = ftell ( filein ) - 2;
  temp_pointer = tds_read_u_long_int ( filein );
 
  *boolean = fgetc ( filein );
 
  fseek ( filein, current_pointer+temp_pointer, SEEK_SET );

  return ( temp_pointer );
}
/******************************************************************************/

unsigned long converter::tds_read_camera_section ( FILE *filein ) {

/******************************************************************************/

  float               camera_eye[3];
  float               camera_focus[3];
  unsigned long int   current_pointer;
  float               lens;
  float               rotation;
  unsigned long int   temp_pointer;
  unsigned short int  u_short_int_val;
 
  current_pointer = ftell ( filein ) - 2;
  temp_pointer = tds_read_u_long_int ( filein );
 
  camera_eye[0] = tds_read_float ( filein );
  camera_eye[1] = tds_read_float ( filein );
  camera_eye[2] = tds_read_float ( filein );
 
  camera_focus[0] = tds_read_float ( filein );
  camera_focus[1] = tds_read_float ( filein );
  camera_focus[2] = tds_read_float ( filein );

  rotation = tds_read_float ( filein );
  lens = tds_read_float ( filein );

  if ( debug == TRUE ) {
    fprintf ( logfile,  " Found camera viewpoint at XYZ = %f %f %f.\n",
      camera_eye[0], camera_eye[1], camera_eye[2] );
    fprintf ( logfile,  "     Found camera focus coordinates at XYZ = %f %f %f.\n",
      camera_focus[0], camera_focus[1], camera_focus[2] );
    fprintf ( logfile,  "     Rotation of camera is:  %f.\n", rotation );
    fprintf ( logfile,  "     Lens in used camera is: %f mm.\n", lens );
  }
 
  if ( ( temp_pointer-38 ) > 0 ) {

    if ( debug == TRUE ) {
      fprintf ( logfile,  "          Found extra camera sections.\n" );
    }

    u_short_int_val = tds_read_u_short_int ( filein );

    if ( u_short_int_val == 0x4710 ) {
      if ( debug == TRUE ) {
        fprintf ( logfile,  "          CAM_SEE_CONE.\n" );
      }
      tds_read_unknown_section ( filein );
    }

    u_short_int_val = tds_read_u_short_int ( filein );

    if ( u_short_int_val == 0x4720 ) {
      if ( debug == TRUE ) {
        fprintf ( logfile,  "          CAM_RANGES.\n" );
      }
      tds_read_unknown_section ( filein );
    }

  }
 
  fseek ( filein, current_pointer+temp_pointer, SEEK_SET );

  return ( temp_pointer );
}
/******************************************************************************/

unsigned long converter::tds_read_edit_section ( FILE *filein, int *views_read ) {

/******************************************************************************/

/*
  Modified:

    18 September 1998
*/
  unsigned long int   chunk_length;
  unsigned long int   current_pointer;
  unsigned char       end_found = FALSE;
  unsigned long int   teller;
  unsigned short int  temp_int;

  current_pointer = ftell ( filein ) - 2;
  chunk_length = tds_read_u_long_int ( filein );
  teller = 6;
 
  while ( end_found == FALSE ) {

    temp_int = tds_read_u_short_int ( filein );
    teller = teller + 2;
 
    switch ( temp_int ) {
      case 0x1100:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "    BIT_MAP section tag of %0X\n", temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0x1201:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "    USE_SOLID_BGND section tag of %0X\n", temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0x1300:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "    V_GRADIENT section tag of %0X\n", temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0x1400:
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0x1420:
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0x1450:
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0x1500:
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0x2200:
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0x2201:
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0x2210:
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0x2300:
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0x2302:
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0x3000:
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0x2100:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "    AMBIENT_LIGHT section tag of %0X\n", temp_int );
        }
        teller = teller + tds_read_ambient_section ( filein );
        break;
      case 0x1200:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "    SOLID_BGND section tag of %0X\n", temp_int );
        }
        teller = teller + tds_read_background_section ( filein );
        break;
      case 0x0100:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "    MASTER_SCALE section tag of %0X\n", temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0x3d3e:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "    MESH_VERSION section tag of %0X\n", temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0xafff:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "    MAT_ENTRY section tag of %0X\n", temp_int );
        }
        teller = teller + tds_read_material_section ( filein );
        break;
      case 0x4000:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "    NAMED_OBJECT section tag of %0X\n", temp_int );
        }
        teller = teller + tds_read_object_section ( filein );
        break;
      case 0x7001:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "    VIEWPORT_LAYOUT section tag of %0X\n", 
            temp_int );
        }
        teller = teller + tds_read_view_section ( filein, views_read );
        break;
      case 0x7012:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "    VIEWPORT_DATA_3 section tag of %0X\n", temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0x7011:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "    VIEWPORT_DATA section tag of %0X\n", temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0x7020:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "    VIEWPORT_SIZE section tag of %0X\n", temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      default:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "    Junk.\n" );
        }
        break;
    }
 
    if ( teller >= chunk_length ) {
      end_found = TRUE;
    }

  }
 
  fseek ( filein, current_pointer+chunk_length, SEEK_SET );

  return ( chunk_length );
}
/******************************************************************************/

float converter::tds_read_float ( FILE *filein ) {

/******************************************************************************/

/*
  Note that we may need to reverse the bytes on floating point values!
*/
  float rval;
  float temp;

  fread ( &temp, sizeof ( float ), 1, filein );

  if ( byte_swap == TRUE ) {
    rval = reverse_bytes_float ( temp );
  }
  else {
    rval = temp;
  }

  return rval;
}
/******************************************************************************/

unsigned long converter::tds_read_keyframe_section ( FILE *filein, int *views_read ) {

/******************************************************************************/

  unsigned long int   current_pointer;
  unsigned char       end_found = FALSE;
  unsigned short int  temp_int;
  unsigned long int   temp_pointer;
  unsigned long int   teller;
 
  current_pointer = ftell ( filein ) - 2;
  temp_pointer = tds_read_u_long_int ( filein );
  teller = 6;
 
  while ( end_found == FALSE ) {

    temp_int = tds_read_u_short_int ( filein );
    teller = teller + 2;

    switch ( temp_int ) {
      case 0x7001:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "    VIEWPORT_LAYOUT main definition section tag of %0X\n",
            temp_int );
        }
        teller = teller + tds_read_view_section ( filein, views_read );
        break;
      case 0xb008:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "    KFSEG frames section tag of %0X\n", temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0xb002:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "    OBJECT_NODE_TAG object description section tag of %0X\n",
            temp_int);
        }
        teller = teller + tds_read_keyframe_objdes_section ( filein );
        break;
      case 0xb009:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "    KFCURTIME section tag of %0X\n", temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0xb00a:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "    KFHDR section tag of %0X\n", temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      default: 
        break;
    }
 
    if ( teller >= temp_pointer ) {
      end_found = TRUE;
    }

  }
 
  fseek ( filein, current_pointer+temp_pointer, SEEK_SET );

  return ( temp_pointer );
}
/******************************************************************************/

unsigned long converter::tds_read_keyframe_objdes_section ( FILE *filein ) {

/******************************************************************************/

/*
  Modified:

    21 September 1998
*/
  unsigned long int   chunk_size;
  unsigned long int   current_pointer;
  unsigned char       end_found = FALSE;
  unsigned short int  temp_int;
  unsigned long int   temp_pointer;
  unsigned long int   teller;
  unsigned long int   u_long_int_val;
  unsigned short int  u_short_int_val;
 
  current_pointer = ftell ( filein ) - 2;
  temp_pointer = tds_read_u_long_int ( filein );
  teller = 6;
 
  while ( end_found == FALSE ) {

    temp_int = tds_read_u_short_int ( filein );
    teller = teller + 2;

    switch ( temp_int ) {
      case 0xb011:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "      INSTANCE_NAME section tag of %0X\n", temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0xb010:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "      NODE_HDR section tag of %0X\n", temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0xb020:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "      POS_TRACK_TAG section tag of %0X\n", temp_int );
        }
        chunk_size = tds_read_u_long_int ( filein );
        u_short_int_val = tds_read_u_short_int ( filein );
        u_short_int_val = tds_read_u_short_int ( filein );
        u_short_int_val = tds_read_u_short_int ( filein );
        u_short_int_val = tds_read_u_short_int ( filein );
        u_short_int_val = tds_read_u_short_int ( filein );
        u_short_int_val = tds_read_u_short_int ( filein );
        u_short_int_val = tds_read_u_short_int ( filein );
        u_short_int_val = tds_read_u_short_int ( filein );
        u_long_int_val = tds_read_u_long_int ( filein );
        origin[0] = tds_read_float ( filein );
        origin[1] = tds_read_float ( filein );
        origin[2] = tds_read_float ( filein );
        teller = teller + 32;
        break;
      case 0xb013:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "      PIVOT section tag of %0X\n", temp_int );
        }
        chunk_size = tds_read_u_long_int ( filein );
        pivot[0] = tds_read_float ( filein );
        pivot[1] = tds_read_float ( filein );
        pivot[2] = tds_read_float ( filein );
        teller = teller + 12;
        break;
      case 0xb014:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "      BOUNDBOX section tag of %0X\n", temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0xb015:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "      MORPH_SMOOTH section tag of %0X\n", temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0xb021:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "      ROT_TRACK_TAG section tag of %0X\n", temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0xb022:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "      SCL_TRACK_TAG section tag of %0X\n", temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0xb030:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "      NODE_ID section tag of %0X\n", temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      default: 
        break;
    }
 
    if ( teller >= temp_pointer ) {
      end_found = TRUE;
    }

  }
 
  fseek ( filein, current_pointer+temp_pointer, SEEK_SET );

  return ( temp_pointer );
}
/******************************************************************************/

unsigned long converter::tds_read_light_section ( FILE *filein ) {

/******************************************************************************/

  unsigned char       boolean;
  unsigned long int   current_pointer;
  unsigned char       end_found = FALSE;
  int                 i;
  float               light_coors[3];
  float               rgb_val[3];
  unsigned long int   teller;
  unsigned short int  temp_int;
  unsigned long int   temp_pointer;
  unsigned char       true_c_val[3];
 
  current_pointer = ftell ( filein ) - 2;
  temp_pointer = tds_read_u_long_int ( filein );
  teller = 6;

  light_coors[0] = tds_read_float ( filein );
  light_coors[1] = tds_read_float ( filein );
  light_coors[2] = tds_read_float ( filein );

  teller = teller + 3 * 4;
 
  if ( debug == TRUE ) {
    fprintf ( logfile,  "     Found light at coordinates XYZ = %f %f %f.\n",
      light_coors[0], light_coors[1], light_coors[2] );
  }
 
  while ( end_found == FALSE ) {

    temp_int = tds_read_u_short_int ( filein );
    teller = teller + 2;
 
    switch ( temp_int ) {
      case 0x0010:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "      COLOR_F RGB color definition section tag of %0X\n", 
            temp_int );
        }
        for ( i = 0; i < 3; i++ ) {
          rgb_val[i] = tds_read_float ( filein );
        }
        teller = teller + 3 * sizeof ( float );
        break;
      case 0x0011:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "      COLOR_24 24 bit color definition section tag of %0X\n",
            temp_int );
        }

        for ( i = 0; i < 3; i++ ) {
          true_c_val[i] = fgetc ( filein );
        }
        teller = teller + 3;
        break;
      case 0x4620:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "      DL_OFF section: %0X\n", temp_int );
        }
        teller = teller + tds_read_boolean ( &boolean, filein );
        if ( debug == TRUE ) {
          if ( boolean == TRUE ) {
            fprintf ( logfile,  "      Light is on\n" );
          }
          else {
            fprintf ( logfile,  "      Light is off\n" );
          }
        }
        break;
      case 0x4610:
        if ( debug == TRUE  ) {
          fprintf ( logfile,  "      DL_SPOTLIGHT section tag of %0X\n", temp_int );
        }
        teller = teller + tds_read_spot_section ( filein );
        break;
      case 0x465a:
        if ( debug == TRUE  ) {
          fprintf ( logfile,  "      DL_OUTER_RANGE section tag of %0X\n", temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      default:
        break;
    }
 
    if ( teller >= temp_pointer ) {
      end_found = TRUE;
    }

  }
 
  fseek ( filein, current_pointer+temp_pointer, SEEK_SET );
 
  return ( temp_pointer );
}
/******************************************************************************/

unsigned long int converter::tds_read_u_long_int ( FILE *filein ) {

/******************************************************************************/

/*
  Modified:
 
    01 October 1998

  Author:

    John Burkardt
*/

  union {
    unsigned long int yint;
    char ychar[4];
  } y;

  if ( byte_swap == TRUE ) {
    y.ychar[3] = fgetc ( filein );
    y.ychar[2] = fgetc ( filein );
    y.ychar[1] = fgetc ( filein );
    y.ychar[0] = fgetc ( filein );
  }
  else {
    y.ychar[0] = fgetc ( filein );
    y.ychar[1] = fgetc ( filein );
    y.ychar[2] = fgetc ( filein );
    y.ychar[3] = fgetc ( filein );
  }

  return y.yint;
}
/******************************************************************************/

int converter::tds_read_long_name ( FILE *filein ) {

/******************************************************************************/

  unsigned char  letter;
  unsigned int   teller;

  teller = 0;
  letter = fgetc ( filein );
/*
  Could be a dummy object. 
*/
  if ( letter == 0 ) {
    strcpy ( temp_name, "Default_name" );
    return -1; 
  }

  temp_name[teller] = letter;
  teller = teller + 1;
 
  do {
    letter = fgetc ( filein );
    temp_name[teller] = letter;
    teller = teller + 1;
  } while ( letter != 0 );
 
  temp_name[teller-1] = 0;
 
  if ( debug == TRUE ) {
    fprintf ( logfile,  "      tds_read_long_name found name: %s.\n", temp_name );
  }

  return teller;
}
/******************************************************************************/

unsigned long converter::tds_read_matdef_section ( FILE *filein ) {

/******************************************************************************/

  unsigned long int  current_pointer;
  unsigned int       teller;
  unsigned long int  temp_pointer;
 
  current_pointer = ftell ( filein ) - 2;
  temp_pointer = tds_read_u_long_int ( filein );
 
  teller = tds_read_long_name ( filein );

  if ( teller == (unsigned int)-1 ) {
    if ( debug == TRUE ) {
      fprintf ( logfile,  "      No material name found.\n" );
    }
  }
  else {
    strcpy ( mat_name, temp_name );
    if ( debug == TRUE ) {
      fprintf ( logfile,  "      Material name %s.\n", mat_name );
    }
  }
 
  fseek ( filein, current_pointer+temp_pointer, SEEK_SET );

  return ( temp_pointer );
}
/******************************************************************************/

unsigned long converter::tds_read_material_section ( FILE *filein ) {

/******************************************************************************/

  unsigned long int   current_pointer;
  unsigned char       end_found = FALSE;
  unsigned short int  temp_int;
  unsigned long int   temp_pointer;
  unsigned long int   teller;
 
  current_pointer = ftell ( filein ) - 2;

  temp_pointer = tds_read_u_long_int ( filein );
  teller = 6;
 
  while ( end_found == FALSE ) {

    temp_int = tds_read_u_short_int ( filein );
    teller = teller + 2;
 
    switch ( temp_int ) {

      case 0xa000:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "     MAT_NAME definition section tag of %0X\n", 
            temp_int );
        }
        teller = teller + tds_read_matdef_section ( filein );
        break;
      case 0xa010:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "     MAT_AMBIENT definition section tag of %0X\n", 
            temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0xa020:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "     MAT_DIFFUSE definition section tag of %0X\n", 
            temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0xa030:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "     MAT_SPECULAR definition section tag of %0X\n", 
            temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0xa040:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "     MAT_SHININESS definition section tag of %0X\n", 
            temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0xa041:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "     MAT_SHIN2PCT definition section tag of %0X\n", 
            temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0xa042:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "     MAT_SHIN3PCT definition section tag of %0X\n", 
            temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0xa050:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "     MAT_TRANSPARENCY definition section tag of %0X\n", 
            temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0xa052:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "     MAT_XPFALL definition section tag of %0X\n", 
            temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0xa053:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "     MAT_REFBLUR definition section tag of %0X\n", 
            temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0xa080:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "     MAT_SELF_ILLUM definition section tag of %0X\n", 
            temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0xa081:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "     MAT_TWO_SIDE definition section tag of %0X\n", 
            temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0xa082:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "     MAT_DECAL definition section tag of %0X\n", 
            temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0xa083:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "     MAT_ADDITIVE definition section tag of %0X\n", 
            temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0xa084:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "     MAT_SELF_ILPCT definition section tag of %0X\n", 
            temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0xa085:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "     MAT_WIRE definition section tag of %0X\n", 
            temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0xa086:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "     MAT_SUPERSMP definition section tag of %0X\n", 
            temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0xa087:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "     MAT_WIRESIZE definition section tag of %0X\n", 
            temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0xa088:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "     MAT_FACEMAP definition section tag of %0X\n", 
            temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0xa08a:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "     MAT_XPFALLIN definition section tag of %0X\n", 
            temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0xa08c:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "     MAT_PHONGSOFT definition section tag of %0X\n", 
            temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0xa08e:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "     MAT_WIREABS definition section tag of %0X\n", 
            temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0xa100:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "     MAT_SHADING definition section tag of %0X\n", 
            temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0xa200:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "     MAT_TEXMAP definition section tag of %0X\n", 
            temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0xa204:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "     MAT_SPECMAP definition section tag of %0X\n", 
            temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0xa210:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "     MAT_OPACMAP definition section tag of %0X\n", 
            temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0xa220:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "     MAT_REFLMAP definition section tag of %0X\n", 
            temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0xa230:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "     MAT_BUMPMAP definition section tag of %0X\n", 
            temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0xa353:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "     MAT_MAP_TEXBLUR definition section tag of %0X\n", 
            temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      default:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "  Junk section tag of %0X\n", temp_int );
        }
        break;
    }
 
    if ( teller >= temp_pointer ) {
      end_found = TRUE;
    }

  }
 
  fseek ( filein, current_pointer+temp_pointer, SEEK_SET );

  return ( temp_pointer );
}
/******************************************************************************/

int converter::tds_read_name ( FILE *filein ) {

/******************************************************************************/

  unsigned char  letter;
  unsigned int   teller;

  teller = 0;
  letter = fgetc ( filein );
/*
  Could be a dummy object.  
*/

  if ( letter == 0 ) {
    strcpy ( temp_name, "Default name" );
    return (-1); 
  }

  temp_name[teller] = letter;
  teller = teller + 1;
 
  do {
    letter = fgetc ( filein );
    temp_name[teller] = letter;
    teller = teller + 1;
  } while ( ( letter != 0 ) && ( teller < 12 ) );
 
  temp_name[teller-1] = 0;
 
  if ( debug == TRUE ) {
    fprintf ( logfile,  "      tds_read_name found name: %s.\n", temp_name );
  }

  return 0;
}
/******************************************************************************/

unsigned long converter::tds_read_obj_section ( FILE *filein ) {

/******************************************************************************/

/*
  Comments:

    Thanks to John F Flanagan for some suggested corrections.

  Modified:

    27 April 1999
*/
  unsigned long int   chunk_size;
  unsigned short int  color_index;
  unsigned long int   current_pointer;
  unsigned char       end_found = FALSE;
  int                 i;
  int                 j;
  int                 num_cor3_prev = 0;
  int                 num_cor3_base;
  int                 num_cor3_inc;
  int                 num_face_inc;
  unsigned long int   u_long_int_val;
  unsigned short int  temp_int;
  unsigned long int   temp_pointer;
  unsigned long int   temp_pointer2;
  unsigned long int   teller; 
 
  current_pointer = ftell ( filein ) - 2;
  temp_pointer = tds_read_u_long_int ( filein );
  teller = 6;
  num_cor3_base = num_cor3;

  while ( end_found == FALSE ) {

    temp_int = tds_read_u_short_int ( filein );
    teller = teller + 2;

    switch ( temp_int ) {

      case 0x4000:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "        NAMED_OBJECT section tag of %0X\n", 
            temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;

      case 0x4100:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "        N_TRI_OBJECT section tag of %0X\n", 
            temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;

      case 0x4110:

        if ( debug == TRUE ) {
          fprintf ( logfile,  "        POINT_ARRAY section tag of %0X\n", temp_int );
        }

        current_pointer = ftell ( filein ) - 2;
        temp_pointer2 = tds_read_u_long_int ( filein );
        num_cor3_inc =  ( int ) tds_read_u_short_int ( filein );
 
        for ( i = num_cor3; i < num_cor3 + num_cor3_inc; i++ ) {
          cor3[0][i] = tds_read_float ( filein );
          cor3[1][i] = tds_read_float ( filein );
          cor3[2][i] = tds_read_float ( filein );
        }
 
	num_cor3_prev = num_cor3;
        num_cor3 = num_cor3 + num_cor3_inc;
        teller = teller + temp_pointer2;
        break;

      case 0x4111:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "        POINT_FLAG_ARRAY faces (2) section tag of %0X\n", 
            temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;

      case 0x4120:

        if ( debug == TRUE ) {
          fprintf ( logfile,  "        FACE_ARRAY section tag of %0X\n", 
            temp_int );
        }

        temp_pointer2 = tds_read_u_long_int ( filein );
        num_face_inc = ( int ) tds_read_u_short_int ( filein );
 
        for ( i = num_face; i < num_face + num_face_inc; i++ ) {
	  // reverse winding of polygons for proper baackface culling
          face[2][i] = tds_read_u_short_int ( filein ) + num_cor3_base;
          face[1][i] = tds_read_u_short_int ( filein ) + num_cor3_base;
          face[0][i] = tds_read_u_short_int ( filein ) + num_cor3_base;
          face_order[i] = 3;
          face_flags[i] = tds_read_u_short_int ( filein );
        }

        temp_int = tds_read_u_short_int ( filein );
        if ( temp_int == 0x4150 ) {
          for ( i = num_face; i < num_face + num_face_inc; i++ ) {
            face_smooth[i] = tds_read_u_long_int ( filein ) + num_cor3_base;
          }
        }
        num_face = num_face + num_face_inc;
        teller = ftell ( filein );
        break;

      case 0x4130:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "        MSH_MAT_GROUP section tag of %0X\n", 
            temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;

      case 0x4140:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "        TEX_VERTS section tag of %0X\n", 
            temp_int );
        }
        temp_pointer2 = tds_read_u_long_int ( filein );
        num_cor3_inc =  ( int ) tds_read_u_short_int ( filein );
	for (i = num_cor3_prev; i < num_cor3_prev+num_cor3_inc; i++) {
	  cor3_uv[0][i] = 1-tds_read_float( filein );
	  cor3_uv[1][i] = 1-tds_read_float( filein );
	}
	teller = teller + temp_pointer2;

//        teller = teller + tds_read_unknown_section ( filein );
        break;

      case 0x4150:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "        SMOOTH_GROUP section tag of %0X\n", 
            temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;

      case 0x4160:

        if ( debug == TRUE ) {
          fprintf ( logfile,  "        MESH_MATRIX section tag of %0X\n", 
            temp_int );
        }

        u_long_int_val = tds_read_u_long_int ( filein );

        for ( j = 0; j < 4; j++ ) {
          for ( i = 0; i < 3; i++ ) {
            transform_mat[j][i] = tds_read_float ( filein );
          }
        }
        transform_mat[0][3] = 0.0;
        transform_mat[1][3] = 0.0;
        transform_mat[2][3] = 0.0;
        transform_mat[3][3] = 0.0;

        teller = teller + 12 * sizeof ( float );
        break;

      case 0x4165:

        if ( debug == TRUE ) {
          fprintf ( logfile,  "        MESH_COLOR section tag of %0X\n", temp_int );
        }

        chunk_size = tds_read_u_long_int ( filein );

        if ( chunk_size == 7 ) {
          color_index = fgetc ( filein );
          teller = teller + 5;
        }
        else {
          color_index = tds_read_u_short_int ( filein );
          teller = teller + 6;
        } 

        break;

      case 0x4170:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "        MESH_TEXTURE_INFO section tag of %0X\n", 
            temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;

      default:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "        JUNK section tag of %0X\n", temp_int );
        }
        break;
    }
 
    if (  teller >= temp_pointer ) {
      end_found = TRUE;
    }

  }
 
  fseek ( filein, current_pointer+temp_pointer, SEEK_SET );

  return ( temp_pointer );
}
/******************************************************************************/

unsigned long converter::tds_read_object_section ( FILE *filein ) {

/******************************************************************************/

  unsigned char       end_found = FALSE;
  unsigned long int   current_pointer;
  int                 int_val;
  unsigned short int  temp_int;
  unsigned long int   temp_pointer;
  unsigned long int   teller;
 
  current_pointer = ftell ( filein ) - 2;
  temp_pointer = tds_read_u_long_int ( filein );
  teller = 6;
/*
  Why don't you read and save the name here?
*/
  int_val = tds_read_name ( filein );

  if ( int_val == -1 ) {
    if ( debug == TRUE ) {
      fprintf ( logfile,  "      Dummy Object found\n" );
    }
  }
  else {
    strcpy ( object_name, temp_name );
  }
 
  while ( end_found == FALSE ) {

    temp_int = tds_read_u_short_int ( filein );
    teller = teller + 2;

    switch ( temp_int ) {
      case 0x4700:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "      N_CAMERA section tag of %0X\n", temp_int );
        }
        teller = teller + tds_read_camera_section ( filein );
        break;
      case 0x4600:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "      N_DIRECT_LIGHT section tag of %0X\n", temp_int );
        }
        teller = teller + tds_read_light_section ( filein );
        break;
      case 0x4100:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "      OBJ_TRIMESH section tag of %0X\n", temp_int );
        }
        teller = teller + tds_read_obj_section ( filein );
        break;
      case 0x4010: 
        if ( debug == TRUE ) {
          fprintf ( logfile,  "      OBJ_HIDDEN section tag of %0X\n", temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0x4012: 
        if ( debug == TRUE ) {
          fprintf ( logfile,  "      OBJ_DOESNT_CAST section tag of %0X\n", temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      default:
        break;
    }
 
    if ( teller >= temp_pointer ) {
      end_found = TRUE;
    }

  }
 
  fseek ( filein, current_pointer+temp_pointer, SEEK_SET );

  return ( temp_pointer );
}
/******************************************************************************/

unsigned short int converter::tds_read_u_short_int ( FILE *filein ) {

/******************************************************************************/

  unsigned char  c1;
  unsigned char  c2;
  short int      ival;

  c1 = fgetc ( filein );
  c2 = fgetc ( filein );

  ival = c1 | ( c2 << 8 );

  return ival;
}
/******************************************************************************/

unsigned long converter::tds_read_spot_section ( FILE *filein ) {

/******************************************************************************/

  unsigned long int  current_pointer;
  float              falloff;
  float              hotspot;
  float              target[4];
  unsigned long int  temp_pointer;

  current_pointer = ftell ( filein ) - 2;
  temp_pointer = tds_read_u_long_int ( filein );
 
  target[0] = tds_read_float ( filein );
  target[1] = tds_read_float ( filein );
  target[2] = tds_read_float ( filein );
  hotspot = tds_read_float ( filein );
  falloff = tds_read_float ( filein );
 
  if ( debug == TRUE ) {
    fprintf ( logfile,  "      The target of the spot is XYZ = %f %f %f.\n",
      target[0], target[1], target[2] );
    fprintf ( logfile,  "      The hotspot of this light is %f.\n", hotspot );
    fprintf ( logfile,  "      The falloff of this light is %f.\n", falloff );
  }
 
  fseek ( filein, current_pointer+temp_pointer, SEEK_SET );

  return ( temp_pointer );
}
/******************************************************************************/

unsigned long converter::tds_read_unknown_section ( FILE *filein ) {

/******************************************************************************/

  unsigned long int  current_pointer;
  unsigned long int  temp_pointer;
 
  current_pointer = ftell ( filein ) - 2;
  temp_pointer = tds_read_u_long_int ( filein );
 
  fseek ( filein, current_pointer+temp_pointer, SEEK_SET );

  return ( temp_pointer );
}
/******************************************************************************/

unsigned long converter::tds_read_view_section ( FILE *filein, int *views_read ) {

/******************************************************************************/

  unsigned long int   current_pointer;
  unsigned char       end_found = FALSE;
  unsigned short int  temp_int;
  unsigned long int   temp_pointer;
  unsigned long int   teller;
 
  current_pointer = ftell ( filein ) - 2;
  temp_pointer = tds_read_u_long_int ( filein );
  teller = 6;
 
  while ( end_found == FALSE ) {

    temp_int = tds_read_u_short_int ( filein );
    teller = teller + 2;

    switch ( temp_int ) {
      case 0x7012:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "     VIEWPORT_DATA_3 section tag of %0X\n", temp_int );
        }
        teller = teller + tds_read_vp_section ( filein, views_read );
        break;
      case 0x7011:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "     VIEWPORT_DATA section tag of %0X\n", temp_int );
        }
        teller = teller + tds_read_unknown_section ( filein );
        break;
      case 0x7020:
        if ( debug == TRUE ) {
          fprintf ( logfile,  "     VIEWPORT_SIZE section tag of %0X\n", temp_int );
        }
        teller = teller + tds_read_vp_section ( filein, views_read );
        break;
      default:
        break;
    }
 
    if ( teller >= temp_pointer ) {
      end_found = TRUE;
    }
 
    if ( *views_read > 3 ) {
      end_found = TRUE;
    }
  }
 
  fseek ( filein, current_pointer+temp_pointer, SEEK_SET );

  return ( temp_pointer );
}
/******************************************************************************/

unsigned long converter::tds_read_vp_section ( FILE *filein, int *views_read ) {

/******************************************************************************/

  unsigned int       attribs;
  unsigned long int  current_pointer;
  int                i;
  int                int_val;
  unsigned int       port;
  unsigned long int  temp_pointer;
  char              *viewports[11] = {
                      "Bogus",
                      "Top",
                      "Bottom",
                      "Left",
                      "Right",
                      "Front",
                      "Back",
                      "User",
                      "Camera",
                      "Light",
                      "Disabled"
                     };

  *views_read = *views_read + 1;
 
  current_pointer = ftell ( filein ) - 2;
  temp_pointer = tds_read_u_long_int ( filein );
 
  attribs = tds_read_u_short_int ( filein );

  if ( attribs == 3 ) {
    if ( debug == TRUE ) {
      fprintf ( logfile,  "<Snap> active in viewport.\n" );
    }
  }

  if ( attribs == 5 ) {
    if ( debug == TRUE ) {
      fprintf ( logfile,  "<Grid> active in viewport.\n" );
    }
  }
/* 
  Read 5 INTS to get to the viewport information. 
*/
  for ( i = 1; i < 6; i++ ) {
    tds_read_u_short_int ( filein ); 
  }

  port = tds_read_u_short_int ( filein );
/*
  Find camera section. 
*/
  if ( ( port == 0xffff ) || ( port == 0 ) ) {

    for ( i = 0; i < 12; i++ ) {
      tds_read_u_short_int ( filein );
    }
 
    int_val = tds_read_name (filein );
 
    if ( int_val == -1 ) {
      if ( debug == TRUE ) {
        fprintf ( logfile,  "      No Camera name found\n" );
      }
    }

    port = 0x0008;
  }
 
  if ( debug == TRUE ) {
    fprintf ( logfile,  "Reading [%s] information with tag:%d\n", viewports[port], port );
  }
 
  fseek ( filein, current_pointer+temp_pointer, SEEK_SET ); 

  return ( temp_pointer );
}
/******************************************************************************/

int converter::tds_write ( FILE *fileout ) {

/******************************************************************************/

/*
  Purpose:

    TDS_WRITE writes graphics information to a 3D Studio Max 3DS file.

  Modified:

    14 October 1998

  Author:

    John Burkardt

*/
  float               float_val;
  int                 i;
  int                 icor3;
  int                 iface;
  int                 j;
  long int            l0002;
  long int            l0100;
  long int            l3d3d;
  long int            l3d3e;
  long int            l4000;
  long int            l4100;
  long int            l4110;
  long int            l4120;
  long int            l4150;
  long int            l4160;
  long int            l4d4d;
  long int            lb000;
  long int            lb002;
  long int            lb00a;
  long int            lb008;
  long int            lb009;
  long int            lb010;
  long int            lb013;
  long int            lb020;
  long int            lb021;
  long int            lb022;
  long int            lb030;
  long int            long_int_val;
  int                 name_length;
  int                 num_bytes;
  short int           short_int_val;
  unsigned short int  u_short_int_val;

  num_bytes = 0;
  name_length = strlen ( object_name );

  l0002 = 10;

  l4150 = 2 + 4 + num_face * 4;
  l4120 = 2 + 4 + 2 + 4 * num_face * 2 + l4150;
  l4160 = 2 + 4 + 4 * 12;
  l4110 = 2 + 4 + 2 + num_cor3 * 3 * 4;
  l4100 = 2 + 4 + l4110 + l4160 + l4120;
  l4000 = 2 + 4 + ( name_length + 1 ) + l4100;
  l0100 = 2 + 4 + 4;
  l3d3e = 2 + 4 + 4;
  l3d3d = 2 + 4 + l3d3e + l0100 + l4000;

  lb022 = 2 + 4 + 32;
  lb021 = 2 + 4 + 9 * 4;
  lb020 = 2 + 4 + 8 * 4;
  lb013 = 2 + 4 + 6 * 2;
  lb010 = 2 + 4 + ( name_length + 1 ) + 3 * 2;
  lb030 = 2 + 4 + 2;
  lb002 = 2 + 4 + lb030 + lb010 + lb013 + lb020 + lb021 + lb022;
  lb009 = 2 + 4 + 4;
  lb008 = 2 + 4 + 2 * 4;
  lb00a = 2 + 4 + 2 + 9 + 2 * 2;
  lb000 = 2 + 4 + lb00a + lb008 + lb009 + lb002;

  l4d4d = 2 + 4 + l0002 + l3d3d + lb000;
/*  
  M3DMAGIC begin.
    tag, size.
*/
  short_int_val = ( short ) 0x4d4d;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  num_bytes = num_bytes + tds_write_long_int ( fileout, l4d4d );
/*
  M3D_VERSION begin.
    tag, size, version.
*/
  short_int_val = ( short ) 0x0002;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  num_bytes = num_bytes + tds_write_long_int ( fileout, l0002 );
  long_int_val = 3;
  num_bytes = num_bytes + tds_write_long_int ( fileout, long_int_val );
/*  
  M3D_VERSION end.
  MDATA begin.
    tag, size.
*/
  short_int_val = ( short ) 0x3d3d;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  num_bytes = num_bytes + tds_write_long_int ( fileout, l3d3d );
/*  
  MESH_VERSION begin.
    tag, size, version.
*/
  short_int_val = ( short ) 0x3d3e;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  num_bytes = num_bytes + tds_write_long_int ( fileout, l3d3e );
  long_int_val = 3;
  num_bytes = num_bytes + tds_write_long_int ( fileout, long_int_val );
/*  
  MESH_VERSION end.  
  MASTER_SCALE begin.  
    tag, size, scale.
*/
  short_int_val = ( short ) 0x0100;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  num_bytes = num_bytes + tds_write_long_int ( fileout, l0100 );
  float_val = 1.0;
  num_bytes = num_bytes + tds_write_float ( fileout, float_val );
/*
  MASTER_SCALE end. 
  NAMED_OBJECT begin. 
    tag, size, name. 
*/
  short_int_val = ( short ) 0x4000;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  num_bytes = num_bytes + tds_write_long_int ( fileout, l4000 );
  num_bytes = num_bytes + tds_write_string ( fileout, object_name );
/*  
  N_TRI_OBJECT begin.  
    tag, size.
*/
  short_int_val = ( short ) 0x4100;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  num_bytes = num_bytes + tds_write_long_int ( fileout, l4100 );
/*
  POINT_ARRAY begin.  
    tag, size, number of points, coordinates of points.
  Warning! number of points could exceed a short!
*/
  short_int_val = ( short ) 0x4110;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  num_bytes = num_bytes + tds_write_long_int ( fileout, l4110 );

  u_short_int_val = ( unsigned short ) num_cor3;
  num_bytes = num_bytes + tds_write_u_short_int ( fileout, u_short_int_val );

  for ( icor3 = 0; icor3 < num_cor3; icor3++ ) {
    for ( j = 0; j < 3; j++ ) {
      num_bytes = num_bytes + tds_write_float ( fileout, cor3[j][icor3] );
    }
  }
/*
  POINT_ARRAY end.
  MESH_MATRIX begin.  
    tag, size, 4 by 3 matrix.
*/
  short_int_val = ( short ) 0x4160;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  num_bytes = num_bytes + tds_write_long_int ( fileout, l4160 );

  for ( i = 0; i < 4; i++ ) {
    for ( j = 0; j < 3; j++ ) {
      float_val = transform_mat[i][j];
      num_bytes = num_bytes + tds_write_float ( fileout, float_val );
    }
  }
/*
  MESH_MATRIX end.  
  FACE_ARRAY begin. 
    tag, size, number of faces, nodes per face. 
  Warning: number of faces could exceed a short!
*/
  short_int_val = ( short ) 0x4120;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  num_bytes = num_bytes + tds_write_long_int ( fileout, l4120 );

  u_short_int_val = ( unsigned short ) num_face;
  num_bytes = num_bytes + tds_write_u_short_int ( fileout, u_short_int_val );

  for ( iface = 0; iface < num_face; iface++ ) {
    for ( j = 0; j < 3; j++ ) {
      short_int_val = face[j][iface];
      num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
    }
    short_int_val = face_flags[iface];
    num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  }
/*
  SMOOTH_GROUP begin.
    tag, size, group for each face.
*/
  short_int_val = ( short ) 0x4150;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  num_bytes = num_bytes + tds_write_long_int ( fileout, l4150 );

  for ( iface = 0; iface < num_face; iface++ ) {
    long_int_val = face_smooth[iface];
    num_bytes = num_bytes + tds_write_long_int ( fileout, long_int_val );
  }
/*
  SMOOTH_GROUP end.
  FACE_ARRAY end.
  N_TRI_OBJECT end.
  NAMED_OBJECT end.
  MDATA end. 
  KFDATA begin.
*/
  u_short_int_val = 0xb000;
  num_bytes = num_bytes + tds_write_u_short_int ( fileout, u_short_int_val );
  num_bytes = num_bytes + tds_write_long_int ( fileout, lb000 );
/*
  KFHDR begin.  
    tag, size, revision, filename, animlen.
*/
  u_short_int_val = 0xb00a;
  num_bytes = num_bytes + tds_write_u_short_int ( fileout, u_short_int_val );
  num_bytes = num_bytes + tds_write_long_int ( fileout, lb00a );
  short_int_val = 5;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  num_bytes = num_bytes + tds_write_string ( fileout, "MAXSCENE" );
  short_int_val = 100;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  short_int_val = 0;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
/*
  KFHDR end.  
  KFSEG begin.  
    tag, size, start, end.
*/
  u_short_int_val = 0xb008;
  num_bytes = num_bytes + tds_write_u_short_int ( fileout, u_short_int_val );
  num_bytes = num_bytes + tds_write_long_int ( fileout, lb008 );
  long_int_val = 0;
  num_bytes = num_bytes + tds_write_long_int ( fileout, long_int_val );
  long_int_val = 100;
  num_bytes = num_bytes + tds_write_long_int ( fileout, long_int_val );
/*
  KFSEG end.  
  KFCURTIME begin.
    tag, size, current_frame.
*/
  u_short_int_val = 0xb009;
  num_bytes = num_bytes + tds_write_u_short_int ( fileout, u_short_int_val );
  num_bytes = num_bytes + tds_write_long_int ( fileout, lb009 );
  long_int_val = 0;
  num_bytes = num_bytes + tds_write_long_int ( fileout, long_int_val );
/*
  KFCURTIME end.
  OBJECT_NODE_TAG begin.
    tag, size.  
*/
  u_short_int_val = 0xb002;
  num_bytes = num_bytes + tds_write_u_short_int ( fileout, u_short_int_val );
  num_bytes = num_bytes + tds_write_long_int ( fileout, lb002 );
/*
  NODE_ID begin.
    tag, size, id.
*/
  u_short_int_val = 0xb030;
  num_bytes = num_bytes + tds_write_u_short_int ( fileout, u_short_int_val );
  num_bytes = num_bytes + tds_write_long_int ( fileout, lb030 );
  short_int_val = 0;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
/*
  NODE_ID end.  
  NODE_HDR begin. 
    tag, size, object_name, flag1, flag2, hierarchy.
*/
  u_short_int_val = 0xb010;
  num_bytes = num_bytes + tds_write_u_short_int ( fileout, u_short_int_val );
  num_bytes = num_bytes + tds_write_long_int ( fileout, lb010 );
  num_bytes = num_bytes + tds_write_string ( fileout, object_name );
  short_int_val = 16384;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  short_int_val = 0;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  short_int_val = -1;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
/*
  NODE_HDR end. 
  PIVOT begin. 
    tag, size, pivot_x, pivot_y, pivot_z.
*/
  u_short_int_val = 0xb013;
  num_bytes = num_bytes + tds_write_u_short_int ( fileout, u_short_int_val );
  num_bytes = num_bytes + tds_write_long_int ( fileout, lb013 );
  for ( i = 0; i < 3; i++ ) {
    float_val = pivot[i];
    num_bytes = num_bytes + tds_write_float ( fileout, float_val );
  }
/*
  PIVOT end. 
  POS_TRACK_TAG begin.  
    tag, size, flag, i1, i2, i3, i4, i5, i6, frame, l1, pos_x, pos_y, pos_z.
*/
  u_short_int_val = 0xb020;
  num_bytes = num_bytes + tds_write_u_short_int ( fileout, u_short_int_val );
  num_bytes = num_bytes + tds_write_long_int ( fileout, lb020 );
  short_int_val = 0;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  short_int_val = 0;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  short_int_val = 0;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  short_int_val = 0;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  short_int_val = 0;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  short_int_val = 1;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  short_int_val = 0;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  short_int_val = 0;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  long_int_val = 0;
  num_bytes = num_bytes + tds_write_long_int ( fileout, long_int_val );
  for ( i = 0; i < 3; i++ ) {
    float_val = origin[i];
    num_bytes = num_bytes + tds_write_float ( fileout, float_val );
  }
/*
  POS_TRACK_TAG end. 
  ROT_TRACK_TAG begin. 
    tag, size, i1, i2, i3, i4, i5, i6, i7, i8, l1, rad, axis_x, axis_y, axis_z. 
*/
  u_short_int_val = 0xb021;
  num_bytes = num_bytes + tds_write_u_short_int ( fileout, u_short_int_val );
  num_bytes = num_bytes + tds_write_long_int ( fileout, lb021 );
  short_int_val = 0;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  short_int_val = 0;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  short_int_val = 0;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  short_int_val = 0;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  short_int_val = 0;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  short_int_val = 1;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  short_int_val = 0;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  short_int_val = 0;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  long_int_val = 0;
  num_bytes = num_bytes + tds_write_long_int ( fileout, long_int_val );
  float_val = 0.0;
  num_bytes = num_bytes + tds_write_float ( fileout, float_val );
  num_bytes = num_bytes + tds_write_float ( fileout, float_val );
  num_bytes = num_bytes + tds_write_float ( fileout, float_val );
  num_bytes = num_bytes + tds_write_float ( fileout, float_val );
/*
  ROT_TRACK_TAG end. 
  SCL_TRACK_TAG begin.  
    tag, size, i1, i2, i3, i4, i5, i6, i7, i8, l1, scale_x, scale_y, scale_z.
*/
  u_short_int_val = 0xb022;
  num_bytes = num_bytes + tds_write_u_short_int ( fileout, u_short_int_val );
  num_bytes = num_bytes + tds_write_long_int ( fileout, lb022 );
  short_int_val = 0;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  short_int_val = 0;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  short_int_val = 0;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  short_int_val = 0;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  short_int_val = 0;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  short_int_val = 1;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  short_int_val = 0;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  short_int_val = 0;
  num_bytes = num_bytes + tds_write_short_int ( fileout, short_int_val );
  long_int_val = 0;
  num_bytes = num_bytes + tds_write_long_int ( fileout, long_int_val );
  float_val = 1.0;
  num_bytes = num_bytes + tds_write_float ( fileout, float_val );
  num_bytes = num_bytes + tds_write_float ( fileout, float_val );
  num_bytes = num_bytes + tds_write_float ( fileout, float_val );
/*  
  SCL_TRACK_TAG end.
  OBJECT_NODE_TAG end.
  KFDATA end.
  M3DMAGIC end. 
*/

/*
  Report.
*/
  fprintf ( logfile,  "TDS_WRITE wrote %d bytes.\n", num_bytes );

  return SUCCESS;
}
/******************************************************************************/

int converter::tds_write_float ( FILE *fileout, float float_val ) {

/******************************************************************************/

/*
  Modified:

    23 September 1998
*/
  float temp;

  if ( byte_swap == TRUE ) {
    temp = reverse_bytes_float ( float_val );
  }
  else {
    temp = float_val;
  }

  fwrite ( &temp, sizeof ( float ), 1, fileout );

  return 4;
}
/******************************************************************************/

int converter::tds_write_long_int ( FILE *fileout, long int int_val ) {

/******************************************************************************/

/*
  Modified:
 
    14 October 1998

  Author:

    John Burkardt
*/
  union {
    long int yint;
    char ychar[4];
  } y;

  y.yint = int_val;

  if ( byte_swap == TRUE ) {
    fputc ( y.ychar[3], fileout );
    fputc ( y.ychar[2], fileout );
    fputc ( y.ychar[1], fileout );
    fputc ( y.ychar[0], fileout );
  }
  else {
    fputc ( y.ychar[0], fileout );
    fputc ( y.ychar[1], fileout );
    fputc ( y.ychar[2], fileout );
    fputc ( y.ychar[3], fileout );
  }

  return 4;
}
/******************************************************************************/

int converter::tds_write_string ( FILE *fileout, char *string ) {

/******************************************************************************/

/*
  Modified:

    23 September 1998

  Author:

    John Burkardt
*/
  char *c;
  int   nchar;

  nchar = 0;

  for ( c = string; nchar < 12; c++ ) {

    fputc ( *c, fileout );
    nchar = nchar + 1;

    if  ( *c == 0 ) {
      return nchar;
    }

  }

  return nchar;
}
/******************************************************************************/

int converter::tds_write_short_int ( FILE *fileout, short int short_int_val ) {

/******************************************************************************/

/*
  Modified:

    14 October 1998

  Author:

    John Burkardt
*/
  union {
    short int yint;
    char ychar[2];
  } y;

  y.yint = short_int_val;

  if ( byte_swap == TRUE ) {
    fputc ( y.ychar[1], fileout );
    fputc ( y.ychar[0], fileout );
  }
  else {
    fputc ( y.ychar[0], fileout );
    fputc ( y.ychar[1], fileout );
  }

  return 2;
}
/******************************************************************************/

int converter::tds_write_u_short_int ( FILE *fileout, unsigned short int short_int_val ) {

/******************************************************************************/

/*
  Modified:

    14 October 1998

  Author:

    John Burkardt
*/
  union {
    unsigned short int yint;
    char ychar[2];
  } y;

  y.yint = short_int_val;

  if ( byte_swap == TRUE ) {
    fputc ( y.ychar[1], fileout );
    fputc ( y.ychar[0], fileout );
  }
  else {
    fputc ( y.ychar[0], fileout );
    fputc ( y.ychar[1], fileout );
  }

  return 2;
}

/******************************************************************************/

void converter::tds_pre_process ( void ) {

/******************************************************************************/

/*
  Purpose:

    TDS_PRE_PROCESS divides the monolithic object into acceptably small pieces.

  Note:

    The 3DS binary format allows an unsigned short int for the number of
    points, and number of faces in an object.  This limits such quantities
    to 65535.  We have at least one interesting object with more faces
    than that.  So we need to tag faces and nodes somehow.

  Modified:

    14 October 1998

  Author:

    John Burkardt
*/
  
  return;
}
