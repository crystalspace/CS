/*
    Copyright (C) 2001 by Jorrit Tyberghein

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

#ifndef __CSOSDEFS_H__
#define __CSOSDEFS_H__

#include <math.h>

// The 2D graphics driver used by the software renderer
#define SOFTWARE_2D_DRIVER_PS2 "crystalspace.graphics2d.ps2d"
#define SOFTWARE_2D_DRIVER SOFTWARE_2D_DRIVER_PS2

// The 2D graphics driver used by OpenGL renderer
#define OPENGL_2D_DRIVER "crystalspace.graphics2d.ps2d"

// The 2D graphics driver used by Glide renderer
#define GLIDE_2D_DRIVER	"crystalspace.graphics2d.glide.x.2"

// The 2D graphics driver used by Glide renderer Version 3
#define GLIDE_2D_DRIVER_V3 "crystalspace.graphics2d.glide.x.3"
// The sound driver
#define SOUND_DRIVER "crystalspace.sound.driver.oss"

#if defined (SYSDEF_DIR)
#  define __NEED_GENERIC_ISDIR
#endif

#if defined (SYSDEF_SOCKETS) && defined (OS_SOLARIS)
extern "C" unsigned long inet_addr(const char*);
#endif

// Include stdio now, so it won't screw up stuff later
#include <stdio.h>
#include <sys/types.h>

// Kill all of the possible defines
#undef fopen
#define fopen ps2fopen
#undef fclose
#define fclose ps2fclose
#undef fread
#define fread ps2fread
#undef fwrite
#define fwrite ps2fwrite
#undef fseek
#define fseek ps2fseek
#undef ferror
#define ferror ps2ferror
#undef feof
#define feof ps2feof
#undef FILE
#define FILE ps2FILE
#undef ftell
#define ftell ps2ftell
#undef fprintf
#define fprintf ps2fprintf
#undef fgetc
#define fgetc ps2fgetc
#undef fputc
#define fputc ps2fputc
#undef fgets
#define fgets ps2fgets
#undef fputs
#define fputs ps2fputs
#undef fflush
#define fflush ps2fflush
#undef access
#define access ps2access
#undef fileno
#define fileno ps2fileno
#undef clearerr
#define clearerr ps2clearerr

#undef stdin
#define stdin ps2stdin
#undef stdout
#define stdout ps2stdout
#undef stderr
#define stderr ps2stderr

// Then defines the functions
extern "C" {
typedef int FILE;
FILE *fopen(const char * filename, const char * mode);
int fclose(FILE *f);
size_t fread(void *buf, size_t size, size_t count, FILE *f);
size_t  fwrite(const void *buf, size_t size, size_t count, FILE *f);
int fseek(FILE *f, long int dist, int pos);
long ftell(FILE *);
int ferror(FILE *);
int feof(FILE *f);
int fprintf(FILE*, const char* msg, ...);
int fgetc(FILE *stream);
int fputc(int c, FILE *stream);
char *fgets(char *s, int size, FILE *stream);
int fputs(const char *s, FILE *stream);
int fflush(FILE *stream);
int access (const char *path, int mode);
int fileno(FILE *stream);
void clearerr (FILE *__stream);

extern FILE* stdin;
extern FILE* stdout;
extern FILE* stderr;
}

#endif // __CSOSDEFS_H__
