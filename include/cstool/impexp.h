/*
    Crystal Space utility library: 3D formats import library
    Based on IVCON by John Burkardt, used with permission
    C++ Class and CS interface: Bruce Williams

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

#ifndef __IMPEXP_H__
#define __IMPEXP_H__

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "isys/vfs.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/*
  The "brain.iv" file requires MAX_FACE = 75448, MAX_COR3 = 37608.
*/

// these numbers need to be adjusted

#define MAX_COLOR 1000
#define MAX_COR3 5000
#define MAX_FACE 10000
#define MAX_INCHARS 256
#define MAX_LEVEL 10
#define MAX_LINE 1500
#define MAX_ORDER 10
#define MAX_TEXMAP 5

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

#define ERROR 0
#define SUCCESS 1

//#define MAX(a,b) ( (a)>(b) ? (a) : (b) ) 
//#define MIN(a,b) ( (a)>(b) ? (b) : (a) )

/*
  COR3(3,MAX_COR3), the coordinates of nodes.

  COR3_NORMAL(3,MAX_COR3), normal vectors associated with nodes.

  COR3_RGB(3,MAX_COR3), RGB colors associated with nodes.

  FACE(MAX_ORDER,MAX_FACE) contains the index of the I-th node making up face J.

  FACE_MAT(MAX_ORDER,MAX_FACE); the material of the I-th node making up face J.

  FACE_NORMAL(3,MAX_FACE), the face normal vectors.

  FACE_ORDER(MAX_FACE), the number of vertices per face.

  LINE_DEX(MAX_LINE), node indices, denoting polylines, each terminated by -1.

  LINE_MAT(MAX_LINE), index into RGBCOLOR for line color.

  MAX_COR3, the maximum number of points.

  MAX_FACE, the maximum number of faces.

  MAX_LINE, the maximum number of line definition items.

  MAX_ORDER, the maximum number of vertices per face.

  MAX_TEXMAP, the maximum number of texture maps.

  NUM_COR3, the number of points.

  NUM_FACE, the number of faces.

  NUM_LINE, the number of line definition items.

  VERTEX_NORMAL(3,MAX_ORDER,MAX_FACE), normals at vertices. 

  VERTEX_RGB(3,MAX_ORDER,MAX_FACE), colors associated with vertices. 
*/

// format readers subclass this in order to provide functionality
// for changing frames
class csConverter_FrameManipulator;

class converter  
{
public:
	converter();
	virtual ~converter();
	int ivcon ( const char* input_filename, bool keep_log = true,
	  bool create_output_file = true, const char* output_filename = NULL,
          iVFS* vfs = NULL );

        void ProcessConfig( iConfigFile* config );
	void set_reverse_normals( int yesno );

