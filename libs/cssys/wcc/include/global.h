#include <malloc.h>
#include <fcntl.h>
#include <io.h>
#include <sys\stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <dos.h>
#include <i86.h>
#include <mem.h>

#include "asmmac.h"

#define SIGN(x)  (((x)<0)?(-1):(1))
#define MAX(a,b) ((a)>(b)?(a):(b))
#define MIN(a,b) ((a)<(b)?(a):(b))
#define ABS(x)   (((x)<0)?(-(x)):(x))

// Structures for issuing real mode interrupts with DPMI
struct _RMWORDREGS {unsigned short ax, bx, cx, dx, si, di, bp, cflag;};
struct _RMBYTEREGS {unsigned char   al, ah, bl, bh, cl, ch, dl, dh;};

typedef union {struct _RMWORDREGS x; struct _RMBYTEREGS h;} RMREGS;
typedef struct
         {
          unsigned short  es;
          unsigned short  cs;
          unsigned short  ss;
          unsigned short  ds;
         } RMSREGS;
