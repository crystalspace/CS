/*
    ZIP archive support for Crystal Space 3D library
    Copyright (C) 1998,1999 by Andrew Zabolotny <bit@eltech.ru>

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

#ifndef __CS_ZIP_H__
#define __CS_ZIP_H__

/**\file
 * ZIP file format
 */

#if defined(__cplusplus) && !defined(CS_COMPILER_BCC)
extern "C" {
#endif

#define Byte z_Byte	/* Kludge to avoid conflicting typedef in zconf.h */
#include <zlib.h>
#undef Byte

#if defined(__cplusplus) && !defined(CS_COMPILER_BCC)
}
#endif

#define CENTRAL_HDR_SIG	'\001','\002'	/* the infamous "PK" signature bytes, */
#define LOCAL_HDR_SIG	'\003','\004'	/*  sans "PK" (so unzip executable not */
#define END_CENTRAL_SIG	'\005','\006'	/*  mistaken for zipfile itself) */
#define EXTD_LOCAL_SIG	'\007','\010'	/* [ASCII "\113" == EBCDIC "\080" ??] */

#define DEF_WBITS	15		/* Default LZ77 window size */
#define ZIP_STORE	0		/* 'STORED' method id */
#define ZIP_DEFLATE	8		/* 'DEFLATE' method id */

typedef uint8  uch;
typedef uint16 ush;
typedef uint32 u32;

#if 0            /* Optimization: use the (const) result of crc32(0L,0,0) */
#  define CRCVAL_INITIAL  crc32(0L, 0, 0)
#else
#  define CRCVAL_INITIAL  0L
#endif

typedef struct
{
  uch version_needed_to_extract[2];
  ush general_purpose_bit_flag;
  ush compression_method;
  ush last_mod_file_time;
  ush last_mod_file_date;
  u32 crc32;
  u32 csize;
  u32 ucsize;
  ush filename_length;
  ush extra_field_length;
} ZIP_local_file_header;

typedef struct
{
  uch version_made_by[2];
  uch version_needed_to_extract[2];
  ush general_purpose_bit_flag;
  ush compression_method;
  ush last_mod_file_time;
  ush last_mod_file_date;
  u32 crc32;
  u32 csize;
  u32 ucsize;
  ush filename_length;
  ush extra_field_length;
  ush file_comment_length;
  ush disk_number_start;
  ush internal_file_attributes;
  u32 external_file_attributes;
  u32 relative_offset_local_header;
} ZIP_central_directory_file_header;

typedef struct
{
  ush number_this_disk;
  ush num_disk_start_cdir;
  ush num_entries_centrl_dir_ths_disk;
  ush total_entries_central_dir;
  u32 size_central_directory;
  u32 offset_start_central_directory;
  ush zipfile_comment_length;
} ZIP_end_central_dir_record;

//--- ZIP_local_file_header layout ---------------------------------------------
#define ZIP_LOCAL_FILE_HEADER_SIZE              26
#      define L_VERSION_NEEDED_TO_EXTRACT_0     0
#      define L_VERSION_NEEDED_TO_EXTRACT_1     1
#      define L_GENERAL_PURPOSE_BIT_FLAG        2
#      define L_COMPRESSION_METHOD              4
#      define L_LAST_MOD_FILE_TIME              6
#      define L_LAST_MOD_FILE_DATE              8
#      define L_CRC32                           10
#      define L_COMPRESSED_SIZE                 14
#      define L_UNCOMPRESSED_SIZE               18
#      define L_FILENAME_LENGTH                 22
#      define L_EXTRA_FIELD_LENGTH              24

//--- ZIP_central_directory_file_header layout ---------------------------------
#define ZIP_CENTRAL_DIRECTORY_FILE_HEADER_SIZE  42
#      define C_VERSION_MADE_BY_0               0
#      define C_VERSION_MADE_BY_1               1
#      define C_VERSION_NEEDED_TO_EXTRACT_0     2
#      define C_VERSION_NEEDED_TO_EXTRACT_1     3
#      define C_GENERAL_PURPOSE_BIT_FLAG        4
#      define C_COMPRESSION_METHOD              6
#      define C_LAST_MOD_FILE_TIME              8
#      define C_LAST_MOD_FILE_DATE              10
#      define C_CRC32                           12
#      define C_COMPRESSED_SIZE                 16
#      define C_UNCOMPRESSED_SIZE               20
#      define C_FILENAME_LENGTH                 24
#      define C_EXTRA_FIELD_LENGTH              26
#      define C_FILE_COMMENT_LENGTH             28
#      define C_DISK_NUMBER_START               30
#      define C_INTERNAL_FILE_ATTRIBUTES        32
#      define C_EXTERNAL_FILE_ATTRIBUTES        34
#      define C_RELATIVE_OFFSET_LOCAL_HEADER    38

//--- ZIP_end_central_dir_record layout ----------------------------------------
#define ZIP_END_CENTRAL_DIR_RECORD_SIZE         18
#      define E_NUMBER_THIS_DISK                0
#      define E_NUM_DISK_WITH_START_CENTRAL_DIR 2
#      define E_NUM_ENTRIES_CENTRL_DIR_THS_DISK 4
#      define E_TOTAL_ENTRIES_CENTRAL_DIR       6
#      define E_SIZE_CENTRAL_DIRECTORY          8
#      define E_OFFSET_START_CENTRAL_DIRECTORY  12
#      define E_ZIPFILE_COMMENT_LENGTH          16

#endif // __CS_ZIP_H__
