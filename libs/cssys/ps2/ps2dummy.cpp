#include "cssysdef.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <eekernel.h>
#include <sifdev.h>

extern "C" {

#define PS2UNUSED() { printf("PS2UNUSED "__FILE__"(%d)\n", __LINE__); return 0; }
#define PS2ASSERT(test) if(!(test)) { printf("PS2ASSERT "__FILE__"(%d):" #test "\n", __LINE__); }

//  int opendir() {PS2UNUSED();}
//  int readdir() {PS2UNUSED();}
//  int closedir() {PS2UNUSED();}

//  int mkdir() {PS2UNUSED();}
//  int getcwd() {PS2UNUSED();}
//  int access() {PS2UNUSED();}
//TODO Azverkan WriteMe  
//  time_t time(time_t*) { return 0; }
//  int link() {PS2UNUSED();}
//  int times() {PS2UNUSED();}

#if 1
struct MyFile {
	int fd;
	int pos;
	bool eof; 
};
#define FILE2SCE(f) (((MyFile*)f)->fd)
#define FILE2POS(f) (((MyFile*)f)->pos)
#define FILE2EOF(f) (((MyFile*)f)->eof)

#define ASSERTSCEFILE(f) { PS2ASSERT(f); if(f) PS2ASSERT(FILE2SCE(f)>=0); }

#define SCEHOST "host0:"
#define SCECDROM "cdrom0:"

//#define PS2FILE_DEBUG

FILE *fopen(const char * filename, const char * mode) {
   bool firstboot=1;
   if(firstboot) {
      sceFsReset();
      firstboot=0;
   }
   
  int flags=0;
  if(mode[0]=='r') {
    if(mode[1]=='w')
      flags=SCE_RDWR | SCE_CREAT;
    else
      flags=SCE_RDONLY;
  } else if(mode[0]=='w')
    flags=SCE_WRONLY | SCE_CREAT;  
  else if(mode[0]=='a')
    flags=SCE_TRUNC | SCE_CREAT;

  FILE *f=new FILE;
  char buf[255];

  strcpy(buf, SCEHOST);
  strcat(buf, filename);
  FILE2SCE(f)=sceOpen(buf, flags);
  
  if(FILE2SCE(f)<0) {
    strcpy(buf, SCECDROM);
    strcat(buf, filename);
    FILE2SCE(f)=sceOpen(buf, flags);
  }
  
  if(FILE2SCE(f)<0) {
     delete f;
     printf("fopen(%s, %s) FAIL\n", filename, mode);
     return NULL;
  }
  
#if 0
  printf("fopen(%s, %s) %d\n", buf, mode, FILE2SCE(f));
#endif
  FILE2EOF(f)=0;

  return f;
}

int fclose(FILE *f) {
  ASSERTSCEFILE(f);

#if PS2FILE_DEBUG
  printf("fclose(%d)\n", FILE2SCE(f));
#endif
  return sceClose(FILE2SCE(f));
}

size_t  fread(void *buf, size_t size, size_t count, FILE *f) {
  ASSERTSCEFILE(f);

  int bytes=sceRead(FILE2SCE(f), buf, size*count);
#if PS2FILE_DEBUG
  printf("fread(%p, %d, %d, %d) %d\n", buf, size, count, FILE2SCE(f), bytes);
#endif
  if(bytes<size*count) 
      FILE2EOF(f)=1;

  return bytes/size;
}

size_t  fwrite(const void *buf, size_t size, size_t count, FILE *f) {
  if(f==stdout || f==stderr) {
    for(int i=0; i<size*count; i++) {
      printf("%c", ((char*)buf)[i]);
    }
    return count;
  }

  ASSERTSCEFILE(f);

  int bytes=sceWrite(FILE2SCE(f), (void*)buf, size*count);
#if PS2FILE_DEBUG
  printf("fwrite(%p, %d, %d, %d) %d\n", buf, size, count, FILE2SCE(f), bytes);
#endif
  return bytes/size;
}

int fseek(FILE *f, long int dist, int pos) {
  ASSERTSCEFILE(f);

  int where=-1;
  if(pos==SEEK_SET) {
    where=SCE_SEEK_SET;
  } else if(pos==SEEK_CUR) {
    where=SCE_SEEK_CUR;
  } else {
    where=SCE_SEEK_END;
  }
  
  FILE2POS(f)=sceLseek(FILE2SCE(f), dist, where);
#if PS2FILE_DEBUG
  printf("fseek(%d, %d, %d) %d\n", FILE2SCE(f), dist, where, FILE2POS(f));
#endif
  return 0;
}

long ftell(FILE *f) {
  ASSERTSCEFILE(f);

#if PS2FILE_DEBUG
  printf("ftell (%d) %d\n", FILE2SCE(f), FILE2POS(f));
#endif
  return FILE2POS(f);
}

int ferror(FILE *f) {
  if(!f)
      return -1;
  if(FILE2SCE(f)<0)
      return -1;
  return 0;
}

int feof(FILE *f) {
  ASSERTSCEFILE(f);

  return FILE2EOF(f);
}

int fgetc(FILE *stream) {
  PS2UNUSED();
}

int fputc(int c, FILE *stream) {
  PS2UNUSED();
}

char *fgets(char *s, int size, FILE *stream) {
  PS2UNUSED();
  return NULL;
}

int fflush(FILE *) {return 0;}

FILE* stdin=NULL;
FILE* stdout=NULL;
FILE* stderr=NULL;

int access (const char *path, int mode) {
    FILE *f=fopen(path, "r");
    if(!f)
	return -1;
    fclose(f);
    return 0;
}

int fileno(FILE *stream) {
    PS2ASSERT(stream);
    return FILE2SCE(stream);
}

void clearerr (FILE *__stream) {}

int fputs(const char *s, FILE *stream) {
  int size=strlen(s);
  return size!=fwrite(s, 1, size, stream);
}

int fprintf(FILE* f, const char* msg, ...) {
  char buf[300];
  va_list arg;
  va_start(arg, msg);
  vsprintf(buf, msg, arg);
  va_end(arg);

  fputs(buf, f);

  return 0;
}

#endif
} //extern "C"