	// set the current 'frame' of animation; the converter
	// class will update its coordinate data, object names, etc.
	// for whatever frame is specified.  Returns the maximum
	// frame number.  The minimum frame number is 0.
	// There must always exist at least one frame, frame 0
	int set_animation_frame(int framenumber);

protected:
	int comline ( const char* input_filename,
	  bool create_output_file, const char* output_filename );

public:

FILE* logfile;

int    debug;
char   filein_name[81];
char   fileout_name[81];

float  cor3[3][MAX_COR3];
float  cor3_normal[3][MAX_COR3];
float  cor3_rgb[3][MAX_COR3];
float  cor3_uv[2][MAX_COR3];
float  temp_cor3[3][MAX_COR3];
float  temp_cor3_uv[2][MAX_COR3];

int    face[MAX_ORDER][MAX_FACE];
int    face_flags[MAX_FACE];
int    face_mat[MAX_ORDER][MAX_FACE];
int    face_texnode[MAX_ORDER][MAX_FACE];
float  face_normal[3][MAX_FACE];
int    face_object[MAX_FACE];
int    face_order[MAX_FACE];
int    face_smooth[MAX_FACE];
char   input[MAX_INCHARS];
char   levnam[MAX_LEVEL][MAX_INCHARS];
int    line_dex[MAX_LINE];
int    line_mat[MAX_LINE];
int    list[MAX_COR3];
char   mat_name[81];
int    max_order2;
float  normal_temp[3][MAX_ORDER*MAX_FACE];

char   texmap_name[MAX_TEXMAP][81];
float  transform_mat[4][4];
float  vertex_normal[3][MAX_ORDER][MAX_FACE];
float  vertex_uv[2][MAX_ORDER][MAX_FACE];
float  vertex_rgb[3][MAX_ORDER][MAX_FACE];

int    num_bad;
int    num_color;
int    num_comment;
int    num_cor3;
int    num_cor3_uv;
int    temp_num_cor3;
int    temp_num_cor3_uv;
int    num_dup;
int    num_face;
int    num_group;
int    num_line;
int    num_object;
int    num_texmap;
int    num_text;
char   object_name[81];

private:

int    i;
int    iarg;
int    icor3;
int    ierror;
int    iface;
int    ivert;
int    revnorm;

int    byte_swap;
float  origin[3];
float  pivot[3];
float  rgbcolor[3][MAX_COLOR];
char   temp_name[81];

// this member holds the frame data, and knows how to
// properly set all the converter data members whenever we
// switch frames.  It also properly frees frame data when destroyed
csConverter_FrameManipulator *frame_builder;

private:

int                ase_read ( FILE *filein );
int                ase_write ( FILE *fileout );
void               ave_face_normal ( void );
int                char_index_last ( char* string, char c );
int                char_pad ( int *char_index, int *null_index, char *string, int MAX_STRING );

void               cor3_2_vertex_rgb ( void );
void               data_check ( void );
void               data_init ( void );
int                data_read ( void );
void               data_report ( void );
int                data_write ( void );
int                dxf_read ( FILE *filein );
int                dxf_write ( FILE *fileout );
int                face_print ( int iface );
int                face_subset ( void );
char              *file_ext ( char *file_name );
void               hello ( void );
void               help ( void );
int                hrc_read ( FILE *filein );
int                hrc_write ( FILE *fileout );
void               init_program_data ( void );
int                interact ( void );
int                iv_read ( FILE *filein );
int                iv_write ( FILE *fileout );
int                ivec_max ( int n, int *a );
int                leqi ( char* string1, char* string2 );
int		   mdl_read ( FILE *filein );
int		   md2_read ( FILE *filein );
int		   md2_write ( FILE *fileout );
void               minmax ( void );
void               news ( void );
int                obj_read ( FILE *filein );
int                obj_write ( FILE *fileout );
void               print_sizes ( );
int                rcol_find ( float a[][MAX_COR3], int m, int n, float r[] );
float              reverse_bytes_float ( float x );
void               set_cor3_normal ( void );
void               set_vertex_normal ( void );
int                smf_read ( FILE *filein );
int                smf_write ( FILE *fileout );
int                stla_read ( FILE *filein );
int                stla_write ( FILE *fileout );
void               tds_pre_process ( void );
int                tds_read ( FILE *filein );
unsigned long int  tds_read_ambient_section ( FILE *filein );
unsigned long int  tds_read_background_section ( FILE *filein );
unsigned long int  tds_read_boolean ( unsigned char *boolean, FILE *filein );
unsigned long int  tds_read_camera_section ( FILE *filein );
unsigned long int  tds_read_edit_section ( FILE *filein, int *views_read );
float              tds_read_float ( FILE *filein );
unsigned long int  tds_read_keyframe_section ( FILE *filein, int *views_read );
unsigned long int  tds_read_keyframe_objdes_section ( FILE *filein );
unsigned long int  tds_read_light_section ( FILE *filein );
unsigned long int  tds_read_u_long_int ( FILE *filein );
int                tds_read_long_name ( FILE *filein );
unsigned long int  tds_read_matdef_section ( FILE *filein );
unsigned long int  tds_read_material_section ( FILE *filein );
int                tds_read_name ( FILE *filein );
unsigned long int  tds_read_obj_section ( FILE *filein );
unsigned long int  tds_read_object_section ( FILE *filein );
unsigned short int tds_read_u_short_int ( FILE *filein );
unsigned long int  tds_read_spot_section ( FILE *filein );
unsigned long int  tds_read_unknown_section ( FILE *filein );
unsigned long int  tds_read_view_section ( FILE *filein, int *views_read );
unsigned long int  tds_read_vp_section ( FILE *filein, int *views_read );
int                tds_write ( FILE *fileout );
int                tds_write_float ( FILE *fileout, float float_val );
int                tds_write_long_int ( FILE *fileout, long int int_val );
int                tds_write_string ( FILE *fileout, char *string );
int                tds_write_short_int ( FILE *fileout, short int int_val );
int                tds_write_u_short_int ( FILE *fileout, unsigned short int int_val );
int                txt_write ( FILE *fileout );
int                vla_read ( FILE *filein );
int                vla_write ( FILE *fileout );
int                makeunique (int vidx, int vtidx);
};

class csConverter_FrameManipulator
{
  public:
    csConverter_FrameManipulator(converter *target);
    virtual ~csConverter_FrameManipulator();

    // returns maximum frame number
    virtual int GetMaxAllowedFrame() const = 0;

    // set the current frame.  return maximum frame number
    virtual int SetFrame(int setto) = 0;

  protected:
    converter* m_data_target;
};

#endif // __IMPEXP_H__
