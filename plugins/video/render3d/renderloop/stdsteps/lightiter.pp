# 1 "lightiter.cpp"
# 1 "<built-in>"
# 1 "<command line>"
# 1 "lightiter.cpp"
# 20 "lightiter.cpp"
# 1 "f:/source-wip/CS/include/cssysdef.h" 1
# 26 "f:/source-wip/CS/include/cssysdef.h"
# 1 "f:/source-wip/CS/include/csdef.h" 1
# 26 "f:/source-wip/CS/include/csdef.h"
# 1 "f:/source-wip/CS/include/platform.h" 1
# 26 "f:/source-wip/CS/include/platform.h"
# 1 "f:/source-wip/CS/include/volatile.h" 1
# 27 "f:/source-wip/CS/include/platform.h" 2
# 27 "f:/source-wip/CS/include/csdef.h" 2
# 1 "f:/source-wip/CS/include/cstypes.h" 1
# 32 "f:/source-wip/CS/include/cstypes.h"
# 1 "d:/util/mingw/include/float.h" 1
# 35 "d:/util/mingw/include/float.h"
# 1 "d:/util/mingw/include/float.h" 1 3
# 35 "d:/util/mingw/include/float.h" 3
# 1 "d:/util/mingw/lib/gcc-lib/mingw32/3.2.3/include/float.h" 1 3
# 36 "d:/util/mingw/include/float.h" 2 3





# 1 "d:/util/mingw/include/_mingw.h" 1 3
# 42 "d:/util/mingw/include/float.h" 2 3
# 112 "d:/util/mingw/include/float.h" 3
extern "C" {





__attribute__((dllimport)) unsigned int __attribute__((__cdecl__)) _controlfp (unsigned int unNew, unsigned int unMask);
__attribute__((dllimport)) unsigned int __attribute__((__cdecl__)) _control87 (unsigned int unNew, unsigned int unMask);


__attribute__((dllimport)) unsigned int __attribute__((__cdecl__)) _clearfp (void);
__attribute__((dllimport)) unsigned int __attribute__((__cdecl__)) _statusfp (void);
# 137 "d:/util/mingw/include/float.h" 3
void __attribute__((__cdecl__)) _fpreset (void);
void __attribute__((__cdecl__)) fpreset (void);


__attribute__((dllimport)) int * __attribute__((__cdecl__)) __fpecode(void);







__attribute__((dllimport)) double __attribute__((__cdecl__)) _chgsign (double);
__attribute__((dllimport)) double __attribute__((__cdecl__)) _copysign (double, double);
__attribute__((dllimport)) double __attribute__((__cdecl__)) _logb (double);
__attribute__((dllimport)) double __attribute__((__cdecl__)) _nextafter (double, double);
__attribute__((dllimport)) double __attribute__((__cdecl__)) _scalb (double, long);

__attribute__((dllimport)) int __attribute__((__cdecl__)) _finite (double);
__attribute__((dllimport)) int __attribute__((__cdecl__)) _fpclass (double);
__attribute__((dllimport)) int __attribute__((__cdecl__)) _isnan (double);


}
# 36 "d:/util/mingw/include/float.h" 2
# 33 "f:/source-wip/CS/include/cstypes.h" 2
# 98 "f:/source-wip/CS/include/cstypes.h"
# 1 "d:/util/mingw/include/stdint.h" 1
# 24 "d:/util/mingw/include/stdint.h"
# 1 "d:/util/mingw/include/stddef.h" 1





# 1 "d:/util/mingw/include/stddef.h" 1 3





# 1 "d:/util/mingw/lib/gcc-lib/mingw32/3.2.3/include/stddef.h" 1 3
# 356 "d:/util/mingw/lib/gcc-lib/mingw32/3.2.3/include/stddef.h" 3
typedef short unsigned int wint_t;
# 7 "d:/util/mingw/include/stddef.h" 2 3
# 7 "d:/util/mingw/include/stddef.h" 2
# 25 "d:/util/mingw/include/stdint.h" 2


typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned uint32_t;
typedef long long int64_t;
typedef unsigned long long uint64_t;


typedef signed char int_least8_t;
typedef unsigned char uint_least8_t;
typedef short int_least16_t;
typedef unsigned short uint_least16_t;
typedef int int_least32_t;
typedef unsigned uint_least32_t;
typedef long long int_least64_t;
typedef unsigned long long uint_least64_t;





typedef char int_fast8_t;
typedef unsigned char uint_fast8_t;
typedef short int_fast16_t;
typedef unsigned short uint_fast16_t;
typedef int int_fast32_t;
typedef unsigned int uint_fast32_t;
typedef long long int_fast64_t;
typedef unsigned long long uint_fast64_t;


typedef int intptr_t;
typedef unsigned uintptr_t;


typedef long long intmax_t;
typedef unsigned long long uintmax_t;
# 99 "f:/source-wip/CS/include/cstypes.h" 2
typedef uint8_t uint8;
typedef int8_t int8;
typedef uint16_t uint16;
typedef int16_t int16;
typedef uint32_t uint32;
typedef int32_t int32;
typedef uint64_t uint64;
typedef int64_t int64;
# 115 "f:/source-wip/CS/include/cstypes.h"
typedef uint32 CS_ID;


typedef unsigned int csTicks;




typedef unsigned int uint;
# 28 "f:/source-wip/CS/include/csdef.h" 2

# 1 "d:/util/mingw/include/stdio.h" 1
# 42 "d:/util/mingw/include/stdio.h"
# 1 "d:/util/mingw/include/stddef.h" 1





# 1 "d:/util/mingw/include/stddef.h" 1 3





# 1 "d:/util/mingw/lib/gcc-lib/mingw32/3.2.3/include/stddef.h" 1 3
# 215 "d:/util/mingw/lib/gcc-lib/mingw32/3.2.3/include/stddef.h" 3
typedef unsigned int size_t;
# 7 "d:/util/mingw/include/stddef.h" 2 3
# 7 "d:/util/mingw/include/stddef.h" 2
# 43 "d:/util/mingw/include/stdio.h" 2

# 1 "d:/util/mingw/include/stdarg.h" 1





# 1 "d:/util/mingw/include/stdarg.h" 1 3





# 1 "d:/util/mingw/lib/gcc-lib/mingw32/3.2.3/include/stdarg.h" 1 3
# 44 "d:/util/mingw/lib/gcc-lib/mingw32/3.2.3/include/stdarg.h" 3
typedef __builtin_va_list __gnuc_va_list;
# 7 "d:/util/mingw/include/stdarg.h" 2 3
# 7 "d:/util/mingw/include/stdarg.h" 2
# 45 "d:/util/mingw/include/stdio.h" 2
# 151 "d:/util/mingw/include/stdio.h"
typedef struct _iobuf
{
        char* _ptr;
        int _cnt;
        char* _base;
        int _flag;
        int _file;
        int _charbuf;
        int _bufsiz;
        char* _tmpfname;
} FILE;
# 176 "d:/util/mingw/include/stdio.h"
__attribute__((dllimport)) FILE _iob[];
# 185 "d:/util/mingw/include/stdio.h"
extern "C" {





__attribute__((dllimport)) FILE* __attribute__((__cdecl__)) fopen (const char*, const char*);
__attribute__((dllimport)) FILE* __attribute__((__cdecl__)) freopen (const char*, const char*, FILE*);
__attribute__((dllimport)) int __attribute__((__cdecl__)) fflush (FILE*);
__attribute__((dllimport)) int __attribute__((__cdecl__)) fclose (FILE*);

__attribute__((dllimport)) int __attribute__((__cdecl__)) remove (const char*);
__attribute__((dllimport)) int __attribute__((__cdecl__)) rename (const char*, const char*);
__attribute__((dllimport)) FILE* __attribute__((__cdecl__)) tmpfile (void);
__attribute__((dllimport)) char* __attribute__((__cdecl__)) tmpnam (char*);
__attribute__((dllimport)) char* __attribute__((__cdecl__)) _tempnam (const char*, const char*);


__attribute__((dllimport)) char* __attribute__((__cdecl__)) tempnam (const char*, const char*);


__attribute__((dllimport)) int __attribute__((__cdecl__)) setvbuf (FILE*, char*, int, size_t);

__attribute__((dllimport)) void __attribute__((__cdecl__)) setbuf (FILE*, char*);





__attribute__((dllimport)) int __attribute__((__cdecl__)) fprintf (FILE*, const char*, ...);
__attribute__((dllimport)) int __attribute__((__cdecl__)) printf (const char*, ...);
__attribute__((dllimport)) int __attribute__((__cdecl__)) sprintf (char*, const char*, ...);
__attribute__((dllimport)) int __attribute__((__cdecl__)) _snprintf (char*, size_t, const char*, ...);
__attribute__((dllimport)) int __attribute__((__cdecl__)) vfprintf (FILE*, const char*, char*);
__attribute__((dllimport)) int __attribute__((__cdecl__)) vprintf (const char*, char*);
__attribute__((dllimport)) int __attribute__((__cdecl__)) vsprintf (char*, const char*, char*);
__attribute__((dllimport)) int __attribute__((__cdecl__)) _vsnprintf (char*, size_t, const char*, char*);


int __attribute__((__cdecl__)) snprintf(char* s, size_t n, const char* format, ...);
extern __inline__ int __attribute__((__cdecl__))
vsnprintf (char* s, size_t n, const char* format, char* arg)
  { return _vsnprintf ( s, n, format, arg); }
int __attribute__((__cdecl__)) vscanf (const char * __restrict__, char*);
int __attribute__((__cdecl__)) vfscanf (FILE * __restrict__, const char * __restrict__,
                     char*);
int __attribute__((__cdecl__)) vsscanf (const char * __restrict__,
                     const char * __restrict__, char*);






__attribute__((dllimport)) int __attribute__((__cdecl__)) fscanf (FILE*, const char*, ...);
__attribute__((dllimport)) int __attribute__((__cdecl__)) scanf (const char*, ...);
__attribute__((dllimport)) int __attribute__((__cdecl__)) sscanf (const char*, const char*, ...);




__attribute__((dllimport)) int __attribute__((__cdecl__)) fgetc (FILE*);
__attribute__((dllimport)) char* __attribute__((__cdecl__)) fgets (char*, int, FILE*);
__attribute__((dllimport)) int __attribute__((__cdecl__)) fputc (int, FILE*);
__attribute__((dllimport)) int __attribute__((__cdecl__)) fputs (const char*, FILE*);
__attribute__((dllimport)) int __attribute__((__cdecl__)) getc (FILE*);
__attribute__((dllimport)) int __attribute__((__cdecl__)) getchar (void);
__attribute__((dllimport)) char* __attribute__((__cdecl__)) gets (char*);
__attribute__((dllimport)) int __attribute__((__cdecl__)) putc (int, FILE*);
__attribute__((dllimport)) int __attribute__((__cdecl__)) putchar (int);
__attribute__((dllimport)) int __attribute__((__cdecl__)) puts (const char*);
__attribute__((dllimport)) int __attribute__((__cdecl__)) ungetc (int, FILE*);





__attribute__((dllimport)) size_t __attribute__((__cdecl__)) fread (void*, size_t, size_t, FILE*);
__attribute__((dllimport)) size_t __attribute__((__cdecl__)) fwrite (const void*, size_t, size_t, FILE*);





__attribute__((dllimport)) int __attribute__((__cdecl__)) fseek (FILE*, long, int);
__attribute__((dllimport)) long __attribute__((__cdecl__)) ftell (FILE*);
__attribute__((dllimport)) void __attribute__((__cdecl__)) rewind (FILE*);
# 295 "d:/util/mingw/include/stdio.h"
typedef long long fpos_t;




__attribute__((dllimport)) int __attribute__((__cdecl__)) fgetpos (FILE*, fpos_t*);
__attribute__((dllimport)) int __attribute__((__cdecl__)) fsetpos (FILE*, const fpos_t*);





__attribute__((dllimport)) void __attribute__((__cdecl__)) clearerr (FILE*);
__attribute__((dllimport)) int __attribute__((__cdecl__)) feof (FILE*);
__attribute__((dllimport)) int __attribute__((__cdecl__)) ferror (FILE*);
__attribute__((dllimport)) void __attribute__((__cdecl__)) perror (const char*);






__attribute__((dllimport)) FILE* __attribute__((__cdecl__)) _popen (const char*, const char*);
__attribute__((dllimport)) int __attribute__((__cdecl__)) _pclose (FILE*);


__attribute__((dllimport)) FILE* __attribute__((__cdecl__)) popen (const char*, const char*);
__attribute__((dllimport)) int __attribute__((__cdecl__)) pclose (FILE*);





__attribute__((dllimport)) int __attribute__((__cdecl__)) _flushall (void);
__attribute__((dllimport)) int __attribute__((__cdecl__)) _fgetchar (void);
__attribute__((dllimport)) int __attribute__((__cdecl__)) _fputchar (int);
__attribute__((dllimport)) FILE* __attribute__((__cdecl__)) _fdopen (int, const char*);
__attribute__((dllimport)) int __attribute__((__cdecl__)) _fileno (FILE*);
__attribute__((dllimport)) int __attribute__((__cdecl__)) _fcloseall(void);

__attribute__((dllimport)) int __attribute__((__cdecl__)) _getmaxstdio(void);
__attribute__((dllimport)) int __attribute__((__cdecl__)) _setmaxstdio(int);



__attribute__((dllimport)) int __attribute__((__cdecl__)) fgetchar (void);
__attribute__((dllimport)) int __attribute__((__cdecl__)) fputchar (int);
__attribute__((dllimport)) FILE* __attribute__((__cdecl__)) fdopen (int, const char*);
__attribute__((dllimport)) int __attribute__((__cdecl__)) fileno (FILE*);
# 352 "d:/util/mingw/include/stdio.h"
__attribute__((dllimport)) int __attribute__((__cdecl__)) fwprintf (FILE*, const wchar_t*, ...);
__attribute__((dllimport)) int __attribute__((__cdecl__)) wprintf (const wchar_t*, ...);
__attribute__((dllimport)) int __attribute__((__cdecl__)) swprintf (wchar_t*, const wchar_t*, ...);
__attribute__((dllimport)) int __attribute__((__cdecl__)) _snwprintf (wchar_t*, size_t, const wchar_t*, ...);
__attribute__((dllimport)) int __attribute__((__cdecl__)) vfwprintf (FILE*, const wchar_t*, char*);
__attribute__((dllimport)) int __attribute__((__cdecl__)) vwprintf (const wchar_t*, char*);
__attribute__((dllimport)) int __attribute__((__cdecl__)) vswprintf (wchar_t*, const wchar_t*, char*);
__attribute__((dllimport)) int __attribute__((__cdecl__)) _vsnwprintf (wchar_t*, size_t, const wchar_t*, char*);
__attribute__((dllimport)) int __attribute__((__cdecl__)) fwscanf (FILE*, const wchar_t*, ...);
__attribute__((dllimport)) int __attribute__((__cdecl__)) wscanf (const wchar_t*, ...);
__attribute__((dllimport)) int __attribute__((__cdecl__)) swscanf (const wchar_t*, const wchar_t*, ...);
__attribute__((dllimport)) wint_t __attribute__((__cdecl__)) fgetwc (FILE*);
__attribute__((dllimport)) wint_t __attribute__((__cdecl__)) fputwc (wchar_t, FILE*);
__attribute__((dllimport)) wint_t __attribute__((__cdecl__)) ungetwc (wchar_t, FILE*);


__attribute__((dllimport)) wchar_t* __attribute__((__cdecl__)) fgetws (wchar_t*, int, FILE*);
__attribute__((dllimport)) int __attribute__((__cdecl__)) fputws (const wchar_t*, FILE*);
__attribute__((dllimport)) wint_t __attribute__((__cdecl__)) getwc (FILE*);
__attribute__((dllimport)) wint_t __attribute__((__cdecl__)) getwchar (void);
__attribute__((dllimport)) wchar_t* __attribute__((__cdecl__)) _getws (wchar_t*);
__attribute__((dllimport)) wint_t __attribute__((__cdecl__)) putwc (wint_t, FILE*);
__attribute__((dllimport)) int __attribute__((__cdecl__)) _putws (const wchar_t*);
__attribute__((dllimport)) wint_t __attribute__((__cdecl__)) putwchar (wint_t);
__attribute__((dllimport)) FILE* __attribute__((__cdecl__)) _wfdopen(int, wchar_t *);
__attribute__((dllimport)) FILE* __attribute__((__cdecl__)) _wfopen (const wchar_t*, const wchar_t*);
__attribute__((dllimport)) FILE* __attribute__((__cdecl__)) _wfreopen (const wchar_t*, const wchar_t*, FILE*);
__attribute__((dllimport)) FILE* __attribute__((__cdecl__)) _wfsopen (const wchar_t*, const wchar_t*, int);
__attribute__((dllimport)) wchar_t* __attribute__((__cdecl__)) _wtmpnam (wchar_t*);
__attribute__((dllimport)) wchar_t* __attribute__((__cdecl__)) _wtempnam (const wchar_t*, const wchar_t*);
__attribute__((dllimport)) int __attribute__((__cdecl__)) _wrename (const wchar_t*, const wchar_t*);
__attribute__((dllimport)) int __attribute__((__cdecl__)) _wremove (const wchar_t*);
__attribute__((dllimport)) void __attribute__((__cdecl__)) _wperror (const wchar_t*);
__attribute__((dllimport)) FILE* __attribute__((__cdecl__)) _wpopen (const wchar_t*, const wchar_t*);



int __attribute__((__cdecl__)) snwprintf (wchar_t* s, size_t n, const wchar_t* format, ...);
extern __inline__ int __attribute__((__cdecl__))
vsnwprintf (wchar_t* s, size_t n, const wchar_t* format, char* arg)
  { return _vsnwprintf ( s, n, format, arg);}
int __attribute__((__cdecl__)) vwscanf (const wchar_t * __restrict__, char*);
int __attribute__((__cdecl__)) vfwscanf (FILE * __restrict__,
                       const wchar_t * __restrict__, char*);
int __attribute__((__cdecl__)) vswscanf (const wchar_t * __restrict__,
                       const wchar_t * __restrict__, char*);
# 406 "d:/util/mingw/include/stdio.h"
__attribute__((dllimport)) FILE* __attribute__((__cdecl__)) wpopen (const wchar_t*, const wchar_t*);






__attribute__((dllimport)) wint_t __attribute__((__cdecl__)) _fgetwchar (void);
__attribute__((dllimport)) wint_t __attribute__((__cdecl__)) _fputwchar (wint_t);
__attribute__((dllimport)) int __attribute__((__cdecl__)) _getw (FILE*);
__attribute__((dllimport)) int __attribute__((__cdecl__)) _putw (int, FILE*);


__attribute__((dllimport)) wint_t __attribute__((__cdecl__)) fgetwchar (void);
__attribute__((dllimport)) wint_t __attribute__((__cdecl__)) fputwchar (wint_t);
__attribute__((dllimport)) int __attribute__((__cdecl__)) getw (FILE*);
__attribute__((dllimport)) int __attribute__((__cdecl__)) putw (int, FILE*);





}
# 30 "f:/source-wip/CS/include/csdef.h" 2
# 1 "d:/util/mingw/include/stdlib.h" 1
# 38 "d:/util/mingw/include/stdlib.h"
# 1 "d:/util/mingw/include/stddef.h" 1





# 1 "d:/util/mingw/include/stddef.h" 1 3





# 1 "d:/util/mingw/lib/gcc-lib/mingw32/3.2.3/include/stddef.h" 1 3
# 7 "d:/util/mingw/include/stddef.h" 2 3
# 7 "d:/util/mingw/include/stddef.h" 2
# 39 "d:/util/mingw/include/stdlib.h" 2
# 77 "d:/util/mingw/include/stdlib.h"
extern "C" {
# 86 "d:/util/mingw/include/stdlib.h"
extern int _argc;
extern char** _argv;




extern int* __attribute__((__cdecl__)) __p___argc(void);
extern char*** __attribute__((__cdecl__)) __p___argv(void);
extern wchar_t*** __attribute__((__cdecl__)) __p___wargv(void);
# 127 "d:/util/mingw/include/stdlib.h"
   __attribute__((dllimport)) int __mb_cur_max;
# 152 "d:/util/mingw/include/stdlib.h"
 __attribute__((dllimport)) int* __attribute__((__cdecl__)) _errno(void);


 __attribute__((dllimport)) int* __attribute__((__cdecl__)) __doserrno(void);







  extern __attribute__((dllimport)) char *** __attribute__((__cdecl__)) __p__environ(void);
  extern __attribute__((dllimport)) wchar_t *** __attribute__((__cdecl__)) __p__wenviron(void);
# 186 "d:/util/mingw/include/stdlib.h"
  __attribute__((dllimport)) int _sys_nerr;
# 210 "d:/util/mingw/include/stdlib.h"
__attribute__((dllimport)) char* _sys_errlist[];
# 224 "d:/util/mingw/include/stdlib.h"
extern __attribute__((dllimport)) unsigned __attribute__((__cdecl__)) int* __p__osver(void);
extern __attribute__((dllimport)) unsigned __attribute__((__cdecl__)) int* __p__winver(void);
extern __attribute__((dllimport)) unsigned __attribute__((__cdecl__)) int* __p__winmajor(void);
extern __attribute__((dllimport)) unsigned __attribute__((__cdecl__)) int* __p__winminor(void);







__attribute__((dllimport)) unsigned int _osver;
__attribute__((dllimport)) unsigned int _winver;
__attribute__((dllimport)) unsigned int _winmajor;
__attribute__((dllimport)) unsigned int _winminor;
# 275 "d:/util/mingw/include/stdlib.h"
__attribute__((dllimport)) char** __attribute__((__cdecl__)) __p__pgmptr(void);

__attribute__((dllimport)) wchar_t** __attribute__((__cdecl__)) __p__wpgmptr(void);
# 308 "d:/util/mingw/include/stdlib.h"
__attribute__((dllimport)) int _fmode;
# 324 "d:/util/mingw/include/stdlib.h"
__attribute__((dllimport)) double __attribute__((__cdecl__)) atof (const char*);
__attribute__((dllimport)) int __attribute__((__cdecl__)) atoi (const char*);
__attribute__((dllimport)) long __attribute__((__cdecl__)) atol (const char*);
__attribute__((dllimport)) int __attribute__((__cdecl__)) _wtoi (const wchar_t *);
__attribute__((dllimport)) long __attribute__((__cdecl__)) _wtol (const wchar_t *);

__attribute__((dllimport)) double __attribute__((__cdecl__)) strtod (const char*, char**);

extern __inline__ float __attribute__((__cdecl__)) strtof (const char *nptr, char **endptr)
  { return (strtod (nptr, endptr));}
long double __attribute__((__cdecl__)) strtold (const char * __restrict__, char ** __restrict__);


__attribute__((dllimport)) long __attribute__((__cdecl__)) strtol (const char*, char**, int);
__attribute__((dllimport)) unsigned long __attribute__((__cdecl__)) strtoul (const char*, char**, int);



__attribute__((dllimport)) double __attribute__((__cdecl__)) wcstod (const wchar_t*, wchar_t**);

extern __inline__ float __attribute__((__cdecl__)) wcstof( const wchar_t *nptr, wchar_t **endptr)
{ return (wcstod(nptr, endptr)); }
long double __attribute__((__cdecl__)) wcstold (const wchar_t * __restrict__, wchar_t ** __restrict__);


__attribute__((dllimport)) long __attribute__((__cdecl__)) wcstol (const wchar_t*, wchar_t**, int);
__attribute__((dllimport)) unsigned long __attribute__((__cdecl__)) wcstoul (const wchar_t*, wchar_t**, int);



__attribute__((dllimport)) size_t __attribute__((__cdecl__)) wcstombs (char*, const wchar_t*, size_t);
__attribute__((dllimport)) int __attribute__((__cdecl__)) wctomb (char*, wchar_t);

__attribute__((dllimport)) int __attribute__((__cdecl__)) mblen (const char*, size_t);
__attribute__((dllimport)) size_t __attribute__((__cdecl__)) mbstowcs (wchar_t*, const char*, size_t);
__attribute__((dllimport)) int __attribute__((__cdecl__)) mbtowc (wchar_t*, const char*, size_t);

__attribute__((dllimport)) int __attribute__((__cdecl__)) rand (void);
__attribute__((dllimport)) void __attribute__((__cdecl__)) srand (unsigned int);

__attribute__((dllimport)) void* __attribute__((__cdecl__)) calloc (size_t, size_t);
__attribute__((dllimport)) void* __attribute__((__cdecl__)) malloc (size_t);
__attribute__((dllimport)) void* __attribute__((__cdecl__)) realloc (void*, size_t);
__attribute__((dllimport)) void __attribute__((__cdecl__)) free (void*);

__attribute__((dllimport)) void __attribute__((__cdecl__)) abort (void) ;
__attribute__((dllimport)) void __attribute__((__cdecl__)) exit (int) ;


int __attribute__((__cdecl__)) atexit (void (*)(void));

__attribute__((dllimport)) int __attribute__((__cdecl__)) system (const char*);
__attribute__((dllimport)) char* __attribute__((__cdecl__)) getenv (const char*);

__attribute__((dllimport)) void* __attribute__((__cdecl__)) bsearch (const void*, const void*, size_t, size_t,
                                 int (*)(const void*, const void*));
__attribute__((dllimport)) void __attribute__((__cdecl__)) qsort (void*, size_t, size_t,
                                 int (*)(const void*, const void*));

__attribute__((dllimport)) int __attribute__((__cdecl__)) abs (int);
__attribute__((dllimport)) long __attribute__((__cdecl__)) labs (long);
# 394 "d:/util/mingw/include/stdlib.h"
typedef struct { int quot, rem; } div_t;
typedef struct { long quot, rem; } ldiv_t;

__attribute__((dllimport)) div_t __attribute__((__cdecl__)) div (int, int);
__attribute__((dllimport)) ldiv_t __attribute__((__cdecl__)) ldiv (long, long);







__attribute__((dllimport)) void __attribute__((__cdecl__)) _beep (unsigned int, unsigned int);
__attribute__((dllimport)) void __attribute__((__cdecl__)) _seterrormode (int);
__attribute__((dllimport)) void __attribute__((__cdecl__)) _sleep (unsigned long);

__attribute__((dllimport)) void __attribute__((__cdecl__)) _exit (int) ;


void __attribute__((__cdecl__)) _Exit(int) ;
extern __inline__ void __attribute__((__cdecl__)) _Exit(int status)
        { _exit(status); }



typedef int (* _onexit_t)(void);
_onexit_t __attribute__((__cdecl__)) _onexit( _onexit_t );

__attribute__((dllimport)) int __attribute__((__cdecl__)) _putenv (const char*);
__attribute__((dllimport)) void __attribute__((__cdecl__)) _searchenv (const char*, const char*, char*);


__attribute__((dllimport)) char* __attribute__((__cdecl__)) _ecvt (double, int, int*, int*);
__attribute__((dllimport)) char* __attribute__((__cdecl__)) _fcvt (double, int, int*, int*);
__attribute__((dllimport)) char* __attribute__((__cdecl__)) _gcvt (double, int, char*);

__attribute__((dllimport)) void __attribute__((__cdecl__)) _makepath (char*, const char*, const char*, const char*, const char*);
__attribute__((dllimport)) void __attribute__((__cdecl__)) _splitpath (const char*, char*, char*, char*, char*);
__attribute__((dllimport)) char* __attribute__((__cdecl__)) _fullpath (char*, const char*, size_t);


__attribute__((dllimport)) char* __attribute__((__cdecl__)) _itoa (int, char*, int);
__attribute__((dllimport)) char* __attribute__((__cdecl__)) _ltoa (long, char*, int);
__attribute__((dllimport)) char* __attribute__((__cdecl__)) _ultoa(unsigned long, char*, int);
__attribute__((dllimport)) wchar_t* __attribute__((__cdecl__)) _itow (int, wchar_t*, int);
__attribute__((dllimport)) wchar_t* __attribute__((__cdecl__)) _ltow (long, wchar_t*, int);
__attribute__((dllimport)) wchar_t* __attribute__((__cdecl__)) _ultow (unsigned long, wchar_t*, int);


__attribute__((dllimport)) __int64 __attribute__((__cdecl__)) _atoi64(const char *);
__attribute__((dllimport)) char* __attribute__((__cdecl__)) _i64toa(__int64, char *, int);
__attribute__((dllimport)) char* __attribute__((__cdecl__)) _ui64toa(unsigned __int64, char *, int);
__attribute__((dllimport)) __int64 __attribute__((__cdecl__)) _wtoi64(const wchar_t *);
__attribute__((dllimport)) wchar_t* __attribute__((__cdecl__)) _i64tow(__int64, wchar_t *, int);
__attribute__((dllimport)) wchar_t* __attribute__((__cdecl__)) _ui64tow(unsigned __int64, wchar_t *, int);

__attribute__((dllimport)) wchar_t* __attribute__((__cdecl__)) _wgetenv(const wchar_t*);
__attribute__((dllimport)) int __attribute__((__cdecl__)) _wputenv(const wchar_t*);
__attribute__((dllimport)) void __attribute__((__cdecl__)) _wsearchenv(const wchar_t*, const wchar_t*, wchar_t*);
__attribute__((dllimport)) void __attribute__((__cdecl__)) _wmakepath(wchar_t*, const wchar_t*, const wchar_t*, const wchar_t*, const wchar_t*);
__attribute__((dllimport)) void __attribute__((__cdecl__)) _wsplitpath (const wchar_t*, wchar_t*, wchar_t*, wchar_t*, wchar_t*);
__attribute__((dllimport)) wchar_t* __attribute__((__cdecl__)) _wfullpath (wchar_t*, const wchar_t*, size_t);

__attribute__((dllimport)) unsigned int __attribute__((__cdecl__)) _rotl(unsigned int, int);
__attribute__((dllimport)) unsigned int __attribute__((__cdecl__)) _rotr(unsigned int, int);
__attribute__((dllimport)) unsigned long __attribute__((__cdecl__)) _lrotl(unsigned long, int);
__attribute__((dllimport)) unsigned long __attribute__((__cdecl__)) _lrotr(unsigned long, int);




__attribute__((dllimport)) int __attribute__((__cdecl__)) putenv (const char*);
__attribute__((dllimport)) void __attribute__((__cdecl__)) searchenv (const char*, const char*, char*);

__attribute__((dllimport)) char* __attribute__((__cdecl__)) itoa (int, char*, int);
__attribute__((dllimport)) char* __attribute__((__cdecl__)) ltoa (long, char*, int);


__attribute__((dllimport)) char* __attribute__((__cdecl__)) ecvt (double, int, int*, int*);
__attribute__((dllimport)) char* __attribute__((__cdecl__)) fcvt (double, int, int*, int*);
__attribute__((dllimport)) char* __attribute__((__cdecl__)) gcvt (double, int, char*);
# 484 "d:/util/mingw/include/stdlib.h"
typedef struct { long long quot, rem; } lldiv_t;

lldiv_t __attribute__((__cdecl__)) lldiv (long long, long long);

extern __inline__ long long __attribute__((__cdecl__)) llabs(long long _j)
  {return (_j >= 0 ? _j : -_j);}

long long __attribute__((__cdecl__)) strtoll (const char* __restrict__, char** __restrict, int);
unsigned long long __attribute__((__cdecl__)) strtoull (const char* __restrict__, char** __restrict__, int);


long long __attribute__((__cdecl__)) atoll (const char *);


long long __attribute__((__cdecl__)) wtoll (const wchar_t *);
char* __attribute__((__cdecl__)) lltoa (long long, char *, int);
char* __attribute__((__cdecl__)) ulltoa (unsigned long long , char *, int);
wchar_t* __attribute__((__cdecl__)) lltow (long long, wchar_t *, int);
wchar_t* __attribute__((__cdecl__)) ulltow (unsigned long long, wchar_t *, int);


extern __inline__ long long __attribute__((__cdecl__)) atoll (const char * _c)
        { return _atoi64 (_c); }
extern __inline__ char* __attribute__((__cdecl__)) lltoa (long long _n, char * _c, int _i)
        { return _i64toa (_n, _c, _i); }
extern __inline__ char* __attribute__((__cdecl__)) ulltoa (unsigned long long _n, char * _c, int _i)
        { return _ui64toa (_n, _c, _i); }
extern __inline__ long long __attribute__((__cdecl__)) wtoll (const wchar_t * _w)
        { return _wtoi64 (_w); }
extern __inline__ wchar_t* __attribute__((__cdecl__)) lltow (long long _n, wchar_t * _w, int _i)
        { return _i64tow (_n, _w, _i); }
extern __inline__ wchar_t* __attribute__((__cdecl__)) ulltow (unsigned long long _n, wchar_t * _w, int _i)
        { return _ui64tow (_n, _w, _i); }
# 529 "d:/util/mingw/include/stdlib.h"
}
# 31 "f:/source-wip/CS/include/csdef.h" 2
# 1 "d:/util/mingw/include/math.h" 1
# 101 "d:/util/mingw/include/math.h"
extern "C" {
# 129 "d:/util/mingw/include/math.h"
__attribute__((dllimport)) double _HUGE;
# 139 "d:/util/mingw/include/math.h"
struct _exception
{
        int type;
        char *name;
        double arg1;
        double arg2;
        double retval;
};


__attribute__((dllimport)) double __attribute__((__cdecl__)) sin (double);
__attribute__((dllimport)) double __attribute__((__cdecl__)) cos (double);
__attribute__((dllimport)) double __attribute__((__cdecl__)) tan (double);
__attribute__((dllimport)) double __attribute__((__cdecl__)) sinh (double);
__attribute__((dllimport)) double __attribute__((__cdecl__)) cosh (double);
__attribute__((dllimport)) double __attribute__((__cdecl__)) tanh (double);
__attribute__((dllimport)) double __attribute__((__cdecl__)) asin (double);
__attribute__((dllimport)) double __attribute__((__cdecl__)) acos (double);
__attribute__((dllimport)) double __attribute__((__cdecl__)) atan (double);
__attribute__((dllimport)) double __attribute__((__cdecl__)) atan2 (double, double);
__attribute__((dllimport)) double __attribute__((__cdecl__)) exp (double);
__attribute__((dllimport)) double __attribute__((__cdecl__)) log (double);
__attribute__((dllimport)) double __attribute__((__cdecl__)) log10 (double);



        double __attribute__((__cdecl__)) pow (double, double);
__attribute__((dllimport)) double __attribute__((__cdecl__)) sqrt (double);
__attribute__((dllimport)) double __attribute__((__cdecl__)) ceil (double);
__attribute__((dllimport)) double __attribute__((__cdecl__)) floor (double);
__attribute__((dllimport)) double __attribute__((__cdecl__)) fabs (double);

extern __inline__ __attribute__((__cdecl__)) double fabs (double x)
{
  double res;
  __asm__ ("fabs;" : "=t" (res) : "0" (x));
  return res;
}

__attribute__((dllimport)) double __attribute__((__cdecl__)) ldexp (double, int);
__attribute__((dllimport)) double __attribute__((__cdecl__)) frexp (double, int*);
__attribute__((dllimport)) double __attribute__((__cdecl__)) modf (double, double*);
__attribute__((dllimport)) double __attribute__((__cdecl__)) fmod (double, double);





struct _complex
{
        double x;
        double y;
};

__attribute__((dllimport)) double __attribute__((__cdecl__)) _cabs (struct _complex);

__attribute__((dllimport)) double __attribute__((__cdecl__)) _hypot (double, double);
__attribute__((dllimport)) double __attribute__((__cdecl__)) _j0 (double);
__attribute__((dllimport)) double __attribute__((__cdecl__)) _j1 (double);
__attribute__((dllimport)) double __attribute__((__cdecl__)) _jn (int, double);
__attribute__((dllimport)) double __attribute__((__cdecl__)) _y0 (double);
__attribute__((dllimport)) double __attribute__((__cdecl__)) _y1 (double);
__attribute__((dllimport)) double __attribute__((__cdecl__)) _yn (int, double);
__attribute__((dllimport)) int __attribute__((__cdecl__)) _matherr (struct _exception *);
# 211 "d:/util/mingw/include/math.h"
__attribute__((dllimport)) double __attribute__((__cdecl__)) _chgsign (double);
__attribute__((dllimport)) double __attribute__((__cdecl__)) _copysign (double, double);
__attribute__((dllimport)) double __attribute__((__cdecl__)) _logb (double);
__attribute__((dllimport)) double __attribute__((__cdecl__)) _nextafter (double, double);
__attribute__((dllimport)) double __attribute__((__cdecl__)) _scalb (double, long);

__attribute__((dllimport)) int __attribute__((__cdecl__)) _finite (double);
__attribute__((dllimport)) int __attribute__((__cdecl__)) _fpclass (double);
__attribute__((dllimport)) int __attribute__((__cdecl__)) _isnan (double);
# 231 "d:/util/mingw/include/math.h"
__attribute__((dllimport)) double __attribute__((__cdecl__)) cabs (struct _complex);
__attribute__((dllimport)) double __attribute__((__cdecl__)) j0 (double);
__attribute__((dllimport)) double __attribute__((__cdecl__)) j1 (double);
__attribute__((dllimport)) double __attribute__((__cdecl__)) jn (int, double);
__attribute__((dllimport)) double __attribute__((__cdecl__)) y0 (double);
__attribute__((dllimport)) double __attribute__((__cdecl__)) y1 (double);
__attribute__((dllimport)) double __attribute__((__cdecl__)) yn (int, double);

__attribute__((dllimport)) double __attribute__((__cdecl__)) chgsign (double);
__attribute__((dllimport)) double __attribute__((__cdecl__)) scalb (double, long);
__attribute__((dllimport)) int __attribute__((__cdecl__)) finite (double);
__attribute__((dllimport)) int __attribute__((__cdecl__)) fpclass (double);
# 281 "d:/util/mingw/include/math.h"
extern int __attribute__((__cdecl__)) __fpclassifyf (float);
extern int __attribute__((__cdecl__)) __fpclassify (double);

extern __inline__ int __attribute__((__cdecl__)) __fpclassifyl (long double x){
  unsigned short sw;
  __asm__ ("fxam; fstsw %%ax;" : "=a" (sw): "t" (x));
  return sw & (0x0100 | 0x0400 | 0x4000 );
}
# 304 "d:/util/mingw/include/math.h"
extern __inline__ int __attribute__((__cdecl__)) __isnan (double _x)
{
  unsigned short sw;
  __asm__ ("fxam;"
           "fstsw %%ax": "=a" (sw) : "t" (_x));
  return (sw & (0x0100 | 0x0400 | (0x0100 | 0x0400) | 0x4000 | (0x0400 | 0x4000)))
    == 0x0100;
}

extern __inline__ int __attribute__((__cdecl__)) __isnanf (float _x)
{
  unsigned short sw;
  __asm__ ("fxam;"
            "fstsw %%ax": "=a" (sw) : "t" (_x));
  return (sw & (0x0100 | 0x0400 | (0x0100 | 0x0400) | 0x4000 | (0x0400 | 0x4000)))
    == 0x0100;
}

extern __inline__ int __attribute__((__cdecl__)) __isnanl (long double _x)
{
  unsigned short sw;
  __asm__ ("fxam;"
            "fstsw %%ax": "=a" (sw) : "t" (_x));
  return (sw & (0x0100 | 0x0400 | (0x0100 | 0x0400) | 0x4000 | (0x0400 | 0x4000)))
    == 0x0100;
}
# 340 "d:/util/mingw/include/math.h"
extern __inline__ int __attribute__((__cdecl__)) __signbit (double x) {
  unsigned short stw;
  __asm__ ( "fxam; fstsw %%ax;": "=a" (stw) : "t" (x));
  return stw & 0x0200;
}

extern __inline__ int __attribute__((__cdecl__)) __signbitf (float x) {
  unsigned short stw;
  __asm__ ("fxam; fstsw %%ax;": "=a" (stw) : "t" (x));
  return stw & 0x0200;
}

extern __inline__ int __attribute__((__cdecl__)) __signbitl (long double x) {
  unsigned short stw;
  __asm__ ("fxam; fstsw %%ax;": "=a" (stw) : "t" (x));
  return stw & 0x0200;
}






extern float __attribute__((__cdecl__)) sinf (float);
extern long double __attribute__((__cdecl__)) sinl (long double);

extern float __attribute__((__cdecl__)) cosf (float);
extern long double __attribute__((__cdecl__)) cosl (long double);

extern float __attribute__((__cdecl__)) tanf (float);
extern long double __attribute__((__cdecl__)) tanl (long double);

extern float __attribute__((__cdecl__)) asinf (float);
extern long double __attribute__((__cdecl__)) asinl (long double);

extern float __attribute__((__cdecl__)) acosf (float);
extern long double __attribute__((__cdecl__)) acosl (long double);

extern float __attribute__((__cdecl__)) atanf (float);
extern long double __attribute__((__cdecl__)) atanl (long double);

extern float __attribute__((__cdecl__)) atan2f (float, float);
extern long double __attribute__((__cdecl__)) atan2l (long double, long double);


extern __inline__ float __attribute__((__cdecl__)) sinhf (float x)
  {return (float) sinh (x);}
extern long double __attribute__((__cdecl__)) sinhl (long double);

extern __inline__ float __attribute__((__cdecl__)) coshf (float x)
  {return (float) cosh (x);}
extern long double __attribute__((__cdecl__)) coshl (long double);

extern __inline__ float __attribute__((__cdecl__)) tanhf (float x)
  {return (float) tanh (x);}
extern long double __attribute__((__cdecl__)) tanhl (long double);






extern __inline__ float __attribute__((__cdecl__)) expf (float x)
  {return (float) exp (x);}
extern long double __attribute__((__cdecl__)) expl (long double);


extern double __attribute__((__cdecl__)) exp2(double);
extern float __attribute__((__cdecl__)) exp2f(float);
extern long double __attribute__((__cdecl__)) exp2l(long double);




extern __inline__ float __attribute__((__cdecl__)) frexpf (float x, int* expn)
  {return (float) frexp (x, expn);}
extern long double __attribute__((__cdecl__)) frexpl (long double, int*);




extern int __attribute__((__cdecl__)) ilogb (double);
extern int __attribute__((__cdecl__)) ilogbf (float);
extern int __attribute__((__cdecl__)) ilogbl (long double);


extern __inline__ float __attribute__((__cdecl__)) ldexpf (float x, int expn)
  {return (float) ldexp (x, expn);}
extern long double __attribute__((__cdecl__)) ldexpl (long double, int);


extern float __attribute__((__cdecl__)) logf (float);
extern long double __attribute__((__cdecl__)) logl (long double);


extern float __attribute__((__cdecl__)) log10f (float);
extern long double __attribute__((__cdecl__)) log10l (long double);


extern double __attribute__((__cdecl__)) log1p(double);
extern float __attribute__((__cdecl__)) log1pf(float);
extern long double __attribute__((__cdecl__)) log1pl(long double);


extern double __attribute__((__cdecl__)) log2 (double);
extern float __attribute__((__cdecl__)) log2f (float);
extern long double __attribute__((__cdecl__)) log2l (long double);


extern double __attribute__((__cdecl__)) logb (double);
extern float __attribute__((__cdecl__)) logbf (float);
extern long double __attribute__((__cdecl__)) logbl (long double);

extern __inline__ double __attribute__((__cdecl__)) logb (double x)
{
  double res;
  __asm__ ("fxtract\n\t"
       "fstp	%%st" : "=t" (res) : "0" (x));
  return res;
}

extern __inline__ float __attribute__((__cdecl__)) logbf (float x)
{
  float res;
  __asm__ ("fxtract\n\t"
       "fstp	%%st" : "=t" (res) : "0" (x));
  return res;
}

extern __inline__ long double __attribute__((__cdecl__)) logbl (long double x)
{
  long double res;
  __asm__ ("fxtract\n\t"
       "fstp	%%st" : "=t" (res) : "0" (x));
  return res;
}


extern float __attribute__((__cdecl__)) modff (float, float*);
extern long double __attribute__((__cdecl__)) modfl (long double, long double*);


extern double __attribute__((__cdecl__)) scalbn (double, int);
extern float __attribute__((__cdecl__)) scalbnf (float, int);
extern long double __attribute__((__cdecl__)) scalbnl (long double, int);

extern double __attribute__((__cdecl__)) scalbln (double, long);
extern float __attribute__((__cdecl__)) scalblnf (float, long);
extern long double __attribute__((__cdecl__)) scalblnl (long double, long);



extern double __attribute__((__cdecl__)) cbrt (double);
extern float __attribute__((__cdecl__)) cbrtf (float);
extern long double __attribute__((__cdecl__)) cbrtl (long double);


extern __inline__ float __attribute__((__cdecl__)) fabsf (float x)
{
  float res;
  __asm__ ("fabs;" : "=t" (res) : "0" (x));
  return res;
}

extern __inline__ long double __attribute__((__cdecl__)) fabsl (long double x)
{
  long double res;
  __asm__ ("fabs;" : "=t" (res) : "0" (x));
  return res;
}


extern double __attribute__((__cdecl__)) hypot (double, double);
extern __inline__ float __attribute__((__cdecl__)) hypotf (float x, float y)
  { return (float) hypot (x, y);}
extern long double __attribute__((__cdecl__)) hypotl (long double, long double);


extern __inline__ float __attribute__((__cdecl__)) powf (float x, float y)
  {return (float) pow (x, y);}
extern long double __attribute__((__cdecl__)) powl (long double, long double);


extern float __attribute__((__cdecl__)) sqrtf (float);
extern long double __attribute__((__cdecl__)) sqrtl (long double);


extern double __attribute__((__cdecl__)) erf (double);
extern float __attribute__((__cdecl__)) erff (float);





extern double __attribute__((__cdecl__)) erfc (double);
extern float __attribute__((__cdecl__)) erfcf (float);





extern double __attribute__((__cdecl__)) lgamma (double);
extern float __attribute__((__cdecl__)) lgammaf (float);
extern long double __attribute__((__cdecl__)) lgammal (long double);


extern double __attribute__((__cdecl__)) tgamma (double);
extern float __attribute__((__cdecl__)) tgammaf (float);
extern long double __attribute__((__cdecl__)) tgammal (long double);


extern float __attribute__((__cdecl__)) ceilf (float);
extern long double __attribute__((__cdecl__)) ceill (long double);


extern float __attribute__((__cdecl__)) floorf (float);
extern long double __attribute__((__cdecl__)) floorl (long double);


extern double __attribute__((__cdecl__)) nearbyint ( double);
extern float __attribute__((__cdecl__)) nearbyintf (float);
extern long double __attribute__((__cdecl__)) nearbyintl (long double);



extern __inline__ double __attribute__((__cdecl__)) rint (double x)
{
  double retval;
  __asm__ ("frndint;": "=t" (retval) : "0" (x));
  return retval;
}

extern __inline__ float __attribute__((__cdecl__)) rintf (float x)
{
  float retval;
  __asm__ ("frndint;" : "=t" (retval) : "0" (x) );
  return retval;
}

extern __inline__ long double __attribute__((__cdecl__)) rintl (long double x)
{
  long double retval;
  __asm__ ("frndint;" : "=t" (retval) : "0" (x) );
  return retval;
}


extern __inline__ long __attribute__((__cdecl__)) lrint (double x)
{
  long retval;
  __asm__ __volatile__ ("fistpl %0" : "=m" (retval) : "t" (x) : "st"); return retval;


}

extern __inline__ long __attribute__((__cdecl__)) lrintf (float x)
{
  long retval;
  __asm__ __volatile__ ("fistpl %0" : "=m" (retval) : "t" (x) : "st"); return retval;


}

extern __inline__ long __attribute__((__cdecl__)) lrintl (long double x)
{
  long retval;
  __asm__ __volatile__ ("fistpl %0" : "=m" (retval) : "t" (x) : "st"); return retval;


}

extern __inline__ long long __attribute__((__cdecl__)) llrint (double x)
{
  long long retval;
  __asm__ __volatile__ ("fistpll %0" : "=m" (retval) : "t" (x) : "st"); return retval;


}

extern __inline__ long long __attribute__((__cdecl__)) llrintf (float x)
{
  long long retval;
  __asm__ __volatile__ ("fistpll %0" : "=m" (retval) : "t" (x) : "st"); return retval;


}

extern __inline__ long long __attribute__((__cdecl__)) llrintl (long double x)
{
  long long retval;
  __asm__ __volatile__ ("fistpll %0" : "=m" (retval) : "t" (x) : "st"); return retval;


}



extern double __attribute__((__cdecl__)) round (double);
extern float __attribute__((__cdecl__)) roundf (float);
extern long double __attribute__((__cdecl__)) roundl (long double);


extern long __attribute__((__cdecl__)) lround (double);
extern long __attribute__((__cdecl__)) lroundf (float);
extern long __attribute__((__cdecl__)) lroundl (long double);

extern long long __attribute__((__cdecl__)) llround (double);
extern long long __attribute__((__cdecl__)) llroundf (float);
extern long long __attribute__((__cdecl__)) llroundl (long double);



extern double __attribute__((__cdecl__)) trunc (double);
extern float __attribute__((__cdecl__)) truncf (float);
extern long double __attribute__((__cdecl__)) truncl (long double);


extern float __attribute__((__cdecl__)) fmodf (float, float);
extern long double __attribute__((__cdecl__)) fmodl (long double, long double);


extern double __attribute__((__cdecl__)) remainder (double, double);
extern float __attribute__((__cdecl__)) remainderf (float, float);
extern long double __attribute__((__cdecl__)) remainderl (long double, long double);


extern double __attribute__((__cdecl__)) remquo(double, double, int *);
extern float __attribute__((__cdecl__)) remquof(float, float, int *);
extern long double __attribute__((__cdecl__)) remquol(long double, long double, int *);


extern double __attribute__((__cdecl__)) copysign (double, double);
extern float __attribute__((__cdecl__)) copysignf (float, float);
extern long double __attribute__((__cdecl__)) copysignl (long double, long double);


extern double __attribute__((__cdecl__)) nan(const char *tagp);
extern float __attribute__((__cdecl__)) nanf(const char *tagp);
extern long double __attribute__((__cdecl__)) nanl(const char *tagp);
# 687 "d:/util/mingw/include/math.h"
extern double __attribute__((__cdecl__)) nextafter (double, double);
extern float __attribute__((__cdecl__)) nextafterf (float, float);







extern double __attribute__((__cdecl__)) fdim (double x, double y);
extern float __attribute__((__cdecl__)) fdimf (float x, float y);
extern long double __attribute__((__cdecl__)) fdiml (long double x, long double y);







extern double __attribute__((__cdecl__)) fmax (double, double);
extern float __attribute__((__cdecl__)) fmaxf (float, float);
extern long double __attribute__((__cdecl__)) fmaxl (long double, long double);


extern double __attribute__((__cdecl__)) fmin (double, double);
extern float __attribute__((__cdecl__)) fminf (float, float);
extern long double __attribute__((__cdecl__)) fminl (long double, long double);



extern double __attribute__((__cdecl__)) fma (double, double, double);
extern float __attribute__((__cdecl__)) fmaf (float, float, float);
extern long double __attribute__((__cdecl__)) fmal (long double, long double, long double);
# 742 "d:/util/mingw/include/math.h"
extern __inline__ int __attribute__((__cdecl__))
__fp_unordered_compare (long double x, long double y){
  unsigned short retval;
  __asm__ ("fucom %%st(1);"
           "fnstsw;": "=a" (retval) : "t" (x), "u" (y));
  return retval;
}
# 771 "d:/util/mingw/include/math.h"
}
# 32 "f:/source-wip/CS/include/csdef.h" 2
# 1 "d:/util/mingw/include/time.h" 1
# 37 "d:/util/mingw/include/time.h"
# 1 "d:/util/mingw/include/stddef.h" 1





# 1 "d:/util/mingw/include/stddef.h" 1 3





# 1 "d:/util/mingw/lib/gcc-lib/mingw32/3.2.3/include/stddef.h" 1 3
# 7 "d:/util/mingw/include/stddef.h" 2 3
# 7 "d:/util/mingw/include/stddef.h" 2
# 38 "d:/util/mingw/include/time.h" 2





# 1 "d:/util/mingw/include/sys/types.h" 1
# 38 "d:/util/mingw/include/sys/types.h"
# 1 "d:/util/mingw/include/stddef.h" 1





# 1 "d:/util/mingw/include/stddef.h" 1 3





# 1 "d:/util/mingw/lib/gcc-lib/mingw32/3.2.3/include/stddef.h" 1 3
# 153 "d:/util/mingw/lib/gcc-lib/mingw32/3.2.3/include/stddef.h" 3
typedef int ptrdiff_t;
# 7 "d:/util/mingw/include/stddef.h" 2 3
# 7 "d:/util/mingw/include/stddef.h" 2
# 39 "d:/util/mingw/include/sys/types.h" 2





typedef long time_t;
# 53 "d:/util/mingw/include/sys/types.h"
typedef long _off_t;


typedef _off_t off_t;







typedef unsigned int _dev_t;





typedef _dev_t dev_t;






typedef short _ino_t;


typedef _ino_t ino_t;






typedef int _pid_t;


typedef _pid_t pid_t;






typedef unsigned short _mode_t;


typedef _mode_t mode_t;






typedef int _sigset_t;


typedef _sigset_t sigset_t;
# 44 "d:/util/mingw/include/time.h" 2
# 69 "d:/util/mingw/include/time.h"
typedef long clock_t;
# 78 "d:/util/mingw/include/time.h"
struct tm
{
        int tm_sec;
        int tm_min;
        int tm_hour;
        int tm_mday;
        int tm_mon;
        int tm_year;
        int tm_wday;
        int tm_yday;
        int tm_isdst;

};


extern "C" {


__attribute__((dllimport)) clock_t __attribute__((__cdecl__)) clock (void);
__attribute__((dllimport)) time_t __attribute__((__cdecl__)) time (time_t*);
__attribute__((dllimport)) double __attribute__((__cdecl__)) difftime (time_t, time_t);
__attribute__((dllimport)) time_t __attribute__((__cdecl__)) mktime (struct tm*);
# 111 "d:/util/mingw/include/time.h"
__attribute__((dllimport)) char* __attribute__((__cdecl__)) asctime (const struct tm*);
__attribute__((dllimport)) char* __attribute__((__cdecl__)) ctime (const time_t*);
__attribute__((dllimport)) struct tm* __attribute__((__cdecl__)) gmtime (const time_t*);
__attribute__((dllimport)) struct tm* __attribute__((__cdecl__)) localtime (const time_t*);


__attribute__((dllimport)) size_t __attribute__((__cdecl__)) strftime (char*, size_t, const char*, const struct tm*);

__attribute__((dllimport)) size_t __attribute__((__cdecl__)) wcsftime (wchar_t*, size_t, const wchar_t*, const struct tm*);


extern __attribute__((dllimport)) void __attribute__((__cdecl__)) _tzset (void);


extern __attribute__((dllimport)) void __attribute__((__cdecl__)) tzset (void);


__attribute__((dllimport)) char* __attribute__((__cdecl__)) _strdate(char*);
__attribute__((dllimport)) char* __attribute__((__cdecl__)) _strtime(char*);
# 142 "d:/util/mingw/include/time.h"
extern __attribute__((dllimport)) int* __attribute__((__cdecl__)) __p__daylight (void);
extern __attribute__((dllimport)) long* __attribute__((__cdecl__)) __p__timezone (void);
extern __attribute__((dllimport)) char** __attribute__((__cdecl__)) __p__tzname (void);

__attribute__((dllimport)) int _daylight;
__attribute__((dllimport)) long _timezone;
__attribute__((dllimport)) char *_tzname[2];
# 180 "d:/util/mingw/include/time.h"
__attribute__((dllimport)) int daylight;
__attribute__((dllimport)) long timezone;
__attribute__((dllimport)) char *tzname[2];





__attribute__((dllimport)) wchar_t* __attribute__((__cdecl__)) _wasctime(const struct tm*);
__attribute__((dllimport)) wchar_t* __attribute__((__cdecl__)) _wctime(const time_t*);
__attribute__((dllimport)) wchar_t* __attribute__((__cdecl__)) _wstrdate(wchar_t*);
__attribute__((dllimport)) wchar_t* __attribute__((__cdecl__)) _wstrtime(wchar_t*);
# 213 "d:/util/mingw/include/time.h"
}
# 33 "f:/source-wip/CS/include/csdef.h" 2
# 1 "d:/util/mingw/include/signal.h" 1
# 62 "d:/util/mingw/include/signal.h"
typedef int sig_atomic_t;
# 76 "d:/util/mingw/include/signal.h"
typedef void (*__p_sig_fn_t)(int);
# 88 "d:/util/mingw/include/signal.h"
extern "C" {
# 97 "d:/util/mingw/include/signal.h"
__attribute__((dllimport)) __p_sig_fn_t __attribute__((__cdecl__)) signal(int, __p_sig_fn_t);




__attribute__((dllimport)) int __attribute__((__cdecl__)) raise (int);


}
# 34 "f:/source-wip/CS/include/csdef.h" 2
# 1 "d:/util/mingw/include/errno.h" 1
# 96 "d:/util/mingw/include/errno.h"
extern "C" {
# 107 "d:/util/mingw/include/errno.h"
__attribute__((dllimport)) int* __attribute__((__cdecl__)) _errno(void);




}
# 35 "f:/source-wip/CS/include/csdef.h" 2
# 1 "d:/util/mingw/include/string.h" 1
# 40 "d:/util/mingw/include/string.h"
# 1 "d:/util/mingw/include/stddef.h" 1





# 1 "d:/util/mingw/include/stddef.h" 1 3





# 1 "d:/util/mingw/lib/gcc-lib/mingw32/3.2.3/include/stddef.h" 1 3
# 7 "d:/util/mingw/include/stddef.h" 2 3
# 7 "d:/util/mingw/include/stddef.h" 2
# 41 "d:/util/mingw/include/string.h" 2





extern "C" {





__attribute__((dllimport)) void* __attribute__((__cdecl__)) memchr (const void*, int, size_t);
__attribute__((dllimport)) int __attribute__((__cdecl__)) memcmp (const void*, const void*, size_t);
__attribute__((dllimport)) void* __attribute__((__cdecl__)) memcpy (void*, const void*, size_t);
__attribute__((dllimport)) void* __attribute__((__cdecl__)) memmove (void*, const void*, size_t);
__attribute__((dllimport)) void* __attribute__((__cdecl__)) memset (void*, int, size_t);
__attribute__((dllimport)) char* __attribute__((__cdecl__)) strcat (char*, const char*);
__attribute__((dllimport)) char* __attribute__((__cdecl__)) strchr (const char*, int);
__attribute__((dllimport)) int __attribute__((__cdecl__)) strcmp (const char*, const char*);
__attribute__((dllimport)) int __attribute__((__cdecl__)) strcoll (const char*, const char*);
__attribute__((dllimport)) char* __attribute__((__cdecl__)) strcpy (char*, const char*);
__attribute__((dllimport)) size_t __attribute__((__cdecl__)) strcspn (const char*, const char*);
__attribute__((dllimport)) char* __attribute__((__cdecl__)) strerror (int);

__attribute__((dllimport)) size_t __attribute__((__cdecl__)) strlen (const char*);
__attribute__((dllimport)) char* __attribute__((__cdecl__)) strncat (char*, const char*, size_t);
__attribute__((dllimport)) int __attribute__((__cdecl__)) strncmp (const char*, const char*, size_t);
__attribute__((dllimport)) char* __attribute__((__cdecl__)) strncpy (char*, const char*, size_t);
__attribute__((dllimport)) char* __attribute__((__cdecl__)) strpbrk (const char*, const char*);
__attribute__((dllimport)) char* __attribute__((__cdecl__)) strrchr (const char*, int);
__attribute__((dllimport)) size_t __attribute__((__cdecl__)) strspn (const char*, const char*);
__attribute__((dllimport)) char* __attribute__((__cdecl__)) strstr (const char*, const char*);
__attribute__((dllimport)) char* __attribute__((__cdecl__)) strtok (char*, const char*);
__attribute__((dllimport)) size_t __attribute__((__cdecl__)) strxfrm (char*, const char*, size_t);





__attribute__((dllimport)) char* __attribute__((__cdecl__)) _strerror (const char *);
__attribute__((dllimport)) void* __attribute__((__cdecl__)) _memccpy (void*, const void*, int, size_t);
__attribute__((dllimport)) int __attribute__((__cdecl__)) _memicmp (const void*, const void*, size_t);
__attribute__((dllimport)) char* __attribute__((__cdecl__)) _strdup (const char*);
__attribute__((dllimport)) int __attribute__((__cdecl__)) _strcmpi (const char*, const char*);
__attribute__((dllimport)) int __attribute__((__cdecl__)) _stricmp (const char*, const char*);
__attribute__((dllimport)) int __attribute__((__cdecl__)) _stricoll (const char*, const char*);
__attribute__((dllimport)) char* __attribute__((__cdecl__)) _strlwr (char*);
__attribute__((dllimport)) int __attribute__((__cdecl__)) _strnicmp (const char*, const char*, size_t);
__attribute__((dllimport)) char* __attribute__((__cdecl__)) _strnset (char*, int, size_t);
__attribute__((dllimport)) char* __attribute__((__cdecl__)) _strrev (char*);
__attribute__((dllimport)) char* __attribute__((__cdecl__)) _strset (char*, int);
__attribute__((dllimport)) char* __attribute__((__cdecl__)) _strupr (char*);
__attribute__((dllimport)) void __attribute__((__cdecl__)) _swab (const char*, char*, size_t);


__attribute__((dllimport)) int __attribute__((__cdecl__)) _strncoll(const char*, const char*, size_t);
__attribute__((dllimport)) int __attribute__((__cdecl__)) _strnicoll(const char*, const char*, size_t);







__attribute__((dllimport)) wchar_t* __attribute__((__cdecl__)) wcscat (wchar_t*, const wchar_t*);
__attribute__((dllimport)) wchar_t* __attribute__((__cdecl__)) wcschr (const wchar_t*, wchar_t);
__attribute__((dllimport)) int __attribute__((__cdecl__)) wcscmp (const wchar_t*, const wchar_t*);
__attribute__((dllimport)) int __attribute__((__cdecl__)) wcscoll (const wchar_t*, const wchar_t*);
__attribute__((dllimport)) wchar_t* __attribute__((__cdecl__)) wcscpy (wchar_t*, const wchar_t*);
__attribute__((dllimport)) size_t __attribute__((__cdecl__)) wcscspn (const wchar_t*, const wchar_t*);

__attribute__((dllimport)) size_t __attribute__((__cdecl__)) wcslen (const wchar_t*);
__attribute__((dllimport)) wchar_t* __attribute__((__cdecl__)) wcsncat (wchar_t*, const wchar_t*, size_t);
__attribute__((dllimport)) int __attribute__((__cdecl__)) wcsncmp(const wchar_t*, const wchar_t*, size_t);
__attribute__((dllimport)) wchar_t* __attribute__((__cdecl__)) wcsncpy(wchar_t*, const wchar_t*, size_t);
__attribute__((dllimport)) wchar_t* __attribute__((__cdecl__)) wcspbrk(const wchar_t*, const wchar_t*);
__attribute__((dllimport)) wchar_t* __attribute__((__cdecl__)) wcsrchr(const wchar_t*, wchar_t);
__attribute__((dllimport)) size_t __attribute__((__cdecl__)) wcsspn(const wchar_t*, const wchar_t*);
__attribute__((dllimport)) wchar_t* __attribute__((__cdecl__)) wcsstr(const wchar_t*, const wchar_t*);
__attribute__((dllimport)) wchar_t* __attribute__((__cdecl__)) wcstok(wchar_t*, const wchar_t*);
__attribute__((dllimport)) size_t __attribute__((__cdecl__)) wcsxfrm(wchar_t*, const wchar_t*, size_t);
# 131 "d:/util/mingw/include/string.h"
__attribute__((dllimport)) wchar_t* __attribute__((__cdecl__)) _wcsdup (const wchar_t*);
__attribute__((dllimport)) int __attribute__((__cdecl__)) _wcsicmp (const wchar_t*, const wchar_t*);
__attribute__((dllimport)) int __attribute__((__cdecl__)) _wcsicoll (const wchar_t*, const wchar_t*);
__attribute__((dllimport)) wchar_t* __attribute__((__cdecl__)) _wcslwr (wchar_t*);
__attribute__((dllimport)) int __attribute__((__cdecl__)) _wcsnicmp (const wchar_t*, const wchar_t*, size_t);
__attribute__((dllimport)) wchar_t* __attribute__((__cdecl__)) _wcsnset (wchar_t*, wchar_t, size_t);
__attribute__((dllimport)) wchar_t* __attribute__((__cdecl__)) _wcsrev (wchar_t*);
__attribute__((dllimport)) wchar_t* __attribute__((__cdecl__)) _wcsset (wchar_t*, wchar_t);
__attribute__((dllimport)) wchar_t* __attribute__((__cdecl__)) _wcsupr (wchar_t*);


__attribute__((dllimport)) int __attribute__((__cdecl__)) _wcsncoll(const wchar_t*, const wchar_t*, size_t);
__attribute__((dllimport)) int __attribute__((__cdecl__)) _wcsnicoll(const wchar_t*, const wchar_t*, size_t);
# 159 "d:/util/mingw/include/string.h"
__attribute__((dllimport)) void* __attribute__((__cdecl__)) memccpy (void*, const void*, int, size_t);
__attribute__((dllimport)) int __attribute__((__cdecl__)) memicmp (const void*, const void*, size_t);
__attribute__((dllimport)) char* __attribute__((__cdecl__)) strdup (const char*);
__attribute__((dllimport)) int __attribute__((__cdecl__)) strcmpi (const char*, const char*);
__attribute__((dllimport)) int __attribute__((__cdecl__)) stricmp (const char*, const char*);
extern __inline__ int __attribute__((__cdecl__))
strcasecmp (const char * __sz1, const char * __sz2)
  {return _stricmp (__sz1, __sz2);}
__attribute__((dllimport)) int __attribute__((__cdecl__)) stricoll (const char*, const char*);
__attribute__((dllimport)) char* __attribute__((__cdecl__)) strlwr (char*);
__attribute__((dllimport)) int __attribute__((__cdecl__)) strnicmp (const char*, const char*, size_t);
extern __inline__ int __attribute__((__cdecl__))
strncasecmp (const char * __sz1, const char * __sz2, size_t __sizeMaxCompare)
  {return _strnicmp (__sz1, __sz2, __sizeMaxCompare);}
__attribute__((dllimport)) char* __attribute__((__cdecl__)) strnset (char*, int, size_t);
__attribute__((dllimport)) char* __attribute__((__cdecl__)) strrev (char*);
__attribute__((dllimport)) char* __attribute__((__cdecl__)) strset (char*, int);
__attribute__((dllimport)) char* __attribute__((__cdecl__)) strupr (char*);

__attribute__((dllimport)) void __attribute__((__cdecl__)) swab (const char*, char*, size_t);



extern __inline__ int __attribute__((__cdecl__))
wcscmpi (const wchar_t * __ws1, const wchar_t * __ws2)
  {return _wcsicmp (__ws1, __ws2);}
__attribute__((dllimport)) wchar_t* __attribute__((__cdecl__)) wcsdup (wchar_t*);
__attribute__((dllimport)) int __attribute__((__cdecl__)) wcsicmp (const wchar_t*, const wchar_t*);
__attribute__((dllimport)) int __attribute__((__cdecl__)) wcsicoll (const wchar_t*, const wchar_t*);
__attribute__((dllimport)) wchar_t* __attribute__((__cdecl__)) wcslwr (wchar_t*);
__attribute__((dllimport)) int __attribute__((__cdecl__)) wcsnicmp (const wchar_t*, const wchar_t*, size_t);
__attribute__((dllimport)) wchar_t* __attribute__((__cdecl__)) wcsnset (wchar_t*, wchar_t, size_t);
__attribute__((dllimport)) wchar_t* __attribute__((__cdecl__)) wcsrev (wchar_t*);
__attribute__((dllimport)) wchar_t* __attribute__((__cdecl__)) wcsset (wchar_t*, wchar_t);
__attribute__((dllimport)) wchar_t* __attribute__((__cdecl__)) wcsupr (wchar_t*);






}
# 36 "f:/source-wip/CS/include/csdef.h" 2
# 1 "d:/util/mingw/include/assert.h" 1
# 36 "d:/util/mingw/include/assert.h"
extern "C" {
# 52 "d:/util/mingw/include/assert.h"
__attribute__((dllimport)) void __attribute__((__cdecl__)) _assert (const char*, const char*, int)



        ;
# 65 "d:/util/mingw/include/assert.h"
}
# 37 "f:/source-wip/CS/include/csdef.h" 2
# 27 "f:/source-wip/CS/include/cssysdef.h" 2
# 73 "f:/source-wip/CS/include/cssysdef.h"
# 1 "f:/source-wip/CS/include/cssys/csosdefs.h" 1
# 30 "f:/source-wip/CS/include/cssys/csosdefs.h"
# 1 "f:/source-wip/CS/include/cssys/win32/csosdefs.h" 1
# 152 "f:/source-wip/CS/include/cssys/win32/csosdefs.h"
# 1 "d:/util/mingw/include/excpt.h" 1
# 39 "d:/util/mingw/include/excpt.h"
# 1 "d:/util/mingw/include/windef.h" 1







extern "C" {
# 222 "d:/util/mingw/include/windef.h"
typedef unsigned long DWORD;
typedef int WINBOOL,*PWINBOOL,*LPWINBOOL;



typedef WINBOOL BOOL;



typedef unsigned char BYTE;

typedef BOOL *PBOOL,*LPBOOL;
typedef unsigned short WORD;
typedef float FLOAT;
typedef FLOAT *PFLOAT;
typedef BYTE *PBYTE,*LPBYTE;
typedef int *PINT,*LPINT;
typedef WORD *PWORD,*LPWORD;
typedef long *LPLONG;
typedef DWORD *PDWORD,*LPDWORD;
typedef const void *PCVOID,*LPCVOID;
typedef int INT;
typedef unsigned int UINT,*PUINT,*LPUINT;

# 1 "d:/util/mingw/include/winnt.h" 1
# 31 "d:/util/mingw/include/winnt.h"
extern "C" {


# 1 "d:/util/mingw/include/winerror.h" 1
# 35 "d:/util/mingw/include/winnt.h" 2
# 49 "d:/util/mingw/include/winnt.h"
typedef char CHAR;
typedef short SHORT;
typedef long LONG;
typedef char CCHAR, *PCCHAR;
typedef unsigned char UCHAR,*PUCHAR;
typedef unsigned short USHORT,*PUSHORT;
typedef unsigned long ULONG,*PULONG;
typedef char *PSZ;

typedef void *PVOID,*LPVOID;





typedef void* PVOID64;
# 77 "d:/util/mingw/include/winnt.h"
typedef wchar_t WCHAR;
typedef WCHAR *PWCHAR,*LPWCH,*PWCH,*NWPSTR,*LPWSTR,*PWSTR;
typedef const WCHAR *LPCWCH,*PCWCH,*LPCWSTR,*PCWSTR;
typedef CHAR *PCHAR,*LPCH,*PCH,*NPSTR,*LPSTR,*PSTR;
typedef const CHAR *LPCCH,*PCSTR,*LPCSTR;
# 92 "d:/util/mingw/include/winnt.h"
typedef CHAR TCHAR;
typedef CHAR _TCHAR;


typedef TCHAR TBYTE,*PTCH,*PTBYTE;
typedef TCHAR *LPTCH,*PTSTR,*LPTSTR,*LP,*PTCHAR;
typedef const TCHAR *LPCTSTR;
# 117 "d:/util/mingw/include/winnt.h"
typedef SHORT *PSHORT;
typedef LONG *PLONG;
typedef void *HANDLE;
typedef HANDLE *PHANDLE,*LPHANDLE;





typedef DWORD LCID;
typedef PDWORD PLCID;
typedef WORD LANGID;
# 141 "d:/util/mingw/include/winnt.h"
typedef double LONGLONG,DWORDLONG;

typedef LONGLONG *PLONGLONG;
typedef DWORDLONG *PDWORDLONG;
typedef DWORDLONG ULONGLONG,*PULONGLONG;
typedef LONGLONG USN;
# 156 "d:/util/mingw/include/winnt.h"
typedef BYTE BOOLEAN,*PBOOLEAN;

typedef BYTE FCHAR;
typedef WORD FSHORT;
typedef DWORD FLONG;


# 1 "d:/util/mingw/include/basetsd.h" 1
# 48 "d:/util/mingw/include/basetsd.h"
extern "C" {

typedef int LONG32, *PLONG32;

typedef int INT32, *PINT32;

typedef unsigned int ULONG32, *PULONG32;
typedef unsigned int DWORD32, *PDWORD32;
typedef unsigned int UINT32, *PUINT32;
# 97 "d:/util/mingw/include/basetsd.h"
typedef int INT_PTR, *PINT_PTR;
typedef unsigned int UINT_PTR, *PUINT_PTR;
typedef long LONG_PTR, *PLONG_PTR;
typedef unsigned long ULONG_PTR, *PULONG_PTR;
typedef unsigned short UHALF_PTR, *PUHALF_PTR;
typedef short HALF_PTR, *PHALF_PTR;
typedef unsigned long HANDLE_PTR;


typedef ULONG_PTR SIZE_T, *PSIZE_T;
typedef LONG_PTR SSIZE_T, *PSSIZE_T;
typedef ULONG_PTR DWORD_PTR, *PDWORD_PTR;
typedef __int64 LONG64, *PLONG64;
typedef __int64 INT64, *PINT64;
typedef unsigned __int64 ULONG64, *PULONG64;
typedef unsigned __int64 DWORD64, *PDWORD64;
typedef unsigned __int64 UINT64, *PUINT64;

}
# 164 "d:/util/mingw/include/winnt.h" 2
# 1155 "d:/util/mingw/include/winnt.h"
typedef DWORD ACCESS_MASK, *PACCESS_MASK;







typedef struct _GUID {
        unsigned long Data1;
        unsigned short Data2;
        unsigned short Data3;
        unsigned char Data4[8];
} GUID, *REFGUID, *LPGUID;


typedef struct _GENERIC_MAPPING {
        ACCESS_MASK GenericRead;
        ACCESS_MASK GenericWrite;
        ACCESS_MASK GenericExecute;
        ACCESS_MASK GenericAll;
} GENERIC_MAPPING, *PGENERIC_MAPPING;
typedef struct _ACE_HEADER {
        BYTE AceType;
        BYTE AceFlags;
        WORD AceSize;
} ACE_HEADER, *PACE_HEADER;
typedef struct _ACCESS_ALLOWED_ACE {
        ACE_HEADER Header;
        ACCESS_MASK Mask;
        DWORD SidStart;
} ACCESS_ALLOWED_ACE, *PACCESS_ALLOWED_ACE;
typedef struct _ACCESS_DENIED_ACE {
        ACE_HEADER Header;
        ACCESS_MASK Mask;
        DWORD SidStart;
} ACCESS_DENIED_ACE, *PACCESS_DENIED_ACE;
typedef struct _SYSTEM_AUDIT_ACE {
        ACE_HEADER Header;
        ACCESS_MASK Mask;
        DWORD SidStart;
} SYSTEM_AUDIT_ACE;
typedef SYSTEM_AUDIT_ACE *PSYSTEM_AUDIT_ACE;
typedef struct _SYSTEM_ALARM_ACE {
        ACE_HEADER Header;
        ACCESS_MASK Mask;
        DWORD SidStart;
} SYSTEM_ALARM_ACE,*PSYSTEM_ALARM_ACE;
typedef struct _ACCESS_ALLOWED_OBJECT_ACE {
        ACE_HEADER Header;
        ACCESS_MASK Mask;
        DWORD Flags;
        GUID ObjectType;
        GUID InheritedObjectType;
        DWORD SidStart;
} ACCESS_ALLOWED_OBJECT_ACE,*PACCESS_ALLOWED_OBJECT_ACE;
typedef struct _ACCESS_DENIED_OBJECT_ACE {
        ACE_HEADER Header;
        ACCESS_MASK Mask;
        DWORD Flags;
        GUID ObjectType;
        GUID InheritedObjectType;
        DWORD SidStart;
} ACCESS_DENIED_OBJECT_ACE,*PACCESS_DENIED_OBJECT_ACE;
typedef struct _SYSTEM_AUDIT_OBJECT_ACE {
        ACE_HEADER Header;
        ACCESS_MASK Mask;
        DWORD Flags;
        GUID ObjectType;
        GUID InheritedObjectType;
        DWORD SidStart;
} SYSTEM_AUDIT_OBJECT_ACE,*PSYSTEM_AUDIT_OBJECT_ACE;
typedef struct _SYSTEM_ALARM_OBJECT_ACE {
        ACE_HEADER Header;
        ACCESS_MASK Mask;
        DWORD Flags;
        GUID ObjectType;
        GUID InheritedObjectType;
        DWORD SidStart;
} SYSTEM_ALARM_OBJECT_ACE,*PSYSTEM_ALARM_OBJECT_ACE;
typedef struct _ACL {
        BYTE AclRevision;
        BYTE Sbz1;
        WORD AclSize;
        WORD AceCount;
        WORD Sbz2;
} ACL,*PACL;
typedef struct _ACL_REVISION_INFORMATION {
        DWORD AclRevision;
} ACL_REVISION_INFORMATION;
typedef struct _ACL_SIZE_INFORMATION {
        DWORD AceCount;
        DWORD AclBytesInUse;
        DWORD AclBytesFree;
} ACL_SIZE_INFORMATION;
# 1264 "d:/util/mingw/include/winnt.h"
typedef struct _FLOATING_SAVE_AREA {
        DWORD ControlWord;
        DWORD StatusWord;
        DWORD TagWord;
        DWORD ErrorOffset;
        DWORD ErrorSelector;
        DWORD DataOffset;
        DWORD DataSelector;
        BYTE RegisterArea[80];
        DWORD Cr0NpxState;
} FLOATING_SAVE_AREA;
typedef struct _CONTEXT {
        DWORD ContextFlags;
        DWORD Dr0;
        DWORD Dr1;
        DWORD Dr2;
        DWORD Dr3;
        DWORD Dr6;
        DWORD Dr7;
        FLOATING_SAVE_AREA FloatSave;
        DWORD SegGs;
        DWORD SegFs;
        DWORD SegEs;
        DWORD SegDs;
        DWORD Edi;
        DWORD Esi;
        DWORD Ebx;
        DWORD Edx;
        DWORD Ecx;
        DWORD Eax;
        DWORD Ebp;
        DWORD Eip;
        DWORD SegCs;
        DWORD EFlags;
        DWORD Esp;
        DWORD SegSs;
        BYTE ExtendedRegisters[512];
} CONTEXT;
# 1783 "d:/util/mingw/include/winnt.h"
typedef CONTEXT *PCONTEXT,*LPCONTEXT;
typedef struct _EXCEPTION_RECORD {
        DWORD ExceptionCode;
        DWORD ExceptionFlags;
        struct _EXCEPTION_RECORD *ExceptionRecord;
        PVOID ExceptionAddress;
        DWORD NumberParameters;
        DWORD ExceptionInformation[15];
} EXCEPTION_RECORD,*PEXCEPTION_RECORD,*LPEXCEPTION_RECORD;
typedef struct _EXCEPTION_POINTERS {
        PEXCEPTION_RECORD ExceptionRecord;
        PCONTEXT ContextRecord;
} EXCEPTION_POINTERS,*PEXCEPTION_POINTERS,*LPEXCEPTION_POINTERS;
typedef union _LARGE_INTEGER {
  struct {
    DWORD LowPart;
    LONG HighPart;
  } u;

  struct {
    DWORD LowPart;
    LONG HighPart;
  };

  LONGLONG QuadPart;
} LARGE_INTEGER, *PLARGE_INTEGER;
typedef union _ULARGE_INTEGER {
  struct {
    DWORD LowPart;
    DWORD HighPart;
  } u;

  struct {
    DWORD LowPart;
    DWORD HighPart;
  };

  ULONGLONG QuadPart;
} ULARGE_INTEGER, *PULARGE_INTEGER;
typedef LARGE_INTEGER LUID,*PLUID;
#pragma pack(push,4)
typedef struct _LUID_AND_ATTRIBUTES {
        LUID Luid;
        DWORD Attributes;
} LUID_AND_ATTRIBUTES, *PLUID_AND_ATTRIBUTES;
#pragma pack(pop)
typedef LUID_AND_ATTRIBUTES LUID_AND_ATTRIBUTES_ARRAY[1];
typedef LUID_AND_ATTRIBUTES_ARRAY *PLUID_AND_ATTRIBUTES_ARRAY;
typedef struct _PRIVILEGE_SET {
        DWORD PrivilegeCount;
        DWORD Control;
        LUID_AND_ATTRIBUTES Privilege[1];
} PRIVILEGE_SET,*PPRIVILEGE_SET;
typedef struct _SECURITY_ATTRIBUTES {
        DWORD nLength;
        LPVOID lpSecurityDescriptor;
        BOOL bInheritHandle;
} SECURITY_ATTRIBUTES,*PSECURITY_ATTRIBUTES,*LPSECURITY_ATTRIBUTES;
typedef enum _SECURITY_IMPERSONATION_LEVEL {
        SecurityAnonymous,
        SecurityIdentification,
        SecurityImpersonation,
        SecurityDelegation
} SECURITY_IMPERSONATION_LEVEL,*PSECURITY_IMPERSONATION_LEVEL;
typedef BOOLEAN SECURITY_CONTEXT_TRACKING_MODE,*PSECURITY_CONTEXT_TRACKING_MODE;
typedef struct _SECURITY_QUALITY_OF_SERVICE {
        DWORD Length;
        SECURITY_IMPERSONATION_LEVEL ImpersonationLevel;
        SECURITY_CONTEXT_TRACKING_MODE ContextTrackingMode;
        BOOLEAN EffectiveOnly;
} SECURITY_QUALITY_OF_SERVICE,*PSECURITY_QUALITY_OF_SERVICE;
typedef PVOID PACCESS_TOKEN;
typedef struct _SE_IMPERSONATION_STATE {
        PACCESS_TOKEN Token;
        BOOLEAN CopyOnOpen;
        BOOLEAN EffectiveOnly;
        SECURITY_IMPERSONATION_LEVEL Level;
} SE_IMPERSONATION_STATE,*PSE_IMPERSONATION_STATE;
typedef struct _SID_IDENTIFIER_AUTHORITY {
        BYTE Value[6];
} SID_IDENTIFIER_AUTHORITY,*PSID_IDENTIFIER_AUTHORITY,*LPSID_IDENTIFIER_AUTHORITY;
typedef PVOID PSID;
typedef struct _SID {
   BYTE Revision;
   BYTE SubAuthorityCount;
   SID_IDENTIFIER_AUTHORITY IdentifierAuthority;
   DWORD SubAuthority[1];
} SID, *PISID;
typedef struct _SID_AND_ATTRIBUTES {
        PSID Sid;
        DWORD Attributes;
} SID_AND_ATTRIBUTES, *PSID_AND_ATTRIBUTES;
typedef SID_AND_ATTRIBUTES SID_AND_ATTRIBUTES_ARRAY[1];
typedef SID_AND_ATTRIBUTES_ARRAY *PSID_AND_ATTRIBUTES_ARRAY;
typedef struct _TOKEN_SOURCE {
        CHAR SourceName[8];
        LUID SourceIdentifier;
} TOKEN_SOURCE,*PTOKEN_SOURCE;
typedef struct _TOKEN_CONTROL {
        LUID TokenId;
        LUID AuthenticationId;
        LUID ModifiedId;
        TOKEN_SOURCE TokenSource;
} TOKEN_CONTROL,*PTOKEN_CONTROL;
typedef struct _TOKEN_DEFAULT_DACL {
        PACL DefaultDacl;
} TOKEN_DEFAULT_DACL,*PTOKEN_DEFAULT_DACL;
typedef struct _TOKEN_GROUPS {
        DWORD GroupCount;
        SID_AND_ATTRIBUTES Groups[1];
} TOKEN_GROUPS,*PTOKEN_GROUPS,*LPTOKEN_GROUPS;
typedef struct _TOKEN_OWNER {
        PSID Owner;
} TOKEN_OWNER,*PTOKEN_OWNER;
typedef struct _TOKEN_PRIMARY_GROUP {
        PSID PrimaryGroup;
} TOKEN_PRIMARY_GROUP,*PTOKEN_PRIMARY_GROUP;
typedef struct _TOKEN_PRIVILEGES {
        DWORD PrivilegeCount;
        LUID_AND_ATTRIBUTES Privileges[1];
} TOKEN_PRIVILEGES,*PTOKEN_PRIVILEGES,*LPTOKEN_PRIVILEGES;
typedef enum tagTOKEN_TYPE { TokenPrimary=1,TokenImpersonation }TOKEN_TYPE, *PTOKEN_TYPE;
typedef struct _TOKEN_STATISTICS {
        LUID TokenId;
        LUID AuthenticationId;
        LARGE_INTEGER ExpirationTime;
        TOKEN_TYPE TokenType;
        SECURITY_IMPERSONATION_LEVEL ImpersonationLevel;
        DWORD DynamicCharged;
        DWORD DynamicAvailable;
        DWORD GroupCount;
        DWORD PrivilegeCount;
        LUID ModifiedId;
} TOKEN_STATISTICS, *PTOKEN_STATISTICS;
typedef struct _TOKEN_USER {
        SID_AND_ATTRIBUTES User;
} TOKEN_USER, *PTOKEN_USER;
typedef DWORD SECURITY_INFORMATION,*PSECURITY_INFORMATION;
typedef WORD SECURITY_DESCRIPTOR_CONTROL,*PSECURITY_DESCRIPTOR_CONTROL;
typedef struct _SECURITY_DESCRIPTOR {
        BYTE Revision;
        BYTE Sbz1;
        SECURITY_DESCRIPTOR_CONTROL Control;
        PSID Owner;
        PSID Group;
        PACL Sacl;
        PACL Dacl;
} SECURITY_DESCRIPTOR, *PSECURITY_DESCRIPTOR, *PISECURITY_DESCRIPTOR;
typedef enum _TOKEN_INFORMATION_CLASS {
        TokenUser=1,TokenGroups,TokenPrivileges,TokenOwner,
        TokenPrimaryGroup,TokenDefaultDacl,TokenSource,TokenType,
        TokenImpersonationLevel,TokenStatistics,TokenRestrictedSids,
        TokenSessionId
} TOKEN_INFORMATION_CLASS;
typedef enum _SID_NAME_USE {
        SidTypeUser=1,SidTypeGroup,SidTypeDomain,SidTypeAlias,SidTypeWellKnownGroup,
        SidTypeDeletedAccount,SidTypeInvalid,SidTypeUnknown
} SID_NAME_USE,*PSID_NAME_USE;
typedef struct _QUOTA_LIMITS {
        SIZE_T PagedPoolLimit;
        SIZE_T NonPagedPoolLimit;
        SIZE_T MinimumWorkingSetSize;
        SIZE_T MaximumWorkingSetSize;
        SIZE_T PagefileLimit;
        LARGE_INTEGER TimeLimit;
} QUOTA_LIMITS,*PQUOTA_LIMITS;
typedef struct _IO_COUNTERS {
        ULONGLONG ReadOperationCount;
        ULONGLONG WriteOperationCount;
        ULONGLONG OtherOperationCount;
        ULONGLONG ReadTransferCount;
        ULONGLONG WriteTransferCount;
        ULONGLONG OtherTransferCount;
} IO_COUNTERS, *PIO_COUNTERS;
typedef struct _FILE_NOTIFY_INFORMATION {
        DWORD NextEntryOffset;
        DWORD Action;
        DWORD FileNameLength;
        WCHAR FileName[1];
} FILE_NOTIFY_INFORMATION,*PFILE_NOTIFY_INFORMATION;
typedef struct _TAPE_ERASE {
        DWORD Type;
        BOOLEAN Immediate;
} TAPE_ERASE,*PTAPE_ERASE;
typedef struct _TAPE_GET_DRIVE_PARAMETERS {
        BOOLEAN ECC;
        BOOLEAN Compression;
        BOOLEAN DataPadding;
        BOOLEAN ReportSetmarks;
        DWORD DefaultBlockSize;
        DWORD MaximumBlockSize;
        DWORD MinimumBlockSize;
        DWORD MaximumPartitionCount;
        DWORD FeaturesLow;
        DWORD FeaturesHigh;
        DWORD EOTWarningZoneSize;
} TAPE_GET_DRIVE_PARAMETERS,*PTAPE_GET_DRIVE_PARAMETERS;
typedef struct _TAPE_GET_MEDIA_PARAMETERS {
        LARGE_INTEGER Capacity;
        LARGE_INTEGER Remaining;
        DWORD BlockSize;
        DWORD PartitionCount;
        BOOLEAN WriteProtected;
} TAPE_GET_MEDIA_PARAMETERS,*PTAPE_GET_MEDIA_PARAMETERS;
typedef struct _TAPE_GET_POSITION {
        ULONG Type;
        ULONG Partition;
        ULONG OffsetLow;
        ULONG OffsetHigh;
} TAPE_GET_POSITION,*PTAPE_GET_POSITION;
typedef struct _TAPE_PREPARE {
        DWORD Operation;
        BOOLEAN Immediate;
} TAPE_PREPARE,*PTAPE_PREPARE;
typedef struct _TAPE_SET_DRIVE_PARAMETERS {
        BOOLEAN ECC;
        BOOLEAN Compression;
        BOOLEAN DataPadding;
        BOOLEAN ReportSetmarks;
        ULONG EOTWarningZoneSize;
} TAPE_SET_DRIVE_PARAMETERS,*PTAPE_SET_DRIVE_PARAMETERS;
typedef struct _TAPE_SET_MEDIA_PARAMETERS {
        ULONG BlockSize;
} TAPE_SET_MEDIA_PARAMETERS,*PTAPE_SET_MEDIA_PARAMETERS;
typedef struct _TAPE_SET_POSITION {
        DWORD Method;
        DWORD Partition;
        LARGE_INTEGER Offset;
        BOOLEAN Immediate;
} TAPE_SET_POSITION,*PTAPE_SET_POSITION;
typedef struct _TAPE_WRITE_MARKS {
        DWORD Type;
        DWORD Count;
        BOOLEAN Immediate;
} TAPE_WRITE_MARKS,*PTAPE_WRITE_MARKS;
typedef struct _TAPE_CREATE_PARTITION {
        DWORD Method;
        DWORD Count;
        DWORD Size;
} TAPE_CREATE_PARTITION,*PTAPE_CREATE_PARTITION;
typedef struct _MEMORY_BASIC_INFORMATION {
        PVOID BaseAddress;
        PVOID AllocationBase;
        DWORD AllocationProtect;
        DWORD RegionSize;
        DWORD State;
        DWORD Protect;
        DWORD Type;
} MEMORY_BASIC_INFORMATION,*PMEMORY_BASIC_INFORMATION;
typedef struct _MESSAGE_RESOURCE_ENTRY {
        WORD Length;
        WORD Flags;
        BYTE Text[1];
} MESSAGE_RESOURCE_ENTRY,*PMESSAGE_RESOURCE_ENTRY;
typedef struct _MESSAGE_RESOURCE_BLOCK {
        DWORD LowId;
        DWORD HighId;
        DWORD OffsetToEntries;
} MESSAGE_RESOURCE_BLOCK,*PMESSAGE_RESOURCE_BLOCK;
typedef struct _MESSAGE_RESOURCE_DATA {
        DWORD NumberOfBlocks;
        MESSAGE_RESOURCE_BLOCK Blocks[1];
} MESSAGE_RESOURCE_DATA,*PMESSAGE_RESOURCE_DATA;
typedef struct _LIST_ENTRY {
        struct _LIST_ENTRY *Flink;
        struct _LIST_ENTRY *Blink;
} LIST_ENTRY,*PLIST_ENTRY;
typedef struct _RTL_CRITICAL_SECTION_DEBUG {
        WORD Type;
        WORD CreatorBackTraceIndex;
        struct _RTL_CRITICAL_SECTION *CriticalSection;
        LIST_ENTRY ProcessLocksList;
        DWORD EntryCount;
        DWORD ContentionCount;
        DWORD Spare[2];
} RTL_CRITICAL_SECTION_DEBUG,*PRTL_CRITICAL_SECTION_DEBUG;
typedef struct _RTL_CRITICAL_SECTION {
        PRTL_CRITICAL_SECTION_DEBUG DebugInfo;
        LONG LockCount;
        LONG RecursionCount;
        HANDLE OwningThread;
        HANDLE LockSemaphore;
        DWORD Reserved;
} RTL_CRITICAL_SECTION,*PRTL_CRITICAL_SECTION;
typedef struct _EVENTLOGRECORD {
        DWORD Length;
        DWORD Reserved;
        DWORD RecordNumber;
        DWORD TimeGenerated;
        DWORD TimeWritten;
        DWORD EventID;
        WORD EventType;
        WORD NumStrings;
        WORD EventCategory;
        WORD ReservedFlags;
        DWORD ClosingRecordNumber;
        DWORD StringOffset;
        DWORD UserSidLength;
        DWORD UserSidOffset;
        DWORD DataLength;
        DWORD DataOffset;
} EVENTLOGRECORD,*PEVENTLOGRECORD;
typedef struct _OSVERSIONINFOA {
        DWORD dwOSVersionInfoSize;
        DWORD dwMajorVersion;
        DWORD dwMinorVersion;
        DWORD dwBuildNumber;
        DWORD dwPlatformId;
        CHAR szCSDVersion[128];
} OSVERSIONINFOA,*POSVERSIONINFOA,*LPOSVERSIONINFOA;
typedef struct _OSVERSIONINFOW {
        DWORD dwOSVersionInfoSize;
        DWORD dwMajorVersion;
        DWORD dwMinorVersion;
        DWORD dwBuildNumber;
        DWORD dwPlatformId;
        WCHAR szCSDVersion[128];
} OSVERSIONINFOW,*POSVERSIONINFOW,*LPOSVERSIONINFOW;
typedef struct _OSVERSIONINFOEXA {
        DWORD dwOSVersionInfoSize;
        DWORD dwMajorVersion;
        DWORD dwMinorVersion;
        DWORD dwBuildNumber;
        DWORD dwPlatformId;
        CHAR szCSDVersion[128];
        WORD wServicePackMajor;
        WORD wServicePackMinor;
        WORD wSuiteMask;
        BYTE wProductType;
        BYTE wReserved;
} OSVERSIONINFOEXA, *POSVERSIONINFOEXA, *LPOSVERSIONINFOEXA;
typedef struct _OSVERSIONINFOEXW {
        DWORD dwOSVersionInfoSize;
        DWORD dwMajorVersion;
        DWORD dwMinorVersion;
        DWORD dwBuildNumber;
        DWORD dwPlatformId;
        WCHAR szCSDVersion[128];
        WORD wServicePackMajor;
        WORD wServicePackMinor;
        WORD wSuiteMask;
        BYTE wProductType;
        BYTE wReserved;
} OSVERSIONINFOEXW, *POSVERSIONINFOEXW, *LPOSVERSIONINFOEXW;
#pragma pack(push,2)
typedef struct _IMAGE_VXD_HEADER {
        WORD e32_magic;
        BYTE e32_border;
        BYTE e32_worder;
        DWORD e32_level;
        WORD e32_cpu;
        WORD e32_os;
        DWORD e32_ver;
        DWORD e32_mflags;
        DWORD e32_mpages;
        DWORD e32_startobj;
        DWORD e32_eip;
        DWORD e32_stackobj;
        DWORD e32_esp;
        DWORD e32_pagesize;
        DWORD e32_lastpagesize;
        DWORD e32_fixupsize;
        DWORD e32_fixupsum;
        DWORD e32_ldrsize;
        DWORD e32_ldrsum;
        DWORD e32_objtab;
        DWORD e32_objcnt;
        DWORD e32_objmap;
        DWORD e32_itermap;
        DWORD e32_rsrctab;
        DWORD e32_rsrccnt;
        DWORD e32_restab;
        DWORD e32_enttab;
        DWORD e32_dirtab;
        DWORD e32_dircnt;
        DWORD e32_fpagetab;
        DWORD e32_frectab;
        DWORD e32_impmod;
        DWORD e32_impmodcnt;
        DWORD e32_impproc;
        DWORD e32_pagesum;
        DWORD e32_datapage;
        DWORD e32_preload;
        DWORD e32_nrestab;
        DWORD e32_cbnrestab;
        DWORD e32_nressum;
        DWORD e32_autodata;
        DWORD e32_debuginfo;
        DWORD e32_debuglen;
        DWORD e32_instpreload;
        DWORD e32_instdemand;
        DWORD e32_heapsize;
        BYTE e32_res3[12];
        DWORD e32_winresoff;
        DWORD e32_winreslen;
        WORD e32_devid;
        WORD e32_ddkver;
} IMAGE_VXD_HEADER,*PIMAGE_VXD_HEADER;
#pragma pack(pop)
#pragma pack(push,4)
typedef struct _IMAGE_FILE_HEADER {
        WORD Machine;
        WORD NumberOfSections;
        DWORD TimeDateStamp;
        DWORD PointerToSymbolTable;
        DWORD NumberOfSymbols;
        WORD SizeOfOptionalHeader;
        WORD Characteristics;
} IMAGE_FILE_HEADER, *PIMAGE_FILE_HEADER;
typedef struct _IMAGE_DATA_DIRECTORY {
        DWORD VirtualAddress;
        DWORD Size;
} IMAGE_DATA_DIRECTORY,*PIMAGE_DATA_DIRECTORY;
typedef struct _IMAGE_OPTIONAL_HEADER {
        WORD Magic;
        BYTE MajorLinkerVersion;
        BYTE MinorLinkerVersion;
        DWORD SizeOfCode;
        DWORD SizeOfInitializedData;
        DWORD SizeOfUninitializedData;
        DWORD AddressOfEntryPoint;
        DWORD BaseOfCode;
        DWORD BaseOfData;
        DWORD ImageBase;
        DWORD SectionAlignment;
        DWORD FileAlignment;
        WORD MajorOperatingSystemVersion;
        WORD MinorOperatingSystemVersion;
        WORD MajorImageVersion;
        WORD MinorImageVersion;
        WORD MajorSubsystemVersion;
        WORD MinorSubsystemVersion;
        DWORD Reserved1;
        DWORD SizeOfImage;
        DWORD SizeOfHeaders;
        DWORD CheckSum;
        WORD Subsystem;
        WORD DllCharacteristics;
        DWORD SizeOfStackReserve;
        DWORD SizeOfStackCommit;
        DWORD SizeOfHeapReserve;
        DWORD SizeOfHeapCommit;
        DWORD LoaderFlags;
        DWORD NumberOfRvaAndSizes;
        IMAGE_DATA_DIRECTORY DataDirectory[16];
} IMAGE_OPTIONAL_HEADER,*PIMAGE_OPTIONAL_HEADER;
typedef struct _IMAGE_ROM_OPTIONAL_HEADER {
        WORD Magic;
        BYTE MajorLinkerVersion;
        BYTE MinorLinkerVersion;
        DWORD SizeOfCode;
        DWORD SizeOfInitializedData;
        DWORD SizeOfUninitializedData;
        DWORD AddressOfEntryPoint;
        DWORD BaseOfCode;
        DWORD BaseOfData;
        DWORD BaseOfBss;
        DWORD GprMask;
        DWORD CprMask[4];
        DWORD GpValue;
} IMAGE_ROM_OPTIONAL_HEADER,*PIMAGE_ROM_OPTIONAL_HEADER;
#pragma pack(pop)
#pragma pack(push,2)
typedef struct _IMAGE_DOS_HEADER {
        WORD e_magic;
        WORD e_cblp;
        WORD e_cp;
        WORD e_crlc;
        WORD e_cparhdr;
        WORD e_minalloc;
        WORD e_maxalloc;
        WORD e_ss;
        WORD e_sp;
        WORD e_csum;
        WORD e_ip;
        WORD e_cs;
        WORD e_lfarlc;
        WORD e_ovno;
        WORD e_res[4];
        WORD e_oemid;
        WORD e_oeminfo;
        WORD e_res2[10];
        LONG e_lfanew;
} IMAGE_DOS_HEADER,*PIMAGE_DOS_HEADER;
typedef struct _IMAGE_OS2_HEADER {
        WORD ne_magic;
        CHAR ne_ver;
        CHAR ne_rev;
        WORD ne_enttab;
        WORD ne_cbenttab;
        LONG ne_crc;
        WORD ne_flags;
        WORD ne_autodata;
        WORD ne_heap;
        WORD ne_stack;
        LONG ne_csip;
        LONG ne_sssp;
        WORD ne_cseg;
        WORD ne_cmod;
        WORD ne_cbnrestab;
        WORD ne_segtab;
        WORD ne_rsrctab;
        WORD ne_restab;
        WORD ne_modtab;
        WORD ne_imptab;
        LONG ne_nrestab;
        WORD ne_cmovent;
        WORD ne_align;
        WORD ne_cres;
        BYTE ne_exetyp;
        BYTE ne_flagsothers;
        WORD ne_pretthunks;
        WORD ne_psegrefbytes;
        WORD ne_swaparea;
        WORD ne_expver;
} IMAGE_OS2_HEADER,*PIMAGE_OS2_HEADER;
#pragma pack(pop)
#pragma pack(push,4)
typedef struct _IMAGE_NT_HEADERS {
        DWORD Signature;
        IMAGE_FILE_HEADER FileHeader;
        IMAGE_OPTIONAL_HEADER OptionalHeader;
} IMAGE_NT_HEADERS,*PIMAGE_NT_HEADERS;
typedef struct _IMAGE_ROM_HEADERS {
        IMAGE_FILE_HEADER FileHeader;
        IMAGE_ROM_OPTIONAL_HEADER OptionalHeader;
} IMAGE_ROM_HEADERS,*PIMAGE_ROM_HEADERS;
typedef struct _IMAGE_SECTION_HEADER {
        BYTE Name[8];
        union {
                DWORD PhysicalAddress;
                DWORD VirtualSize;
        } Misc;
        DWORD VirtualAddress;
        DWORD SizeOfRawData;
        DWORD PointerToRawData;
        DWORD PointerToRelocations;
        DWORD PointerToLinenumbers;
        WORD NumberOfRelocations;
        WORD NumberOfLinenumbers;
        DWORD Characteristics;
} IMAGE_SECTION_HEADER,*PIMAGE_SECTION_HEADER;
#pragma pack(pop)
#pragma pack(push,2)
typedef struct _IMAGE_SYMBOL {
        union {
                BYTE ShortName[8];
                struct {
                        DWORD Short;
                        DWORD Long;
                } Name;
                PBYTE LongName[2];
        } N;
        DWORD Value;
        SHORT SectionNumber;
        WORD Type;
        BYTE StorageClass;
        BYTE NumberOfAuxSymbols;
} IMAGE_SYMBOL,*PIMAGE_SYMBOL;
typedef union _IMAGE_AUX_SYMBOL {
        struct {
                DWORD TagIndex;
                union {
                        struct {
                                WORD Linenumber;
                                WORD Size;
                        } LnSz;
                        DWORD TotalSize;
                } Misc;
                union {
                        struct {
                                DWORD PointerToLinenumber;
                                DWORD PointerToNextFunction;
                        } Function;
                        struct {
                                WORD Dimension[4];
                        } Array;
                } FcnAry;
                WORD TvIndex;
        } Sym;
        struct {
                BYTE Name[18];
        } File;
        struct {
                DWORD Length;
                WORD NumberOfRelocations;
                WORD NumberOfLinenumbers;
                DWORD CheckSum;
                SHORT Number;
                BYTE Selection;
        } Section;
} IMAGE_AUX_SYMBOL,*PIMAGE_AUX_SYMBOL;
typedef struct _IMAGE_COFF_SYMBOLS_HEADER {
        DWORD NumberOfSymbols;
        DWORD LvaToFirstSymbol;
        DWORD NumberOfLinenumbers;
        DWORD LvaToFirstLinenumber;
        DWORD RvaToFirstByteOfCode;
        DWORD RvaToLastByteOfCode;
        DWORD RvaToFirstByteOfData;
        DWORD RvaToLastByteOfData;
} IMAGE_COFF_SYMBOLS_HEADER,*PIMAGE_COFF_SYMBOLS_HEADER;
typedef struct _IMAGE_RELOCATION {
        union {
                DWORD VirtualAddress;
                DWORD RelocCount;
        } ;
        DWORD SymbolTableIndex;
        WORD Type;
} IMAGE_RELOCATION,*PIMAGE_RELOCATION;
#pragma pack(pop)
#pragma pack(push,4)
typedef struct _IMAGE_BASE_RELOCATION {
        DWORD VirtualAddress;
        DWORD SizeOfBlock;
} IMAGE_BASE_RELOCATION,*PIMAGE_BASE_RELOCATION;
#pragma pack(pop)
#pragma pack(push,2)
typedef struct _IMAGE_LINENUMBER {
        union {
                DWORD SymbolTableIndex;
                DWORD VirtualAddress;
        } Type;
        WORD Linenumber;
} IMAGE_LINENUMBER,*PIMAGE_LINENUMBER;
#pragma pack(pop)
#pragma pack(push,4)
typedef struct _IMAGE_ARCHIVE_MEMBER_HEADER {
        BYTE Name[16];
        BYTE Date[12];
        BYTE UserID[6];
        BYTE GroupID[6];
        BYTE Mode[8];
        BYTE Size[10];
        BYTE EndHeader[2];
} IMAGE_ARCHIVE_MEMBER_HEADER,*PIMAGE_ARCHIVE_MEMBER_HEADER;
typedef struct _IMAGE_EXPORT_DIRECTORY {
        DWORD Characteristics;
        DWORD TimeDateStamp;
        WORD MajorVersion;
        WORD MinorVersion;
        DWORD Name;
        DWORD Base;
        DWORD NumberOfFunctions;
        DWORD NumberOfNames;
        PDWORD *AddressOfFunctions;
        PDWORD *AddressOfNames;
        PWORD *AddressOfNameOrdinals;
} IMAGE_EXPORT_DIRECTORY,*PIMAGE_EXPORT_DIRECTORY;
typedef struct _IMAGE_IMPORT_BY_NAME {
        WORD Hint;
        BYTE Name[1];
} IMAGE_IMPORT_BY_NAME,*PIMAGE_IMPORT_BY_NAME;
typedef struct _IMAGE_THUNK_DATA {
        union {
                PBYTE ForwarderString;
                PDWORD Function;
                DWORD Ordinal;
                PIMAGE_IMPORT_BY_NAME AddressOfData;
        } u1;
} IMAGE_THUNK_DATA,*PIMAGE_THUNK_DATA;
typedef struct _IMAGE_IMPORT_DESCRIPTOR {
        union {
                DWORD Characteristics;
                PIMAGE_THUNK_DATA OriginalFirstThunk;
        } ;
        DWORD TimeDateStamp;
        DWORD ForwarderChain;
        DWORD Name;
        PIMAGE_THUNK_DATA FirstThunk;
} IMAGE_IMPORT_DESCRIPTOR,*PIMAGE_IMPORT_DESCRIPTOR;
typedef struct _IMAGE_BOUND_IMPORT_DESCRIPTOR {
        DWORD TimeDateStamp;
        WORD OffsetModuleName;
        WORD NumberOfModuleForwarderRefs;
} IMAGE_BOUND_IMPORT_DESCRIPTOR,*PIMAGE_BOUND_IMPORT_DESCRIPTOR;
typedef struct _IMAGE_BOUND_FORWARDER_REF {
        DWORD TimeDateStamp;
        WORD OffsetModuleName;
        WORD Reserved;
} IMAGE_BOUND_FORWARDER_REF,*PIMAGE_BOUND_FORWARDER_REF;
typedef void(__attribute__((__stdcall__)) *PIMAGE_TLS_CALLBACK)(PVOID,DWORD,PVOID);
typedef struct _IMAGE_TLS_DIRECTORY {
        DWORD StartAddressOfRawData;
        DWORD EndAddressOfRawData;
        PDWORD AddressOfIndex;
        PIMAGE_TLS_CALLBACK *AddressOfCallBacks;
        DWORD SizeOfZeroFill;
        DWORD Characteristics;
} IMAGE_TLS_DIRECTORY,*PIMAGE_TLS_DIRECTORY;
typedef struct _IMAGE_RESOURCE_DIRECTORY {
        DWORD Characteristics;
        DWORD TimeDateStamp;
        WORD MajorVersion;
        WORD MinorVersion;
        WORD NumberOfNamedEntries;
        WORD NumberOfIdEntries;
} IMAGE_RESOURCE_DIRECTORY,*PIMAGE_RESOURCE_DIRECTORY;
 typedef struct _IMAGE_RESOURCE_DIRECTORY_ENTRY {
        union {
                struct {
                        DWORD NameOffset:31;
                        DWORD NameIsString:1;
                };
                DWORD Name;
                WORD Id;
        } ;
        union {
                DWORD OffsetToData;
                struct {
                        DWORD OffsetToDirectory:31;
                        DWORD DataIsDirectory:1;
                } ;
        } ;
} IMAGE_RESOURCE_DIRECTORY_ENTRY,*PIMAGE_RESOURCE_DIRECTORY_ENTRY;
typedef struct _IMAGE_RESOURCE_DIRECTORY_STRING {
        WORD Length;
        CHAR NameString[1];
} IMAGE_RESOURCE_DIRECTORY_STRING,*PIMAGE_RESOURCE_DIRECTORY_STRING;
typedef struct _IMAGE_RESOURCE_DIR_STRING_U {
        WORD Length;
        WCHAR NameString[1];
} IMAGE_RESOURCE_DIR_STRING_U,*PIMAGE_RESOURCE_DIR_STRING_U;
typedef struct _IMAGE_RESOURCE_DATA_ENTRY {
        DWORD OffsetToData;
        DWORD Size;
        DWORD CodePage;
        DWORD Reserved;
} IMAGE_RESOURCE_DATA_ENTRY,*PIMAGE_RESOURCE_DATA_ENTRY;
typedef struct _IMAGE_LOAD_CONFIG_DIRECTORY {
        DWORD Characteristics;
        DWORD TimeDateStamp;
        WORD MajorVersion;
        WORD MinorVersion;
        DWORD GlobalFlagsClear;
        DWORD GlobalFlagsSet;
        DWORD CriticalSectionDefaultTimeout;
        DWORD DeCommitFreeBlockThreshold;
        DWORD DeCommitTotalFreeThreshold;
        PVOID LockPrefixTable;
        DWORD MaximumAllocationSize;
        DWORD VirtualMemoryThreshold;
        DWORD ProcessHeapFlags;
        DWORD Reserved[4];
} IMAGE_LOAD_CONFIG_DIRECTORY,*PIMAGE_LOAD_CONFIG_DIRECTORY;
typedef struct _IMAGE_RUNTIME_FUNCTION_ENTRY {
        DWORD BeginAddress;
        DWORD EndAddress;
        PVOID ExceptionHandler;
        PVOID HandlerData;
        DWORD PrologEndAddress;
} IMAGE_RUNTIME_FUNCTION_ENTRY,*PIMAGE_RUNTIME_FUNCTION_ENTRY;
typedef struct _IMAGE_DEBUG_DIRECTORY {
        DWORD Characteristics;
        DWORD TimeDateStamp;
        WORD MajorVersion;
        WORD MinorVersion;
        DWORD Type;
        DWORD SizeOfData;
        DWORD AddressOfRawData;
        DWORD PointerToRawData;
} IMAGE_DEBUG_DIRECTORY,*PIMAGE_DEBUG_DIRECTORY;
typedef struct _FPO_DATA {
        DWORD ulOffStart;
        DWORD cbProcSize;
        DWORD cdwLocals;
        WORD cdwParams;
        WORD cbProlog:8;
        WORD cbRegs:3;
        WORD fHasSEH:1;
        WORD fUseBP:1;
        WORD reserved:1;
        WORD cbFrame:2;
} FPO_DATA,*PFPO_DATA;
typedef struct _IMAGE_DEBUG_MISC {
        DWORD DataType;
        DWORD Length;
        BOOLEAN Unicode;
        BYTE Reserved[3];
        BYTE Data[1];
} IMAGE_DEBUG_MISC,*PIMAGE_DEBUG_MISC;
typedef struct _IMAGE_FUNCTION_ENTRY {
        DWORD StartingAddress;
        DWORD EndingAddress;
        DWORD EndOfPrologue;
} IMAGE_FUNCTION_ENTRY,*PIMAGE_FUNCTION_ENTRY;
typedef struct _IMAGE_SEPARATE_DEBUG_HEADER {
        WORD Signature;
        WORD Flags;
        WORD Machine;
        WORD Characteristics;
        DWORD TimeDateStamp;
        DWORD CheckSum;
        DWORD ImageBase;
        DWORD SizeOfImage;
        DWORD NumberOfSections;
        DWORD ExportedNamesSize;
        DWORD DebugDirectorySize;
        DWORD SectionAlignment;
        DWORD Reserved[2];
} IMAGE_SEPARATE_DEBUG_HEADER,*PIMAGE_SEPARATE_DEBUG_HEADER;
#pragma pack(pop)
typedef enum _CM_SERVICE_NODE_TYPE {
        DriverType=1,
        FileSystemType=2,
        Win32ServiceOwnProcess=16,
        Win32ServiceShareProcess=32,
        AdapterType=4,
        RecognizerType=8
} SERVICE_NODE_TYPE;
typedef enum _CM_SERVICE_LOAD_TYPE {
        BootLoad=0,
        SystemLoad=1,
        AutoLoad=2,
        DemandLoad=3,
        DisableLoad=4
} SERVICE_LOAD_TYPE;
typedef enum _CM_ERROR_CONTROL_TYPE {
        IgnoreError=0,
        NormalError=1,
        SevereError=2,
        CriticalError=3
} SERVICE_ERROR_TYPE;
typedef struct _NT_TIB {
        struct _EXCEPTION_REGISTRATION_RECORD *ExceptionList;
        PVOID StackBase;
        PVOID StackLimit;
        PVOID SubSystemTib;
        union {
                PVOID FiberData;
                DWORD Version;
        } ;
        PVOID ArbitraryUserPointer;
        struct _NT_TIB *Self;
} NT_TIB,*PNT_TIB;
typedef struct _REPARSE_DATA_BUFFER {
        DWORD ReparseTag;
        WORD ReparseDataLength;
        WORD Reserved;
        union {
                struct {
                        WORD SubstituteNameOffset;
                        WORD SubstituteNameLength;
                        WORD PrintNameOffset;
                        WORD PrintNameLength;
                        WCHAR PathBuffer[1];
                } SymbolicLinkReparseBuffer;
                struct {
                        WORD SubstituteNameOffset;
                        WORD SubstituteNameLength;
                        WORD PrintNameOffset;
                        WORD PrintNameLength;
                        WCHAR PathBuffer[1];
                } MountPointReparseBuffer;
                struct {
                        BYTE DataBuffer[1];
                } GenericReparseBuffer;
        } ;
} REPARSE_DATA_BUFFER, *PREPARSE_DATA_BUFFER;
typedef struct _REPARSE_GUID_DATA_BUFFER {
        DWORD ReparseTag;
        WORD ReparseDataLength;
        WORD Reserved;
        GUID ReparseGuid;
        struct {
                BYTE DataBuffer[1];
        } GenericReparseBuffer;
} REPARSE_GUID_DATA_BUFFER, *PREPARSE_GUID_DATA_BUFFER;
typedef struct _REPARSE_POINT_INFORMATION {
        WORD ReparseDataLength;
        WORD UnparsedNameLength;
} REPARSE_POINT_INFORMATION, *PREPARSE_POINT_INFORMATION;

typedef union _FILE_SEGMENT_ELEMENT {
        PVOID64 Buffer;
        ULONGLONG Alignment;
}FILE_SEGMENT_ELEMENT, *PFILE_SEGMENT_ELEMENT;
# 2706 "d:/util/mingw/include/winnt.h"
typedef enum _JOBOBJECTINFOCLASS {
        JobObjectBasicAccountingInformation = 1,
        JobObjectBasicLimitInformation,
        JobObjectBasicProcessIdList,
        JobObjectBasicUIRestrictions,
        JobObjectSecurityLimitInformation,
        JobObjectEndOfJobTimeInformation,
        JobObjectAssociateCompletionPortInformation,
        JobObjectBasicAndIoAccountingInformation,
        JobObjectExtendedLimitInformation,
        JobObjectJobSetInformation,
        MaxJobObjectInfoClass
} JOBOBJECTINFOCLASS;
typedef struct _JOBOBJECT_BASIC_ACCOUNTING_INFORMATION {
        LARGE_INTEGER TotalUserTime;
        LARGE_INTEGER TotalKernelTime;
        LARGE_INTEGER ThisPeriodTotalUserTime;
        LARGE_INTEGER ThisPeriodTotalKernelTime;
        DWORD TotalPageFaultCount;
        DWORD TotalProcesses;
        DWORD ActiveProcesses;
        DWORD TotalTerminatedProcesses;
} JOBOBJECT_BASIC_ACCOUNTING_INFORMATION,*PJOBOBJECT_BASIC_ACCOUNTING_INFORMATION;
typedef struct _JOBOBJECT_BASIC_LIMIT_INFORMATION {
        LARGE_INTEGER PerProcessUserTimeLimit;
        LARGE_INTEGER PerJobUserTimeLimit;
        DWORD LimitFlags;
        SIZE_T MinimumWorkingSetSize;
        SIZE_T MaximumWorkingSetSize;
        DWORD ActiveProcessLimit;
        ULONG_PTR Affinity;
        DWORD PriorityClass;
        DWORD SchedulingClass;
} JOBOBJECT_BASIC_LIMIT_INFORMATION,*PJOBOBJECT_BASIC_LIMIT_INFORMATION;
typedef struct _JOBOBJECT_BASIC_PROCESS_ID_LIST {
        DWORD NumberOfAssignedProcesses;
        DWORD NumberOfProcessIdsInList;
        ULONG_PTR ProcessIdList[1];
} JOBOBJECT_BASIC_PROCESS_ID_LIST, *PJOBOBJECT_BASIC_PROCESS_ID_LIST;
typedef struct _JOBOBJECT_BASIC_UI_RESTRICTIONS {
        DWORD UIRestrictionsClass;
} JOBOBJECT_BASIC_UI_RESTRICTIONS,*PJOBOBJECT_BASIC_UI_RESTRICTIONS;
typedef struct _JOBOBJECT_SECURITY_LIMIT_INFORMATION {
        DWORD SecurityLimitFlags;
        HANDLE JobToken;
        PTOKEN_GROUPS SidsToDisable;
        PTOKEN_PRIVILEGES PrivilegesToDelete;
        PTOKEN_GROUPS RestrictedSids;
} JOBOBJECT_SECURITY_LIMIT_INFORMATION,*PJOBOBJECT_SECURITY_LIMIT_INFORMATION;
typedef struct _JOBOBJECT_END_OF_JOB_TIME_INFORMATION {
        DWORD EndOfJobTimeAction;
} JOBOBJECT_END_OF_JOB_TIME_INFORMATION,*PJOBOBJECT_END_OF_JOB_TIME_INFORMATION;
typedef struct _JOBOBJECT_ASSOCIATE_COMPLETION_PORT {
        PVOID CompletionKey;
        HANDLE CompletionPort;
} JOBOBJECT_ASSOCIATE_COMPLETION_PORT,*PJOBOBJECT_ASSOCIATE_COMPLETION_PORT;
typedef struct _JOBOBJECT_BASIC_AND_IO_ACCOUNTING_INFORMATION {
        JOBOBJECT_BASIC_ACCOUNTING_INFORMATION BasicInfo;
        IO_COUNTERS IoInfo;
} JOBOBJECT_BASIC_AND_IO_ACCOUNTING_INFORMATION,*PJOBOBJECT_BASIC_AND_IO_ACCOUNTING_INFORMATION;
typedef struct _JOBOBJECT_EXTENDED_LIMIT_INFORMATION {
        JOBOBJECT_BASIC_LIMIT_INFORMATION BasicLimitInformation;
        IO_COUNTERS IoInfo;
        SIZE_T ProcessMemoryLimit;
        SIZE_T JobMemoryLimit;
        SIZE_T PeakProcessMemoryUsed;
        SIZE_T PeakJobMemoryUsed;
} JOBOBJECT_EXTENDED_LIMIT_INFORMATION,*PJOBOBJECT_EXTENDED_LIMIT_INFORMATION;
typedef struct _JOBOBJECT_JOBSET_INFORMATION {
        DWORD MemberLevel;
} JOBOBJECT_JOBSET_INFORMATION,*PJOBOBJECT_JOBSET_INFORMATION;



# 1 "d:/util/mingw/include/pshpack4.h" 1

#pragma pack(push,4)
# 2781 "d:/util/mingw/include/winnt.h" 2






typedef enum _LATENCY_TIME {
        LT_DONT_CARE,
        LT_LOWEST_LATENCY
} LATENCY_TIME, *PLATENCY_TIME;

typedef enum _SYSTEM_POWER_STATE {
        PowerSystemUnspecified,
        PowerSystemWorking,
        PowerSystemSleeping1,
        PowerSystemSleeping2,
        PowerSystemSleeping3,
        PowerSystemHibernate,
        PowerSystemShutdown,
        PowerSystemMaximum
} SYSTEM_POWER_STATE, *PSYSTEM_POWER_STATE;


typedef enum {
        PowerActionNone,
        PowerActionReserved,
        PowerActionSleep,
        PowerActionHibernate,
        PowerActionShutdown,
        PowerActionShutdownReset,
        PowerActionShutdownOff,
        PowerActionWarmEject
} POWER_ACTION, *PPOWER_ACTION;

typedef enum _DEVICE_POWER_STATE {
        PowerDeviceUnspecified,
        PowerDeviceD0,
        PowerDeviceD1,
        PowerDeviceD2,
        PowerDeviceD3,
        PowerDeviceMaximum
} DEVICE_POWER_STATE, *PDEVICE_POWER_STATE;

typedef struct {
        DWORD Granularity;
        DWORD Capacity;
} BATTERY_REPORTING_SCALE, *PBATTERY_REPORTING_SCALE;

typedef struct _POWER_ACTION_POLICY {
        POWER_ACTION Action;
        ULONG Flags;
        ULONG EventCode;
} POWER_ACTION_POLICY, *PPOWER_ACTION_POLICY;
# 2862 "d:/util/mingw/include/winnt.h"
typedef struct _SYSTEM_POWER_LEVEL {
        BOOLEAN Enable;
        UCHAR Spare[3];
        ULONG BatteryLevel;
        POWER_ACTION_POLICY PowerPolicy;
        SYSTEM_POWER_STATE MinSystemState;
} SYSTEM_POWER_LEVEL, *PSYSTEM_POWER_LEVEL;

typedef struct _SYSTEM_POWER_POLICY {
        ULONG Revision;
        POWER_ACTION_POLICY PowerButton;
        POWER_ACTION_POLICY SleepButton;
        POWER_ACTION_POLICY LidClose;
        SYSTEM_POWER_STATE LidOpenWake;
        ULONG Reserved;
        POWER_ACTION_POLICY Idle;
        ULONG IdleTimeout;
        UCHAR IdleSensitivity;
        UCHAR DynamicThrottle;
        UCHAR Spare2[2];
        SYSTEM_POWER_STATE MinSleep;
        SYSTEM_POWER_STATE MaxSleep;
        SYSTEM_POWER_STATE ReducedLatencySleep;
        ULONG WinLogonFlags;
        ULONG Spare3;
        ULONG DozeS4Timeout;
        ULONG BroadcastCapacityResolution;
        SYSTEM_POWER_LEVEL DischargePolicy[4];
        ULONG VideoTimeout;
        BOOLEAN VideoDimDisplay;
        ULONG VideoReserved[3];
        ULONG SpindownTimeout;
        BOOLEAN OptimizeForPower;
        UCHAR FanThrottleTolerance;
        UCHAR ForcedThrottle;
        UCHAR MinThrottle;
        POWER_ACTION_POLICY OverThrottled;
} SYSTEM_POWER_POLICY, *PSYSTEM_POWER_POLICY;

typedef struct _SYSTEM_POWER_CAPABILITIES {
        BOOLEAN PowerButtonPresent;
        BOOLEAN SleepButtonPresent;
        BOOLEAN LidPresent;
        BOOLEAN SystemS1;
        BOOLEAN SystemS2;
        BOOLEAN SystemS3;
        BOOLEAN SystemS4;
        BOOLEAN SystemS5;
        BOOLEAN HiberFilePresent;
        BOOLEAN FullWake;
        BOOLEAN VideoDimPresent;
        BOOLEAN ApmPresent;
        BOOLEAN UpsPresent;
        BOOLEAN ThermalControl;
        BOOLEAN ProcessorThrottle;
        UCHAR ProcessorMinThrottle;
        UCHAR ProcessorMaxThrottle;
        UCHAR spare2[4];
        BOOLEAN DiskSpinDown;
        UCHAR spare3[8];
        BOOLEAN SystemBatteriesPresent;
        BOOLEAN BatteriesAreShortTerm;
        BATTERY_REPORTING_SCALE BatteryScale[3];
        SYSTEM_POWER_STATE AcOnLineWake;
        SYSTEM_POWER_STATE SoftLidWake;
        SYSTEM_POWER_STATE RtcWake;
        SYSTEM_POWER_STATE MinDeviceWakeState;
        SYSTEM_POWER_STATE DefaultLowLatencyWake;
} SYSTEM_POWER_CAPABILITIES, *PSYSTEM_POWER_CAPABILITIES;

typedef struct _SYSTEM_BATTERY_STATE {
        BOOLEAN AcOnLine;
        BOOLEAN BatteryPresent;
        BOOLEAN Charging;
        BOOLEAN Discharging;
        BOOLEAN Spare1[4];
        ULONG MaxCapacity;
        ULONG RemainingCapacity;
        ULONG Rate;
        ULONG EstimatedTime;
        ULONG DefaultAlert1;
        ULONG DefaultAlert2;
} SYSTEM_BATTERY_STATE, *PSYSTEM_BATTERY_STATE;


typedef struct _SYSTEM_POWER_INFORMATION {
        ULONG MaxIdlenessAllowed;
        ULONG Idleness;
        ULONG TimeRemaining;
        UCHAR CoolingMode;
} SYSTEM_POWER_INFORMATION, *PSYSTEM_POWER_INFORMATION;


typedef struct _PROCESSOR_POWER_POLICY_INFO {
        ULONG TimeCheck;
        ULONG DemoteLimit;
        ULONG PromoteLimit;
        UCHAR DemotePercent;
        UCHAR PromotePercent;
        UCHAR Spare[2];
        ULONG AllowDemotion : 1;
        ULONG AllowPromotion : 1;
        ULONG Reserved : 30;
} PROCESSOR_POWER_POLICY_INFO, *PPROCESSOR_POWER_POLICY_INFO;
typedef struct _PROCESSOR_POWER_POLICY {
        ULONG Revision;
        UCHAR DynamicThrottle;
        UCHAR Spare[3];
        ULONG Reserved;
        ULONG PolicyCount;
        PROCESSOR_POWER_POLICY_INFO Policy[3];
} PROCESSOR_POWER_POLICY, *PPROCESSOR_POWER_POLICY;
typedef struct _ADMINISTRATOR_POWER_POLICY {
        SYSTEM_POWER_STATE MinSleep;
        SYSTEM_POWER_STATE MaxSleep;
        ULONG MinVideoTimeout;
        ULONG MaxVideoTimeout;
        ULONG MinSpindownTimeout;
        ULONG MaxSpindownTimeout;
} ADMINISTRATOR_POWER_POLICY, *PADMINISTRATOR_POWER_POLICY;
# 1 "d:/util/mingw/include/poppack.h" 1

#pragma pack(pop)
# 2983 "d:/util/mingw/include/winnt.h" 2







typedef OSVERSIONINFOA OSVERSIONINFO,*POSVERSIONINFO,*LPOSVERSIONINFO;
typedef OSVERSIONINFOEXA OSVERSIONINFOEX,*POSVERSIONINFOEX,*LPOSVERSIONINFOEX;
# 3028 "d:/util/mingw/include/winnt.h"
extern PVOID GetCurrentFiber(void);
#pragma aux GetCurrentFiber = "mov	eax, dword ptr fs:0x10" value [eax] modify [eax];




extern PVOID GetFiberData(void);
#pragma aux GetFiberData = "mov	eax, dword ptr fs:0x10" "mov	eax, [eax]" value [eax] modify [eax];
# 3046 "d:/util/mingw/include/winnt.h"
}
# 247 "d:/util/mingw/include/windef.h" 2

typedef UINT_PTR WPARAM;
typedef LONG_PTR LPARAM;
typedef LONG_PTR LRESULT;

typedef LONG HRESULT;



typedef WORD ATOM;

typedef HANDLE HGLOBAL;
typedef HANDLE HLOCAL;
typedef HANDLE GLOBALHANDLE;
typedef HANDLE LOCALHANDLE;
typedef void *HGDIOBJ;
typedef struct HACCEL__{int i;}*HACCEL;
typedef struct HBITMAP__{int i;}*HBITMAP;
typedef struct HBRUSH__{int i;}*HBRUSH;
typedef struct HCOLORSPACE__{int i;}*HCOLORSPACE;
typedef struct HDC__{int i;}*HDC;
typedef struct HGLRC__{int i;}*HGLRC;
typedef struct HDESK__{int i;}*HDESK;
typedef struct HENHMETAFILE__{int i;}*HENHMETAFILE;
typedef struct HFONT__{int i;}*HFONT;
typedef struct HICON__{int i;}*HICON;
typedef struct HKEY__{int i;}*HKEY;


typedef struct HMONITOR__{int i;}*HMONITOR;

typedef struct HTERMINAL__{int i;}*HTERMINAL;
typedef struct HWINEVENTHOOK__{int i;}*HWINEVENTHOOK;

typedef HKEY *PHKEY;
typedef struct HMENU__{int i;}*HMENU;
typedef struct HMETAFILE__{int i;}*HMETAFILE;
typedef struct HINSTANCE__{int i;}*HINSTANCE;
typedef HINSTANCE HMODULE;
typedef struct HPALETTE__{int i;}*HPALETTE;
typedef struct HPEN__{int i;}*HPEN;
typedef struct HRGN__{int i;}*HRGN;
typedef struct HRSRC__{int i;}*HRSRC;
typedef struct HSTR__{int i;}*HSTR;
typedef struct HTASK__{int i;}*HTASK;
typedef struct HWND__{int i;}*HWND;
typedef struct HWINSTA__{int i;}*HWINSTA;
typedef struct HKL__{int i;}*HKL;
typedef int HFILE;
typedef HICON HCURSOR;
typedef DWORD COLORREF;
typedef int (__attribute__((__stdcall__)) *FARPROC)();
typedef int (__attribute__((__stdcall__)) *NEARPROC)();
typedef int (__attribute__((__stdcall__)) *PROC)();
typedef struct tagRECT {
        LONG left;
        LONG top;
        LONG right;
        LONG bottom;
} RECT,*PRECT,*LPRECT;
typedef const RECT *LPCRECT;
typedef struct tagRECTL {
        LONG left;
        LONG top;
        LONG right;
        LONG bottom;
} RECTL,*PRECTL,*LPRECTL;
typedef const RECTL *LPCRECTL;
typedef struct tagPOINT {
        LONG x;
        LONG y;
} POINT,POINTL,*PPOINT,*LPPOINT,*PPOINTL,*LPPOINTL;
typedef struct tagSIZE {
        LONG cx;
        LONG cy;
} SIZE,SIZEL,*PSIZE,*LPSIZE,*PSIZEL,*LPSIZEL;
typedef struct tagPOINTS {
        SHORT x;
        SHORT y;
} POINTS,*PPOINTS,*LPPOINTS;


}
# 40 "d:/util/mingw/include/excpt.h" 2
# 53 "d:/util/mingw/include/excpt.h"
typedef enum {
        ExceptionContinueExecution,
        ExceptionContinueSearch,
        ExceptionNestedException,
        ExceptionCollidedUnwind
} EXCEPTION_DISPOSITION;
# 67 "d:/util/mingw/include/excpt.h"
extern "C" {






typedef EXCEPTION_DISPOSITION (*PEXCEPTION_HANDLER)
                (struct _EXCEPTION_RECORD*, void*, struct _CONTEXT*, void*);





typedef struct _EXCEPTION_REGISTRATION
{
        struct _EXCEPTION_REGISTRATION* prev;
        PEXCEPTION_HANDLER handler;
} EXCEPTION_REGISTRATION, *PEXCEPTION_REGISTRATION;

typedef EXCEPTION_REGISTRATION EXCEPTION_REGISTRATION_RECORD;
typedef PEXCEPTION_REGISTRATION PEXCEPTION_REGISTRATION_RECORD;
# 116 "d:/util/mingw/include/excpt.h"
}
# 153 "f:/source-wip/CS/include/cssys/win32/csosdefs.h" 2

# 1 "d:/util/mingw/include/stdarg.h" 1





# 1 "d:/util/mingw/include/stdarg.h" 1 3





# 1 "d:/util/mingw/lib/gcc-lib/mingw32/3.2.3/include/stdarg.h" 1 3
# 111 "d:/util/mingw/lib/gcc-lib/mingw32/3.2.3/include/stdarg.h" 3
typedef __gnuc_va_list va_list;
# 7 "d:/util/mingw/include/stdarg.h" 2 3
# 7 "d:/util/mingw/include/stdarg.h" 2
# 155 "f:/source-wip/CS/include/cssys/win32/csosdefs.h" 2

# 1 "d:/util/mingw/include/winbase.h" 1
# 11 "d:/util/mingw/include/winbase.h"
extern "C" {
# 512 "d:/util/mingw/include/winbase.h"
typedef struct _FILETIME {
        DWORD dwLowDateTime;
        DWORD dwHighDateTime;
} FILETIME,*PFILETIME,*LPFILETIME;
typedef struct _BY_HANDLE_FILE_INFORMATION {
        DWORD dwFileAttributes;
        FILETIME ftCreationTime;
        FILETIME ftLastAccessTime;
        FILETIME ftLastWriteTime;
        DWORD dwVolumeSerialNumber;
        DWORD nFileSizeHigh;
        DWORD nFileSizeLow;
        DWORD nNumberOfLinks;
        DWORD nFileIndexHigh;
        DWORD nFileIndexLow;
} BY_HANDLE_FILE_INFORMATION,*LPBY_HANDLE_FILE_INFORMATION;
typedef struct _DCB {
        DWORD DCBlength;
        DWORD BaudRate;
        DWORD fBinary:1;
        DWORD fParity:1;
        DWORD fOutxCtsFlow:1;
        DWORD fOutxDsrFlow:1;
        DWORD fDtrControl:2;
        DWORD fDsrSensitivity:1;
        DWORD fTXContinueOnXoff:1;
        DWORD fOutX:1;
        DWORD fInX:1;
        DWORD fErrorChar:1;
        DWORD fNull:1;
        DWORD fRtsControl:2;
        DWORD fAbortOnError:1;
        DWORD fDummy2:17;
        WORD wReserved;
        WORD XonLim;
        WORD XoffLim;
        BYTE ByteSize;
        BYTE Parity;
        BYTE StopBits;
        char XonChar;
        char XoffChar;
        char ErrorChar;
        char EofChar;
        char EvtChar;
        WORD wReserved1;
} DCB,*LPDCB;
typedef struct _COMM_CONFIG {
        DWORD dwSize;
        WORD wVersion;
        WORD wReserved;
        DCB dcb;
        DWORD dwProviderSubType;
        DWORD dwProviderOffset;
        DWORD dwProviderSize;
        WCHAR wcProviderData[1];
} COMMCONFIG,*LPCOMMCONFIG;
typedef struct _COMMPROP {
        WORD wPacketLength;
        WORD wPacketVersion;
        DWORD dwServiceMask;
        DWORD dwReserved1;
        DWORD dwMaxTxQueue;
        DWORD dwMaxRxQueue;
        DWORD dwMaxBaud;
        DWORD dwProvSubType;
        DWORD dwProvCapabilities;
        DWORD dwSettableParams;
        DWORD dwSettableBaud;
        WORD wSettableData;
        WORD wSettableStopParity;
        DWORD dwCurrentTxQueue;
        DWORD dwCurrentRxQueue;
        DWORD dwProvSpec1;
        DWORD dwProvSpec2;
        WCHAR wcProvChar[1];
} COMMPROP,*LPCOMMPROP;
typedef struct _COMMTIMEOUTS {
        DWORD ReadIntervalTimeout;
        DWORD ReadTotalTimeoutMultiplier;
        DWORD ReadTotalTimeoutConstant;
        DWORD WriteTotalTimeoutMultiplier;
        DWORD WriteTotalTimeoutConstant;
} COMMTIMEOUTS,*LPCOMMTIMEOUTS;
typedef struct _COMSTAT {
        DWORD fCtsHold:1;
        DWORD fDsrHold:1;
        DWORD fRlsdHold:1;
        DWORD fXoffHold:1;
        DWORD fXoffSent:1;
        DWORD fEof:1;
        DWORD fTxim:1;
        DWORD fReserved:25;
        DWORD cbInQue;
        DWORD cbOutQue;
} COMSTAT,*LPCOMSTAT;
typedef DWORD (__attribute__((__stdcall__)) *LPTHREAD_START_ROUTINE)(LPVOID);
typedef struct _CREATE_PROCESS_DEBUG_INFO {
        HANDLE hFile;
        HANDLE hProcess;
        HANDLE hThread;
        LPVOID lpBaseOfImage;
        DWORD dwDebugInfoFileOffset;
        DWORD nDebugInfoSize;
        LPVOID lpThreadLocalBase;
        LPTHREAD_START_ROUTINE lpStartAddress;
        LPVOID lpImageName;
        WORD fUnicode;
} CREATE_PROCESS_DEBUG_INFO,*LPCREATE_PROCESS_DEBUG_INFO;
typedef struct _CREATE_THREAD_DEBUG_INFO {
        HANDLE hThread;
        LPVOID lpThreadLocalBase;
        LPTHREAD_START_ROUTINE lpStartAddress;
} CREATE_THREAD_DEBUG_INFO,*LPCREATE_THREAD_DEBUG_INFO;
typedef struct _EXCEPTION_DEBUG_INFO {
        EXCEPTION_RECORD ExceptionRecord;
        DWORD dwFirstChance;
} EXCEPTION_DEBUG_INFO,*LPEXCEPTION_DEBUG_INFO;
typedef struct _EXIT_THREAD_DEBUG_INFO {
        DWORD dwExitCode;
} EXIT_THREAD_DEBUG_INFO,*LPEXIT_THREAD_DEBUG_INFO;
typedef struct _EXIT_PROCESS_DEBUG_INFO {
        DWORD dwExitCode;
} EXIT_PROCESS_DEBUG_INFO,*LPEXIT_PROCESS_DEBUG_INFO;
typedef struct _LOAD_DLL_DEBUG_INFO {
        HANDLE hFile;
        LPVOID lpBaseOfDll;
        DWORD dwDebugInfoFileOffset;
        DWORD nDebugInfoSize;
        LPVOID lpImageName;
        WORD fUnicode;
} LOAD_DLL_DEBUG_INFO,*LPLOAD_DLL_DEBUG_INFO;
typedef struct _UNLOAD_DLL_DEBUG_INFO {
        LPVOID lpBaseOfDll;
} UNLOAD_DLL_DEBUG_INFO,*LPUNLOAD_DLL_DEBUG_INFO;
typedef struct _OUTPUT_DEBUG_STRING_INFO {
        LPSTR lpDebugStringData;
        WORD fUnicode;
        WORD nDebugStringLength;
} OUTPUT_DEBUG_STRING_INFO,*LPOUTPUT_DEBUG_STRING_INFO;
typedef struct _RIP_INFO {
        DWORD dwError;
        DWORD dwType;
} RIP_INFO,*LPRIP_INFO;
typedef struct _DEBUG_EVENT {
        DWORD dwDebugEventCode;
        DWORD dwProcessId;
        DWORD dwThreadId;
        union {
                EXCEPTION_DEBUG_INFO Exception;
                CREATE_THREAD_DEBUG_INFO CreateThread;
                CREATE_PROCESS_DEBUG_INFO CreateProcessInfo;
                EXIT_THREAD_DEBUG_INFO ExitThread;
                EXIT_PROCESS_DEBUG_INFO ExitProcess;
                LOAD_DLL_DEBUG_INFO LoadDll;
                UNLOAD_DLL_DEBUG_INFO UnloadDll;
                OUTPUT_DEBUG_STRING_INFO DebugString;
                RIP_INFO RipInfo;
        } u;
} DEBUG_EVENT,*LPDEBUG_EVENT;
typedef struct _OVERLAPPED {
        DWORD Internal;
        DWORD InternalHigh;
        DWORD Offset;
        DWORD OffsetHigh;
        HANDLE hEvent;
} OVERLAPPED,*POVERLAPPED,*LPOVERLAPPED;
typedef struct _STARTUPINFOA {
        DWORD cb;
        LPSTR lpReserved;
        LPSTR lpDesktop;
        LPSTR lpTitle;
        DWORD dwX;
        DWORD dwY;
        DWORD dwXSize;
        DWORD dwYSize;
        DWORD dwXCountChars;
        DWORD dwYCountChars;
        DWORD dwFillAttribute;
        DWORD dwFlags;
        WORD wShowWindow;
        WORD cbReserved2;
        PBYTE lpReserved2;
        HANDLE hStdInput;
        HANDLE hStdOutput;
        HANDLE hStdError;
} STARTUPINFOA,*LPSTARTUPINFOA;
typedef struct _STARTUPINFOW {
        DWORD cb;
        LPWSTR lpReserved;
        LPWSTR lpDesktop;
        LPWSTR lpTitle;
        DWORD dwX;
        DWORD dwY;
        DWORD dwXSize;
        DWORD dwYSize;
        DWORD dwXCountChars;
        DWORD dwYCountChars;
        DWORD dwFillAttribute;
        DWORD dwFlags;
        WORD wShowWindow;
        WORD cbReserved2;
        PBYTE lpReserved2;
        HANDLE hStdInput;
        HANDLE hStdOutput;
        HANDLE hStdError;
} STARTUPINFOW,*LPSTARTUPINFOW;
typedef struct _PROCESS_INFORMATION {
        HANDLE hProcess;
        HANDLE hThread;
        DWORD dwProcessId;
        DWORD dwThreadId;
} PROCESS_INFORMATION,*LPPROCESS_INFORMATION;
typedef struct _CRITICAL_SECTION_DEBUG {
        WORD Type;
        WORD CreatorBackTraceIndex;
        struct _CRITICAL_SECTION *CriticalSection;
        LIST_ENTRY ProcessLocksList;
        DWORD EntryCount;
        DWORD ContentionCount;
        DWORD Spare [2];
} CRITICAL_SECTION_DEBUG,*PCRITICAL_SECTION_DEBUG;
typedef struct _CRITICAL_SECTION {
        PCRITICAL_SECTION_DEBUG DebugInfo;
        LONG LockCount;
        LONG RecursionCount;
        HANDLE OwningThread;
        HANDLE LockSemaphore;
        DWORD SpinCount;
} CRITICAL_SECTION,*PCRITICAL_SECTION,*LPCRITICAL_SECTION;
typedef struct _SYSTEMTIME {
        WORD wYear;
        WORD wMonth;
        WORD wDayOfWeek;
        WORD wDay;
        WORD wHour;
        WORD wMinute;
        WORD wSecond;
        WORD wMilliseconds;
} SYSTEMTIME,*LPSYSTEMTIME;
typedef struct _WIN32_FILE_ATTRIBUTE_DATA {
        DWORD dwFileAttributes;
        FILETIME ftCreationTime;
        FILETIME ftLastAccessTime;
        FILETIME ftLastWriteTime;
        DWORD nFileSizeHigh;
        DWORD nFileSizeLow;
} WIN32_FILE_ATTRIBUTE_DATA,*LPWIN32_FILE_ATTRIBUTE_DATA;
typedef struct _WIN32_FIND_DATAA {
        DWORD dwFileAttributes;
        FILETIME ftCreationTime;
        FILETIME ftLastAccessTime;
        FILETIME ftLastWriteTime;
        DWORD nFileSizeHigh;
        DWORD nFileSizeLow;
        DWORD dwReserved0;
        DWORD dwReserved1;
        CHAR cFileName[260];
        CHAR cAlternateFileName[14];
} WIN32_FIND_DATAA,*PWIN32_FIND_DATAA,*LPWIN32_FIND_DATAA;
typedef struct _WIN32_FIND_DATAW {
        DWORD dwFileAttributes;
        FILETIME ftCreationTime;
        FILETIME ftLastAccessTime;
        FILETIME ftLastWriteTime;
        DWORD nFileSizeHigh;
        DWORD nFileSizeLow;
        DWORD dwReserved0;
        DWORD dwReserved1;
        WCHAR cFileName[260];
        WCHAR cAlternateFileName[14];
} WIN32_FIND_DATAW,*PWIN32_FIND_DATAW,*LPWIN32_FIND_DATAW;
typedef struct _WIN32_STREAM_ID {
        DWORD dwStreamId;
        DWORD dwStreamAttributes;
        LARGE_INTEGER Size;
        DWORD dwStreamNameSize;
        WCHAR cStreamName[1];
} WIN32_STREAM_ID;
typedef enum _FINDEX_INFO_LEVELS {
        FindExInfoStandard,
        FindExInfoMaxInfoLevel
} FINDEX_INFO_LEVELS;
typedef enum _FINDEX_SEARCH_OPS {
        FindExSearchNameMatch,
        FindExSearchLimitToDirectories,
        FindExSearchLimitToDevices,
        FindExSearchMaxSearchOp
} FINDEX_SEARCH_OPS;
typedef enum _ACL_INFORMATION_CLASS {
        AclRevisionInformation=1,
        AclSizeInformation
} ACL_INFORMATION_CLASS;
typedef struct tagHW_PROFILE_INFOA {
        DWORD dwDockInfo;
        CHAR szHwProfileGuid[39];
        CHAR szHwProfileName[80];
} HW_PROFILE_INFOA,*LPHW_PROFILE_INFOA;
typedef struct tagHW_PROFILE_INFOW {
        DWORD dwDockInfo;
        WCHAR szHwProfileGuid[39];
        WCHAR szHwProfileName[80];
} HW_PROFILE_INFOW,*LPHW_PROFILE_INFOW;
typedef enum _GET_FILEEX_INFO_LEVELS {
        GetFileExInfoStandard,
        GetFileExMaxInfoLevel
} GET_FILEEX_INFO_LEVELS;
typedef struct _SYSTEM_INFO {
        union {
                DWORD dwOemId;
                struct {
                        WORD wProcessorArchitecture;
                        WORD wReserved;
                } ;
        } ;
        DWORD dwPageSize;
        PVOID lpMinimumApplicationAddress;
        PVOID lpMaximumApplicationAddress;
        DWORD dwActiveProcessorMask;
        DWORD dwNumberOfProcessors;
        DWORD dwProcessorType;
        DWORD dwAllocationGranularity;
        WORD wProcessorLevel;
        WORD wProcessorRevision;
} SYSTEM_INFO,*LPSYSTEM_INFO;
typedef struct _SYSTEM_POWER_STATUS {
        BYTE ACLineStatus;
        BYTE BatteryFlag;
        BYTE BatteryLifePercent;
        BYTE Reserved1;
        DWORD BatteryLifeTime;
        DWORD BatteryFullLifeTime;
} SYSTEM_POWER_STATUS,*LPSYSTEM_POWER_STATUS;
typedef struct _TIME_ZONE_INFORMATION {
        LONG Bias;
        WCHAR StandardName[32];
        SYSTEMTIME StandardDate;
        LONG StandardBias;
        WCHAR DaylightName[32];
        SYSTEMTIME DaylightDate;
        LONG DaylightBias;
} TIME_ZONE_INFORMATION,*LPTIME_ZONE_INFORMATION;
typedef struct _MEMORYSTATUS {
        DWORD dwLength;
        DWORD dwMemoryLoad;
        DWORD dwTotalPhys;
        DWORD dwAvailPhys;
        DWORD dwTotalPageFile;
        DWORD dwAvailPageFile;
        DWORD dwTotalVirtual;
        DWORD dwAvailVirtual;
} MEMORYSTATUS,*LPMEMORYSTATUS;
# 876 "d:/util/mingw/include/winbase.h"
typedef struct _LDT_ENTRY {
        WORD LimitLow;
        WORD BaseLow;
        union {
                struct {
                        BYTE BaseMid;
                        BYTE Flags1;
                        BYTE Flags2;
                        BYTE BaseHi;
                } Bytes;
                struct {
                        DWORD BaseMid:8;
                        DWORD Type:5;
                        DWORD Dpl:2;
                        DWORD Pres:1;
                        DWORD LimitHi:4;
                        DWORD Sys:1;
                        DWORD Reserved_0:1;
                        DWORD Default_Big:1;
                        DWORD Granularity:1;
                        DWORD BaseHi:8;
                } Bits;
        } HighWord;
} LDT_ENTRY,*PLDT_ENTRY,*LPLDT_ENTRY;
typedef struct _PROCESS_HEAP_ENTRY {
        PVOID lpData;
        DWORD cbData;
        BYTE cbOverhead;
        BYTE iRegionIndex;
        WORD wFlags;
        union {
                struct {
                        HANDLE hMem;
                        DWORD dwReserved[3];
                } Block;
                struct {
                        DWORD dwCommittedSize;
                        DWORD dwUnCommittedSize;
                        LPVOID lpFirstBlock;
                        LPVOID lpLastBlock;
                } Region;
        } ;
} PROCESS_HEAP_ENTRY,*LPPROCESS_HEAP_ENTRY;
typedef struct _OFSTRUCT {
        BYTE cBytes;
        BYTE fFixedDisk;
        WORD nErrCode;
        WORD Reserved1;
        WORD Reserved2;
        CHAR szPathName[128];
} OFSTRUCT,*LPOFSTRUCT,*POFSTRUCT;
typedef struct _WIN_CERTIFICATE {
      DWORD dwLength;
      WORD wRevision;
      WORD wCertificateType;
      BYTE bCertificate[1];
} WIN_CERTIFICATE, *LPWIN_CERTIFICATE;

typedef DWORD(__attribute__((__stdcall__)) *LPPROGRESS_ROUTINE)(LARGE_INTEGER,LARGE_INTEGER,LARGE_INTEGER,LARGE_INTEGER,DWORD,DWORD,HANDLE,HANDLE,LPVOID);
typedef void(__attribute__((__stdcall__)) *LPFIBER_START_ROUTINE)(PVOID);
typedef BOOL(__attribute__((__stdcall__)) *ENUMRESLANGPROC)(HMODULE,LPCTSTR,LPCTSTR,WORD,LONG);
typedef BOOL(__attribute__((__stdcall__)) *ENUMRESNAMEPROC)(HMODULE,LPCTSTR,LPTSTR,LONG);
typedef BOOL(__attribute__((__stdcall__)) *ENUMRESTYPEPROC)(HMODULE,LPTSTR,LONG);
typedef void(__attribute__((__stdcall__)) *LPOVERLAPPED_COMPLETION_ROUTINE)(DWORD,DWORD,LPOVERLAPPED);
typedef LONG(__attribute__((__stdcall__)) *PTOP_LEVEL_EXCEPTION_FILTER)(LPEXCEPTION_POINTERS);
typedef PTOP_LEVEL_EXCEPTION_FILTER LPTOP_LEVEL_EXCEPTION_FILTER;
typedef void(__attribute__((__stdcall__)) *PAPCFUNC)(DWORD);
typedef void(__attribute__((__stdcall__)) *PTIMERAPCROUTINE)(PVOID,DWORD,DWORD);



int __attribute__((__stdcall__)) WinMain(HINSTANCE,HINSTANCE,LPSTR,int);



int __attribute__((__stdcall__)) wWinMain(HINSTANCE,HINSTANCE,LPWSTR,int);
long __attribute__((__stdcall__)) _hread(HFILE,LPVOID,long);
long __attribute__((__stdcall__)) _hwrite(HFILE,LPCSTR,long);
HFILE __attribute__((__stdcall__)) _lclose(HFILE);
HFILE __attribute__((__stdcall__)) _lcreat(LPCSTR,int);
LONG __attribute__((__stdcall__)) _llseek(HFILE,LONG,int);
HFILE __attribute__((__stdcall__)) _lopen(LPCSTR,int);
UINT __attribute__((__stdcall__)) _lread(HFILE,LPVOID,UINT);
UINT __attribute__((__stdcall__)) _lwrite(HFILE,LPCSTR,UINT);

BOOL __attribute__((__stdcall__)) AccessCheck(PSECURITY_DESCRIPTOR,HANDLE,DWORD,PGENERIC_MAPPING,PPRIVILEGE_SET,PDWORD,PDWORD,PBOOL);
BOOL __attribute__((__stdcall__)) AccessCheckAndAuditAlarmA(LPCSTR,LPVOID,LPSTR,LPSTR,PSECURITY_DESCRIPTOR,DWORD,PGENERIC_MAPPING,BOOL,PDWORD,PBOOL,PBOOL);
BOOL __attribute__((__stdcall__)) AccessCheckAndAuditAlarmW(LPCWSTR,LPVOID,LPWSTR,LPWSTR,PSECURITY_DESCRIPTOR,DWORD,PGENERIC_MAPPING,BOOL,PDWORD,PBOOL,PBOOL);
BOOL __attribute__((__stdcall__)) AddAccessAllowedAce(PACL,DWORD,DWORD,PSID);
BOOL __attribute__((__stdcall__)) AddAccessDeniedAce(PACL,DWORD,DWORD,PSID);




BOOL __attribute__((__stdcall__)) AddAce(PACL,DWORD,DWORD,PVOID,DWORD);
ATOM __attribute__((__stdcall__)) AddAtomA(LPCSTR);
ATOM __attribute__((__stdcall__)) AddAtomW(LPCWSTR);
BOOL __attribute__((__stdcall__)) AddAuditAccessAce(PACL,DWORD,DWORD,PSID,BOOL,BOOL);
BOOL __attribute__((__stdcall__)) AdjustTokenGroups(HANDLE,BOOL,PTOKEN_GROUPS,DWORD,PTOKEN_GROUPS,PDWORD);
BOOL __attribute__((__stdcall__)) AdjustTokenPrivileges(HANDLE,BOOL,PTOKEN_PRIVILEGES,DWORD,PTOKEN_PRIVILEGES,PDWORD);
BOOL __attribute__((__stdcall__)) AllocateAndInitializeSid(PSID_IDENTIFIER_AUTHORITY,BYTE,DWORD,DWORD,DWORD,DWORD,DWORD,DWORD,DWORD,DWORD,PSID*);
BOOL __attribute__((__stdcall__)) AllocateLocallyUniqueId(PLUID);
BOOL __attribute__((__stdcall__)) AreAllAccessesGranted(DWORD,DWORD);
BOOL __attribute__((__stdcall__)) AreAnyAccessesGranted(DWORD,DWORD);
BOOL __attribute__((__stdcall__)) AreFileApisANSI(void);
BOOL __attribute__((__stdcall__)) BackupEventLogA(HANDLE,LPCSTR);
BOOL __attribute__((__stdcall__)) BackupEventLogW(HANDLE,LPCWSTR);
BOOL __attribute__((__stdcall__)) BackupRead(HANDLE,LPBYTE,DWORD,LPDWORD,BOOL,BOOL,LPVOID*);
BOOL __attribute__((__stdcall__)) BackupSeek(HANDLE,DWORD,DWORD,LPDWORD,LPDWORD,LPVOID*);
BOOL __attribute__((__stdcall__)) BackupWrite(HANDLE,LPBYTE,DWORD,LPDWORD,BOOL,BOOL,LPVOID*);
BOOL __attribute__((__stdcall__)) Beep(DWORD,DWORD);
HANDLE __attribute__((__stdcall__)) BeginUpdateResourceA(LPCSTR,BOOL);
HANDLE __attribute__((__stdcall__)) BeginUpdateResourceW(LPCWSTR,BOOL);
BOOL __attribute__((__stdcall__)) BuildCommDCBA(LPCSTR,LPDCB);
BOOL __attribute__((__stdcall__)) BuildCommDCBW(LPCWSTR,LPDCB);
BOOL __attribute__((__stdcall__)) BuildCommDCBAndTimeoutsA(LPCSTR,LPDCB,LPCOMMTIMEOUTS);
BOOL __attribute__((__stdcall__)) BuildCommDCBAndTimeoutsW(LPCWSTR,LPDCB,LPCOMMTIMEOUTS);
BOOL __attribute__((__stdcall__)) CallNamedPipeA(LPCSTR,PVOID,DWORD,PVOID,DWORD,PDWORD,DWORD);
BOOL __attribute__((__stdcall__)) CallNamedPipeW(LPCWSTR,PVOID,DWORD,PVOID,DWORD,PDWORD,DWORD);
BOOL __attribute__((__stdcall__)) CancelIo(HANDLE);
BOOL __attribute__((__stdcall__)) CancelWaitableTimer(HANDLE);
BOOL __attribute__((__stdcall__)) ClearCommBreak(HANDLE);
BOOL __attribute__((__stdcall__)) ClearCommError(HANDLE,PDWORD,LPCOMSTAT);
BOOL __attribute__((__stdcall__)) ClearEventLogA(HANDLE,LPCSTR);
BOOL __attribute__((__stdcall__)) ClearEventLogW(HANDLE,LPCWSTR);
BOOL __attribute__((__stdcall__)) CloseEventLog(HANDLE);
BOOL __attribute__((__stdcall__)) CloseHandle(HANDLE);
BOOL __attribute__((__stdcall__)) CommConfigDialogA(LPCSTR,HWND,LPCOMMCONFIG);
BOOL __attribute__((__stdcall__)) CommConfigDialogW(LPCWSTR,HWND,LPCOMMCONFIG);
LONG __attribute__((__stdcall__)) CompareFileTime(const FILETIME*,const FILETIME*);
BOOL __attribute__((__stdcall__)) ConnectNamedPipe(HANDLE,LPOVERLAPPED);
BOOL __attribute__((__stdcall__)) ContinueDebugEvent(DWORD,DWORD,DWORD);
PVOID __attribute__((__stdcall__)) ConvertThreadToFiber(PVOID);
BOOL __attribute__((__stdcall__)) CopyFileA(LPCSTR,LPCSTR,BOOL);
BOOL __attribute__((__stdcall__)) CopyFileW(LPCWSTR,LPCWSTR,BOOL);
BOOL __attribute__((__stdcall__)) CopyFileExA(LPCSTR,LPCSTR,LPPROGRESS_ROUTINE,LPVOID,LPBOOL,DWORD);
BOOL __attribute__((__stdcall__)) CopyFileExW(LPCWSTR,LPCWSTR,LPPROGRESS_ROUTINE,LPVOID,LPBOOL,DWORD);
# 1021 "d:/util/mingw/include/winbase.h"
BOOL __attribute__((__stdcall__)) CopySid(DWORD,PSID,PSID);
BOOL __attribute__((__stdcall__)) CreateDirectoryA(LPCSTR,LPSECURITY_ATTRIBUTES);
BOOL __attribute__((__stdcall__)) CreateDirectoryW(LPCWSTR,LPSECURITY_ATTRIBUTES);
BOOL __attribute__((__stdcall__)) CreateDirectoryExA(LPCSTR,LPCSTR,LPSECURITY_ATTRIBUTES);
BOOL __attribute__((__stdcall__)) CreateDirectoryExW(LPCWSTR,LPCWSTR,LPSECURITY_ATTRIBUTES);
HANDLE __attribute__((__stdcall__)) CreateEventA(LPSECURITY_ATTRIBUTES,BOOL,BOOL,LPCSTR);
HANDLE __attribute__((__stdcall__)) CreateEventW(LPSECURITY_ATTRIBUTES,BOOL,BOOL,LPCWSTR);
LPVOID __attribute__((__stdcall__)) CreateFiber(SIZE_T,LPFIBER_START_ROUTINE,LPVOID);
LPVOID __attribute__((__stdcall__)) CreateFiberEx(SIZE_T,SIZE_T,DWORD,LPFIBER_START_ROUTINE,LPVOID);
HANDLE __attribute__((__stdcall__)) CreateFileA(LPCSTR,DWORD,DWORD,LPSECURITY_ATTRIBUTES,DWORD,DWORD,HANDLE);
HANDLE __attribute__((__stdcall__)) CreateFileW(LPCWSTR,DWORD,DWORD,LPSECURITY_ATTRIBUTES,DWORD,DWORD,HANDLE);
HANDLE __attribute__((__stdcall__)) CreateFileMappingA(HANDLE,LPSECURITY_ATTRIBUTES,DWORD,DWORD,DWORD,LPCSTR);
HANDLE __attribute__((__stdcall__)) CreateFileMappingW(HANDLE,LPSECURITY_ATTRIBUTES,DWORD,DWORD,DWORD,LPCWSTR);




HANDLE __attribute__((__stdcall__)) CreateIoCompletionPort(HANDLE,HANDLE,DWORD,DWORD);




HANDLE __attribute__((__stdcall__)) CreateMailslotA(LPCSTR,DWORD,DWORD,LPSECURITY_ATTRIBUTES);
HANDLE __attribute__((__stdcall__)) CreateMailslotW(LPCWSTR,DWORD,DWORD,LPSECURITY_ATTRIBUTES);
HANDLE __attribute__((__stdcall__)) CreateMutexA(LPSECURITY_ATTRIBUTES,BOOL,LPCSTR);
HANDLE __attribute__((__stdcall__)) CreateMutexW(LPSECURITY_ATTRIBUTES,BOOL,LPCWSTR);
HANDLE __attribute__((__stdcall__)) CreateNamedPipeA(LPCSTR,DWORD,DWORD,DWORD,DWORD,DWORD,DWORD,LPSECURITY_ATTRIBUTES);
HANDLE __attribute__((__stdcall__)) CreateNamedPipeW(LPCWSTR,DWORD,DWORD,DWORD,DWORD,DWORD,DWORD,LPSECURITY_ATTRIBUTES);
BOOL __attribute__((__stdcall__)) CreatePipe(PHANDLE,PHANDLE,LPSECURITY_ATTRIBUTES,DWORD);
BOOL __attribute__((__stdcall__)) CreatePrivateObjectSecurity(PSECURITY_DESCRIPTOR,PSECURITY_DESCRIPTOR,PSECURITY_DESCRIPTOR*,BOOL,HANDLE,PGENERIC_MAPPING);
BOOL __attribute__((__stdcall__)) CreateProcessA(LPCSTR,LPSTR,LPSECURITY_ATTRIBUTES,LPSECURITY_ATTRIBUTES,BOOL,DWORD,PVOID,LPCSTR,LPSTARTUPINFOA,LPPROCESS_INFORMATION);
BOOL __attribute__((__stdcall__)) CreateProcessW(LPCWSTR,LPWSTR,LPSECURITY_ATTRIBUTES,LPSECURITY_ATTRIBUTES,BOOL,DWORD,PVOID,LPCWSTR,LPSTARTUPINFOW,LPPROCESS_INFORMATION);
BOOL __attribute__((__stdcall__)) CreateProcessAsUserA(HANDLE,LPCSTR,LPSTR,LPSECURITY_ATTRIBUTES,LPSECURITY_ATTRIBUTES,BOOL,DWORD,PVOID,LPCSTR,LPSTARTUPINFOA,LPPROCESS_INFORMATION);
BOOL __attribute__((__stdcall__)) CreateProcessAsUserW(HANDLE,LPCWSTR,LPWSTR,LPSECURITY_ATTRIBUTES,LPSECURITY_ATTRIBUTES,BOOL,DWORD,PVOID,LPCWSTR,LPSTARTUPINFOW,LPPROCESS_INFORMATION);
HANDLE __attribute__((__stdcall__)) CreateRemoteThread(HANDLE,LPSECURITY_ATTRIBUTES,DWORD,LPTHREAD_START_ROUTINE,LPVOID,DWORD,LPDWORD);
HANDLE __attribute__((__stdcall__)) CreateSemaphoreA(LPSECURITY_ATTRIBUTES,LONG,LONG,LPCSTR);
HANDLE __attribute__((__stdcall__)) CreateSemaphoreW(LPSECURITY_ATTRIBUTES,LONG,LONG,LPCWSTR);
DWORD __attribute__((__stdcall__)) CreateTapePartition(HANDLE,DWORD,DWORD,DWORD);
HANDLE __attribute__((__stdcall__)) CreateThread(LPSECURITY_ATTRIBUTES,DWORD,LPTHREAD_START_ROUTINE,PVOID,DWORD,PDWORD);
HANDLE __attribute__((__stdcall__)) CreateWaitableTimerA(LPSECURITY_ATTRIBUTES,BOOL,LPCSTR);
HANDLE __attribute__((__stdcall__)) CreateWaitableTimerW(LPSECURITY_ATTRIBUTES,BOOL,LPCWSTR);
BOOL __attribute__((__stdcall__)) DebugActiveProcess(DWORD);
void __attribute__((__stdcall__)) DebugBreak(void);
BOOL __attribute__((__stdcall__)) DefineDosDeviceA(DWORD,LPCSTR,LPCSTR);
BOOL __attribute__((__stdcall__)) DefineDosDeviceW(DWORD,LPCWSTR,LPCWSTR);

BOOL __attribute__((__stdcall__)) DeleteAce(PACL,DWORD);
ATOM __attribute__((__stdcall__)) DeleteAtom(ATOM);
void __attribute__((__stdcall__)) DeleteCriticalSection(PCRITICAL_SECTION);
void __attribute__((__stdcall__)) DeleteFiber(PVOID);
BOOL __attribute__((__stdcall__)) DeleteFileA(LPCSTR);
BOOL __attribute__((__stdcall__)) DeleteFileW(LPCWSTR);
BOOL __attribute__((__stdcall__)) DeregisterEventSource(HANDLE);
BOOL __attribute__((__stdcall__)) DestroyPrivateObjectSecurity(PSECURITY_DESCRIPTOR*);
BOOL __attribute__((__stdcall__)) DeviceIoControl(HANDLE,DWORD,PVOID,DWORD,PVOID,DWORD,PDWORD,POVERLAPPED);
BOOL __attribute__((__stdcall__)) DisableThreadLibraryCalls(HMODULE);
BOOL __attribute__((__stdcall__)) DisconnectNamedPipe(HANDLE);
BOOL __attribute__((__stdcall__)) DosDateTimeToFileTime(WORD,WORD,LPFILETIME);
BOOL __attribute__((__stdcall__)) DuplicateHandle(HANDLE,HANDLE,HANDLE,PHANDLE,DWORD,BOOL,DWORD);
BOOL __attribute__((__stdcall__)) DuplicateToken(HANDLE,SECURITY_IMPERSONATION_LEVEL,PHANDLE);
BOOL __attribute__((__stdcall__)) DuplicateTokenEx(HANDLE,DWORD,LPSECURITY_ATTRIBUTES,SECURITY_IMPERSONATION_LEVEL,TOKEN_TYPE,PHANDLE);
BOOL __attribute__((__stdcall__)) EncryptFileA(LPCSTR);
BOOL __attribute__((__stdcall__)) EncryptFileW(LPCWSTR);
BOOL __attribute__((__stdcall__)) EndUpdateResourceA(HANDLE,BOOL);
BOOL __attribute__((__stdcall__)) EndUpdateResourceW(HANDLE,BOOL);
void __attribute__((__stdcall__)) EnterCriticalSection(LPCRITICAL_SECTION);
BOOL __attribute__((__stdcall__)) EnumResourceLanguagesA(HMODULE,LPCSTR,LPCSTR,ENUMRESLANGPROC,LONG_PTR);
BOOL __attribute__((__stdcall__)) EnumResourceLanguagesW(HMODULE,LPCWSTR,LPCWSTR,ENUMRESLANGPROC,LONG_PTR);
BOOL __attribute__((__stdcall__)) EnumResourceNamesA(HMODULE,LPCSTR,ENUMRESNAMEPROC,LONG_PTR);
BOOL __attribute__((__stdcall__)) EnumResourceNamesW(HMODULE,LPCWSTR,ENUMRESNAMEPROC,LONG_PTR);
BOOL __attribute__((__stdcall__)) EnumResourceTypesA(HMODULE,ENUMRESTYPEPROC,LONG_PTR);
BOOL __attribute__((__stdcall__)) EnumResourceTypesW(HMODULE,ENUMRESTYPEPROC,LONG_PTR);
BOOL __attribute__((__stdcall__)) EqualPrefixSid(PSID,PSID);
BOOL __attribute__((__stdcall__)) EqualSid(PSID,PSID);
DWORD __attribute__((__stdcall__)) EraseTape(HANDLE,DWORD,BOOL);
BOOL __attribute__((__stdcall__)) EscapeCommFunction(HANDLE,DWORD);
DECLSPEC_NORETURN void __attribute__((__stdcall__)) ExitProcess(UINT);
DECLSPEC_NORETURN void __attribute__((__stdcall__)) ExitThread(DWORD);
DWORD __attribute__((__stdcall__)) ExpandEnvironmentStringsA(LPCSTR,LPSTR,DWORD);
DWORD __attribute__((__stdcall__)) ExpandEnvironmentStringsW(LPCWSTR,LPWSTR,DWORD);
void __attribute__((__stdcall__)) FatalAppExitA(UINT,LPCSTR);
void __attribute__((__stdcall__)) FatalAppExitW(UINT,LPCWSTR);
void __attribute__((__stdcall__)) FatalExit(int);
BOOL __attribute__((__stdcall__)) FileEncryptionStatusA(LPCSTR,LPDWORD);
BOOL __attribute__((__stdcall__)) FileEncryptionStatusW(LPCWSTR,LPDWORD);
BOOL __attribute__((__stdcall__)) FileTimeToDosDateTime(const FILETIME *,LPWORD,LPWORD);
BOOL __attribute__((__stdcall__)) FileTimeToLocalFileTime(const FILETIME *,LPFILETIME);
BOOL __attribute__((__stdcall__)) FileTimeToSystemTime(const FILETIME *,LPSYSTEMTIME);
ATOM __attribute__((__stdcall__)) FindAtomA(LPCSTR);
ATOM __attribute__((__stdcall__)) FindAtomW(LPCWSTR);
BOOL __attribute__((__stdcall__)) FindClose(HANDLE);
BOOL __attribute__((__stdcall__)) FindCloseChangeNotification(HANDLE);
HANDLE __attribute__((__stdcall__)) FindFirstChangeNotificationA(LPCSTR,BOOL,DWORD);
HANDLE __attribute__((__stdcall__)) FindFirstChangeNotificationW(LPCWSTR,BOOL,DWORD);
HANDLE __attribute__((__stdcall__)) FindFirstFileA(LPCSTR,LPWIN32_FIND_DATAA);
HANDLE __attribute__((__stdcall__)) FindFirstFileW(LPCWSTR,LPWIN32_FIND_DATAW);
HANDLE __attribute__((__stdcall__)) FindFirstFileExA(LPCSTR,FINDEX_INFO_LEVELS,PVOID,FINDEX_SEARCH_OPS,PVOID,DWORD);
HANDLE __attribute__((__stdcall__)) FindFirstFileExW(LPCWSTR,FINDEX_INFO_LEVELS,PVOID,FINDEX_SEARCH_OPS,PVOID,DWORD);
BOOL __attribute__((__stdcall__)) FindFirstFreeAce(PACL,PVOID*);




BOOL __attribute__((__stdcall__)) FindNextChangeNotification(HANDLE);
BOOL __attribute__((__stdcall__)) FindNextFileA(HANDLE,LPWIN32_FIND_DATAA);
BOOL __attribute__((__stdcall__)) FindNextFileW(HANDLE,LPWIN32_FIND_DATAW);





HRSRC __attribute__((__stdcall__)) FindResourceA(HMODULE,LPCSTR,LPCSTR);
HRSRC __attribute__((__stdcall__)) FindResourceW(HINSTANCE,LPCWSTR,LPCWSTR);
HRSRC __attribute__((__stdcall__)) FindResourceExA(HINSTANCE,LPCSTR,LPCSTR,WORD);
HRSRC __attribute__((__stdcall__)) FindResourceExW(HINSTANCE,LPCWSTR,LPCWSTR,WORD);
BOOL __attribute__((__stdcall__)) FlushFileBuffers(HANDLE);
BOOL __attribute__((__stdcall__)) FlushInstructionCache(HANDLE,PCVOID,DWORD);
BOOL __attribute__((__stdcall__)) FlushViewOfFile(PCVOID,DWORD);
DWORD __attribute__((__stdcall__)) FormatMessageA(DWORD,PCVOID,DWORD,DWORD,LPSTR,DWORD,va_list*);
DWORD __attribute__((__stdcall__)) FormatMessageW(DWORD,PCVOID,DWORD,DWORD,LPWSTR,DWORD,va_list*);
BOOL __attribute__((__stdcall__)) FreeEnvironmentStringsA(LPSTR);
BOOL __attribute__((__stdcall__)) FreeEnvironmentStringsW(LPWSTR);
BOOL __attribute__((__stdcall__)) FreeLibrary(HMODULE);
DECLSPEC_NORETURN void __attribute__((__stdcall__)) FreeLibraryAndExitThread(HMODULE,DWORD);



BOOL __attribute__((__stdcall__)) FreeResource(HGLOBAL);

PVOID __attribute__((__stdcall__)) FreeSid(PSID);
BOOL __attribute__((__stdcall__)) GetAce(PACL,DWORD,LPVOID*);
BOOL __attribute__((__stdcall__)) GetAclInformation(PACL,PVOID,DWORD,ACL_INFORMATION_CLASS);
UINT __attribute__((__stdcall__)) GetAtomNameA(ATOM,LPSTR,int);
UINT __attribute__((__stdcall__)) GetAtomNameW(ATOM,LPWSTR,int);
BOOL __attribute__((__stdcall__)) GetBinaryTypeA(LPCSTR,PDWORD);
BOOL __attribute__((__stdcall__)) GetBinaryTypeW(LPCWSTR,PDWORD);
LPSTR __attribute__((__stdcall__)) GetCommandLineA(void);
LPWSTR __attribute__((__stdcall__)) GetCommandLineW(void);
BOOL __attribute__((__stdcall__)) GetCommConfig(HANDLE,LPCOMMCONFIG,PDWORD);
BOOL __attribute__((__stdcall__)) GetCommMask(HANDLE,PDWORD);
BOOL __attribute__((__stdcall__)) GetCommModemStatus(HANDLE,PDWORD);
BOOL __attribute__((__stdcall__)) GetCommProperties(HANDLE,LPCOMMPROP);
BOOL __attribute__((__stdcall__)) GetCommState(HANDLE,LPDCB);
BOOL __attribute__((__stdcall__)) GetCommTimeouts(HANDLE,LPCOMMTIMEOUTS);
DWORD __attribute__((__stdcall__)) GetCompressedFileSizeA(LPCSTR,PDWORD);
DWORD __attribute__((__stdcall__)) GetCompressedFileSizeW(LPCWSTR,PDWORD);
BOOL __attribute__((__stdcall__)) GetComputerNameA(LPSTR,PDWORD);
BOOL __attribute__((__stdcall__)) GetComputerNameW(LPWSTR,PDWORD);
DWORD __attribute__((__stdcall__)) GetCurrentDirectoryA(DWORD,LPSTR);
DWORD __attribute__((__stdcall__)) GetCurrentDirectoryW(DWORD,LPWSTR);
BOOL __attribute__((__stdcall__)) GetCurrentHwProfileA(LPHW_PROFILE_INFOA);
BOOL __attribute__((__stdcall__)) GetCurrentHwProfileW(LPHW_PROFILE_INFOW);
HANDLE __attribute__((__stdcall__)) GetCurrentProcess(void);
DWORD __attribute__((__stdcall__)) GetCurrentProcessId(void);
HANDLE __attribute__((__stdcall__)) GetCurrentThread(void);
DWORD __attribute__((__stdcall__)) GetCurrentThreadId(void);

BOOL __attribute__((__stdcall__)) GetDefaultCommConfigA(LPCSTR,LPCOMMCONFIG,PDWORD);
BOOL __attribute__((__stdcall__)) GetDefaultCommConfigW(LPCWSTR,LPCOMMCONFIG,PDWORD);
BOOL __attribute__((__stdcall__)) GetDiskFreeSpaceA(LPCSTR,PDWORD,PDWORD,PDWORD,PDWORD);
BOOL __attribute__((__stdcall__)) GetDiskFreeSpaceW(LPCWSTR,PDWORD,PDWORD,PDWORD,PDWORD);
BOOL __attribute__((__stdcall__)) GetDiskFreeSpaceExA(LPCSTR,PULARGE_INTEGER,PULARGE_INTEGER,PULARGE_INTEGER);
BOOL __attribute__((__stdcall__)) GetDiskFreeSpaceExW(LPCWSTR,PULARGE_INTEGER,PULARGE_INTEGER,PULARGE_INTEGER);
UINT __attribute__((__stdcall__)) GetDriveTypeA(LPCSTR);
UINT __attribute__((__stdcall__)) GetDriveTypeW(LPCWSTR);
LPSTR __attribute__((__stdcall__)) GetEnvironmentStrings(void);
LPSTR __attribute__((__stdcall__)) GetEnvironmentStringsA(void);
LPWSTR __attribute__((__stdcall__)) GetEnvironmentStringsW(void);
DWORD __attribute__((__stdcall__)) GetEnvironmentVariableA(LPCSTR,LPSTR,DWORD);
DWORD __attribute__((__stdcall__)) GetEnvironmentVariableW(LPCWSTR,LPWSTR,DWORD);
BOOL __attribute__((__stdcall__)) GetExitCodeProcess(HANDLE,PDWORD);
BOOL __attribute__((__stdcall__)) GetExitCodeThread(HANDLE,PDWORD);
DWORD __attribute__((__stdcall__)) GetFileAttributesA(LPCSTR);
DWORD __attribute__((__stdcall__)) GetFileAttributesW(LPCWSTR);
BOOL __attribute__((__stdcall__)) GetFileAttributesExA(LPCSTR,GET_FILEEX_INFO_LEVELS,PVOID);
BOOL __attribute__((__stdcall__)) GetFileAttributesExW(LPCWSTR,GET_FILEEX_INFO_LEVELS,PVOID);
BOOL __attribute__((__stdcall__)) GetFileInformationByHandle(HANDLE,LPBY_HANDLE_FILE_INFORMATION);
BOOL __attribute__((__stdcall__)) GetFileSecurityA(LPCSTR,SECURITY_INFORMATION,PSECURITY_DESCRIPTOR,DWORD,PDWORD);
BOOL __attribute__((__stdcall__)) GetFileSecurityW(LPCWSTR,SECURITY_INFORMATION,PSECURITY_DESCRIPTOR,DWORD,PDWORD);
DWORD __attribute__((__stdcall__)) GetFileSize(HANDLE,PDWORD);
BOOL __attribute__((__stdcall__)) GetFileSizeEx(HANDLE,PLARGE_INTEGER);
BOOL __attribute__((__stdcall__)) GetFileTime(HANDLE,LPFILETIME,LPFILETIME,LPFILETIME);
DWORD __attribute__((__stdcall__)) GetFileType(HANDLE);

DWORD __attribute__((__stdcall__)) GetFullPathNameA(LPCSTR,DWORD,LPSTR,LPSTR*);
DWORD __attribute__((__stdcall__)) GetFullPathNameW(LPCWSTR,DWORD,LPWSTR,LPWSTR*);
BOOL __attribute__((__stdcall__)) GetHandleInformation(HANDLE,PDWORD);
BOOL __attribute__((__stdcall__)) GetKernelObjectSecurity(HANDLE,SECURITY_INFORMATION,PSECURITY_DESCRIPTOR,DWORD,PDWORD);
DWORD __attribute__((__stdcall__)) GetLengthSid(PSID);
void __attribute__((__stdcall__)) GetLocalTime(LPSYSTEMTIME);
DWORD __attribute__((__stdcall__)) GetLogicalDrives(void);
DWORD __attribute__((__stdcall__)) GetLogicalDriveStringsA(DWORD,LPSTR);
DWORD __attribute__((__stdcall__)) GetLogicalDriveStringsW(DWORD,LPWSTR);
DWORD __attribute__((__stdcall__)) GetLongPathNameA(LPCSTR,LPSTR,DWORD);
DWORD __attribute__((__stdcall__)) GetLongPathNameW(LPCWSTR,LPWSTR,DWORD);
BOOL __attribute__((__stdcall__)) GetMailslotInfo(HANDLE,PDWORD,PDWORD,PDWORD,PDWORD);
DWORD __attribute__((__stdcall__)) GetModuleFileNameA(HINSTANCE,LPSTR,DWORD);
DWORD __attribute__((__stdcall__)) GetModuleFileNameW(HINSTANCE,LPWSTR,DWORD);
HMODULE __attribute__((__stdcall__)) GetModuleHandleA(LPCSTR);
HMODULE __attribute__((__stdcall__)) GetModuleHandleW(LPCWSTR);
BOOL __attribute__((__stdcall__)) GetNamedPipeHandleStateA(HANDLE,PDWORD,PDWORD,PDWORD,PDWORD,LPSTR,DWORD);
BOOL __attribute__((__stdcall__)) GetNamedPipeHandleStateW(HANDLE,PDWORD,PDWORD,PDWORD,PDWORD,LPWSTR,DWORD);
BOOL __attribute__((__stdcall__)) GetNamedPipeInfo(HANDLE,PDWORD,PDWORD,PDWORD,PDWORD);
BOOL __attribute__((__stdcall__)) GetNumberOfEventLogRecords(HANDLE,PDWORD);
BOOL __attribute__((__stdcall__)) GetOldestEventLogRecord(HANDLE,PDWORD);
BOOL __attribute__((__stdcall__)) GetOverlappedResult(HANDLE,LPOVERLAPPED,PDWORD,BOOL);
DWORD __attribute__((__stdcall__)) GetPriorityClass(HANDLE);
BOOL __attribute__((__stdcall__)) GetPrivateObjectSecurity(PSECURITY_DESCRIPTOR,SECURITY_INFORMATION,PSECURITY_DESCRIPTOR,DWORD,PDWORD);
UINT __attribute__((__stdcall__)) GetPrivateProfileIntA(LPCSTR,LPCSTR,INT,LPCSTR);
UINT __attribute__((__stdcall__)) GetPrivateProfileIntW(LPCWSTR,LPCWSTR,INT,LPCWSTR);
DWORD __attribute__((__stdcall__)) GetPrivateProfileSectionA(LPCSTR,LPSTR,DWORD,LPCSTR);
DWORD __attribute__((__stdcall__)) GetPrivateProfileSectionW(LPCWSTR,LPWSTR,DWORD,LPCWSTR);
DWORD __attribute__((__stdcall__)) GetPrivateProfileSectionNamesA(LPSTR,DWORD,LPCSTR);
DWORD __attribute__((__stdcall__)) GetPrivateProfileSectionNamesW(LPWSTR,DWORD,LPCWSTR);
DWORD __attribute__((__stdcall__)) GetPrivateProfileStringA(LPCSTR,LPCSTR,LPCSTR,LPSTR,DWORD,LPCSTR);
DWORD __attribute__((__stdcall__)) GetPrivateProfileStringW(LPCWSTR,LPCWSTR,LPCWSTR,LPWSTR,DWORD,LPCWSTR);
BOOL __attribute__((__stdcall__)) GetPrivateProfileStructA(LPCSTR,LPCSTR,LPVOID,UINT,LPCSTR);
BOOL __attribute__((__stdcall__)) GetPrivateProfileStructW(LPCWSTR,LPCWSTR,LPVOID,UINT,LPCWSTR);
FARPROC __attribute__((__stdcall__)) GetProcAddress(HINSTANCE,LPCSTR);
BOOL __attribute__((__stdcall__)) GetProcessAffinityMask(HANDLE,PDWORD,PDWORD);
HANDLE __attribute__((__stdcall__)) GetProcessHeap(void);
DWORD __attribute__((__stdcall__)) GetProcessHeaps(DWORD,PHANDLE);
BOOL __attribute__((__stdcall__)) GetProcessPriorityBoost(HANDLE,PBOOL);
BOOL __attribute__((__stdcall__)) GetProcessShutdownParameters(PDWORD,PDWORD);
BOOL __attribute__((__stdcall__)) GetProcessTimes(HANDLE,LPFILETIME,LPFILETIME,LPFILETIME,LPFILETIME);
DWORD __attribute__((__stdcall__)) GetProcessVersion(DWORD);
HWINSTA __attribute__((__stdcall__)) GetProcessWindowStation(void);
BOOL __attribute__((__stdcall__)) GetProcessWorkingSetSize(HANDLE,PDWORD,PDWORD);
UINT __attribute__((__stdcall__)) GetProfileIntA(LPCSTR,LPCSTR,INT);
UINT __attribute__((__stdcall__)) GetProfileIntW(LPCWSTR,LPCWSTR,INT);
DWORD __attribute__((__stdcall__)) GetProfileSectionA(LPCSTR,LPSTR,DWORD);
DWORD __attribute__((__stdcall__)) GetProfileSectionW(LPCWSTR,LPWSTR,DWORD);
DWORD __attribute__((__stdcall__)) GetProfileStringA(LPCSTR,LPCSTR,LPCSTR,LPSTR,DWORD);
DWORD __attribute__((__stdcall__)) GetProfileStringW(LPCWSTR,LPCWSTR,LPCWSTR,LPWSTR,DWORD);
BOOL __attribute__((__stdcall__)) GetQueuedCompletionStatus(HANDLE,PDWORD,PDWORD,LPOVERLAPPED*,DWORD);
BOOL __attribute__((__stdcall__)) GetSecurityDescriptorControl(PSECURITY_DESCRIPTOR,PSECURITY_DESCRIPTOR_CONTROL,PDWORD);
BOOL __attribute__((__stdcall__)) GetSecurityDescriptorDacl(PSECURITY_DESCRIPTOR,LPBOOL,PACL*,LPBOOL);
BOOL __attribute__((__stdcall__)) GetSecurityDescriptorGroup(PSECURITY_DESCRIPTOR,PSID*,LPBOOL);
DWORD __attribute__((__stdcall__)) GetSecurityDescriptorLength(PSECURITY_DESCRIPTOR);
BOOL __attribute__((__stdcall__)) GetSecurityDescriptorOwner(PSECURITY_DESCRIPTOR,PSID*,LPBOOL);
BOOL __attribute__((__stdcall__)) GetSecurityDescriptorSacl(PSECURITY_DESCRIPTOR,LPBOOL,PACL*,LPBOOL);
DWORD __attribute__((__stdcall__)) GetShortPathNameA(LPCSTR,LPSTR,DWORD);
DWORD __attribute__((__stdcall__)) GetShortPathNameW(LPCWSTR,LPWSTR,DWORD);
PSID_IDENTIFIER_AUTHORITY __attribute__((__stdcall__)) GetSidIdentifierAuthority(PSID);
DWORD __attribute__((__stdcall__)) GetSidLengthRequired(UCHAR);
PDWORD __attribute__((__stdcall__)) GetSidSubAuthority(PSID,DWORD);
PUCHAR __attribute__((__stdcall__)) GetSidSubAuthorityCount(PSID);
void __attribute__((__stdcall__)) GetStartupInfoA(LPSTARTUPINFOA);
void __attribute__((__stdcall__)) GetStartupInfoW(LPSTARTUPINFOW);
HANDLE __attribute__((__stdcall__)) GetStdHandle(DWORD);
UINT __attribute__((__stdcall__)) GetSystemDirectoryA(LPSTR,UINT);
UINT __attribute__((__stdcall__)) GetSystemDirectoryW(LPWSTR,UINT);
void __attribute__((__stdcall__)) GetSystemInfo(LPSYSTEM_INFO);
BOOL __attribute__((__stdcall__)) GetSystemPowerStatus(LPSYSTEM_POWER_STATUS);
void __attribute__((__stdcall__)) GetSystemTime(LPSYSTEMTIME);



BOOL __attribute__((__stdcall__)) GetSystemTimeAdjustment(PDWORD,PDWORD,PBOOL);
void __attribute__((__stdcall__)) GetSystemTimeAsFileTime(LPFILETIME);
DWORD __attribute__((__stdcall__)) GetTapeParameters(HANDLE,DWORD,PDWORD,PVOID);
DWORD __attribute__((__stdcall__)) GetTapePosition(HANDLE,DWORD,PDWORD,PDWORD,PDWORD);
DWORD __attribute__((__stdcall__)) GetTapeStatus(HANDLE);
UINT __attribute__((__stdcall__)) GetTempFileNameA(LPCSTR,LPCSTR,UINT,LPSTR);
UINT __attribute__((__stdcall__)) GetTempFileNameW(LPCWSTR,LPCWSTR,UINT,LPWSTR);
DWORD __attribute__((__stdcall__)) GetTempPathA(DWORD,LPSTR);
DWORD __attribute__((__stdcall__)) GetTempPathW(DWORD,LPWSTR);
BOOL __attribute__((__stdcall__)) GetThreadContext(HANDLE,LPCONTEXT);
int __attribute__((__stdcall__)) GetThreadPriority(HANDLE);
BOOL __attribute__((__stdcall__)) GetThreadPriorityBoost(HANDLE,PBOOL);
BOOL __attribute__((__stdcall__)) GetThreadSelectorEntry(HANDLE,DWORD,LPLDT_ENTRY);
BOOL __attribute__((__stdcall__)) GetThreadTimes(HANDLE,LPFILETIME,LPFILETIME,LPFILETIME,LPFILETIME);
DWORD __attribute__((__stdcall__)) GetTickCount(void);
DWORD __attribute__((__stdcall__)) GetTimeZoneInformation(LPTIME_ZONE_INFORMATION);
BOOL __attribute__((__stdcall__)) GetTokenInformation(HANDLE,TOKEN_INFORMATION_CLASS,PVOID,DWORD,PDWORD);
BOOL __attribute__((__stdcall__)) GetUserNameA (LPSTR,PDWORD);
BOOL __attribute__((__stdcall__)) GetUserNameW(LPWSTR,PDWORD);
DWORD __attribute__((__stdcall__)) GetVersion(void);
BOOL __attribute__((__stdcall__)) GetVersionExA(LPOSVERSIONINFOA);
BOOL __attribute__((__stdcall__)) GetVersionExW(LPOSVERSIONINFOW);
BOOL __attribute__((__stdcall__)) GetVolumeInformationA(LPCSTR,LPSTR,DWORD,PDWORD,PDWORD,PDWORD,LPSTR,DWORD);
BOOL __attribute__((__stdcall__)) GetVolumeInformationW(LPCWSTR,LPWSTR,DWORD,PDWORD,PDWORD,PDWORD,LPWSTR,DWORD);
UINT __attribute__((__stdcall__)) GetWindowsDirectoryA(LPSTR,UINT);
UINT __attribute__((__stdcall__)) GetWindowsDirectoryW(LPWSTR,UINT);
DWORD __attribute__((__stdcall__)) GetWindowThreadProcessId(HWND,PDWORD);
ATOM __attribute__((__stdcall__)) GlobalAddAtomA(LPCSTR);
ATOM __attribute__((__stdcall__)) GlobalAddAtomW( LPCWSTR);
HGLOBAL __attribute__((__stdcall__)) GlobalAlloc(UINT,DWORD);
UINT __attribute__((__stdcall__)) GlobalCompact(DWORD);
ATOM __attribute__((__stdcall__)) GlobalDeleteAtom(ATOM);
HGLOBAL GlobalDiscard(HGLOBAL);
ATOM __attribute__((__stdcall__)) GlobalFindAtomA(LPCSTR);
ATOM __attribute__((__stdcall__)) GlobalFindAtomW(LPCWSTR);
void __attribute__((__stdcall__)) GlobalFix(HGLOBAL);
UINT __attribute__((__stdcall__)) GlobalFlags(HGLOBAL);
HGLOBAL __attribute__((__stdcall__)) GlobalFree(HGLOBAL);
UINT __attribute__((__stdcall__)) GlobalGetAtomNameA(ATOM,LPSTR,int);
UINT __attribute__((__stdcall__)) GlobalGetAtomNameW(ATOM,LPWSTR,int);
HGLOBAL __attribute__((__stdcall__)) GlobalHandle(PCVOID);
LPVOID __attribute__((__stdcall__)) GlobalLock(HGLOBAL);
void __attribute__((__stdcall__)) GlobalMemoryStatus(LPMEMORYSTATUS);



HGLOBAL __attribute__((__stdcall__)) GlobalReAlloc(HGLOBAL,DWORD,UINT);
DWORD __attribute__((__stdcall__)) GlobalSize(HGLOBAL);
void __attribute__((__stdcall__)) GlobalUnfix(HGLOBAL);
BOOL __attribute__((__stdcall__)) GlobalUnlock(HGLOBAL);
BOOL __attribute__((__stdcall__)) GlobalUnWire(HGLOBAL);
PVOID __attribute__((__stdcall__)) GlobalWire(HGLOBAL);

PVOID __attribute__((__stdcall__)) HeapAlloc(HANDLE,DWORD,DWORD);
UINT __attribute__((__stdcall__)) HeapCompact(HANDLE,DWORD);
HANDLE __attribute__((__stdcall__)) HeapCreate(DWORD,DWORD,DWORD);
BOOL __attribute__((__stdcall__)) HeapDestroy(HANDLE);
BOOL __attribute__((__stdcall__)) HeapFree(HANDLE,DWORD,PVOID);
BOOL __attribute__((__stdcall__)) HeapLock(HANDLE);
PVOID __attribute__((__stdcall__)) HeapReAlloc(HANDLE,DWORD,PVOID,DWORD);
DWORD __attribute__((__stdcall__)) HeapSize(HANDLE,DWORD,PCVOID);
BOOL __attribute__((__stdcall__)) HeapUnlock(HANDLE);
BOOL __attribute__((__stdcall__)) HeapValidate(HANDLE,DWORD,PCVOID);
BOOL __attribute__((__stdcall__)) HeapWalk(HANDLE,LPPROCESS_HEAP_ENTRY);
BOOL __attribute__((__stdcall__)) ImpersonateLoggedOnUser(HANDLE);
BOOL __attribute__((__stdcall__)) ImpersonateNamedPipeClient(HANDLE);
BOOL __attribute__((__stdcall__)) ImpersonateSelf(SECURITY_IMPERSONATION_LEVEL);
BOOL __attribute__((__stdcall__)) InitAtomTable(DWORD);
BOOL __attribute__((__stdcall__)) InitializeAcl(PACL,DWORD,DWORD);
void __attribute__((__stdcall__)) InitializeCriticalSection(LPCRITICAL_SECTION);




BOOL __attribute__((__stdcall__)) InitializeSecurityDescriptor(PSECURITY_DESCRIPTOR,DWORD);
BOOL __attribute__((__stdcall__)) InitializeSid (PSID,PSID_IDENTIFIER_AUTHORITY,BYTE);


LONG __attribute__((__stdcall__)) InterlockedCompareExchange(LPLONG,LONG,LONG);



LONG __attribute__((__stdcall__)) InterlockedDecrement(LPLONG);
LONG __attribute__((__stdcall__)) InterlockedExchange(LPLONG,LONG);



LONG __attribute__((__stdcall__)) InterlockedExchangeAdd(LPLONG,LONG);
LONG __attribute__((__stdcall__)) InterlockedIncrement(LPLONG);

BOOL __attribute__((__stdcall__)) IsBadCodePtr(FARPROC);
BOOL __attribute__((__stdcall__)) IsBadHugeReadPtr(PCVOID,UINT);
BOOL __attribute__((__stdcall__)) IsBadHugeWritePtr(PVOID,UINT);
BOOL __attribute__((__stdcall__)) IsBadReadPtr(PCVOID,UINT);
BOOL __attribute__((__stdcall__)) IsBadStringPtrA(LPCSTR,UINT);
BOOL __attribute__((__stdcall__)) IsBadStringPtrW(LPCWSTR,UINT);
BOOL __attribute__((__stdcall__)) IsBadWritePtr(PVOID,UINT);
BOOL __attribute__((__stdcall__)) IsDebuggerPresent(void);
BOOL __attribute__((__stdcall__)) IsProcessorFeaturePresent(DWORD);
BOOL __attribute__((__stdcall__)) IsTextUnicode(PCVOID,int,LPINT);
BOOL __attribute__((__stdcall__)) IsValidAcl(PACL);
BOOL __attribute__((__stdcall__)) IsValidSecurityDescriptor(PSECURITY_DESCRIPTOR);
BOOL __attribute__((__stdcall__)) IsValidSid(PSID);
void __attribute__((__stdcall__)) LeaveCriticalSection(LPCRITICAL_SECTION);

HINSTANCE __attribute__((__stdcall__)) LoadLibraryA(LPCSTR);
HINSTANCE __attribute__((__stdcall__)) LoadLibraryExA(LPCSTR,HANDLE,DWORD);
HINSTANCE __attribute__((__stdcall__)) LoadLibraryExW(LPCWSTR,HANDLE,DWORD);
HINSTANCE __attribute__((__stdcall__)) LoadLibraryW(LPCWSTR);
DWORD __attribute__((__stdcall__)) LoadModule(LPCSTR,PVOID);
HGLOBAL __attribute__((__stdcall__)) LoadResource(HINSTANCE,HRSRC);
HLOCAL __attribute__((__stdcall__)) LocalAlloc(UINT,UINT);
UINT __attribute__((__stdcall__)) LocalCompact(UINT);
HLOCAL LocalDiscard(HLOCAL);
BOOL __attribute__((__stdcall__)) LocalFileTimeToFileTime(const FILETIME *,LPFILETIME);
UINT __attribute__((__stdcall__)) LocalFlags(HLOCAL);
HLOCAL __attribute__((__stdcall__)) LocalFree(HLOCAL);
HLOCAL __attribute__((__stdcall__)) LocalHandle(LPCVOID);
PVOID __attribute__((__stdcall__)) LocalLock(HLOCAL);
HLOCAL __attribute__((__stdcall__)) LocalReAlloc(HLOCAL,UINT,UINT);
UINT __attribute__((__stdcall__)) LocalShrink(HLOCAL,UINT);
UINT __attribute__((__stdcall__)) LocalSize(HLOCAL);
BOOL __attribute__((__stdcall__)) LocalUnlock(HLOCAL);
BOOL __attribute__((__stdcall__)) LockFile(HANDLE,DWORD,DWORD,DWORD,DWORD);
BOOL __attribute__((__stdcall__)) LockFileEx(HANDLE,DWORD,DWORD,DWORD,DWORD,LPOVERLAPPED);
PVOID __attribute__((__stdcall__)) LockResource(HGLOBAL);

BOOL __attribute__((__stdcall__)) LogonUserA(LPSTR,LPSTR,LPSTR,DWORD,DWORD,PHANDLE);
BOOL __attribute__((__stdcall__)) LogonUserW(LPWSTR,LPWSTR,LPWSTR,DWORD,DWORD,PHANDLE);
BOOL __attribute__((__stdcall__)) LookupAccountNameA(LPCSTR,LPCSTR,PSID,PDWORD,LPSTR,PDWORD,PSID_NAME_USE);
BOOL __attribute__((__stdcall__)) LookupAccountNameW(LPCWSTR,LPCWSTR,PSID,PDWORD,LPWSTR,PDWORD,PSID_NAME_USE);
BOOL __attribute__((__stdcall__)) LookupAccountSidA(LPCSTR,PSID,LPSTR,PDWORD,LPSTR,PDWORD,PSID_NAME_USE);
BOOL __attribute__((__stdcall__)) LookupAccountSidW(LPCWSTR,PSID,LPWSTR,PDWORD,LPWSTR,PDWORD,PSID_NAME_USE);
BOOL __attribute__((__stdcall__)) LookupPrivilegeDisplayNameA(LPCSTR,LPCSTR,LPSTR,PDWORD,PDWORD);
BOOL __attribute__((__stdcall__)) LookupPrivilegeDisplayNameW(LPCWSTR,LPCWSTR,LPWSTR,PDWORD,PDWORD);
BOOL __attribute__((__stdcall__)) LookupPrivilegeNameA(LPCSTR,PLUID,LPSTR,PDWORD);
BOOL __attribute__((__stdcall__)) LookupPrivilegeNameW(LPCWSTR,PLUID,LPWSTR,PDWORD);
BOOL __attribute__((__stdcall__)) LookupPrivilegeValueA(LPCSTR,LPCSTR,PLUID);
BOOL __attribute__((__stdcall__)) LookupPrivilegeValueW(LPCWSTR,LPCWSTR,PLUID);
LPSTR __attribute__((__stdcall__)) lstrcatA(LPSTR,LPCSTR);
LPWSTR __attribute__((__stdcall__)) lstrcatW(LPWSTR,LPCWSTR);
int __attribute__((__stdcall__)) lstrcmpA(LPCSTR,LPCSTR);
int __attribute__((__stdcall__)) lstrcmpiA(LPCSTR,LPCSTR);
int __attribute__((__stdcall__)) lstrcmpiW( LPCWSTR,LPCWSTR);
int __attribute__((__stdcall__)) lstrcmpW(LPCWSTR,LPCWSTR);
LPSTR __attribute__((__stdcall__)) lstrcpyA(LPSTR,LPCSTR);
LPSTR __attribute__((__stdcall__)) lstrcpynA(LPSTR,LPCSTR,int);
LPWSTR __attribute__((__stdcall__)) lstrcpynW(LPWSTR,LPCWSTR,int);
LPWSTR __attribute__((__stdcall__)) lstrcpyW(LPWSTR,LPCWSTR);
int __attribute__((__stdcall__)) lstrlenA(LPCSTR);
int __attribute__((__stdcall__)) lstrlenW(LPCWSTR);
BOOL __attribute__((__stdcall__)) MakeAbsoluteSD(PSECURITY_DESCRIPTOR,PSECURITY_DESCRIPTOR,PDWORD,PACL,PDWORD,PACL,PDWORD,PSID,PDWORD,PSID,PDWORD);

BOOL __attribute__((__stdcall__)) MakeSelfRelativeSD(PSECURITY_DESCRIPTOR,PSECURITY_DESCRIPTOR,PDWORD);
void __attribute__((__stdcall__)) MapGenericMask(PDWORD,PGENERIC_MAPPING);
PVOID __attribute__((__stdcall__)) MapViewOfFile(HANDLE,DWORD,DWORD,DWORD,DWORD);
PVOID __attribute__((__stdcall__)) MapViewOfFileEx(HANDLE,DWORD,DWORD,DWORD,DWORD,PVOID);
BOOL __attribute__((__stdcall__)) MoveFileA(LPCSTR,LPCSTR);
BOOL __attribute__((__stdcall__)) MoveFileExA(LPCSTR,LPCSTR,DWORD);
BOOL __attribute__((__stdcall__)) MoveFileExW(LPCWSTR,LPCWSTR,DWORD);
BOOL __attribute__((__stdcall__)) MoveFileW(LPCWSTR,LPCWSTR);
int __attribute__((__stdcall__)) MulDiv(int,int,int);
BOOL __attribute__((__stdcall__)) NotifyChangeEventLog(HANDLE,HANDLE);
BOOL __attribute__((__stdcall__)) ObjectCloseAuditAlarmA(LPCSTR,PVOID,BOOL);
BOOL __attribute__((__stdcall__)) ObjectCloseAuditAlarmW(LPCWSTR,PVOID,BOOL);
BOOL __attribute__((__stdcall__)) ObjectDeleteAuditAlarmA(LPCSTR,PVOID,BOOL);
BOOL __attribute__((__stdcall__)) ObjectDeleteAuditAlarmW(LPCWSTR,PVOID,BOOL);
BOOL __attribute__((__stdcall__)) ObjectOpenAuditAlarmA(LPCSTR,PVOID,LPSTR,LPSTR,PSECURITY_DESCRIPTOR,HANDLE,DWORD,DWORD,PPRIVILEGE_SET,BOOL,BOOL,PBOOL);
BOOL __attribute__((__stdcall__)) ObjectOpenAuditAlarmW(LPCWSTR,PVOID,LPWSTR,LPWSTR,PSECURITY_DESCRIPTOR,HANDLE,DWORD,DWORD,PPRIVILEGE_SET,BOOL,BOOL,PBOOL);
BOOL __attribute__((__stdcall__)) ObjectPrivilegeAuditAlarmA(LPCSTR,PVOID,HANDLE,DWORD,PPRIVILEGE_SET,BOOL);
BOOL __attribute__((__stdcall__)) ObjectPrivilegeAuditAlarmW(LPCWSTR,PVOID,HANDLE,DWORD,PPRIVILEGE_SET,BOOL);
HANDLE __attribute__((__stdcall__)) OpenBackupEventLogA(LPCSTR,LPCSTR);
HANDLE __attribute__((__stdcall__)) OpenBackupEventLogW(LPCWSTR,LPCWSTR);
HANDLE __attribute__((__stdcall__)) OpenEventA(DWORD,BOOL,LPCSTR);
HANDLE __attribute__((__stdcall__)) OpenEventLogA (LPCSTR,LPCSTR);
HANDLE __attribute__((__stdcall__)) OpenEventLogW(LPCWSTR,LPCWSTR);
HANDLE __attribute__((__stdcall__)) OpenEventW(DWORD,BOOL,LPCWSTR);
HFILE __attribute__((__stdcall__)) OpenFile(LPCSTR,LPOFSTRUCT,UINT);
HANDLE __attribute__((__stdcall__)) OpenFileMappingA(DWORD,BOOL,LPCSTR);
HANDLE __attribute__((__stdcall__)) OpenFileMappingW(DWORD,BOOL,LPCWSTR);
HANDLE __attribute__((__stdcall__)) OpenMutexA(DWORD,BOOL,LPCSTR);
HANDLE __attribute__((__stdcall__)) OpenMutexW(DWORD,BOOL,LPCWSTR);
HANDLE __attribute__((__stdcall__)) OpenProcess(DWORD,BOOL,DWORD);
BOOL __attribute__((__stdcall__)) OpenProcessToken(HANDLE,DWORD,PHANDLE);
HANDLE __attribute__((__stdcall__)) OpenSemaphoreA(DWORD,BOOL,LPCSTR);
HANDLE __attribute__((__stdcall__)) OpenSemaphoreW(DWORD,BOOL,LPCWSTR);



BOOL __attribute__((__stdcall__)) OpenThreadToken(HANDLE,DWORD,BOOL,PHANDLE);
HANDLE __attribute__((__stdcall__)) OpenWaitableTimerA(DWORD,BOOL,LPCSTR);
HANDLE __attribute__((__stdcall__)) OpenWaitableTimerW(DWORD,BOOL,LPCWSTR);
void __attribute__((__stdcall__)) OutputDebugStringA(LPCSTR);
void __attribute__((__stdcall__)) OutputDebugStringW(LPCWSTR);
BOOL __attribute__((__stdcall__)) PeekNamedPipe(HANDLE,PVOID,DWORD,PDWORD,PDWORD,PDWORD);
BOOL __attribute__((__stdcall__)) PostQueuedCompletionStatus(HANDLE,DWORD,DWORD,LPOVERLAPPED);
DWORD __attribute__((__stdcall__)) PrepareTape(HANDLE,DWORD,BOOL);
BOOL __attribute__((__stdcall__)) PrivilegeCheck (HANDLE,PPRIVILEGE_SET,PBOOL);
BOOL __attribute__((__stdcall__)) PrivilegedServiceAuditAlarmA(LPCSTR,LPCSTR,HANDLE,PPRIVILEGE_SET,BOOL);
BOOL __attribute__((__stdcall__)) PrivilegedServiceAuditAlarmW(LPCWSTR,LPCWSTR,HANDLE,PPRIVILEGE_SET,BOOL);
BOOL __attribute__((__stdcall__)) PulseEvent(HANDLE);
BOOL __attribute__((__stdcall__)) PurgeComm(HANDLE,DWORD);
DWORD __attribute__((__stdcall__)) QueryDosDeviceA(LPCSTR,LPSTR,DWORD);
DWORD __attribute__((__stdcall__)) QueryDosDeviceW(LPCWSTR,LPWSTR,DWORD);
BOOL __attribute__((__stdcall__)) QueryPerformanceCounter(PLARGE_INTEGER);
BOOL __attribute__((__stdcall__)) QueryPerformanceFrequency(PLARGE_INTEGER);
DWORD __attribute__((__stdcall__)) QueueUserAPC(PAPCFUNC,HANDLE,DWORD);
void __attribute__((__stdcall__)) RaiseException(DWORD,DWORD,DWORD,const DWORD*);
BOOL __attribute__((__stdcall__)) ReadDirectoryChangesW(HANDLE,PVOID,DWORD,BOOL,DWORD,PDWORD,LPOVERLAPPED,LPOVERLAPPED_COMPLETION_ROUTINE);
BOOL __attribute__((__stdcall__)) ReadEventLogA(HANDLE,DWORD,DWORD,PVOID,DWORD,DWORD *,DWORD *);
BOOL __attribute__((__stdcall__)) ReadEventLogW(HANDLE,DWORD,DWORD,PVOID,DWORD,DWORD *,DWORD *);
BOOL __attribute__((__stdcall__)) ReadFile(HANDLE,PVOID,DWORD,PDWORD,LPOVERLAPPED);
BOOL __attribute__((__stdcall__)) ReadFileEx(HANDLE,PVOID,DWORD,LPOVERLAPPED,LPOVERLAPPED_COMPLETION_ROUTINE);
BOOL __attribute__((__stdcall__)) ReadFileScatter(HANDLE,FILE_SEGMENT_ELEMENT*,DWORD,LPDWORD,LPOVERLAPPED);
BOOL __attribute__((__stdcall__)) ReadProcessMemory(HANDLE,PCVOID,PVOID,DWORD,PDWORD);
HANDLE __attribute__((__stdcall__)) RegisterEventSourceA (LPCSTR,LPCSTR);
HANDLE __attribute__((__stdcall__)) RegisterEventSourceW(LPCWSTR,LPCWSTR);
BOOL __attribute__((__stdcall__)) ReleaseMutex(HANDLE);
BOOL __attribute__((__stdcall__)) ReleaseSemaphore(HANDLE,LONG,LPLONG);
BOOL __attribute__((__stdcall__)) RemoveDirectoryA(LPCSTR);
BOOL __attribute__((__stdcall__)) RemoveDirectoryW(LPCWSTR);
BOOL __attribute__((__stdcall__)) ReportEventA(HANDLE,WORD,WORD,DWORD,PSID,WORD,DWORD,LPCSTR*,PVOID);
BOOL __attribute__((__stdcall__)) ReportEventW(HANDLE,WORD,WORD,DWORD,PSID,WORD,DWORD,LPCWSTR*,PVOID);
BOOL __attribute__((__stdcall__)) ResetEvent(HANDLE);
DWORD __attribute__((__stdcall__)) ResumeThread(HANDLE);
BOOL __attribute__((__stdcall__)) RevertToSelf(void);
DWORD __attribute__((__stdcall__)) SearchPathA(LPCSTR,LPCSTR,LPCSTR,DWORD,LPSTR,LPSTR*);
DWORD __attribute__((__stdcall__)) SearchPathW(LPCWSTR,LPCWSTR,LPCWSTR,DWORD,LPWSTR,LPWSTR*);
BOOL __attribute__((__stdcall__)) SetAclInformation(PACL,PVOID,DWORD,ACL_INFORMATION_CLASS);
BOOL __attribute__((__stdcall__)) SetCommBreak(HANDLE);
BOOL __attribute__((__stdcall__)) SetCommConfig(HANDLE,LPCOMMCONFIG,DWORD);
BOOL __attribute__((__stdcall__)) SetCommMask(HANDLE,DWORD);
BOOL __attribute__((__stdcall__)) SetCommState(HANDLE,LPDCB);
BOOL __attribute__((__stdcall__)) SetCommTimeouts(HANDLE,LPCOMMTIMEOUTS);
BOOL __attribute__((__stdcall__)) SetComputerNameA(LPCSTR);
BOOL __attribute__((__stdcall__)) SetComputerNameW(LPCWSTR);
BOOL __attribute__((__stdcall__)) SetCurrentDirectoryA(LPCSTR);
BOOL __attribute__((__stdcall__)) SetCurrentDirectoryW(LPCWSTR);
BOOL __attribute__((__stdcall__)) SetDefaultCommConfigA(LPCSTR,LPCOMMCONFIG,DWORD);
BOOL __attribute__((__stdcall__)) SetDefaultCommConfigW(LPCWSTR,LPCOMMCONFIG,DWORD);
BOOL __attribute__((__stdcall__)) SetEndOfFile(HANDLE);
BOOL __attribute__((__stdcall__)) SetEnvironmentVariableA(LPCSTR,LPCSTR);
BOOL __attribute__((__stdcall__)) SetEnvironmentVariableW(LPCWSTR,LPCWSTR);
UINT __attribute__((__stdcall__)) SetErrorMode(UINT);
BOOL __attribute__((__stdcall__)) SetEvent(HANDLE);
void __attribute__((__stdcall__)) SetFileApisToANSI(void);
void __attribute__((__stdcall__)) SetFileApisToOEM(void);
BOOL __attribute__((__stdcall__)) SetFileAttributesA(LPCSTR,DWORD);
BOOL __attribute__((__stdcall__)) SetFileAttributesW(LPCWSTR,DWORD);
DWORD __attribute__((__stdcall__)) SetFilePointer(HANDLE,LONG,PLONG,DWORD);
BOOL __attribute__((__stdcall__)) SetFilePointerEx(HANDLE,LARGE_INTEGER,PLARGE_INTEGER,DWORD);
BOOL __attribute__((__stdcall__)) SetFileSecurityA(LPCSTR,SECURITY_INFORMATION,PSECURITY_DESCRIPTOR);
BOOL __attribute__((__stdcall__)) SetFileSecurityW(LPCWSTR,SECURITY_INFORMATION,PSECURITY_DESCRIPTOR);
BOOL __attribute__((__stdcall__)) SetFileTime(HANDLE,const FILETIME*,const FILETIME*,const FILETIME*);
UINT __attribute__((__stdcall__)) SetHandleCount(UINT);
BOOL __attribute__((__stdcall__)) SetHandleInformation(HANDLE,DWORD,DWORD);
BOOL __attribute__((__stdcall__)) SetKernelObjectSecurity(HANDLE,SECURITY_INFORMATION,PSECURITY_DESCRIPTOR);
void __attribute__((__stdcall__)) SetLastError(DWORD);
void __attribute__((__stdcall__)) SetLastErrorEx(DWORD,DWORD);
BOOL __attribute__((__stdcall__)) SetLocalTime(const SYSTEMTIME*);
BOOL __attribute__((__stdcall__)) SetMailslotInfo(HANDLE,DWORD);
BOOL __attribute__((__stdcall__)) SetNamedPipeHandleState(HANDLE,PDWORD,PDWORD,PDWORD);
BOOL __attribute__((__stdcall__)) SetPriorityClass(HANDLE,DWORD);
BOOL __attribute__((__stdcall__)) SetPrivateObjectSecurity(SECURITY_INFORMATION,PSECURITY_DESCRIPTOR,PSECURITY_DESCRIPTOR *,PGENERIC_MAPPING,HANDLE);
BOOL __attribute__((__stdcall__)) SetProcessAffinityMask(HANDLE,DWORD);
BOOL __attribute__((__stdcall__)) SetProcessPriorityBoost(HANDLE,BOOL);
BOOL __attribute__((__stdcall__)) SetProcessShutdownParameters(DWORD,DWORD);
BOOL __attribute__((__stdcall__)) SetProcessWorkingSetSize(HANDLE,DWORD,DWORD);
BOOL __attribute__((__stdcall__)) SetSecurityDescriptorControl(PSECURITY_DESCRIPTOR,SECURITY_DESCRIPTOR_CONTROL,SECURITY_DESCRIPTOR_CONTROL);
BOOL __attribute__((__stdcall__)) SetSecurityDescriptorDacl(PSECURITY_DESCRIPTOR,BOOL,PACL,BOOL);
BOOL __attribute__((__stdcall__)) SetSecurityDescriptorGroup(PSECURITY_DESCRIPTOR,PSID,BOOL);
BOOL __attribute__((__stdcall__)) SetSecurityDescriptorOwner(PSECURITY_DESCRIPTOR,PSID,BOOL);
BOOL __attribute__((__stdcall__)) SetSecurityDescriptorSacl(PSECURITY_DESCRIPTOR,BOOL,PACL,BOOL);
BOOL __attribute__((__stdcall__)) SetStdHandle(DWORD,HANDLE);

BOOL __attribute__((__stdcall__)) SetSystemPowerState(BOOL,BOOL);
BOOL __attribute__((__stdcall__)) SetSystemTime(const SYSTEMTIME*);
BOOL __attribute__((__stdcall__)) SetSystemTimeAdjustment(DWORD,BOOL);
DWORD __attribute__((__stdcall__)) SetTapeParameters(HANDLE,DWORD,PVOID);
DWORD __attribute__((__stdcall__)) SetTapePosition(HANDLE,DWORD,DWORD,DWORD,DWORD,BOOL);
DWORD __attribute__((__stdcall__)) SetThreadAffinityMask(HANDLE,DWORD);
BOOL __attribute__((__stdcall__)) SetThreadContext(HANDLE,const CONTEXT*);
DWORD __attribute__((__stdcall__)) SetThreadIdealProcessor(HANDLE,DWORD);
BOOL __attribute__((__stdcall__)) SetThreadPriority(HANDLE,int);
BOOL __attribute__((__stdcall__)) SetThreadPriorityBoost(HANDLE,BOOL);
BOOL __attribute__((__stdcall__)) SetThreadToken (PHANDLE,HANDLE);
BOOL __attribute__((__stdcall__)) SetTimeZoneInformation(const TIME_ZONE_INFORMATION *);
BOOL __attribute__((__stdcall__)) SetTokenInformation(HANDLE,TOKEN_INFORMATION_CLASS,PVOID,DWORD);
LPTOP_LEVEL_EXCEPTION_FILTER __attribute__((__stdcall__)) SetUnhandledExceptionFilter(LPTOP_LEVEL_EXCEPTION_FILTER);
BOOL __attribute__((__stdcall__)) SetupComm(HANDLE,DWORD,DWORD);
BOOL __attribute__((__stdcall__)) SetVolumeLabelA(LPCSTR,LPCSTR);
BOOL __attribute__((__stdcall__)) SetVolumeLabelW(LPCWSTR,LPCWSTR);
BOOL __attribute__((__stdcall__)) SetWaitableTimer(HANDLE,const LARGE_INTEGER*,LONG,PTIMERAPCROUTINE,PVOID,BOOL);
BOOL __attribute__((__stdcall__)) SignalObjectAndWait(HANDLE,HANDLE,DWORD,BOOL);
DWORD __attribute__((__stdcall__)) SizeofResource(HINSTANCE,HRSRC);
void __attribute__((__stdcall__)) Sleep(DWORD);
DWORD __attribute__((__stdcall__)) SleepEx(DWORD,BOOL);
DWORD __attribute__((__stdcall__)) SuspendThread(HANDLE);
void __attribute__((__stdcall__)) SwitchToFiber(PVOID);
BOOL __attribute__((__stdcall__)) SwitchToThread(void);
BOOL __attribute__((__stdcall__)) SystemTimeToFileTime(const SYSTEMTIME*,LPFILETIME);
BOOL __attribute__((__stdcall__)) SystemTimeToTzSpecificLocalTime(LPTIME_ZONE_INFORMATION,LPSYSTEMTIME,LPSYSTEMTIME);
BOOL __attribute__((__stdcall__)) TerminateProcess(HANDLE,UINT);
BOOL __attribute__((__stdcall__)) TerminateThread(HANDLE,DWORD);
DWORD __attribute__((__stdcall__)) TlsAlloc(void);
BOOL __attribute__((__stdcall__)) TlsFree(DWORD);
PVOID __attribute__((__stdcall__)) TlsGetValue(DWORD);
BOOL __attribute__((__stdcall__)) TlsSetValue(DWORD,PVOID);
BOOL __attribute__((__stdcall__)) TransactNamedPipe(HANDLE,PVOID,DWORD,PVOID,DWORD,PDWORD,LPOVERLAPPED);
BOOL __attribute__((__stdcall__)) TransmitCommChar(HANDLE,char);
BOOL __attribute__((__stdcall__)) TryEnterCriticalSection(LPCRITICAL_SECTION);
LONG __attribute__((__stdcall__)) UnhandledExceptionFilter(LPEXCEPTION_POINTERS);
BOOL __attribute__((__stdcall__)) UnlockFile(HANDLE,DWORD,DWORD,DWORD,DWORD);
BOOL __attribute__((__stdcall__)) UnlockFileEx(HANDLE,DWORD,DWORD,DWORD,LPOVERLAPPED);


BOOL __attribute__((__stdcall__)) UnmapViewOfFile(PVOID);
BOOL __attribute__((__stdcall__)) UpdateResourceA(HANDLE,LPCSTR,LPCSTR,WORD,PVOID,DWORD);
BOOL __attribute__((__stdcall__)) UpdateResourceW(HANDLE,LPCWSTR,LPCWSTR,WORD,PVOID,DWORD);
BOOL __attribute__((__stdcall__)) VerifyVersionInfoA(LPOSVERSIONINFOEXA,DWORD,DWORDLONG);
BOOL __attribute__((__stdcall__)) VerifyVersionInfoW(LPOSVERSIONINFOEXW,DWORD,DWORDLONG);
PVOID __attribute__((__stdcall__)) VirtualAlloc(PVOID,DWORD,DWORD,DWORD);
PVOID __attribute__((__stdcall__)) VirtualAllocEx(HANDLE,PVOID,DWORD,DWORD,DWORD);
BOOL __attribute__((__stdcall__)) VirtualFree(PVOID,DWORD,DWORD);
BOOL __attribute__((__stdcall__)) VirtualFreeEx(HANDLE,PVOID,DWORD,DWORD);
BOOL __attribute__((__stdcall__)) VirtualLock(PVOID,DWORD);
BOOL __attribute__((__stdcall__)) VirtualProtect(PVOID,DWORD,DWORD,PDWORD);
BOOL __attribute__((__stdcall__)) VirtualProtectEx(HANDLE,PVOID,DWORD,DWORD,PDWORD);
DWORD __attribute__((__stdcall__)) VirtualQuery(LPCVOID,PMEMORY_BASIC_INFORMATION,DWORD);
DWORD __attribute__((__stdcall__)) VirtualQueryEx(HANDLE,LPCVOID,PMEMORY_BASIC_INFORMATION,DWORD);
BOOL __attribute__((__stdcall__)) VirtualUnlock(PVOID,DWORD);
BOOL __attribute__((__stdcall__)) WaitCommEvent(HANDLE,PDWORD,LPOVERLAPPED);
BOOL __attribute__((__stdcall__)) WaitForDebugEvent(LPDEBUG_EVENT,DWORD);
DWORD __attribute__((__stdcall__)) WaitForMultipleObjects(DWORD,const HANDLE*,BOOL,DWORD);
DWORD __attribute__((__stdcall__)) WaitForMultipleObjectsEx(DWORD,const HANDLE*,BOOL,DWORD,BOOL);
DWORD __attribute__((__stdcall__)) WaitForSingleObject(HANDLE,DWORD);
DWORD __attribute__((__stdcall__)) WaitForSingleObjectEx(HANDLE,DWORD,BOOL);
BOOL __attribute__((__stdcall__)) WaitNamedPipeA(LPCSTR,DWORD);
BOOL __attribute__((__stdcall__)) WaitNamedPipeW(LPCWSTR,DWORD);
BOOL __attribute__((__stdcall__)) WinLoadTrustProvider(GUID*);
BOOL __attribute__((__stdcall__)) WriteFile(HANDLE,PCVOID,DWORD,PDWORD,LPOVERLAPPED);
BOOL __attribute__((__stdcall__)) WriteFileEx(HANDLE,PCVOID,DWORD,LPOVERLAPPED,LPOVERLAPPED_COMPLETION_ROUTINE);
BOOL __attribute__((__stdcall__)) WriteFileGather(HANDLE,FILE_SEGMENT_ELEMENT*,DWORD,LPDWORD,LPOVERLAPPED);
BOOL __attribute__((__stdcall__)) WritePrivateProfileSectionA(LPCSTR,LPCSTR,LPCSTR);
BOOL __attribute__((__stdcall__)) WritePrivateProfileSectionW(LPCWSTR,LPCWSTR,LPCWSTR);
BOOL __attribute__((__stdcall__)) WritePrivateProfileStringA(LPCSTR,LPCSTR,LPCSTR,LPCSTR);
BOOL __attribute__((__stdcall__)) WritePrivateProfileStringW(LPCWSTR,LPCWSTR,LPCWSTR,LPCWSTR);
BOOL __attribute__((__stdcall__)) WritePrivateProfileStructA(LPCSTR,LPCSTR,LPVOID,UINT,LPCSTR);
BOOL __attribute__((__stdcall__)) WritePrivateProfileStructW(LPCWSTR,LPCWSTR,LPVOID,UINT,LPCWSTR);
BOOL __attribute__((__stdcall__)) WriteProcessMemory(HANDLE,PVOID,PVOID,DWORD,PDWORD);
BOOL __attribute__((__stdcall__)) WriteProfileSectionA(LPCSTR,LPCSTR);
BOOL __attribute__((__stdcall__)) WriteProfileSectionW(LPCWSTR,LPCWSTR);
BOOL __attribute__((__stdcall__)) WriteProfileStringA(LPCSTR,LPCSTR,LPCSTR);
BOOL __attribute__((__stdcall__)) WriteProfileStringW(LPCWSTR,LPCWSTR,LPCWSTR);
DWORD __attribute__((__stdcall__)) WriteTapemark(HANDLE,DWORD,DWORD,BOOL);
# 1802 "d:/util/mingw/include/winbase.h"
typedef STARTUPINFOA STARTUPINFO,*LPSTARTUPINFO;
typedef WIN32_FIND_DATAA WIN32_FIND_DATA,*LPWIN32_FIND_DATA;
typedef HW_PROFILE_INFOA HW_PROFILE_INFO,*LPHW_PROFILE_INFO;
# 1950 "d:/util/mingw/include/winbase.h"
}
# 157 "f:/source-wip/CS/include/cssys/win32/csosdefs.h" 2
# 1 "d:/util/mingw/include/malloc.h" 1
# 45 "d:/util/mingw/include/malloc.h"
typedef struct _heapinfo
{
        int* _pentry;
        size_t _size;
        int _useflag;
} _HEAPINFO;






extern "C" {





__attribute__((dllimport)) int _heapwalk (_HEAPINFO*);





__attribute__((dllimport)) int heapwalk (_HEAPINFO*);





__attribute__((dllimport)) int _heapchk (void);
__attribute__((dllimport)) int _heapmin (void);
__attribute__((dllimport)) int _heapset (unsigned int);

__attribute__((dllimport)) size_t _msize (void*);
__attribute__((dllimport)) size_t _get_sbh_threshold (void);
__attribute__((dllimport)) int _set_sbh_threshold (size_t);
__attribute__((dllimport)) void* _expand (void*, size_t);


}
# 158 "f:/source-wip/CS/include/cssys/win32/csosdefs.h" 2
# 31 "f:/source-wip/CS/include/cssys/csosdefs.h" 2
# 74 "f:/source-wip/CS/include/cssysdef.h" 2
# 722 "f:/source-wip/CS/include/cssysdef.h"
extern void (*fatal_exit) (int errorcode, bool canreturn);
# 21 "lightiter.cpp" 2

# 1 "f:/source-wip/CS/include/iutil/document.h" 1
# 28 "f:/source-wip/CS/include/iutil/document.h"
# 1 "f:/source-wip/CS/include/csutil/scf.h" 1
# 31 "f:/source-wip/CS/include/csutil/scf.h"
# 1 "f:/source-wip/CS/include/csutil/ref.h" 1
# 25 "f:/source-wip/CS/include/csutil/ref.h"
template <class T> class csRef;
# 43 "f:/source-wip/CS/include/csutil/ref.h"
template <class T>
class csPtr
{
private:
  friend class csRef<T>;
  T* obj;

public:
  csPtr (T* p) : obj (p) { }

  template <class T2>
  explicit csPtr (csRef<T2> const& r) : obj((T2*)r) { if (obj) obj->IncRef(); }
# 68 "f:/source-wip/CS/include/csutil/ref.h"
  csPtr (const csPtr<T>& copy)
  {
    obj = copy.obj;



  }
};







template <class T>
class csRef
{
private:
  T* obj;

public:





  csRef () : obj (0) {}






  csRef (const csPtr<T>& newobj)
  {
    obj = newobj.obj;





    ((csPtr<T>&)newobj).obj = (T*)0xffffffff;
  }





  csRef (T* newobj) : obj (newobj)
  {
    if (obj)
      obj->IncRef ();
  }




  csRef (csRef const& other) : obj (other.obj)
  {
    if (obj)
      obj->IncRef ();
  }




  ~csRef ()
  {
    if (obj)
      obj->DecRef ();
  }
# 150 "f:/source-wip/CS/include/csutil/ref.h"
  csRef& operator = (const csPtr<T>& newobj)
  {
    T* oldobj = obj;

    obj = newobj.obj;





    ((csPtr<T>&)newobj).obj = (T*)0xffffffff;
    if (oldobj)
      oldobj->DecRef ();
    return *this;
  }
# 178 "f:/source-wip/CS/include/csutil/ref.h"
  csRef& operator = (T* newobj)
  {
    if (obj != newobj)
    {
      T* oldobj = obj;




      obj = newobj;
      if (newobj)
  newobj->IncRef ();
      if (oldobj)
  oldobj->DecRef ();
    }
    return *this;
  }
# 218 "f:/source-wip/CS/include/csutil/ref.h"
  void AttachNew (csPtr<T> newObj)
  {




    *this = newObj;
  }




  csRef& operator = (csRef const& other)
  {
    this->operator=(other.obj);
    return *this;
  }


  inline friend bool operator == (const csRef& r1, const csRef& r2)
  {
    return r1.obj == r2.obj;
  }

  inline friend bool operator != (const csRef& r1, const csRef& r2)
  {
    return r1.obj != r2.obj;
  }

  inline friend bool operator == (const csRef& r1, T* obj)
  {
    return r1.obj == obj;
  }

  inline friend bool operator != (const csRef& r1, T* obj)
  {
    return r1.obj != obj;
  }

  inline friend bool operator == (T* obj, const csRef& r1)
  {
    return r1.obj == obj;
  }

  inline friend bool operator != (T* obj, const csRef& r1)
  {
    return r1.obj != obj;
  }


  T* operator -> () const
  { return obj; }


  operator T* () const
  { return obj; }


  T& operator* () const
  { return *obj; }





  bool IsValid () const
  { return (obj != 0); }
};
# 32 "f:/source-wip/CS/include/csutil/scf.h" 2




typedef unsigned long scfInterfaceID;
# 72 "f:/source-wip/CS/include/csutil/scf.h"
struct iBase
{

  virtual void IncRef () = 0;

  virtual void DecRef () = 0;

  virtual int GetRefCount () = 0;

  virtual void *QueryInterface (scfInterfaceID iInterfaceID, int iVersion) = 0;




  static void* QueryInterfaceSafe (iBase* ibase, scfInterfaceID iInterfaceID,
        int iVersion)
  {
    if (ibase == 0) return 0;
    else return ibase->QueryInterface (iInterfaceID, iVersion);
  }
};
# 533 "f:/source-wip/CS/include/csutil/scf.h"
struct iFactory : public iBase
{

  virtual void *CreateInstance () = 0;

  virtual void TryUnload () = 0;

  virtual const char *QueryDescription () = 0;

  virtual const char *QueryDependencies () = 0;

  virtual const char *QueryClassID () = 0;
};



struct iDocument;
struct iStrVector;


typedef void* (*scfFactoryFunc)(iBase*);
# 611 "f:/source-wip/CS/include/csutil/scf.h"
extern void scfInitialize (iDocument* = 0);







static inline bool scfCompatibleVersion (int iVersion, int iItfVersion)
{
  return ((iVersion & 0xff000000) == (iItfVersion & 0xff000000))
      && ((iVersion & 0x00ffffff) <= (iItfVersion & 0x00ffffff));
}
# 635 "f:/source-wip/CS/include/csutil/scf.h"
struct iSCF : public iBase
{





  static iSCF* SCF;
# 660 "f:/source-wip/CS/include/csutil/scf.h"
  virtual void RegisterClasses (iDocument*) = 0;







  virtual bool ClassRegistered (const char *iClassID) = 0;
# 685 "f:/source-wip/CS/include/csutil/scf.h"
  virtual void *CreateInstance (const char *iClassID,
        const char *iInterface, int iVersion) = 0;






  virtual const char *GetClassDescription (const char *iClassID) = 0;






  virtual const char *GetClassDependencies (const char *iClassID) = 0;







  virtual void UnloadUnusedModules () = 0;
# 717 "f:/source-wip/CS/include/csutil/scf.h"
  virtual bool RegisterClass (const char *iClassID,
        const char *iLibraryName, const char *iFactoryClass,
        const char *Description, const char *Dependencies = 0) = 0;







  virtual bool RegisterClass (scfFactoryFunc, const char *iClassID,
        const char *Description, const char *Dependencies = 0) = 0;







  virtual bool UnregisterClass (const char *iClassID) = 0;






  virtual scfInterfaceID GetInterfaceID (const char *iInterface) = 0;







  virtual void Finish () = 0;
# 763 "f:/source-wip/CS/include/csutil/scf.h"
  virtual iStrVector* QueryClassList (char const* pattern) = 0;
};

const int iFactory_VERSION = ((0 << 24) | (0 << 16) | 1); inline static scfInterfaceID iFactory_scfGetID () { static scfInterfaceID ID = (scfInterfaceID)-1; if (ID == (scfInterfaceID)(-1)) ID = iSCF::SCF->GetInterfaceID ("iFactory"); return ID; };
const int iBase_VERSION = ((0 << 24) | (1 << 16) | 0); inline static scfInterfaceID iBase_scfGetID () { static scfInterfaceID ID = (scfInterfaceID)-1; if (ID == (scfInterfaceID)(-1)) ID = iSCF::SCF->GetInterfaceID ("iBase"); return ID; };
const int iSCF_VERSION = ((0 << 24) | (1 << 16) | 0); inline static scfInterfaceID iSCF_scfGetID () { static scfInterfaceID ID = (scfInterfaceID)-1; if (ID == (scfInterfaceID)(-1)) ID = iSCF::SCF->GetInterfaceID ("iSCF"); return ID; };
# 29 "f:/source-wip/CS/include/iutil/document.h" 2


struct iDocumentNode;
struct iDocumentAttribute;
struct iFile;
struct iDataBuffer;
struct iString;
struct iVFS;




enum csDocumentNodeType
{

  CS_NODE_DOCUMENT = 1,

  CS_NODE_ELEMENT,

  CS_NODE_COMMENT,

  CS_NODE_UNKNOWN,

  CS_NODE_TEXT,

  CS_NODE_DECLARATION
};
# 70 "f:/source-wip/CS/include/iutil/document.h"
const int iDocumentAttributeIterator_VERSION = ((0 << 24) | (0 << 16) | 1); inline static scfInterfaceID iDocumentAttributeIterator_scfGetID () { static scfInterfaceID ID = (scfInterfaceID)-1; if (ID == (scfInterfaceID)(-1)) ID = iSCF::SCF->GetInterfaceID ("iDocumentAttributeIterator"); return ID; };




struct iDocumentAttributeIterator : public iBase
{

  virtual bool HasNext () = 0;

  virtual csRef<iDocumentAttribute> Next () = 0;
};



const int iDocumentAttribute_VERSION = ((0 << 24) | (0 << 16) | 1); inline static scfInterfaceID iDocumentAttribute_scfGetID () { static scfInterfaceID ID = (scfInterfaceID)-1; if (ID == (scfInterfaceID)(-1)) ID = iSCF::SCF->GetInterfaceID ("iDocumentAttribute"); return ID; };




struct iDocumentAttribute : public iBase
{

  virtual const char* GetName () = 0;

  virtual const char* GetValue () = 0;

  virtual int GetValueAsInt () = 0;

  virtual float GetValueAsFloat () = 0;

  virtual void SetName (const char* name) = 0;

  virtual void SetValue (const char* value) = 0;

  virtual void SetValueAsInt (int v) = 0;

  virtual void SetValueAsFloat (float f) = 0;
};



const int iDocumentNodeIterator_VERSION = ((0 << 24) | (0 << 16) | 1); inline static scfInterfaceID iDocumentNodeIterator_scfGetID () { static scfInterfaceID ID = (scfInterfaceID)-1; if (ID == (scfInterfaceID)(-1)) ID = iSCF::SCF->GetInterfaceID ("iDocumentNodeIterator"); return ID; };




struct iDocumentNodeIterator : public iBase
{

  virtual bool HasNext () = 0;

  virtual csRef<iDocumentNode> Next () = 0;
};



const int iDocumentNode_VERSION = ((0 << 24) | (4 << 16) | 1); inline static scfInterfaceID iDocumentNode_scfGetID () { static scfInterfaceID ID = (scfInterfaceID)-1; if (ID == (scfInterfaceID)(-1)) ID = iSCF::SCF->GetInterfaceID ("iDocumentNode"); return ID; };




struct iDocumentNode : public iBase
{



  virtual csDocumentNodeType GetType () = 0;
# 146 "f:/source-wip/CS/include/iutil/document.h"
  virtual bool Equals (iDocumentNode* other) = 0;
# 160 "f:/source-wip/CS/include/iutil/document.h"
  virtual const char* GetValue () = 0;
# 173 "f:/source-wip/CS/include/iutil/document.h"
  virtual void SetValue (const char* value) = 0;

  virtual void SetValueAsInt (int value) = 0;

  virtual void SetValueAsFloat (float value) = 0;


  virtual csRef<iDocumentNode> GetParent () = 0;




  virtual csRef<iDocumentNodeIterator> GetNodes () = 0;

  virtual csRef<iDocumentNodeIterator> GetNodes (const char* value) = 0;

  virtual csRef<iDocumentNode> GetNode (const char* value) = 0;


  virtual void RemoveNode (const csRef<iDocumentNode>& child) = 0;

  virtual void RemoveNodes () = 0;







  virtual csRef<iDocumentNode> CreateNodeBefore (csDocumentNodeType type,
        iDocumentNode* before = 0) = 0;





  virtual const char* GetContentsValue () = 0;





  virtual int GetContentsValueAsInt () = 0;





  virtual float GetContentsValueAsFloat () = 0;




  virtual csRef<iDocumentAttributeIterator> GetAttributes () = 0;

  virtual csRef<iDocumentAttribute> GetAttribute (const char* name) = 0;

  virtual const char* GetAttributeValue (const char* name) = 0;

  virtual int GetAttributeValueAsInt (const char* name) = 0;

  virtual float GetAttributeValueAsFloat (const char* name) = 0;


  virtual void RemoveAttribute (const csRef<iDocumentAttribute>& attr) = 0;

  virtual void RemoveAttributes () = 0;


  virtual void SetAttribute (const char* name, const char* value) = 0;

  virtual void SetAttributeAsInt (const char* name, int value) = 0;

  virtual void SetAttributeAsFloat (const char* name, float value) = 0;
};



const int iDocument_VERSION = ((0 << 24) | (2 << 16) | 0); inline static scfInterfaceID iDocument_scfGetID () { static scfInterfaceID ID = (scfInterfaceID)-1; if (ID == (scfInterfaceID)(-1)) ID = iSCF::SCF->GetInterfaceID ("iDocument"); return ID; };




struct iDocument : public iBase
{

  virtual void Clear () = 0;


  virtual csRef<iDocumentNode> CreateRoot () = 0;


  virtual csRef<iDocumentNode> GetRoot () = 0;







  virtual const char* Parse (iFile* file) = 0;







  virtual const char* Parse (iDataBuffer* buf) = 0;







  virtual const char* Parse (iString* str) = 0;







  virtual const char* Parse (const char* buf) = 0;






  virtual const char* Write (iFile* file) = 0;






  virtual const char* Write (iString* str) = 0;






  virtual const char* Write (iVFS* vfs, const char* filename) = 0;
# 327 "f:/source-wip/CS/include/iutil/document.h"
  virtual int Changeable () = 0;
};



const int iDocumentSystem_VERSION = ((0 << 24) | (0 << 16) | 1); inline static scfInterfaceID iDocumentSystem_scfGetID () { static scfInterfaceID ID = (scfInterfaceID)-1; if (ID == (scfInterfaceID)(-1)) ID = iSCF::SCF->GetInterfaceID ("iDocumentSystem"); return ID; };




struct iDocumentSystem : public iBase
{

  virtual csRef<iDocument> CreateDocument () = 0;
};
# 23 "lightiter.cpp" 2
# 1 "f:/source-wip/CS/include/ivideo/rndbuf.h" 1
# 31 "f:/source-wip/CS/include/ivideo/rndbuf.h"
# 1 "f:/source-wip/CS/include/csutil/strset.h" 1
# 22 "f:/source-wip/CS/include/csutil/strset.h"
# 1 "f:/source-wip/CS/include/csutil/strhash.h" 1
# 22 "f:/source-wip/CS/include/csutil/strhash.h"
# 1 "f:/source-wip/CS/include/csutil/hashmap.h" 1
# 22 "f:/source-wip/CS/include/csutil/hashmap.h"
# 1 "f:/source-wip/CS/include/csutil/parray.h" 1
# 25 "f:/source-wip/CS/include/csutil/parray.h"
typedef int csArrayCompareFunction (void* item1, void* item2);
typedef int csArrayCompareKeyFunction (void* item, void* key);
# 37 "f:/source-wip/CS/include/csutil/parray.h"
template <class T>
class csPArray
{
private:
  int count, limit, threshold;
  T** root;

public:




  csPArray (int ilimit = 0, int ithreshold = 0)
  {
    count = 0;
    limit = (ilimit > 0 ? ilimit : 0);
    threshold = (ithreshold > 0 ? ithreshold : 16);
    if (limit != 0)
      root = (T**)malloc (limit * sizeof(T*));
    else
      root = 0;
  }




  void DeleteAll ()
  {
    if (root)
    {
      free (root);
      root = 0;
      limit = count = 0;
    }
  }




  ~csPArray ()
  {
    DeleteAll ();
  }







  void TransferTo (csPArray<T>& destination)
  {
    destination.DeleteAll ();
    destination.root = root;
    destination.count = count;
    destination.limit = limit;
    destination.threshold = threshold;
    root = 0;
    limit = count = 0;
  }


  void SetLength (int n)
  {
    count = n;

    if (n > limit || (limit > threshold && n < limit - threshold))
    {
      n = ((n + threshold - 1) / threshold ) * threshold;
      if (!n)
        DeleteAll ();
      else if (root == 0)
        root = (T**)malloc (n * sizeof(T*));
      else
        root = (T**)realloc (root, n * sizeof(T*));
      limit = n;
    }
  }


  int Length () const
  {
    return count;
  }


  int Limit () const
  {
    return limit;
  }


  T* Get (int n) const
  {
    ;
    return root[n];
  }


  const T*& operator [] (int n) const
  {
    ;
    return root[n];
  }


  T*& operator [] (int n)
  {
    ;
    if (n >= count)
      SetLength (n + 1);
    return root[n];
  }


  int Find (T* which) const
  {
    int i;
    for (i = 0 ; i < Length () ; i++)
      if (root[i] == which)
        return i;
    return -1;
  }


  int Push (T* what)
  {
    SetLength (count + 1);
    root [count - 1] = what;
    return (count - 1);
  }


  int PushSmart (T* what)
  {
    int n = Find (what);
    return (n == -1) ? Push (what) : n;
  }


  T* Pop ()
  {
    T* ret = root [count - 1];
    SetLength (count - 1);
    return ret;
  }


  T* Top () const
  {
    return root [count - 1];
  }


  bool Delete (int n)
  {
    if (n >= 0 && n < count)
    {
      const int ncount = count - 1;
      const int nmove = ncount - n;
      if (nmove > 0)
      {
        memmove (&root [n], &root [n + 1], nmove * sizeof (T*));
      }
      SetLength (ncount);
      return true;
    }
    else
      return false;
  }


  bool Delete (T* item)
  {
    int n = Find (item);
    if (n == -1) return false;
    else return Delete (n);
  }


  bool Insert (int n, T* item)
  {
    if (n <= count)
    {
      SetLength (count + 1);
      const int nmove = (count - n - 1);
      if (nmove > 0)
      {
        memmove (&root [n + 1], &root [n], nmove * sizeof (T*));
      }
      root [n] = item;
      return true;
    }
    else
     return false;
  }




  int FindSortedKey (void* key, csArrayCompareKeyFunction* comparekey) const
  {
    int l = 0, r = Length () - 1;
    while (l <= r)
    {
      int m = (l + r) / 2;
      int cmp = comparekey (root [m], key);

      if (cmp == 0)
        return m;
      else if (cmp < 0)
        l = m + 1;
      else
        r = m - 1;
    }
    return -1;
  }





  int InsertSorted (T* item, csArrayCompareFunction* compare)
  {
    int m = 0, l = 0, r = Length () - 1;
    while (l <= r)
    {
      m = (l + r) / 2;
      int cmp = compare (root [m], item);

      if (cmp == 0)
      {
        Insert (++m, item);
        return m;
      }
      else if (cmp < 0)
        l = m + 1;
      else
        r = m - 1;
    }
    if (r == m)
      m++;
    Insert (m, item);
    return m;
  }
};
# 291 "f:/source-wip/CS/include/csutil/parray.h"
template <class T>
class csPDelArray
{
private:
  int count, limit, threshold;
  T** root;

public:




  csPDelArray (int ilimit = 0, int ithreshold = 0)
  {
    count = 0;
    limit = (ilimit > 0 ? ilimit : 0);
    threshold = (ithreshold > 0 ? ithreshold : 16);
    if (limit != 0)
      root = (T**)calloc (limit, sizeof(T*));
    else
      root = 0;
  }




  void DeleteAll ()
  {
    if (root)
    {
      int i;
      for (i = 0 ; i < limit ; i++)
        delete root[i];
      free (root);
      root = 0;
      limit = count = 0;
    }
  }




  ~csPDelArray ()
  {
    DeleteAll ();
  }







  void TransferTo (csPDelArray<T>& destination)
  {
    destination.DeleteAll ();
    destination.root = root;
    destination.count = count;
    destination.limit = limit;
    destination.threshold = threshold;
    root = 0;
    limit = count = 0;
  }


  void SetLength (int n)
  {

    int i;
    for (i = n ; i < count ; i++) { delete root[i]; root[i] = 0; }

    int old_count = count;
    count = n;

    if (n > limit || (limit > threshold && n < limit - threshold))
    {
      n = ((n + threshold - 1) / threshold ) * threshold;
      if (!n)
      {
        DeleteAll ();
      }
      else if (root == 0)
      {
        root = (T**)calloc (n, sizeof(T*));
      }
      else
      {
        T** newroot = (T**)calloc (n, sizeof(T*));
        memcpy (newroot, root, old_count * sizeof (T*));
        free (root);
        root = newroot;
      }
      limit = n;
    }
  }


  int Length () const
  {
    return count;
  }


  int Limit () const
  {
    return limit;
  }


  T* Get (int n) const
  {
    ;
    return root[n];
  }


  T* operator [] (int n) const
  {
    ;
    return root[n];
  }


  void Put (int n, T* ptr)
  {
    ;
    if (n >= count)
      SetLength (n + 1);
    delete root[n];
    root[n] = ptr;
  }


  int Find (T* which) const
  {
    int i;
    for (i = 0 ; i < Length () ; i++)
      if (root[i] == which)
        return i;
    return -1;
  }


  int Push (T* what)
  {
    SetLength (count + 1);
    root [count - 1] = what;
    return (count - 1);
  }


  int PushSmart (T* what)
  {
    int n = Find (what);
    return (n == -1) ? Push (what) : n;
  }





  T* Pop ()
  {
    T* ret = root [count - 1];
    root [count-1] = 0;
    SetLength (count - 1);
    return ret;
  }


  T* Top () const
  {
    return root [count - 1];
  }






  T* GetAndClear (int n)
  {
    ;
    T* ret = root[n];
    root[n] = 0;
    return ret;
  }






  T* Extract (int n)
  {
    T* rc = 0;
    if (n >= 0 && n < count)
    {
      rc = root[n]; root[n] = 0;
      const int ncount = count - 1;
      const int nmove = ncount - n;
      if (nmove > 0)
      {
        memmove (&root [n], &root [n + 1], nmove * sizeof (T*));
        root[count-1] = 0;
      }
      SetLength (ncount);
    }
    return rc;
  }


  bool Delete (int n)
  {
    T* p = Extract (n);
    if (p)
    {
      delete p;
      return true;
    }
    else
    {
      return false;
    }
  }


  bool Delete (T* item)
  {
    int n = Find (item);
    if (n == -1) return false;
    else return Delete (n);
  }


  bool Insert (int n, T* item)
  {
    if (n <= count)
    {
      SetLength (count + 1);
      const int nmove = (count - n - 1);
      if (nmove > 0)
      {
        memmove (&root [n + 1], &root [n], nmove * sizeof (T*));
      }
      root [n] = item;
      return true;
    }
    else
     return false;
  }




  int FindSortedKey (void* key, csArrayCompareKeyFunction* comparekey) const
  {
    int l = 0, r = Length () - 1;
    while (l <= r)
    {
      int m = (l + r) / 2;
      int cmp = comparekey (root [m], key);

      if (cmp == 0)
        return m;
      else if (cmp < 0)
        l = m + 1;
      else
        r = m - 1;
    }
    return -1;
  }





  int InsertSorted (T* item, csArrayCompareFunction* compare)
  {
    int m = 0, l = 0, r = Length () - 1;
    while (l <= r)
    {
      m = (l + r) / 2;
      int cmp = compare (root [m], item);

      if (cmp == 0)
      {
        Insert (++m, item);
        return m;
      }
      else if (cmp < 0)
        l = m + 1;
      else
        r = m - 1;
    }
    if (r == m)
      m++;
    Insert (m, item);
    return m;
  }
};
# 23 "f:/source-wip/CS/include/csutil/hashmap.h" 2
# 1 "f:/source-wip/CS/include/csutil/array.h" 1
# 26 "f:/source-wip/CS/include/csutil/array.h"
# 1 "d:/util/mingw/include/c++/3.2.3/new" 1 3
# 41 "d:/util/mingw/include/c++/3.2.3/new" 3
# 1 "d:/util/mingw/include/c++/3.2.3/cstddef" 1 3
# 47 "d:/util/mingw/include/c++/3.2.3/cstddef" 3

# 1 "d:/util/mingw/include/stddef.h" 1 3





# 1 "d:/util/mingw/include/stddef.h" 1 3





# 1 "d:/util/mingw/lib/gcc-lib/mingw32/3.2.3/include/stddef.h" 1 3
# 7 "d:/util/mingw/include/stddef.h" 2 3
# 7 "d:/util/mingw/include/stddef.h" 2 3
# 49 "d:/util/mingw/include/c++/3.2.3/cstddef" 2 3

namespace std
{
  using ::ptrdiff_t;
  using ::size_t;
}
# 42 "d:/util/mingw/include/c++/3.2.3/new" 2 3
# 1 "d:/util/mingw/include/c++/3.2.3/exception" 1 3
# 40 "d:/util/mingw/include/c++/3.2.3/exception" 3
extern "C++" {

namespace std
{






  class exception
  {
  public:
    exception() throw() { }
    virtual ~exception() throw();


    virtual const char* what() const throw();
  };



  class bad_exception : public exception
  {
  public:
    bad_exception() throw() { }


    virtual ~bad_exception() throw();
  };


  typedef void (*terminate_handler) ();

  typedef void (*unexpected_handler) ();


  terminate_handler set_terminate(terminate_handler) throw();


  void terminate() ;


  unexpected_handler set_unexpected(unexpected_handler) throw();


  void unexpected() ;
# 98 "d:/util/mingw/include/c++/3.2.3/exception" 3
  bool uncaught_exception() throw();
}

namespace __gnu_cxx
{
# 111 "d:/util/mingw/include/c++/3.2.3/exception" 3
  void __verbose_terminate_handler ();
}

}
# 43 "d:/util/mingw/include/c++/3.2.3/new" 2 3

extern "C++" {

namespace std
{


  class bad_alloc : public exception
  {
  public:
    bad_alloc() throw() { }


    virtual ~bad_alloc() throw();
  };

  struct nothrow_t { };
  extern const nothrow_t nothrow;


  typedef void (*new_handler)();

  new_handler set_new_handler(new_handler) throw();
}
# 79 "d:/util/mingw/include/c++/3.2.3/new" 3
void* operator new(std::size_t) throw (std::bad_alloc);
void* operator new[](std::size_t) throw (std::bad_alloc);
void operator delete(void*) throw();
void operator delete[](void*) throw();
void* operator new(std::size_t, const std::nothrow_t&) throw();
void* operator new[](std::size_t, const std::nothrow_t&) throw();
void operator delete(void*, const std::nothrow_t&) throw();
void operator delete[](void*, const std::nothrow_t&) throw();


inline void* operator new(std::size_t, void* __p) throw() { return __p; }
inline void* operator new[](std::size_t, void* __p) throw() { return __p; }


inline void operator delete (void*, void*) throw() { };
inline void operator delete[](void*, void*) throw() { };

}
# 27 "f:/source-wip/CS/include/csutil/array.h" 2
# 35 "f:/source-wip/CS/include/csutil/array.h"
template <class T>
class csArray
{
private:
  int count;
  int capacity;
  int threshold;
  T* root;

  void ConstructElement (int n, T const& src)
  {
    new (static_cast<void*>(root + n)) T(src);
  }

  void DestroyElement (int n)
  {
    (root + n)->T::~T();
  }


  void AdjustCapacity (int n)
  {
    if (n > capacity || (capacity > threshold && n < capacity - threshold))
    {
      n = ((n + threshold - 1) / threshold ) * threshold;
      if (root == 0)
        root = (T*)malloc (n * sizeof(T));
      else
        root = (T*)realloc (root, n * sizeof(T));
      capacity = n;
    }
  }




  void SetLengthUnsafe (int n)
  {
    if (n > capacity)
      AdjustCapacity (n);
    count = n;
  }

public:




  csArray (int icapacity = 0, int ithreshold = 0)
  {
    count = 0;
    capacity = (icapacity > 0 ? icapacity : 0);
    threshold = (ithreshold > 0 ? ithreshold : 16);
    if (capacity != 0)
      root = (T*)malloc (capacity * sizeof(T));
    else
      root = 0;
  }


  csArray (const csArray& other)
  {
    count = 0;
    capacity = 0;
    threshold = other.threshold;
    root = 0;
    for (int i=0;i<other.Length();i++)
      Push (other[i]);
  }







  void TransferTo (csArray<T>& destination)
  {
    destination.DeleteAll ();
    destination.root = root;
    destination.count = count;
    destination.capacity = capacity;
    destination.threshold = threshold;
    root = 0;
    capacity = count = 0;
  }

  csArray<T>& operator = (const csArray& other)
  {
    if (&other == this)
      return *this;


    DeleteAll ();

    for (int i=0;i<other.Length();i++)
      Push(other[i]);
    return *this;
  }


  void DeleteAll ()
  {
    for (int i = 0; i < count; i++)
      DestroyElement (i);
    if (root != 0)
    {
      free (root);
      root = 0;
      capacity = 0;
      count = 0;
    }
  }






  void Truncate (int n)
  {
    ;
    ;
    if (n < count)
    {
      for (int i = n; i < count; i++)
        DestroyElement(i);
      SetLengthUnsafe(n);
    }
  }
# 173 "f:/source-wip/CS/include/csutil/array.h"
  void SetLength (int n, T const& what)
  {
    if (n <= count)
    {
      Truncate (n);
    }
    else
    {
      int old_len = Length ();
      SetLengthUnsafe (n);
      for (int i = old_len ; i < n ; i++)
        ConstructElement (i, what);
    }
  }






  void Empty()
  { Truncate(0); }



  ~csArray ()
  {
    DeleteAll ();
  }


  int Length () const
  {
    return count;
  }


  int Capacity () const
  {
    return capacity;
  }







  void SetCapacity (int n)
  {
    if (n > Length ())
      AdjustCapacity (n);
  }






  void ShrinkBestFit ()
  {
    if (count == 0)
    {
      DeleteAll ();
    }
    else if (count != capacity)
    {
      capacity = count;
      root = (T*)realloc (root, capacity * sizeof(T));
    }
  }


  T const& Get (int n) const
  {
    ;
    return root[n];
  }


  T& Get (int n)
  {
    ;
    return root[n];
  }


  T const& operator [] (int n) const
  {
    return Get(n);
  }


  T& operator [] (int n)
  {
    return Get(n);
  }


  int Find (T const& which) const
  {
    for (int i = 0, n = Length(); i < n; i++)
      if (root[i] == which)
        return i;
    return -1;
  }


  int Push (T const& what)
  {
    SetLengthUnsafe (count + 1);
    ConstructElement (count - 1, what);
    return (count - 1);
  }





  int PushSmart (T const& what)
  {
    int const n = Find (what);
    return (n < 0) ? Push (what) : n;
  }


  T Pop ()
  {
    ;
    T ret(root [count - 1]);
    DestroyElement (count - 1);
    SetLengthUnsafe (count - 1);
    return ret;
  }


  T const& Top () const
  {
    ;
    return root [count - 1];
  }


  bool DeleteIndex (int n)
  {
    if (n >= 0 && n < count)
    {
      int const ncount = count - 1;
      int const nmove = ncount - n;
      DestroyElement (n);
      if (nmove > 0)
        memmove (root + n, root + n + 1, nmove * sizeof(T));
      SetLengthUnsafe (ncount);
      return true;
    }
    else
      return false;
  }





  void DeleteRange (int start, int end)
  {
    if (start >= count) return;
    if (end < 0) return;
    if (start < 0) start = 0;
    if (end >= count) end = count-1;
    int i;
    for (i = start ; i < end ; i++)
      DestroyElement (i);

    int const range_size = end-start+1;
    int const ncount = count - range_size;
    int const nmove = count - end - 1;
    if (nmove > 0)
      memmove (root + start, root + start + range_size, nmove * sizeof(T));
    SetLengthUnsafe (ncount);
  }


  bool Delete (T const& item)
  {
    int const n = Find (item);
    if (n >= 0)
      return DeleteIndex (n);
    return false;
  }


  bool Insert (int n, T const& item)
  {
    if (n <= count)
    {
      SetLengthUnsafe (count + 1);
      int const nmove = (count - n - 1);
      if (nmove > 0)
        memmove (root + n + 1, root + n, nmove * sizeof(T));
      ConstructElement (n, item);
      return true;
    }
    else
      return false;
  }
};
# 24 "f:/source-wip/CS/include/csutil/hashmap.h" 2

class csHashMapReversible;
class csHashIteratorReversible;

class csHashMap;


typedef uint32 csHashKey;

typedef void* csHashObject;


csHashKey csHashCompute(char const*);

csHashKey csHashCompute(char const*, int length);




struct csHashElement
{
  csHashKey key;
  csHashObject object;
};


typedef csArray<csHashElement> csHashBucket;

typedef csArray<csHashBucket> csHashBucketVector;
# 62 "f:/source-wip/CS/include/csutil/hashmap.h"
class csGlobalHashIterator
{
  friend class csHashMap;
  friend class csGlobalHashIteratorReversible;

private:

  csHashBucket* bucket;

  int element_index;

  uint32 bucket_index;

  uint32 bucket_len;

  uint32 nbuckets;

  csHashMap* hash;

private:

  void GotoNextElement ();

public:





  csGlobalHashIterator (csHashMap* hash);


  bool HasNext ();

  csHashObject Next ();




  void DeleteNext ();
};
# 112 "f:/source-wip/CS/include/csutil/hashmap.h"
class csHashIterator
{
  friend class csHashMap;
  friend class csHashIteratorReversible;

private:

  csHashBucket* bucket;

  int element_index;

  int current_index;

  uint32 bucket_index;

  csHashKey key;

  csHashMap* hash;

private:

  void GotoNextSameKey ();

public:





  csHashIterator (csHashMap* hash, csHashKey Key);


  bool HasNext ();

  csHashObject Next ();




  void DeleteNext ();
};







class csHashMap
{
  friend class csHashIterator;
  friend class csGlobalHashIterator;
  friend class csHashMapReversible;

private:

  csHashBucketVector Buckets;

  uint32 NumBuckets;

  int hash_elements;


  void ChangeBuckets (uint32 newsize);




  void PutInternal (uint32 idx, csHashKey key, csHashObject object);



  static uint32 FindLargerPrime (uint32 num);

public:
  static uint32 prime_table[];
# 199 "f:/source-wip/CS/include/csutil/hashmap.h"
  csHashMap (uint32 size = 53);





  virtual ~csHashMap ();






  void Put (csHashKey key, csHashObject object);
# 221 "f:/source-wip/CS/include/csutil/hashmap.h"
  csHashObject Get (csHashKey key) const;







  void Delete (csHashKey key, csHashObject object);




  void DeleteAll (csHashKey key);




  void DeleteAll ();




  void DumpStats ();
};






class csHashSet
{
private:
  csHashMap map;

public:




  csHashSet (uint32 size = 211);





  void Add (csHashObject object);







  void AddNoTest (csHashObject object);




  bool In (csHashObject object);




  void DeleteAll ();





  void Delete (csHashObject object);


  inline csHashMap *GetHashMap () {return &map;}
};
# 23 "f:/source-wip/CS/include/csutil/strhash.h" 2
# 32 "f:/source-wip/CS/include/csutil/strhash.h"
typedef uint32 csStringID;

csStringID const csInvalidStringID = (csStringID) ~0;

class csStringHash;






class csStringHashIterator
{
  friend class csStringHash;

private:
  csGlobalHashIterator* hashIt;

public:






  csStringHashIterator (csStringHash* hash);


  bool HasNext ();

  csStringID Next ();
};





class csStringHash
{
private:
  friend class csStringHashIterator;

  csHashMap Registry;

public:

  csStringHash (uint32 size = 211);

  ~csStringHash ();





  const char* Register (const char *s, csStringID id);





  csStringID Request (const char *s);





  const char* Request (csStringID id);




  void Clear ();
};
# 23 "f:/source-wip/CS/include/csutil/strset.h" 2




class csStringSet;






class csStringSetIterator
{
  friend class csStringSet;

private:
  csStringHashIterator* hashIt;

public:






  csStringSetIterator (csStringSet* hash);


  bool HasNext ();

  csStringID Next ();
};







class csStringSet
{
  friend class csStringSetIterator;

  csStringHash Registry;
  csHashMap reverse_mapping;
  csStringID IDCounter;
public:

  csStringSet (uint32 size = 211);

  ~csStringSet ();





  csStringID Request (const char *s);





  const char* Request (csStringID id);





  void Clear ();
};
# 32 "f:/source-wip/CS/include/ivideo/rndbuf.h" 2

# 1 "f:/source-wip/CS/include/ivideo/material.h" 1
# 31 "f:/source-wip/CS/include/ivideo/material.h"
# 1 "f:/source-wip/CS/include/csutil/csvector.h" 1
# 41 "f:/source-wip/CS/include/csutil/csvector.h"
class csBasicVector
{
protected:
  int count,limit,threshold;
  void** root;

public:




  csBasicVector (int ilimit = 0, int ithreshold = 0);


  virtual ~csBasicVector();


  inline void* & operator [] (int n);

  inline void* & operator [] (int n) const;

  inline void* & Get (int n) const;


  void SetLength (int n);


  inline int Length () const;

  inline int Limit () const;


  bool Delete (int n);

  bool DeleteChunk (int n,int size);

  bool Delete (void* Item);


  inline void Exchange (int n1, int n2);

  int Find (void* which) const;


  inline int Push (void* what);

  inline int PushSmart (void* what);

  inline void* Pop ();

  inline void* Top () const;


  bool Insert (int n, void* Item);

  bool InsertChunk (int n, int size, void** Item);
};
# 114 "f:/source-wip/CS/include/csutil/csvector.h"
class csVector : public csBasicVector
{
public:




  csVector (int ilimit = 8, int ithreshold = 16)
    : csBasicVector(ilimit, ithreshold) {}


  virtual ~csVector () {}


  int FindKey (const void* Key, int Mode = 0) const;

  int FindSortedKey (const void* Key, int Mode = 0) const;

  void QuickSort (int Left, int Right, int Mode = 0);

  void QuickSort (int Mode = 0);


  bool Delete (int n, bool FreeIt = true);

  bool Delete (void* Item, bool FreeIt = true);

  bool Replace (int n, void* what, bool FreePrevious = true);

  void DeleteAll (bool FreeThem = true);


  int InsertSorted (void* Item, int *oEqual = 0, int Mode = 0);

  virtual bool FreeItem (void* Item);

  virtual int Compare (void* Item1, void* Item2, int Mode) const;

  virtual int CompareKey (void* Item, const void* Key, int Mode) const;
};

inline void* & csBasicVector::operator [] (int n)
{
  ;
  if (n >= count)
    SetLength (n + 1);
  return (root [n]);
}

inline void* & csBasicVector::operator [] (int n) const
{
  ;
  ;
  return (root [n]);
}

inline void* & csBasicVector::Get (int n) const
{
  ;
  ;
  return (root [n]);
}

inline int csBasicVector::Limit () const
{
  return (limit);
}

inline int csBasicVector::Length () const
{
  return (count);
}

inline bool csBasicVector::Delete (void* Item)
{
  int n = Find (Item);
  if (n == -1) return false;
  else return Delete (n);
}

inline int csBasicVector::Push (void* what)
{
  SetLength (count + 1);
  root [count - 1] = what;
  return (count - 1);
}

inline int csBasicVector::PushSmart (void* what)
{
  int n = Find (what);
  return (n == -1) ? Push (what) : n;
}

inline void* csBasicVector::Pop ()
{
  if (count<=0)
    return 0;

  void* ret = root [count - 1];
  SetLength (count - 1);
  return (ret);
}

inline void* csBasicVector::Top () const
{
  return root [count - 1];
}

inline void csBasicVector::Exchange (int n1, int n2)
{
  void* tmp = root [n1];
  root [n1] = root [n2];
  root [n2] = tmp;
}

inline bool csVector::Delete (void* Item, bool FreeIt)
{
  int n = Find (Item);
  if (n == -1) return false;
  else return Delete (n, FreeIt);
}

inline void csVector::QuickSort (int Mode)
{
  if (count > 0)
    QuickSort (0, count - 1, Mode);
}
# 32 "f:/source-wip/CS/include/ivideo/material.h" 2
# 40 "f:/source-wip/CS/include/ivideo/material.h"
struct iEffectDefinition;
struct iTextureHandle;
struct csRGBpixel;
struct csRGBcolor;
struct iShader;
struct iShaderWrapper;
struct iShaderVariable;
class csSymbolTable;





struct csTextureLayer
{

  csRef<iTextureHandle> txt_handle;

  uint mode;

  float uscale, vscale;

  float ushift, vshift;
};

const int iMaterial_VERSION = ((0 << 24) | (0 << 16) | 6); inline static scfInterfaceID iMaterial_scfGetID () { static scfInterfaceID ID = (scfInterfaceID)-1; if (ID == (scfInterfaceID)(-1)) ID = iSCF::SCF->GetInterfaceID ("iMaterial"); return ID; };
# 74 "f:/source-wip/CS/include/ivideo/material.h"
struct iMaterial : public iBase
{
# 100 "f:/source-wip/CS/include/ivideo/material.h"
  virtual void SetEffect (iEffectDefinition *ed) = 0;




  virtual iEffectDefinition *GetEffect () = 0;




  virtual iTextureHandle *GetTexture () = 0;





  virtual int GetTextureLayerCount () = 0;




  virtual csTextureLayer* GetTextureLayer (int idx) = 0;





  virtual void GetFlatColor (csRGBpixel &oColor, bool useTextureMean=1) = 0;



  virtual void SetFlatColor (const csRGBcolor& col) = 0;




  virtual void GetReflection (
    float &oDiffuse, float &oAmbient, float &oReflection) = 0;



  virtual void SetReflection (float oDiffuse, float oAmbient,
    float oReflection) = 0;
};

const int iMaterialHandle_VERSION = ((0 << 24) | (0 << 16) | 2); inline static scfInterfaceID iMaterialHandle_scfGetID () { static scfInterfaceID ID = (scfInterfaceID)-1; if (ID == (scfInterfaceID)(-1)) ID = iSCF::SCF->GetInterfaceID ("iMaterialHandle"); return ID; };





struct iMaterialHandle : public iBase
{
# 163 "f:/source-wip/CS/include/ivideo/material.h"
  virtual iTextureHandle *GetTexture () = 0;





  virtual void GetFlatColor (csRGBpixel &oColor) = 0;




  virtual void GetReflection (float &oDiffuse, float &oAmbient,
        float &oReflection) = 0;







  virtual void Prepare () = 0;
};
# 34 "f:/source-wip/CS/include/ivideo/rndbuf.h" 2
# 1 "f:/source-wip/CS/include/ivideo/render3d.h" 1
# 33 "f:/source-wip/CS/include/ivideo/render3d.h"
class csRect;
class csReversibleTransform;
class csStringSet;
class csPlane3;
class csVector3;

class csRenderMesh;

struct iClipper2D;
struct iGraphics2D;
struct iMaterialHandle;
struct iTextureManager;
struct iTextureHandle;
struct iRenderBuffer;
struct iRenderBufferManager;
struct iLightingManager;
# 105 "f:/source-wip/CS/include/ivideo/render3d.h"
enum csZBufMode
{


  CS_ZBUF_NONE = 0x00000000,

  CS_ZBUF_FILL = 0x00000001,

  CS_ZBUF_TEST = 0x00000002,

  CS_ZBUF_USE = 0x00000003,

  CS_ZBUF_FILLONLY = 0x00000004,

  CS_ZBUF_EQUAL = 0x00000005,

  CS_ZBUF_INVERT = 0x00000006
};


enum csVertexAttrib
{
  CS_VATTRIB_0 = 0,
  CS_VATTRIB_1 = 1,
  CS_VATTRIB_2 = 2,
  CS_VATTRIB_3 = 3,
  CS_VATTRIB_4 = 4,
  CS_VATTRIB_5 = 5,
  CS_VATTRIB_6 = 6,
  CS_VATTRIB_7 = 7,
  CS_VATTRIB_8 = 8,
  CS_VATTRIB_9 = 9,
  CS_VATTRIB_10 = 10,
  CS_VATTRIB_11 = 11,
  CS_VATTRIB_12 = 12,
  CS_VATTRIB_13 = 13,
  CS_VATTRIB_14 = 14,
  CS_VATTRIB_15 = 15,
  CS_VATTRIB_POSITION = 0,
  CS_VATTRIB_WEIGHT = 1,
  CS_VATTRIB_NORMAL = 2,
  CS_VATTRIB_COLOR = 3,
  CS_VATTRIB_PRIMARY_COLOR = 3,
  CS_VATTRIB_SECONDARY_COLOR = 4,
  CS_VATTRIB_FOGCOORD = 5,
  CS_VATTRIB_TEXCOORD = 8,
  CS_VATTRIB_TEXCOORD0 = 8,
  CS_VATTRIB_TEXCOORD1 = 9,
  CS_VATTRIB_TEXCOORD2 = 10,
  CS_VATTRIB_TEXCOORD3 = 11
};
# 262 "f:/source-wip/CS/include/ivideo/render3d.h"
enum R3D_RENDERSTATEOPTION
{
};
# 283 "f:/source-wip/CS/include/ivideo/render3d.h"
class csRender3dCaps
{
};

const int iRender3D_VERSION = ((0 << 24) | (0 << 16) | 1); inline static scfInterfaceID iRender3D_scfGetID () { static scfInterfaceID ID = (scfInterfaceID)-1; if (ID == (scfInterfaceID)(-1)) ID = iSCF::SCF->GetInterfaceID ("iGraphics3D"); return ID; };






struct iGraphics3D : public iBase
{

  virtual bool Open () = 0;


  virtual void Close () = 0;





  virtual iGraphics2D* GetDriver2D () = 0;


  virtual iTextureManager* GetTextureManager () = 0;





  virtual iRenderBufferManager* GetBufferManager () = 0;


  virtual iLightingManager* GetLightingManager () = 0;


  virtual bool ActivateBuffer (csVertexAttrib attrib, iRenderBuffer* buffer) = 0;


  virtual void DeactivateBuffer (csVertexAttrib attrib) = 0;


  virtual bool ActivateTexture (iTextureHandle *txthandle, int unit = 0) = 0;


  virtual bool ActivateTexture (iMaterialHandle *matwrapper, int layer, int unit = 0) = 0;


  virtual void DeactivateTexture (int unit = 0) = 0;


  virtual void SetDimensions (int width, int height) = 0;

  virtual int GetWidth () const = 0;

  virtual int GetHeight () const = 0;


  virtual const csRender3dCaps* GetCaps () const = 0;


  virtual void SetPerspectiveCenter (int x, int y) = 0;


  virtual void GetPerspectiveCenter (int& x, int& y) const = 0;


  virtual void SetPerspectiveAspect (float aspect) = 0;


  virtual float GetPerspectiveAspect () const = 0;


  virtual void SetObjectToCamera (csReversibleTransform* wvmatrix) = 0;
  virtual csReversibleTransform* GetWVMatrix () = 0;
# 374 "f:/source-wip/CS/include/ivideo/render3d.h"
  virtual void SetRenderTarget (iTextureHandle* handle,
    bool persistent = false) = 0;


  virtual iTextureHandle* GetRenderTarget () = 0;


  virtual bool BeginDraw (int drawflags) = 0;


  virtual void FinishDraw () = 0;


  virtual void Print (csRect* area) = 0;


  virtual void DrawMesh (csRenderMesh* mymesh) = 0;


  virtual void SetWriteMask (bool red, bool green, bool blue, bool alpha) = 0;


  virtual void GetWriteMask (bool &red, bool &green, bool &blue, bool &alpha) const = 0;


  virtual void SetZMode (csZBufMode mode) = 0;


  virtual void EnableZOffset () = 0;


  virtual void DisableZOffset () = 0;


  virtual void SetShadowState (int state) = 0;


  virtual void DrawLine(const csVector3 & v1,
  const csVector3 & v2, float fov, int color) = 0;






  virtual void SetClipper (iClipper2D* clipper, int cliptype) = 0;




  virtual iClipper2D* GetClipper () = 0;




  virtual int GetClipType () const = 0;


  virtual void SetNearPlane (const csPlane3& pl) = 0;


  virtual void ResetNearPlane () = 0;


  virtual const csPlane3& GetNearPlane () const = 0;


  virtual bool HasNearPlane () const = 0;


  virtual int GetMaxLights () const = 0;


  virtual void SetLightParameter (int i, int param, csVector3 value) = 0;


  virtual void EnableLight (int i) = 0;


  virtual void DisableLight (int i) = 0;


  virtual void EnablePVL () = 0;


  virtual void DisablePVL () = 0;


  virtual bool SetRenderState (R3D_RENDERSTATEOPTION op, long val) = 0;


  virtual long GetRenderState (R3D_RENDERSTATEOPTION op) const = 0;
};
# 35 "f:/source-wip/CS/include/ivideo/rndbuf.h" 2

class csVector3;
class csVector2;
class csColor;

struct iLightingInfo;
struct iTextureHandle;
struct iMaterialWrapper;






enum csRenderBufferType
{
  CS_BUF_DYNAMIC,
  CS_BUF_STATIC,
  CS_BUF_INDEX
};


enum csRenderBufferComponentType
{
  CS_BUFCOMP_BYTE,
  CS_BUFCOMP_UNSIGNED_BYTE,
  CS_BUFCOMP_SHORT,
  CS_BUFCOMP_UNSIGNED_SHORT,
  CS_BUFCOMP_INT,
  CS_BUFCOMP_UNSIGNED_INT,
  CS_BUFCOMP_FLOAT,
  CS_BUFCOMP_DOUBLE
};






enum csRenderBufferLockType
{
  CS_BUF_LOCK_NOLOCK,
  CS_BUF_LOCK_NORMAL,
  CS_BUF_LOCK_RENDER
};

const int iRenderBuffer_VERSION = ((0 << 24) | (0 << 16) | 2); inline static scfInterfaceID iRenderBuffer_scfGetID () { static scfInterfaceID ID = (scfInterfaceID)-1; if (ID == (scfInterfaceID)(-1)) ID = iSCF::SCF->GetInterfaceID ("iRenderBuffer"); return ID; };





struct iRenderBuffer : public iBase
{




  virtual void* Lock(csRenderBufferLockType lockType) = 0;


  virtual void Release() = 0;


  virtual int GetComponentCount () const = 0;


  virtual csRenderBufferComponentType GetComponentType () const = 0;


  virtual bool IsDiscarded() const = 0;


  virtual void CanDiscard(bool value) = 0;


  virtual csRenderBufferType GetBufferType() const = 0;


  virtual int GetSize() const = 0;

};

const int iRenderBufferManager_VERSION = ((0 << 24) | (0 << 16) | 2); inline static scfInterfaceID iRenderBufferManager_scfGetID () { static scfInterfaceID ID = (scfInterfaceID)-1; if (ID == (scfInterfaceID)(-1)) ID = iSCF::SCF->GetInterfaceID ("iRenderBufferManager"); return ID; };

struct iRenderBufferManager : public iBase
{

  virtual csPtr<iRenderBuffer> CreateBuffer(int buffersize,
    csRenderBufferType type,
    csRenderBufferComponentType comptype,
    int compcount) = 0;

};

const int iStreamSource_VERSION = ((0 << 24) | (0 << 16) | 1); inline static scfInterfaceID iStreamSource_scfGetID () { static scfInterfaceID ID = (scfInterfaceID)-1; if (ID == (scfInterfaceID)(-1)) ID = iSCF::SCF->GetInterfaceID ("iStreamSource"); return ID; };

struct iStreamSource : public iBase
{

  virtual iRenderBuffer* GetBuffer (csStringID name) = 0;
};


enum csRenderMeshType
{
  CS_MESHTYPE_TRIANGLES,
  CS_MESHTYPE_QUADS,
  CS_MESHTYPE_TRIANGLESTRIP,
  CS_MESHTYPE_TRIANGLEFAN,
  CS_MESHTYPE_POINTS,
  CS_MESHTYPE_LINES,
  CS_MESHTYPE_LINESTRIP
};

class csRenderMesh
{
public:

public:

  csRenderMesh ()
  {
    mixmode = 0x00000000;
  }

  virtual ~csRenderMesh () {}
# 172 "f:/source-wip/CS/include/ivideo/rndbuf.h"
  csZBufMode z_buf_mode;


  uint mixmode;


  int clip_portal;


  int clip_plane;


  int clip_z_plane;


  bool do_mirror;


  csRenderMeshType meshtype;


  unsigned int indexstart;


  unsigned int indexend;


  iStreamSource* streamsource;



  iMaterialWrapper* material;


  csReversibleTransform *transform;
};
# 24 "lightiter.cpp" 2
# 1 "f:/source-wip/CS/include/iengine/camera.h" 1
# 30 "f:/source-wip/CS/include/iengine/camera.h"
# 1 "f:/source-wip/CS/include/csgeom/transfrm.h" 1
# 29 "f:/source-wip/CS/include/csgeom/transfrm.h"
# 1 "f:/source-wip/CS/include/csgeom/matrix3.h" 1
# 33 "f:/source-wip/CS/include/csgeom/matrix3.h"
# 1 "f:/source-wip/CS/include/csgeom/vector3.h" 1
# 33 "f:/source-wip/CS/include/csgeom/vector3.h"
# 1 "f:/source-wip/CS/include/csgeom/math3d_d.h" 1
# 36 "f:/source-wip/CS/include/csgeom/math3d_d.h"
class csDVector3;
class csDMatrix3;
class csVector3;

inline double dSqr (double d)
{
  return d * d;
}




class csDVector3
{
public:

  double x;

  double y;

  double z;






  csDVector3 () {}






  csDVector3 (double m) : x(m), y(m), z(m) {}


  csDVector3 (double ix, double iy, double iz = 0) { x = ix; y = iy; z = iz; }


  csDVector3 (const csDVector3& v) { x = v.x; y = v.y; z = v.z; }


  csDVector3 (const csVector3&);


  inline friend
  csDVector3 operator+ (const csDVector3& v1, const csDVector3& v2)
  { return csDVector3(v1.x+v2.x, v1.y+v2.y, v1.z+v2.z); }


  inline friend
  csDVector3 operator- (const csDVector3& v1, const csDVector3& v2)
  { return csDVector3(v1.x-v2.x, v1.y-v2.y, v1.z-v2.z); }


  inline friend double operator* (const csDVector3& v1, const csDVector3& v2)
  { return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z; }


  inline friend
  csDVector3 operator% (const csDVector3& v1, const csDVector3& v2)
  {
    return csDVector3 (v1.y*v2.z-v1.z*v2.y,
                       v1.z*v2.x-v1.x*v2.z,
                       v1.x*v2.y-v1.y*v2.x);
  }


  void Cross (const csDVector3 & px, const csDVector3 & py)
  {
    x = px.y*py.z - px.z*py.y;
    y = px.z*py.x - px.x*py.z;
    z = px.x*py.y - px.y*py.x;
  }


  inline friend csDVector3 operator* (const csDVector3& v, double f)
  { return csDVector3(v.x*f, v.y*f, v.z*f); }


  inline friend csDVector3 operator* (double f, const csDVector3& v)
  { return csDVector3(v.x*f, v.y*f, v.z*f); }


  inline friend csDVector3 operator/ (const csDVector3& v, double f)
  { f = 1.0f/f; return csDVector3(v.x*f, v.y*f, v.z*f); }


  inline friend bool operator== (const csDVector3& v1, const csDVector3& v2)
  { return v1.x==v2.x && v1.y==v2.y && v1.z==v2.z; }


  inline friend bool operator!= (const csDVector3& v1, const csDVector3& v2)
  { return v1.x!=v2.x || v1.y!=v2.y || v1.z!=v2.z; }


  inline friend
  csDVector3 operator>> (const csDVector3& v1, const csDVector3& v2)
  { return v2*(v1*v2)/(v2*v2); }


  inline friend
  csDVector3 operator<< (const csDVector3& v1, const csDVector3& v2)
  { return v1*(v1*v2)/(v1*v1); }


  inline friend bool operator< (const csDVector3& v, double f)
  { return ((v.x)<0?-(v.x):(v.x))<f && ((v.y)<0?-(v.y):(v.y))<f && ((v.z)<0?-(v.z):(v.z))<f; }


  inline friend bool operator> (double f, const csDVector3& v)
  { return ((v.x)<0?-(v.x):(v.x))<f && ((v.y)<0?-(v.y):(v.y))<f && ((v.z)<0?-(v.z):(v.z))<f; }


  inline double operator[](int n) const {return !n?x:n&1?y:z;}


  inline double & operator[](int n){return !n?x:n&1?y:z;}


  inline csDVector3& operator+= (const csDVector3& v)
  {
    x += v.x;
    y += v.y;
    z += v.z;

    return *this;
  }


  inline csDVector3& operator-= (const csDVector3& v)
  {
    x -= v.x;
    y -= v.y;
    z -= v.z;

    return *this;
  }


  inline csDVector3& operator*= (double f)
  { x *= f; y *= f; z *= f; return *this; }


  inline csDVector3& operator/= (double f)
  { x /= f; y /= f; z /= f; return *this; }


  inline csDVector3 operator+ () const { return *this; }


  inline csDVector3 operator- () const { return csDVector3(-x,-y,-z); }


  inline void Set (double sx, double sy, double sz) { x = sx; y = sy; z = sz; }


  double Norm () const;


  double SquaredNorm () const;






  csDVector3 Unit () const { return (*this)/(this->Norm()); }


  inline static double Norm (const csDVector3& v) { return v.Norm(); }


  inline static csDVector3 Unit (const csDVector3& v) { return v.Unit(); }


  void Normalize();
};





class csDMatrix3
{
public:
  double m11, m12, m13;
  double m21, m22, m23;
  double m31, m32, m33;

public:

  csDMatrix3 ();


  csDMatrix3 (double m11, double m12, double m13,
            double m21, double m22, double m23,
            double m31, double m32, double m33);


  inline csDVector3 Row1() const { return csDVector3 (m11,m12,m13); }


  inline csDVector3 Row2() const { return csDVector3 (m21,m22,m23); }


  inline csDVector3 Row3() const { return csDVector3 (m31,m32,m33); }


  inline csDVector3 Col1() const { return csDVector3 (m11,m21,m31); }


  inline csDVector3 Col2() const { return csDVector3 (m12,m22,m32); }


  inline csDVector3 Col3() const { return csDVector3 (m13,m23,m33); }


  inline void Set (double m11, double m12, double m13,
                   double m21, double m22, double m23,
                   double m31, double m32, double m33)
  {
    csDMatrix3::m11 = m11; csDMatrix3::m12 = m12; csDMatrix3::m13 = m13;
    csDMatrix3::m21 = m21; csDMatrix3::m22 = m22; csDMatrix3::m23 = m23;
    csDMatrix3::m31 = m31; csDMatrix3::m32 = m32; csDMatrix3::m33 = m33;
  }


  csDMatrix3& operator+= (const csDMatrix3& m);


  csDMatrix3& operator-= (const csDMatrix3& m);


  csDMatrix3& operator*= (const csDMatrix3& m);


  csDMatrix3& operator*= (double s);


  csDMatrix3& operator/= (double s);


  inline csDMatrix3 operator+ () const { return *this; }

  inline csDMatrix3 operator- () const
  {
   return csDMatrix3(-m11,-m12,-m13,
                    -m21,-m22,-m23,
                    -m31,-m32,-m33);
  }


  void Transpose ();


  csDMatrix3 GetTranspose () const;


  inline csDMatrix3 GetInverse () const
  {
    csDMatrix3 C(
             (m22*m33 - m23*m32), -(m12*m33 - m13*m32), (m12*m23 - m13*m22),
            -(m21*m33 - m23*m31), (m11*m33 - m13*m31), -(m11*m23 - m13*m21),
             (m21*m32 - m22*m31), -(m11*m32 - m12*m31), (m11*m22 - m12*m21) );
    double s = (double)1./(m11*C.m11 + m12*C.m21 + m13*C.m31);

    C *= s;

    return C;
  }


  void Invert() { *this = GetInverse (); }


  double Determinant () const;


  void Identity ();


  friend csDMatrix3 operator+ (const csDMatrix3& m1, const csDMatrix3& m2);

  friend csDMatrix3 operator- (const csDMatrix3& m1, const csDMatrix3& m2);

  friend csDMatrix3 operator* (const csDMatrix3& m1, const csDMatrix3& m2);


  inline friend csDVector3 operator* (const csDMatrix3& m, const csDVector3& v)
  {
   return csDVector3 (m.m11*v.x + m.m12*v.y + m.m13*v.z,
                     m.m21*v.x + m.m22*v.y + m.m23*v.z,
                     m.m31*v.x + m.m32*v.y + m.m33*v.z);
  }


  friend csDMatrix3 operator* (const csDMatrix3& m, double f);

  friend csDMatrix3 operator* (double f, const csDMatrix3& m);

  friend csDMatrix3 operator/ (const csDMatrix3& m, double f);

  friend bool operator== (const csDMatrix3& m1, const csDMatrix3& m2);

  friend bool operator!= (const csDMatrix3& m1, const csDMatrix3& m2);

  friend bool operator< (const csDMatrix3& m, double f);

  friend bool operator> (double f, const csDMatrix3& m);
};







class csDPlane
{
public:

  csDVector3 norm;


  double DD;


  csDPlane () : norm(0,0,1), DD(0) {}


  csDPlane (const csDVector3& plane_norm, double d=0) :
  norm(plane_norm), DD(d) {}


  csDPlane (double a, double b, double c, double d=0) : norm(a,b,c), DD(d) {}


  inline csDVector3& Normal () { return norm; }

  inline const csDVector3& Normal () const { return norm; }


  inline double A () const { return norm.x; }

  inline double B () const { return norm.y; }

  inline double C () const { return norm.z; }

  inline double D () const { return DD; }


  inline double& A () { return norm.x; }

  inline double& B () { return norm.y; }

  inline double& C () { return norm.z; }

  inline double& D () { return DD; }


  inline void Set (double a, double b, double c, double d)
   { norm.x = a; norm.y = b; norm.z = c; DD = d; }


  inline double Classify (const csDVector3& pt) const { return norm*pt+DD; }


  static double Classify (double A, double B, double C, double D,
                         const csDVector3& pt)
  { return A*pt.x + B*pt.y + C*pt.z + D; }






  inline double Distance (const csDVector3& pt) const
  { return ((Classify (pt))<0?-(Classify (pt)):(Classify (pt))); }


  void Invert () { norm = -norm; DD = -DD; }


  void Normalize ()
  {
    double f = norm.Norm ();
    if (f) { norm /= f; DD /= f; }
  }

};





class csDMath3
{
public:







  static int WhichSide3D (const csDVector3& p,
                          const csDVector3& v1, const csDVector3& v2)
  {

    double s = p.x*(v1.y*v2.z-v1.z*v2.y) + p.y*(v1.z*v2.x-v1.x*v2.z) +
              p.z*(v1.x*v2.y-v1.y*v2.x);
    if (s < 0) return 1;
    else if (s > 0) return -1;
    else return 0;
  }






  static bool Visible (const csDVector3& p, const csDVector3& t1,
                       const csDVector3& t2, const csDVector3& t3);






  static bool Visible (const csDVector3& p, const csDPlane& pl)
  { return pl.Classify (p) <= 0; }
# 479 "f:/source-wip/CS/include/csgeom/math3d_d.h"
  static void Between (const csDVector3& v1, const csDVector3& v2,
                       csDVector3& v, double pct, double wid);







  static void SetMinMax (const csDVector3& v,
                         csDVector3& min, csDVector3& max)
  {
    if (v.x > max.x) max.x = v.x; else if (v.x < min.x ) min.x = v.x;
    if (v.y > max.y) max.y = v.y; else if (v.y < min.y ) min.y = v.y;
    if (v.z > max.z) max.z = v.z; else if (v.z < min.z ) min.z = v.z;
  }






  inline static double Area3 (const csDVector3 &a, const csDVector3 &b,
                             const csDVector3 &c)
  {
    csDVector3 v1 = b - a;
    csDVector3 v2 = c - a;
    return ((v1.y * v2.z + v1.z * v2.x + v1.x * v2.y) -
            (v1.y * v2.x + v1.x * v2.z + v1.z * v2.y));
  }






  inline static void CalcNormal (csDVector3& norm, const csDVector3& v1,
                                 const csDVector3& v2, const csDVector3& v3)
  {
    norm = (v1-v2)%(v1-v3);
  }






  static void CalcNormal (csDVector3& norm,
                          const csDVector3& v, const csDVector3& u)
  { norm = u%v; }







  static void CalcPlane (const csDVector3& v1, const csDVector3& v2,
         const csDVector3& v3, csDVector3& normal, double& D)
  {
    normal = (v1-v2)%(v1-v3);
    D = - (normal * v1);
  }







  static bool PlanesEqual (const csDPlane& p1, const csDPlane& p2)
  {
    return ( ( p1.norm - p2.norm) < (double).001 ) &&
             ( ((p1.DD-p2.DD)<0?-(p1.DD-p2.DD):(p1.DD-p2.DD)) < (double).001 );
  }






  static bool PlanesClose (const csDPlane& p1, const csDPlane& p2);
};





class csDSquaredDist
{
public:

  static double PointPoint (const csDVector3& p1, const csDVector3& p2)
  { return dSqr (p1.x - p2.x) + dSqr (p1.y - p2.y) + dSqr (p1.z - p2.z); }


  static double PointLine (const csDVector3& p,
                          const csDVector3& l1, const csDVector3& l2);


  static double PointPlane (const csDVector3& p, const csDPlane& plane)
  { double r = plane.Classify (p); return r * r; }







  static double PointPoly (const csDVector3& p, csDVector3 *V, int n,
                          const csDPlane& plane, double sqdist = -1);
};






class csDIntersect3
{
public:




  static void Plane (
    const csDVector3& u, const csDVector3& v,
    const csDVector3& normal, const csDVector3& a,
    csDVector3& isect);
# 617 "f:/source-wip/CS/include/csgeom/math3d_d.h"
  static bool Plane (
    const csDVector3& u, const csDVector3& v,
    double A, double B, double C, double D,
    csDVector3& isect,
    double& dist);
# 631 "f:/source-wip/CS/include/csgeom/math3d_d.h"
  static bool Plane (
    const csDVector3& u, const csDVector3& v,
    const csDPlane& p,
    csDVector3& isect,
    double& dist);






  static bool Planes(const csDPlane& p1, const csDPlane& p2,
                     const csDPlane& p3, csDVector3& isect);







  static double Z0Plane (
    const csDVector3& u, const csDVector3& v,
    csDVector3& isect);







  static double ZPlane (double zval,
    const csDVector3& u, const csDVector3& v,
    csDVector3& isect);





  static double XFrustum (
    double A, const csDVector3& u, const csDVector3& v, csDVector3& isect);





  static double YFrustum (
    double B, const csDVector3& u, const csDVector3& v, csDVector3& isect);
};
# 34 "f:/source-wip/CS/include/csgeom/vector3.h" 2




class csVector3
{
public:

  float x;

  float y;

  float z;






  csVector3 () {}






  csVector3 (float m) : x(m), y(m), z(m) {}


  csVector3 (float ix, float iy, float iz = 0) : x(ix), y(iy), z(iz) {}


  csVector3 (const csVector3& v) : x(v.x), y(v.y), z(v.z) {}


  csVector3 (const csDVector3&);


  inline friend csVector3 operator+ (const csVector3& v1, const csVector3& v2)
  { return csVector3(v1.x+v2.x, v1.y+v2.y, v1.z+v2.z); }


  inline friend csDVector3 operator+ (const csDVector3& v1, const csVector3& v2)
  { return csDVector3(v1.x+v2.x, v1.y+v2.y, v1.z+v2.z); }


  inline friend csDVector3 operator+ (const csVector3& v1, const csDVector3& v2)
  { return csDVector3(v1.x+v2.x, v1.y+v2.y, v1.z+v2.z); }


  inline friend csVector3 operator- (const csVector3& v1, const csVector3& v2)
  { return csVector3(v1.x-v2.x, v1.y-v2.y, v1.z-v2.z); }


  inline friend csDVector3 operator- (const csVector3& v1, const csDVector3& v2)
  { return csDVector3(v1.x-v2.x, v1.y-v2.y, v1.z-v2.z); }


  inline friend csDVector3 operator- (const csDVector3& v1, const csVector3& v2)
  { return csDVector3(v1.x-v2.x, v1.y-v2.y, v1.z-v2.z); }


  inline friend float operator* (const csVector3& v1, const csVector3& v2)
  { return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z; }


  inline friend csVector3 operator% (const csVector3& v1, const csVector3& v2)
  {
    return csVector3 (v1.y*v2.z-v1.z*v2.y,
                      v1.z*v2.x-v1.x*v2.z,
                      v1.x*v2.y-v1.y*v2.x);
  }


  void Cross (const csVector3 & px, const csVector3 & py)
  {
    x = px.y*py.z - px.z*py.y;
    y = px.z*py.x - px.x*py.z;
    z = px.x*py.y - px.y*py.x;
  }


  inline friend csVector3 operator* (const csVector3& v, float f)
  { return csVector3(v.x*f, v.y*f, v.z*f); }


  inline friend csVector3 operator* (float f, const csVector3& v)
  { return csVector3(v.x*f, v.y*f, v.z*f); }


  inline friend csDVector3 operator* (const csVector3& v, double f)
  { return csDVector3(v) * f; }


  inline friend csDVector3 operator* (double f, const csVector3& v)
  { return csDVector3(v) * f; }


  inline friend csVector3 operator* (const csVector3& v, int f)
  { return v * (float)f; }


  inline friend csVector3 operator* (int f, const csVector3& v)
  { return v * (float)f; }


  inline friend csVector3 operator/ (const csVector3& v, float f)
  { f = 1.0f/f; return csVector3(v.x*f, v.y*f, v.z*f); }


  inline friend csDVector3 operator/ (const csVector3& v, double f)
  { return csDVector3(v) / f; }


  inline friend csVector3 operator/ (const csVector3& v, int f)
  { return v / (float)f; }


  inline friend bool operator== (const csVector3& v1, const csVector3& v2)
  { return v1.x==v2.x && v1.y==v2.y && v1.z==v2.z; }


  inline friend bool operator!= (const csVector3& v1, const csVector3& v2)
  { return v1.x!=v2.x || v1.y!=v2.y || v1.z!=v2.z; }


  inline friend csVector3 operator>> (const csVector3& v1, const csVector3& v2)
  { return v2*(v1*v2)/(v2*v2); }


  inline friend csVector3 operator<< (const csVector3& v1, const csVector3& v2)
  { return v1*(v1*v2)/(v1*v1); }


  inline friend bool operator< (const csVector3& v, float f)
  { return ((v.x)<0?-(v.x):(v.x))<f && ((v.y)<0?-(v.y):(v.y))<f && ((v.z)<0?-(v.z):(v.z))<f; }


  inline friend bool operator> (float f, const csVector3& v)
  { return ((v.x)<0?-(v.x):(v.x))<f && ((v.y)<0?-(v.y):(v.y))<f && ((v.z)<0?-(v.z):(v.z))<f; }


  inline float operator[] (int n) const { return !n?x:n&1?y:z; }


  inline float & operator[] (int n) { return !n?x:n&1?y:z; }


  inline csVector3& operator+= (const csVector3& v)
  {
    x += v.x;
    y += v.y;
    z += v.z;

    return *this;
  }


  inline csVector3& operator-= (const csVector3& v)
  {
    x -= v.x;
    y -= v.y;
    z -= v.z;

    return *this;
  }


  inline csVector3& operator*= (float f)
  { x *= f; y *= f; z *= f; return *this; }


  inline csVector3& operator/= (float f)
  { f = 1.0f / f; x *= f; y *= f; z *= f; return *this; }


  inline csVector3 operator+ () const { return *this; }


  inline csVector3 operator- () const { return csVector3(-x,-y,-z); }


  inline void Set (float sx, float sy, float sz) { x = sx; y = sy; z = sz; }


  inline void Set (const csVector3& v) { x = v.x; y = v.y; z = v.z; }


  float Norm () const;


  float SquaredNorm () const
  { return x * x + y * y + z * z; }






  csVector3 Unit () const { return (*this)/(this->Norm()); }


  inline static float Norm (const csVector3& v) { return v.Norm(); }


  inline static csVector3 Unit (const csVector3& v) { return v.Unit(); }


  void Normalize ();


  inline bool IsZero (float precision = 0.000001f) const
  { return (((x)<0?-(x):(x)) < precision) && (((y)<0?-(y):(y)) < precision)
            && (((z)<0?-(z):(z)) < precision);
  }
};
# 34 "f:/source-wip/CS/include/csgeom/matrix3.h" 2

class csQuaternion;




class csMatrix3
{
public:
  float m11, m12, m13;
  float m21, m22, m23;
  float m31, m32, m33;

public:

  csMatrix3 ()
      : m11(1), m12(0), m13(0),
        m21(0), m22(1), m23(0),
        m31(0), m32(0), m33(1)
  {}


  csMatrix3 (float am11, float am12, float am13,
             float am21, float am22, float am23,
             float am31, float am32, float am33)
      : m11(am11), m12(am12), m13(am13),
        m21(am21), m22(am22), m23(am23),
        m31(am31), m32(am32), m33(am33)
  {}


  explicit csMatrix3 (const csQuaternion &quat) { Set (quat); }


  inline csVector3 Row1() const { return csVector3 (m11,m12,m13); }


  inline csVector3 Row2() const { return csVector3 (m21,m22,m23); }


  inline csVector3 Row3() const { return csVector3 (m31,m32,m33); }


  inline csVector3 Col1() const { return csVector3 (m11,m21,m31); }


  inline csVector3 Col2() const { return csVector3 (m12,m22,m32); }


  inline csVector3 Col3() const { return csVector3 (m13,m23,m33); }


  inline void Set (float m11, float m12, float m13,
                   float m21, float m22, float m23,
                   float m31, float m32, float m33)
  {
    csMatrix3::m11 = m11; csMatrix3::m12 = m12; csMatrix3::m13 = m13;
    csMatrix3::m21 = m21; csMatrix3::m22 = m22; csMatrix3::m23 = m23;
    csMatrix3::m31 = m31; csMatrix3::m32 = m32; csMatrix3::m33 = m33;
  }


  void Set (const csQuaternion &quat);


  csMatrix3& operator+= (const csMatrix3& m);


  csMatrix3& operator-= (const csMatrix3& m);


  csMatrix3& operator*= (const csMatrix3& m);


  csMatrix3& operator*= (float s);


  csMatrix3& operator/= (float s);


  inline csMatrix3 operator+ () const { return *this; }

  inline csMatrix3 operator- () const
  {
    return csMatrix3(-m11,-m12,-m13,
                     -m21,-m22,-m23,
                    -m31,-m32,-m33);
  }


  void Transpose ();


  csMatrix3 GetTranspose () const;


  inline csMatrix3 GetInverse () const
  {
    csMatrix3 C(
             (m22*m33 - m23*m32), -(m12*m33 - m13*m32), (m12*m23 - m13*m22),
            -(m21*m33 - m23*m31), (m11*m33 - m13*m31), -(m11*m23 - m13*m21),
             (m21*m32 - m22*m31), -(m11*m32 - m12*m31), (m11*m22 - m12*m21) );
    float s = (float)1./(m11*C.m11 + m12*C.m21 + m13*C.m31);

    C *= s;

    return C;
  }


  void Invert() { *this = GetInverse (); }


  float Determinant () const;


  void Identity ();


  bool IsIdentity () const;


  friend csMatrix3 operator+ (const csMatrix3& m1, const csMatrix3& m2);

  friend csMatrix3 operator- (const csMatrix3& m1, const csMatrix3& m2);

  friend csMatrix3 operator* (const csMatrix3& m1, const csMatrix3& m2);


  inline friend csVector3 operator* (const csMatrix3& m, const csVector3& v)
  {
    return csVector3 (m.m11*v.x + m.m12*v.y + m.m13*v.z,
                      m.m21*v.x + m.m22*v.y + m.m23*v.z,
                      m.m31*v.x + m.m32*v.y + m.m33*v.z);
  }


  friend csMatrix3 operator* (const csMatrix3& m, float f);

  friend csMatrix3 operator* (float f, const csMatrix3& m);

  friend csMatrix3 operator/ (const csMatrix3& m, float f);

  friend bool operator== (const csMatrix3& m1, const csMatrix3& m2);

  friend bool operator!= (const csMatrix3& m1, const csMatrix3& m2);

  friend bool operator< (const csMatrix3& m, float f);

  friend bool operator> (float f, const csMatrix3& m);
};


class csXRotMatrix3 : public csMatrix3
{
public:







  csXRotMatrix3 (float angle);
};


class csYRotMatrix3 : public csMatrix3
{
public:







  csYRotMatrix3 (float angle);
};


class csZRotMatrix3 : public csMatrix3
{
public:







  csZRotMatrix3 (float angle);
};


class csXScaleMatrix3 : public csMatrix3
{
public:



  csXScaleMatrix3 (float scaler) : csMatrix3(scaler, 0, 0, 0, 1, 0, 0, 0, 1) {}
};


class csYScaleMatrix3 : public csMatrix3
{
public:



  csYScaleMatrix3 (float scaler) : csMatrix3(1, 0, 0, 0, scaler, 0, 0, 0, 1) {}
};


class csZScaleMatrix3 : public csMatrix3
{
public:



  csZScaleMatrix3 (float scaler) : csMatrix3(1, 0, 0, 0, 1, 0, 0, 0, scaler) {}
};
# 30 "f:/source-wip/CS/include/csgeom/transfrm.h" 2
# 1 "f:/source-wip/CS/include/csgeom/plane3.h" 1
# 40 "f:/source-wip/CS/include/csgeom/plane3.h"
class csPlane3
{
public:

  csVector3 norm;


  float DD;




  csPlane3 () : norm(0,0,1), DD(0) {}




  csPlane3 (const csVector3& plane_norm, float d=0) : norm(plane_norm), DD(d) {}




  csPlane3 (float a, float b, float c, float d=0) : norm(a,b,c), DD(d) {}







  csPlane3 (const csVector3& v1, const csVector3& v2, const csVector3& v3);






  csPlane3 (const csVector3& v2, const csVector3& v3)
  {
    norm = v2 % v3; DD = 0;
  }


  inline csVector3& Normal () { return norm; }

  inline const csVector3& Normal () const { return norm; }


  inline float A () const { return norm.x; }

  inline float B () const { return norm.y; }

  inline float C () const { return norm.z; }

  inline float D () const { return DD; }


  inline float& A () { return norm.x; }

  inline float& B () { return norm.y; }

  inline float& C () { return norm.z; }

  inline float& D () { return DD; }


  inline void Set (float a, float b, float c, float d)
  { norm.x = a; norm.y = b; norm.z = c; DD = d; }


  inline void Set (const csVector3& normal, float d)
  { norm = normal; DD = d; }







  void Set (const csVector3& v1, const csVector3& v2, const csVector3& v3);






  void Set (const csVector3& v2, const csVector3& v3)
  {
    norm = v2 % v3; DD = 0;
  }
# 141 "f:/source-wip/CS/include/csgeom/plane3.h"
  inline float Classify (const csVector3& pt) const { return norm*pt+DD; }





  static float Classify (float A, float B, float C, float D,
                         const csVector3& pt)
  {
    return A*pt.x + B*pt.y + C*pt.z + D;
  }







  inline float Distance (const csVector3& pt) const
  { return ((Classify (pt))<0?-(Classify (pt)):(Classify (pt))); }





  void Invert () { norm = -norm; DD = -DD; }




  void Normalize ()
  {
    float f = norm.Norm ();
    if (f) { norm /= f; DD /= f; }
  }
# 187 "f:/source-wip/CS/include/csgeom/plane3.h"
  bool ClipPolygon (csVector3*& pverts, int& num_verts, bool reversed = false);
};
# 31 "f:/source-wip/CS/include/csgeom/transfrm.h" 2
# 1 "f:/source-wip/CS/include/csgeom/sphere.h" 1
# 31 "f:/source-wip/CS/include/csgeom/sphere.h"
class csTransform;




class csSphere
{
private:
  csVector3 center;
  float radius;

public:

  csSphere ()
  {
    center.Set (0, 0, 0);
    radius = 0;
  }


  csSphere (const csVector3& center, float radius)
  {
    csSphere::center = center;
    csSphere::radius = radius;
  }


  csSphere (const csSphere& s) { center = s.center; radius = s.radius; }


  csVector3& GetCenter () { return center; }

  const csVector3& GetCenter () const { return center; }

  void SetCenter (const csVector3& c) { center = c; }

  float GetRadius () const { return radius; }

  void SetRadius (float r) { radius = r; }


  void Union (const csVector3& ocenter, float oradius);


  friend csSphere operator+ (const csSphere& s1, const csSphere& s2);

  csSphere& operator+= (const csSphere& s)
  {
    Union (s.center, s.radius);
    return *this;
  }
};
# 32 "f:/source-wip/CS/include/csgeom/transfrm.h" 2

class csReversibleTransform;







class csTransform
{
protected:

  csMatrix3 m_o2t;

  csVector3 v_o2t;

public:



  csTransform () : m_o2t (), v_o2t (0, 0, 0) {}
# 62 "f:/source-wip/CS/include/csgeom/transfrm.h"
  csTransform (const csMatrix3& other2this, const csVector3& origin_pos) :
        m_o2t (other2this), v_o2t (origin_pos) {}




  void Identity ()
  {
    SetO2TTranslation (csVector3 (0));
    SetO2T (csMatrix3 ());
  }





  bool IsIdentity () const
  {
    if (((v_o2t.x)<0?-(v_o2t.x):(v_o2t.x)) >= 0.000001f) return false;
    if (((v_o2t.y)<0?-(v_o2t.y):(v_o2t.y)) >= 0.000001f) return false;
    if (((v_o2t.z)<0?-(v_o2t.z):(v_o2t.z)) >= 0.000001f) return false;
    if (((m_o2t.m11-1)<0?-(m_o2t.m11-1):(m_o2t.m11-1)) >= 0.000001f) return false;
    if (((m_o2t.m12)<0?-(m_o2t.m12):(m_o2t.m12)) >= 0.000001f) return false;
    if (((m_o2t.m13)<0?-(m_o2t.m13):(m_o2t.m13)) >= 0.000001f) return false;
    if (((m_o2t.m21)<0?-(m_o2t.m21):(m_o2t.m21)) >= 0.000001f) return false;
    if (((m_o2t.m22-1)<0?-(m_o2t.m22-1):(m_o2t.m22-1)) >= 0.000001f) return false;
    if (((m_o2t.m23)<0?-(m_o2t.m23):(m_o2t.m23)) >= 0.000001f) return false;
    if (((m_o2t.m31)<0?-(m_o2t.m31):(m_o2t.m31)) >= 0.000001f) return false;
    if (((m_o2t.m32)<0?-(m_o2t.m32):(m_o2t.m32)) >= 0.000001f) return false;
    if (((m_o2t.m33-1)<0?-(m_o2t.m33-1):(m_o2t.m33-1)) >= 0.000001f) return false;
    return true;
  }





  inline const csMatrix3& GetO2T () const { return m_o2t; }






  inline const csVector3& GetO2TTranslation () const { return v_o2t; }





  inline const csVector3& GetOrigin () const { return v_o2t; }





  virtual void SetO2T (const csMatrix3& m) { m_o2t = m; }






  virtual void SetO2TTranslation (const csVector3& v) { v_o2t = v; }





  inline void SetOrigin (const csVector3& v) { SetO2TTranslation (v); }






  inline void Translate (const csVector3& v) { SetO2TTranslation (v_o2t + v); }






  inline csVector3 Other2This (const csVector3& v) const
  {
    return m_o2t * (v - v_o2t);
  }






  csVector3 Other2ThisRelative (const csVector3& v) const
  { return m_o2t * v; }






  csPlane3 Other2This (const csPlane3& p) const;







  csPlane3 Other2ThisRelative (const csPlane3& p) const;
# 180 "f:/source-wip/CS/include/csgeom/transfrm.h"
  void Other2This (const csPlane3& p, const csVector3& point,
        csPlane3& result) const;




  csSphere Other2This (const csSphere& s) const;





  friend csVector3 operator* (const csVector3& v, const csTransform& t);





  friend csVector3 operator* (const csTransform& t, const csVector3& v);





  friend csVector3& operator*= (csVector3& v, const csTransform& t);





  friend csPlane3 operator* (const csPlane3& p, const csTransform& t);





  friend csPlane3 operator* (const csTransform& t, const csPlane3& p);





  friend csPlane3& operator*= (csPlane3& p, const csTransform& t);





  friend csSphere operator* (const csSphere& p, const csTransform& t);





  friend csSphere operator* (const csTransform& t, const csSphere& p);





  friend csSphere& operator*= (csSphere& p, const csTransform& t);





  friend csMatrix3 operator* (const csMatrix3& m, const csTransform& t);





  friend csMatrix3 operator* (const csTransform& t, const csMatrix3& m);





  friend csMatrix3& operator*= (csMatrix3& m, const csTransform& t);
# 271 "f:/source-wip/CS/include/csgeom/transfrm.h"
  friend csTransform operator* (const csTransform& t1,
                              const csReversibleTransform& t2);






  static csTransform GetReflect (const csPlane3& pl);
};
# 289 "f:/source-wip/CS/include/csgeom/transfrm.h"
class csReversibleTransform : public csTransform
{
protected:

  csMatrix3 m_t2o;




  csReversibleTransform (const csMatrix3& o2t, const csMatrix3& t2o,
    const csVector3& pos) : csTransform (o2t,pos), m_t2o (t2o) {}

public:



  csReversibleTransform () : csTransform (), m_t2o () {}
# 314 "f:/source-wip/CS/include/csgeom/transfrm.h"
  csReversibleTransform (const csMatrix3& o2t, const csVector3& pos) :
    csTransform (o2t,pos) { m_t2o = m_o2t.GetInverse (); }




  csReversibleTransform (const csTransform& t) :
    csTransform (t) { m_t2o = m_o2t.GetInverse (); }




  csReversibleTransform (const csReversibleTransform& t) :
    csTransform (t) { m_t2o = t.m_t2o; }





  inline const csMatrix3& GetT2O () const { return m_t2o; }





  inline csVector3 GetT2OTranslation () const { return -m_o2t*v_o2t; }




  csReversibleTransform GetInverse () const
  { return csReversibleTransform (m_t2o, m_o2t, -m_o2t*v_o2t); }





  virtual void SetO2T (const csMatrix3& m)
  { m_o2t = m; m_t2o = m_o2t.GetInverse (); }






  virtual void SetT2O (const csMatrix3& m)
  { m_t2o = m; m_o2t = m_t2o.GetInverse (); }






  csVector3 This2Other (const csVector3& v) const
  { return v_o2t + m_t2o * v; }






  inline csVector3 This2OtherRelative (const csVector3& v) const
  { return m_t2o * v; }







  csPlane3 This2Other (const csPlane3& p) const;







  csPlane3 This2OtherRelative (const csPlane3& p) const;
# 402 "f:/source-wip/CS/include/csgeom/transfrm.h"
  void This2Other (const csPlane3& p, const csVector3& point,
        csPlane3& result) const;




  csSphere This2Other (const csSphere& s) const;






  void RotateOther (const csVector3& v, float angle);






  void RotateThis (const csVector3& v, float angle);
# 431 "f:/source-wip/CS/include/csgeom/transfrm.h"
  void RotateOther (const csMatrix3& m) { SetT2O (m * m_t2o); }
# 440 "f:/source-wip/CS/include/csgeom/transfrm.h"
  void RotateThis (const csMatrix3& m) { SetT2O (m_t2o * m); }
# 450 "f:/source-wip/CS/include/csgeom/transfrm.h"
  void LookAt (const csVector3& v, const csVector3& up);





  friend csVector3 operator/ (const csVector3& v,
        const csReversibleTransform& t);





  friend csVector3& operator/= (csVector3& v, const csReversibleTransform& t);





  friend csPlane3 operator/ (const csPlane3& p, const csReversibleTransform& t);





  friend csPlane3& operator/= (csPlane3& p, const csReversibleTransform& t);





  friend csSphere operator/ (const csSphere& p, const csReversibleTransform& t);
# 495 "f:/source-wip/CS/include/csgeom/transfrm.h"
  friend csReversibleTransform& operator*= (csReversibleTransform& t1,
                                          const csReversibleTransform& t2)
  {
    t1.v_o2t = t2.m_t2o*t1.v_o2t;
    t1.v_o2t += t2.v_o2t;
    t1.m_o2t *= t2.m_o2t;
    t1.m_t2o *= t1.m_t2o;
    return t1;
  }
# 517 "f:/source-wip/CS/include/csgeom/transfrm.h"
  friend csReversibleTransform operator* (const csReversibleTransform& t1,
                                        const csReversibleTransform& t2)
  {
    return csReversibleTransform (t1.m_o2t*t2.m_o2t, t2.m_t2o*t1.m_t2o,
                             t2.v_o2t + t2.m_t2o*t1.v_o2t);
  }
# 536 "f:/source-wip/CS/include/csgeom/transfrm.h"
  friend csTransform operator* (const csTransform& t1,
                              const csReversibleTransform& t2);
# 551 "f:/source-wip/CS/include/csgeom/transfrm.h"
  friend csReversibleTransform& operator/= (csReversibleTransform& t1,
                                          const csReversibleTransform& t2);
# 566 "f:/source-wip/CS/include/csgeom/transfrm.h"
  friend csReversibleTransform operator/ (const csReversibleTransform& t1,
                                        const csReversibleTransform& t2);
};







class csOrthoTransform : public csReversibleTransform
{
public:



  csOrthoTransform () : csReversibleTransform () {}




  csOrthoTransform (const csMatrix3& o2t, const csVector3& pos) :
    csReversibleTransform (o2t, o2t.GetTranspose (), pos) { }




  csOrthoTransform (const csTransform& t) :
    csReversibleTransform (t.GetO2T (), t.GetO2T ().GetTranspose (),
        t.GetO2TTranslation ())
  { }





  virtual void SetO2T (const csMatrix3& m)
  { m_o2t = m; m_t2o = m_o2t.GetTranspose (); }






  virtual void SetT2O (const csMatrix3& m)
  { m_t2o = m; m_o2t = m_t2o.GetTranspose (); }
};
# 31 "f:/source-wip/CS/include/iengine/camera.h" 2
# 46 "f:/source-wip/CS/include/iengine/camera.h"
class csCamera;
class csVector3;
class csVector2;
struct iSector;
struct iPolygon3D;

const int iCamera_VERSION = ((0 << 24) | (2 << 16) | 0); inline static scfInterfaceID iCamera_scfGetID () { static scfInterfaceID ID = (scfInterfaceID)-1; if (ID == (scfInterfaceID)(-1)) ID = iSCF::SCF->GetInterfaceID ("iCamera"); return ID; };
# 77 "f:/source-wip/CS/include/iengine/camera.h"
struct iCamera : public iBase
{

  virtual iCamera *Clone () const = 0;


  virtual int GetFOV () const = 0;

  virtual float GetInvFOV () const = 0;

  virtual float GetFOVAngle () const = 0;





  virtual void SetFOV (int fov, int width) = 0;




  virtual void SetFOVAngle (float fov, int width) = 0;





  virtual float GetShiftX () const = 0;




  virtual float GetShiftY () const = 0;




  virtual void SetPerspectiveCenter (float x, float y) = 0;
# 124 "f:/source-wip/CS/include/iengine/camera.h"
  virtual csOrthoTransform& GetTransform () = 0;


  virtual const csOrthoTransform& GetTransform () const = 0;





  virtual void SetTransform (const csOrthoTransform& tr) = 0;







  virtual void MoveWorld (const csVector3& v, bool cd = true) = 0;



  virtual void Move (const csVector3& v, bool cd = true) = 0;







  virtual void MoveWorldUnrestricted (const csVector3& v) = 0;







  virtual void MoveUnrestricted (const csVector3& v) = 0;


  virtual iSector* GetSector () const = 0;

  virtual void SetSector (iSector*) = 0;





  virtual void Correct (int n) = 0;


  virtual bool IsMirrored () const = 0;

  virtual void SetMirrored (bool m) = 0;
# 186 "f:/source-wip/CS/include/iengine/camera.h"
  virtual iPolygon3D* GetHit (csVector3& v) = 0;
# 195 "f:/source-wip/CS/include/iengine/camera.h"
  virtual csPlane3* GetFarPlane () const = 0;
# 205 "f:/source-wip/CS/include/iengine/camera.h"
  virtual void SetFarPlane (csPlane3* fp) = 0;







  virtual long GetCameraNumber () const = 0;


  virtual void Perspective (const csVector3& v, csVector2& p) const = 0;

  virtual void InvPerspective (const csVector2& p, float z,
        csVector3& v) const = 0;
# 228 "f:/source-wip/CS/include/iengine/camera.h"
  virtual void OnlyPortals (bool hop) = 0;


  virtual bool GetOnlyPortals () = 0;
};
# 25 "lightiter.cpp" 2
# 1 "f:/source-wip/CS/include/iengine/light.h" 1
# 30 "f:/source-wip/CS/include/iengine/light.h"
# 1 "f:/source-wip/CS/include/iengine/fview.h" 1
# 31 "f:/source-wip/CS/include/iengine/fview.h"
# 1 "f:/source-wip/CS/include/csgeom/box.h" 1
# 31 "f:/source-wip/CS/include/csgeom/box.h"
# 1 "f:/source-wip/CS/include/csgeom/vector2.h" 1
# 32 "f:/source-wip/CS/include/csgeom/vector2.h"
class csVector2
{
public:

  float x;

  float y;


  csVector2 () {}


  csVector2 (float x, float y) { csVector2::x = x; csVector2::y = y; }


  inline void Set (float ix, float iy)
  { x = ix; y = iy; }


  inline void Set (const csVector2& v)
  { x = v.x; y = v.y; }


  static float Norm (const csVector2& v);


  float Norm () const;


  float SquaredNorm () const
  { return x * x + y * y; }


  void Rotate (float angle);


  csVector2& operator+= (const csVector2& v)
  { x += v.x; y += v.y; return *this; }


  csVector2& operator-= (const csVector2& v)
  { x -= v.x; y -= v.y; return *this; }


  csVector2& operator*= (float f) { x *= f; y *= f; return *this; }


  csVector2& operator/= (float f) { f = 1.0f / f; x *= f; y *= f; return *this; }


  inline csVector2 operator+ () const { return *this; }


  inline csVector2 operator- () const { return csVector2(-x,-y); }


  friend csVector2 operator+ (const csVector2& v1, const csVector2& v2);

  friend csVector2 operator- (const csVector2& v1, const csVector2& v2);

  friend float operator* (const csVector2& v1, const csVector2& v2);

  friend csVector2 operator* (const csVector2& v, float f);

  friend csVector2 operator* (float f, const csVector2& v);

  friend csVector2 operator/ (const csVector2& v, float f);

  friend bool operator== (const csVector2& v1, const csVector2& v2);

  friend bool operator!= (const csVector2& v1, const csVector2& v2);


  inline friend bool operator< (const csVector2& v, float f)
  { return ((v.x)<0?-(v.x):(v.x))<f && ((v.y)<0?-(v.y):(v.y))<f; }


  inline friend bool operator> (float f, const csVector2& v)
  { return ((v.x)<0?-(v.x):(v.x))<f && ((v.y)<0?-(v.y):(v.y))<f; }
};
# 32 "f:/source-wip/CS/include/csgeom/box.h" 2

# 1 "f:/source-wip/CS/include/csgeom/segment.h" 1
# 34 "f:/source-wip/CS/include/csgeom/segment.h"
class csSegment2
{
private:

  csVector2 start;

  csVector2 end;

public:

  csSegment2 (const csVector2& s, const csVector2& e) { start = s; end = e; }

  csSegment2 () { }

  ~csSegment2 () { }


  void Set (const csVector2& s, const csVector2& e)
  { start = s; end = e; }


  void SetStart (const csVector2& s) { start = s; }


  void SetEnd (const csVector2& e) { end = e; }


  const csVector2& Start () const { return start; }


  const csVector2& End () const { return end; }


  csVector2& Start () { return start; }


  csVector2& End () { return end; }
};




class csSegment3
{
private:

  csVector3 start;

  csVector3 end;

public:

  csSegment3 (const csVector3& s, const csVector3& e) { start = s; end = e; }

  csSegment3 () { }


  void Set (const csVector3& s, const csVector3& e)
  { start = s; end = e; }


  void SetStart (const csVector3& s) { start = s; }


  void SetEnd (const csVector3& e) { end = e; }


  const csVector3& Start () const { return start; }


  const csVector3& End () const { return end; }


  csVector3& Start () { return start; }


  csVector3& End () { return end; }
};
# 34 "f:/source-wip/CS/include/csgeom/box.h" 2

class csPlane3;
class csTransform;
# 86 "f:/source-wip/CS/include/csgeom/box.h"
class csBox2
{
private:
  struct bEdge
  {
    uint8 v1, v2;
  };


  static bEdge edges[8];

protected:

  csVector2 minbox;

  csVector2 maxbox;

public:

  float MinX () const { return minbox.x; }

  float MinY () const { return minbox.y; }

  float MaxX () const { return maxbox.x; }

  float MaxY () const { return maxbox.y; }

  float Min (int idx) const { return idx ? minbox.y : minbox.x; }

  float Max (int idx) const { return idx ? maxbox.y : maxbox.x; }

  const csVector2& Min () const { return minbox; }

  const csVector2& Max () const { return maxbox; }
# 128 "f:/source-wip/CS/include/csgeom/box.h"
  csVector2 GetCorner (int corner) const;




  csVector2 GetCenter () const { return (minbox+maxbox)/2; }





  void SetCenter (const csVector2& c);




  void SetSize (const csVector2& s);





  void GetEdgeInfo (int edge, int& v1, int& v2) const
  {
    v1 = edges[edge].v1;
    v2 = edges[edge].v2;
  }





  csSegment2 GetEdge (int edge) const
  {
    return csSegment2 (GetCorner (edges[edge].v1), GetCorner (edges[edge].v2));
  }





  void GetEdge (int edge, csSegment2& e) const
  {
    e.SetStart (GetCorner (edges[edge].v1));
    e.SetEnd (GetCorner (edges[edge].v2));
  }







  static bool Intersect (float minx, float miny, float maxx, float maxy,
    csVector2* poly, int num_poly);







  static bool Intersect (const csVector2& minbox, const csVector2& maxbox,
    csVector2* poly, int num_poly)
  {
    return Intersect (minbox.x, minbox.y, maxbox.x, maxbox.y, poly, num_poly);
  }







  bool Intersect (csVector2* poly, int num_poly) const
  {
    return Intersect (minbox, maxbox, poly, num_poly);
  }


  bool In (float x, float y) const
  {
    if (x < minbox.x || x > maxbox.x) return false;
    if (y < minbox.y || y > maxbox.y) return false;
    return true;
  }


  bool In (const csVector2& v) const
  {
    return In (v.x, v.y);
  }


  bool Overlap (const csBox2& box) const
  {
    if (maxbox.x < box.minbox.x || minbox.x > box.maxbox.x) return false;
    if (maxbox.y < box.minbox.y || minbox.y > box.maxbox.y) return false;
    return true;
  }


  bool Contains (const csBox2& box) const
  {
    return (box.minbox.x >= minbox.x && box.maxbox.x <= maxbox.x) &&
           (box.minbox.y >= minbox.y && box.maxbox.y <= maxbox.y);
  }


  bool Empty () const
  {
    if (minbox.x > maxbox.x) return true;
    if (minbox.y > maxbox.y) return true;
    return false;
  }





  float SquaredOriginDist () const;






  float SquaredOriginMaxDist () const;


  void StartBoundingBox ()
  {
    minbox.x = 1000000000.; minbox.y = 1000000000.;
    maxbox.x = -1000000000.; maxbox.y = -1000000000.;
  }


  void StartBoundingBox (const csVector2& v)
  {
    minbox = v;
    maxbox = v;
  }


  void StartBoundingBox (float x, float y)
  {
    minbox.x = maxbox.x = x;
    minbox.y = maxbox.y = y;
  }


  void AddBoundingVertex (float x, float y)
  {
    if (x < minbox.x) minbox.x = x; if (x > maxbox.x) maxbox.x = x;
    if (y < minbox.y) minbox.y = y; if (y > maxbox.y) maxbox.y = y;
  }


  void AddBoundingVertex (const csVector2& v)
  {
    AddBoundingVertex (v.x, v.y);
  }






  void AddBoundingVertexSmart (float x, float y)
  {
    if (x < minbox.x) minbox.x = x; else if (x > maxbox.x) maxbox.x = x;
    if (y < minbox.y) minbox.y = y; else if (y > maxbox.y) maxbox.y = y;
  }






  void AddBoundingVertexSmart (const csVector2& v)
  {
    AddBoundingVertexSmart (v.x, v.y);
  }
# 327 "f:/source-wip/CS/include/csgeom/box.h"
  csBox2 () : minbox (1000000000., 1000000000.),
             maxbox (-1000000000., -1000000000.) {}


  csBox2 (const csVector2& v) : minbox (v.x, v.y), maxbox (v.x, v.y) {}


  csBox2 (float x1, float y1, float x2, float y2) :
    minbox (x1, y1), maxbox (x2, y2)
  { if (Empty ()) StartBoundingBox (); }


  void Set (const csVector2& bmin, const csVector2& bmax)
  {
    minbox = bmin;
    maxbox = bmax;
  }


  void Set (float x1, float y1, float x2, float y2)
  {
    if (x1>x2 || y1>y2) StartBoundingBox();
    else { minbox.x = x1; minbox.y = y1; maxbox.x = x2; maxbox.y = y2; }
  }


  void SetMin (int idx, float val)
  {
    if (idx == 1) minbox.y = val;
    else minbox.x = val;
  }


  void SetMax (int idx, float val)
  {
    if (idx == 1) maxbox.y = val;
    else maxbox.x = val;
  }


  csBox2& operator+= (const csBox2& box);

  csBox2& operator+= (const csVector2& point);

  csBox2& operator*= (const csBox2& box);

  bool TestIntersect (const csBox2& box) const;


  friend csBox2 operator+ (const csBox2& box1, const csBox2& box2);

  friend csBox2 operator+ (const csBox2& box, const csVector2& point);

  friend csBox2 operator* (const csBox2& box1, const csBox2& box2);


  friend bool operator== (const csBox2& box1, const csBox2& box2);

  friend bool operator!= (const csBox2& box1, const csBox2& box2);

  friend bool operator< (const csBox2& box1, const csBox2& box2);

  friend bool operator> (const csBox2& box1, const csBox2& box2);

  friend bool operator< (const csVector2& point, const csBox2& box);
};
# 497 "f:/source-wip/CS/include/csgeom/box.h"
class csBox3
{
protected:

  csVector3 minbox;

  csVector3 maxbox;

  struct bEdge
  {
    uint8 v1, v2;
    uint8 fl, fr;
  };

  typedef uint8 bFace[4];




  static bEdge edges[24];

  static bFace faces[6];
public:

  float MinX () const { return minbox.x; }

  float MinY () const { return minbox.y; }

  float MinZ () const { return minbox.z; }

  float MaxX () const { return maxbox.x; }

  float MaxY () const { return maxbox.y; }

  float MaxZ () const { return maxbox.z; }

  float Min (int idx) const
  { return idx == 1 ? minbox.y : idx == 0 ? minbox.x : minbox.z; }

  float Max (int idx) const
  { return idx == 1 ? maxbox.y : idx == 0 ? maxbox.x : maxbox.z; }

  const csVector3& Min () const { return minbox; }

  const csVector3& Max () const { return maxbox; }
# 551 "f:/source-wip/CS/include/csgeom/box.h"
  csVector3 GetCorner (int corner) const;





  void GetEdgeInfo (int edge, int& v1, int& v2, int& fleft, int& fright) const
  {
    v1 = edges[edge].v1;
    v2 = edges[edge].v2;
    fleft = edges[edge].fl;
    fright = edges[edge].fr;
  }





  uint8* GetFaceEdges (int face) const
  {
    return faces[face];
  }




  csVector3 GetCenter () const { return (minbox+maxbox)/2; }





  void SetCenter (const csVector3& c);




  void SetSize (const csVector3& s);





  csBox2 GetSide (int side) const;







  int GetVisibleSides (const csVector3& pos, int* visible_sides) const;





  static int OtherSide (int side)
  {
    return side ^ 1;
  }






  csSegment3 GetEdge (int edge) const
  {
    return csSegment3 (GetCorner (edges[edge].v1), GetCorner (edges[edge].v2));
  }






  void GetEdge (int edge, csSegment3& e) const
  {
    e.SetStart (GetCorner (edges[edge].v1));
    e.SetEnd (GetCorner (edges[edge].v2));
  }


  bool In (float x, float y, float z) const
  {
    if (x < minbox.x || x > maxbox.x) return false;
    if (y < minbox.y || y > maxbox.y) return false;
    if (z < minbox.z || z > maxbox.z) return false;
    return true;
  }


  bool In (const csVector3& v) const
  {
    return In (v.x, v.y, v.z);
  }


  bool Overlap (const csBox3& box) const
  {
    if (maxbox.x < box.minbox.x || minbox.x > box.maxbox.x) return false;
    if (maxbox.y < box.minbox.y || minbox.y > box.maxbox.y) return false;
    if (maxbox.z < box.minbox.z || minbox.z > box.maxbox.z) return false;
    return true;
  }


  bool Contains (const csBox3& box) const
  {
    return (box.minbox.x >= minbox.x && box.maxbox.x <= maxbox.x) &&
           (box.minbox.y >= minbox.y && box.maxbox.y <= maxbox.y) &&
           (box.minbox.z >= minbox.z && box.maxbox.z <= maxbox.z);
  }


  bool Empty () const
  {
    if (minbox.x > maxbox.x) return true;
    if (minbox.y > maxbox.y) return true;
    if (minbox.z > maxbox.z) return true;
    return false;
  }


  void StartBoundingBox ()
  {
    minbox.x = 1000000000.;
    minbox.y = 1000000000.;
    minbox.z = 1000000000.;
    maxbox.x = -1000000000.;
    maxbox.y = -1000000000.;
    maxbox.z = -1000000000.;
  }


  void StartBoundingBox (const csVector3& v)
  {
    minbox = v; maxbox = v;
  }


  void AddBoundingVertex (float x, float y, float z)
  {
    if (x < minbox.x) minbox.x = x; if (x > maxbox.x) maxbox.x = x;
    if (y < minbox.y) minbox.y = y; if (y > maxbox.y) maxbox.y = y;
    if (z < minbox.z) minbox.z = z; if (z > maxbox.z) maxbox.z = z;
  }


  void AddBoundingVertex (const csVector3& v)
  {
    AddBoundingVertex (v.x, v.y, v.z);
  }






  void AddBoundingVertexSmart (float x, float y, float z)
  {
    if (x < minbox.x) minbox.x = x; else if (x > maxbox.x) maxbox.x = x;
    if (y < minbox.y) minbox.y = y; else if (y > maxbox.y) maxbox.y = y;
    if (z < minbox.z) minbox.z = z; else if (z > maxbox.z) maxbox.z = z;
  }






  void AddBoundingVertexSmart (const csVector3& v)
  {
    AddBoundingVertexSmart (v.x, v.y, v.z);
  }
# 743 "f:/source-wip/CS/include/csgeom/box.h"
  csBox3 () :
    minbox ( 1000000000.,
             1000000000.,
             1000000000.),
    maxbox (-1000000000.,
            -1000000000.,
            -1000000000.) {}


  csBox3 (const csVector3& v) : minbox (v), maxbox (v) { }


  csBox3 (const csVector3& v1, const csVector3& v2) :
        minbox (v1), maxbox (v2)
  { if (Empty ()) StartBoundingBox (); }


  csBox3 (float x1, float y1, float z1, float x2, float y2, float z2) :
    minbox (x1, y1, z1), maxbox (x2, y2, z2)
  { if (Empty ()) StartBoundingBox (); }


  void Set (const csVector3& bmin, const csVector3& bmax)
  {
    minbox = bmin;
    maxbox = bmax;
  }


  void Set (float x1, float y1, float z1, float x2, float y2, float z2)
  {
    if (x1>x2 || y1>y2 || z1>z2) StartBoundingBox();
    else
    {
      minbox.x = x1; minbox.y = y1; minbox.z = z1;
      maxbox.x = x2; maxbox.y = y2; maxbox.z = z2;
    }
  }


  void SetMin (int idx, float val)
  {
    if (idx == 1) minbox.y = val;
    else if (idx == 0) minbox.x = val;
    else minbox.z = val;
  }


  void SetMax (int idx, float val)
  {
    if (idx == 1) maxbox.y = val;
    else if (idx == 0) maxbox.x = val;
    else maxbox.z = val;
  }




  bool AdjacentX (const csBox3& other) const;




  bool AdjacentY (const csBox3& other) const;




  bool AdjacentZ (const csBox3& other) const;







  int Adjacent (const csBox3& other) const;







  int CalculatePointSegment (const csVector3& pos) const;
# 837 "f:/source-wip/CS/include/csgeom/box.h"
  void GetConvexOutline (const csVector3& pos,
        csVector3* array, int& num_array, bool bVisible=false) const;




  bool Between (const csBox3& box1, const csBox3& box2) const;





  void ManhattanDistance (const csBox3& other, csVector3& dist) const;





  float SquaredOriginDist () const;






  float SquaredOriginMaxDist () const;
# 875 "f:/source-wip/CS/include/csgeom/box.h"
  bool ProjectBox (const csTransform& trans, float fov, float sx, float sy,
        csBox2& sbox, float& min_z, float& max_z) const;


  csBox3& operator+= (const csBox3& box);

  csBox3& operator+= (const csVector3& point);

  csBox3& operator*= (const csBox3& box);

  bool TestIntersect (const csBox3& box) const;


  friend csBox3 operator+ (const csBox3& box1, const csBox3& box2);

  friend csBox3 operator+ (const csBox3& box, const csVector3& point);

  friend csBox3 operator* (const csBox3& box1, const csBox3& box2);


  friend bool operator== (const csBox3& box1, const csBox3& box2);

  friend bool operator!= (const csBox3& box1, const csBox3& box2);

  friend bool operator< (const csBox3& box1, const csBox3& box2);

  friend bool operator> (const csBox3& box1, const csBox3& box2);

  friend bool operator< (const csVector3& point, const csBox3& box);
};
# 32 "f:/source-wip/CS/include/iengine/fview.h" 2
# 1 "f:/source-wip/CS/include/iengine/shadows.h" 1
# 30 "f:/source-wip/CS/include/iengine/shadows.h"
# 1 "f:/source-wip/CS/include/csgeom/frustum.h" 1
# 29 "f:/source-wip/CS/include/csgeom/frustum.h"
# 1 "f:/source-wip/CS/include/csgeom/math3d.h" 1
# 35 "f:/source-wip/CS/include/csgeom/math3d.h"
# 1 "f:/source-wip/CS/include/csgeom/plane2.h" 1
# 36 "f:/source-wip/CS/include/csgeom/plane2.h"
class csPoly2D;






class csPlane2
{
public:

  csVector2 norm;


  float CC;


  csPlane2 () : norm (0,1), CC (0) {}


  csPlane2 (const csVector2& plane_norm, float c=0) : norm (plane_norm), CC (c) {}


  csPlane2 (float a, float b, float c=0) : norm (a,b), CC (c) {}


  inline void Set (const csVector2& v1, const csVector2& v2)
  {
    norm.x = v2.y-v1.y;
    norm.y = -(v2.x-v1.x);
    CC = - (v2 * norm);
  }


  inline void Set (const csSegment2& s)
  {
    Set (s.Start (), s.End ());
  }


  csPlane2 (const csVector2& v1, const csVector2& v2)
  {
    Set (v1, v2);
  }


  csPlane2 (const csSegment2& s)
  {
    Set (s);
  }


  inline csVector2& Normal () { return norm; }


  inline csVector2 GetNormal () const { return norm; }


  inline float A () const { return norm.x; }

  inline float B () const { return norm.y; }

  inline float C () const { return CC; }


  inline float& A () { return norm.x; }

  inline float& B () { return norm.y; }

  inline float& C () { return CC; }


  inline void Set (float a, float b, float c)
  { norm.x = a; norm.y = b; CC = c; }


  inline float Classify (const csVector2& pt) const { return norm*pt+CC; }


  static float Classify (float A, float B, float C,
                         const csVector2& pt)
  { return A*pt.x + B*pt.y + C; }






  inline float Distance (const csVector2& pt) const
  { return ((Classify (pt))<0?-(Classify (pt)):(Classify (pt))); }







  inline float SquaredDistance (const csVector2& pt) const
  {
    return Classify (pt) / norm.SquaredNorm ();
  }


  void Invert () { norm = -norm; CC = -CC; }


  void Normalize ()
  {
    float f = norm.Norm ();
    if (f) { norm /= f; CC /= f; }
  }
};
# 36 "f:/source-wip/CS/include/csgeom/math3d.h" 2

# 1 "f:/source-wip/CS/include/iutil/dbghelp.h" 1
# 29 "f:/source-wip/CS/include/iutil/dbghelp.h"
struct iString;
struct iGraphics3D;

const int iDebugHelper_VERSION = ((0 << 24) | (0 << 16) | 1); inline static scfInterfaceID iDebugHelper_scfGetID () { static scfInterfaceID ID = (scfInterfaceID)-1; if (ID == (scfInterfaceID)(-1)) ID = iSCF::SCF->GetInterfaceID ("iDebugHelper"); return ID; };
# 62 "f:/source-wip/CS/include/iutil/dbghelp.h"
struct iDebugHelper : public iBase
{
# 76 "f:/source-wip/CS/include/iutil/dbghelp.h"
  virtual int GetSupportedTests () const = 0;







  virtual csPtr<iString> UnitTest () = 0;







  virtual csPtr<iString> StateTest () = 0;





  virtual csTicks Benchmark (int num_iterations) = 0;





  virtual csPtr<iString> Dump () = 0;





  virtual void Dump (iGraphics3D* g3d) = 0;






  virtual bool DebugCommand (const char* cmd) = 0;
};
# 38 "f:/source-wip/CS/include/csgeom/math3d.h" 2

class csDVector3;
class csPoly3D;
class csBox3;

inline float fSqr (float f)
{
  return f * f;
}





class csMath3
{
public:
# 67 "f:/source-wip/CS/include/csgeom/math3d.h"
  static int WhichSide3D (const csVector3& p,
                          const csVector3& v1, const csVector3& v2)
  {

    float s = p.x*(v1.y*v2.z-v1.z*v2.y) + p.y*(v1.z*v2.x-v1.x*v2.z) +
              p.z*(v1.x*v2.y-v1.y*v2.x);
    if (s < 0) return 1;
    else if (s > 0) return -1;
    else return 0;
  }






  static bool Visible (const csVector3& p, const csVector3& t1,
                       const csVector3& t2, const csVector3& t3);






  static bool Visible (const csVector3& p, const csPlane3& pl)
  { return pl.Classify (p) <= 0; }
# 101 "f:/source-wip/CS/include/csgeom/math3d.h"
  static bool FindIntersection(const csVector3 tri1[3],
                               const csVector3 tri2[3],
                               csVector3 line[2]);
# 114 "f:/source-wip/CS/include/csgeom/math3d.h"
  static void Between (const csVector3& v1, const csVector3& v2, csVector3& v,
                       float pct, float wid);







  static void SetMinMax (const csVector3& v,
                         csVector3& min, csVector3& max)
  {
    if (v.x > max.x) max.x = v.x; else if (v.x < min.x ) min.x = v.x;
    if (v.y > max.y) max.y = v.y; else if (v.y < min.y ) min.y = v.y;
    if (v.z > max.z) max.z = v.z; else if (v.z < min.z ) min.z = v.z;
  }






  inline static float Area3 (const csVector3 &a, const csVector3 &b,
                             const csVector3 &c)
  {
    csVector3 v1 = b - a;
    csVector3 v2 = c - a;
    return ((v1.y * v2.z + v1.z * v2.x + v1.x * v2.y) -
            (v1.y * v2.x + v1.x * v2.z + v1.z * v2.y));
  }






  inline static void CalcNormal (csVector3& norm, const csVector3& v1,
                                 const csVector3& v2, const csVector3& v3)
  {
    norm = (v1-v2)%(v1-v3);
  }






  static void CalcNormal (csVector3& norm,
                          const csVector3& v, const csVector3& u)
  { norm = u%v; }







  static void CalcPlane (const csVector3& v1, const csVector3& v2,
         const csVector3& v3, csVector3& normal, float& D)
  {
    CalcNormal (normal, v1, v2, v3);
    D = - (normal * v1);
  }







  static bool PlanesEqual (const csPlane3& p1, const csPlane3& p2)
  {
    return ( ( p1.norm - p2.norm) < (float).001 ) &&
             ( ((p1.DD-p2.DD)<0?-(p1.DD-p2.DD):(p1.DD-p2.DD)) < (float).001 );
  }






  static bool PlanesClose (const csPlane3& p1, const csPlane3& p2);
# 204 "f:/source-wip/CS/include/csgeom/math3d.h"
  static int OuterPlanes (const csBox3& box1, const csBox3& box2,
    csPlane3* planes);
# 214 "f:/source-wip/CS/include/csgeom/math3d.h"
  static int FindObserverSides (const csBox3& box1, const csBox3& box2,
        int* sides);






  static void SpherePosition (float angle_xz, float angle_vert,
        csVector3& pos);
};





class csSquaredDist
{
public:

  static float PointPoint (const csVector3& p1, const csVector3& p2)
  { return fSqr (p1.x - p2.x) + fSqr (p1.y - p2.y) + fSqr (p1.z - p2.z); }


  static float PointLine (const csVector3& p,
                          const csVector3& l1, const csVector3& l2);


  static float PointPlane (const csVector3& p, const csPlane3& plane)
  { float r = plane.Classify (p); return r * r; }







  static float PointPoly (const csVector3& p, csVector3 *V, int n,
                          const csPlane3& plane, float sqdist = -1);
};






class csIntersect3
{
public:






  static bool IntersectPolygon (const csPlane3& plane, csPoly3D* poly,
        csSegment3& segment);
# 281 "f:/source-wip/CS/include/csgeom/math3d.h"
  static int IntersectSegment (csPlane3* planes, int num_planes,
        csSegment3& seg);






  static bool IntersectTriangle (const csVector3& tr1,
        const csVector3& tr2, const csVector3& tr3,
        const csSegment3& seg, csVector3& isect);





  static bool Plane (
    const csVector3& u, const csVector3& v,
    const csVector3& normal, const csVector3& a,
    csVector3& isect, float& dist);
# 310 "f:/source-wip/CS/include/csgeom/math3d.h"
  static bool Plane (
    const csVector3& u, const csVector3& v,
    const csPlane3& p,
    csVector3& isect,
    float& dist);






  static bool Planes (const csPlane3& p1, const csPlane3& p2,
        const csPlane3& p3, csVector3& isect);







  static bool PlaneXPlane (const csPlane3& p1, float x2, csPlane2& isect);







  static bool PlaneYPlane (const csPlane3& p1, float y2, csPlane2& isect);







  static bool PlaneZPlane (const csPlane3& p1, float z2, csPlane2& isect);







  static bool PlaneAxisPlane (const csPlane3& p1, int nr, float pos,
        csPlane2& isect)
  {
    switch (nr)
    {
      case 0: return PlaneXPlane (p1, pos, isect);
      case 1: return PlaneYPlane (p1, pos, isect);
      case 2: return PlaneZPlane (p1, pos, isect);
    }
    return false;
  }







  static float Z0Plane (
    const csVector3& v1, const csVector3& v2,
    csVector3& isect);







  static float Z0Plane (
    const csSegment3& uv,
    csVector3& isect)
  {
    return Z0Plane (uv.Start (), uv.End (), isect);
  }







  static float ZPlane (float zval,
    const csVector3& u, const csVector3& v,
    csVector3& isect);







  static float ZPlane (float zval,
    const csSegment3& uv,
    csVector3& isect)
  {
    return ZPlane (zval, uv.Start (), uv.End (), isect);
  }





  static float XFrustum (
    float A, const csVector3& u, const csVector3& v, csVector3& isect);





  static float XFrustum (
    float A, const csSegment3& uv, csVector3& isect)
  {
    return XFrustum (A, uv.Start (), uv.End (), isect);
  }





  static float YFrustum (
    float B, const csVector3& u, const csVector3& v, csVector3& isect);





  static float YFrustum (
    float B, const csSegment3& uv, csVector3& isect)
  {
    return YFrustum (B, uv.Start (), uv.End (), isect);
  }
# 455 "f:/source-wip/CS/include/csgeom/math3d.h"
  static int BoxSegment (const csBox3& box, const csSegment3& segment,
        csVector3& isect, float* pr = 0);
# 467 "f:/source-wip/CS/include/csgeom/math3d.h"
  static bool BoxFrustum (const csBox3& box, csPlane3* frustum,
        uint32 inClipMask, uint32& outClipMask);





  static bool BoxSphere (const csBox3& box, const csVector3& center,
                  float sqradius);
};





class csGeomDebugHelper : public iDebugHelper
{
public:
  csGeomDebugHelper ();
  virtual ~csGeomDebugHelper () { }

  int scfRefCount; public: iBase *scfParent; virtual void IncRef (); virtual void DecRef (); virtual int GetRefCount (); virtual void *QueryInterface (scfInterfaceID iInterfaceID, int iVersion);
  virtual int GetSupportedTests () const
  {
    return 1;
  }
  virtual csPtr<iString> UnitTest ();
  virtual csPtr<iString> StateTest ()
  {
    return 0;
  }
  virtual csTicks Benchmark (int )
  {
    return 0;
  }
  virtual csPtr<iString> Dump ()
  {
    return 0;
  }
  virtual void Dump (iGraphics3D* )
  {
  }
  virtual bool DebugCommand (const char*)
  {
    return false;
  }
};
# 30 "f:/source-wip/CS/include/csgeom/frustum.h" 2
# 1 "f:/source-wip/CS/include/csgeom/vtpool.h" 1
# 34 "f:/source-wip/CS/include/csgeom/vtpool.h"
class csVertexArrayPool
{
public:

  virtual ~csVertexArrayPool () { }





  virtual csVector3* GetVertexArray (int n) = 0;





  virtual void FreeVertexArray (csVector3* ar, int n) = 0;
};






class csDefaultVertexArrayPool : public csVertexArrayPool
{
public:
  csDefaultVertexArrayPool ();





  virtual csVector3* GetVertexArray (int n)
  {
    return new csVector3[n];
  }


  virtual void FreeVertexArray (csVector3* ar, int)
  {
    delete[] ar;
  }


  static csDefaultVertexArrayPool *default_pool; static csDefaultVertexArrayPool &GetDefaultPool ();
};







class csStackedVertexArrayPool : public csVertexArrayPool
{
private:
  csVector3* pool;
  int lastn, maxn;

public:

  csStackedVertexArrayPool (int maxn)
  {
    csStackedVertexArrayPool::maxn = maxn;
    lastn = 0;
    pool = new csVector3[maxn];
  }


  virtual ~csStackedVertexArrayPool ()
  {
    delete[] pool;
  }


  virtual csVector3* GetVertexArray (int n)
  {
    if (lastn+n > maxn) return 0;
    lastn += n;
    return pool+lastn-n;
  }


  virtual void FreeVertexArray (csVector3* ar, int n)
  {
    if (ar == pool+lastn-n) lastn -= n;
  }


  void Clear ()
  {
    lastn = 0;
  }
};






class csPooledVertexArrayPool : public csVertexArrayPool
{
private:
  struct PoolEl
  {
    PoolEl* next;
    int n;
    csVector3 first_vertex;
  };

  PoolEl* pool[6];
  PoolEl* miscpool;

public:

  csPooledVertexArrayPool ();


  virtual ~csPooledVertexArrayPool ();


  virtual csVector3* GetVertexArray (int n);


  virtual void FreeVertexArray (csVector3* ar, int n);


  static csPooledVertexArrayPool *default_pool; static csPooledVertexArrayPool &GetDefaultPool ();
};
# 31 "f:/source-wip/CS/include/csgeom/frustum.h" 2

class csTransform;
# 55 "f:/source-wip/CS/include/csgeom/frustum.h"
struct csClipInfo
{



  int type;
  union
  {
    struct { int idx; } original;
    struct { int i1, i2; float r; } onedge;
    struct { csClipInfo* ci1, * ci2; float r; } inside;
  };

  csClipInfo () : type (0) { }
  void Clear ();
  ~csClipInfo () { Clear (); }


  void Copy (csClipInfo& other)
  {
    if (&other == this) return;
    Clear ();
    type = other.type;
    if (type == 2)
    {
      inside.r = other.inside.r;
      inside.ci1 = new csClipInfo ();
      inside.ci1->Copy (*other.inside.ci1);
      inside.ci2 = new csClipInfo ();
      inside.ci2->Copy (*other.inside.ci2);
    }
    else if (type == 0)
      original.idx = other.original.idx;
    else
      onedge = other.onedge;
  }


  void Move (csClipInfo& other)
  {
    if (&other == this) return;
    Clear ();
    type = other.type;
    if (type == 2)
      inside = other.inside;
    else if (type == 0)
      original.idx = other.original.idx;
    else
      onedge = other.onedge;
    other.type = 0;
  }

  void Dump (int indent)
  {
    char ind[255];
    int i;
    for (i = 0 ; i < indent ; i++) ind[i] = ' ';
    ind[i] = 0;
    switch (type)
    {
      case 0:
        printf ("%s ORIGINAL idx=%d\n", ind, original.idx);
  break;
      case 1:
        printf ("%s ONEDGE i1=%d i2=%d r=%g\n", ind, onedge.i1, onedge.i2,
    onedge.r);
        break;
      case 2:
        printf ("%s INSIDE r=%g\n", ind, inside.r);
  inside.ci1->Dump (indent+2);
  inside.ci2->Dump (indent+2);
  break;
    }
    fflush ((&_iob[1]));
  }
};
# 142 "f:/source-wip/CS/include/csgeom/frustum.h"
class csFrustum
{
private:

  csVertexArrayPool* pool;


  csVector3 origin;






  csVector3* vertices;

  int num_vertices;

  int max_vertices;


  csPlane3* backplane;
# 172 "f:/source-wip/CS/include/csgeom/frustum.h"
  bool wide;





  bool mirrored;


  int ref_count;


  void Clear ();


  void ExtendVertexArray (int num);

public:


  csFrustum (const csVector3& o) : pool (&csDefaultVertexArrayPool::GetDefaultPool()),
    origin (o), vertices (0), num_vertices (0), max_vertices (0),
  backplane (0), wide (false), mirrored (false), ref_count (1)
  { }


  csFrustum (const csVector3& o, csVertexArrayPool* pl) : pool (pl),
    origin (o), vertices (0), num_vertices (0), max_vertices (0),
  backplane (0), wide (false), mirrored (false), ref_count (1)
  { }






  csFrustum (const csVector3& o, csVector3* verts, int num_verts,
        csPlane3* backp = 0);






  csFrustum (const csVector3& o, int num_verts,
        csVertexArrayPool* pl, csPlane3* backp = 0);


  csFrustum (const csFrustum &copy);


  virtual ~csFrustum ();


  void SetOrigin (const csVector3& o) { origin = o; }


  csVector3& GetOrigin () { return origin; }


  const csVector3& GetOrigin () const { return origin; }






  void SetMirrored (bool m) { mirrored = m; }


  bool IsMirrored () { return mirrored; }







  void SetBackPlane (csPlane3& plane);




  csPlane3* GetBackPlane () { return backplane; }




  void RemoveBackPlane ();




  void AddVertex (const csVector3& v);




  int GetVertexCount () { return num_vertices; }




  csVector3& GetVertex (int idx)
  {
    ;
    return vertices[idx];
  }




  csVector3* GetVertices () { return vertices; }




  void Transform (csTransform* trans);






  void ClipToPlane (csVector3& v1, csVector3& v2);
# 306 "f:/source-wip/CS/include/csgeom/frustum.h"
  static void ClipToPlane (csVector3* vertices, int& num_vertices,
  csClipInfo* clipinfo, const csVector3& v1, const csVector3& v2);
# 317 "f:/source-wip/CS/include/csgeom/frustum.h"
  static void ClipToPlane (csVector3* vertices, int& num_vertices,
  csClipInfo* clipinfo, const csPlane3& plane);







  void ClipPolyToPlane (csPlane3* plane);
# 336 "f:/source-wip/CS/include/csgeom/frustum.h"
  csPtr<csFrustum> Intersect (const csFrustum& other);
# 352 "f:/source-wip/CS/include/csgeom/frustum.h"
  csPtr<csFrustum> Intersect (csVector3* poly, int num);
# 368 "f:/source-wip/CS/include/csgeom/frustum.h"
  static csPtr<csFrustum> Intersect (
    const csVector3& frust_origin, csVector3* frust, int num_frust,
    csVector3* poly, int num);
# 386 "f:/source-wip/CS/include/csgeom/frustum.h"
  static csPtr<csFrustum> Intersect (
    const csVector3& frust_origin, csVector3* frust, int num_frust,
    const csVector3& v1, const csVector3& v2, const csVector3& v3);






  static int Classify (csVector3* frustum, int num_frust,
    csVector3* poly, int num_poly);





  static int BatchClassify (csVector3* frustum, csVector3* frustumNormals, int num_frust,
          csVector3* poly, int num_poly);





  bool Contains (const csVector3& point);







  static bool Contains (csVector3* frustum, int num_frust,
    const csVector3& point);






  static bool Contains (csVector3* frustum, int num_frust,
    const csPlane3& plane, const csVector3& point);


  bool IsEmpty () const { return !wide && vertices == 0; }


  bool IsInfinite () const { return wide && vertices == 0 && backplane == 0; }


  bool IsWide () const { return wide && vertices == 0; }





  void MakeInfinite ();




  void MakeEmpty ();


  void IncRef () { ref_count++; }

  void DecRef () { if (ref_count == 1) delete this; else ref_count--; }
};
# 31 "f:/source-wip/CS/include/iengine/shadows.h" 2

struct iShadowBlock;
struct iShadowBlockList;
class csTransform;
class csPlane3;
class csVector3;

const int iShadowIterator_VERSION = ((0 << 24) | (0 << 16) | 1); inline static scfInterfaceID iShadowIterator_scfGetID () { static scfInterfaceID ID = (scfInterfaceID)-1; if (ID == (scfInterfaceID)(-1)) ID = iSCF::SCF->GetInterfaceID ("iShadowIterator"); return ID; };





struct iShadowIterator : public iBase
{

  virtual void Reset () = 0;

  virtual bool HasNext () = 0;

  virtual csFrustum* Next () = 0;

  virtual void* GetUserData () = 0;

  virtual bool IsRelevant () = 0;

  virtual void MarkRelevant (bool rel) = 0;

  virtual void DeleteCurrent () = 0;

  virtual iShadowBlock* GetCurrentShadowBlock () = 0;

  virtual iShadowBlock* GetNextShadowBlock () = 0;
};

const int iShadowBlock_VERSION = ((0 << 24) | (0 << 16) | 3); inline static scfInterfaceID iShadowBlock_scfGetID () { static scfInterfaceID ID = (scfInterfaceID)-1; if (ID == (scfInterfaceID)(-1)) ID = iSCF::SCF->GetInterfaceID ("iShadowBlock"); return ID; };





struct iShadowBlock : public iBase
{

  virtual iShadowIterator* GetShadowIterator (bool reverse = false) = 0;

  virtual void DeleteShadows () = 0;







  virtual void AddRelevantShadows (iShadowBlock* source,
    csTransform* trans = 0) = 0;






  virtual void AddRelevantShadows (iShadowBlockList* source) = 0;






  virtual void AddAllShadows (iShadowBlockList* source) = 0;






  virtual void AddUniqueRelevantShadows (iShadowBlockList* source) = 0;






  virtual csFrustum* AddShadow (const csVector3& origin, void* userData,
    int num_verts, csPlane3& backplane) = 0;


  virtual void UnlinkShadow (int idx) = 0;


  virtual int GetShadowCount () = 0;


  virtual csFrustum* GetShadow (int idx) = 0;




  virtual void Transform (csTransform* trans) = 0;


  virtual const csBox3& GetBoundingBox () = 0;
};

const int iShadowBlockList_VERSION = ((0 << 24) | (0 << 16) | 5); inline static scfInterfaceID iShadowBlockList_scfGetID () { static scfInterfaceID ID = (scfInterfaceID)-1; if (ID == (scfInterfaceID)(-1)) ID = iSCF::SCF->GetInterfaceID ("iShadowBlockList"); return ID; };





struct iShadowBlockList : public iBase
{

  virtual iShadowIterator* GetShadowIterator (bool reverse = false) = 0;






  virtual iShadowIterator* GetShadowIterator (
    const csBox3& bbox, bool reverse = false) = 0;


  virtual iShadowBlock* NewShadowBlock (int num_shadows = 30) = 0;


  virtual iShadowBlock* GetFirstShadowBlock () = 0;

  virtual iShadowBlock* GetLastShadowBlock () = 0;

  virtual iShadowBlock* GetNextShadowBlock (iShadowBlock* s) = 0;

  virtual iShadowBlock* GetPreviousShadowBlock (iShadowBlock* s) = 0;

  virtual void RemoveLastShadowBlock () = 0;

  virtual void DeleteAllShadows () = 0;







  virtual uint32 MarkNewRegion () = 0;





  virtual void RestoreRegion (uint32 prev) = 0;





  virtual bool FromCurrentRegion (iShadowBlock* block) = 0;
};
# 33 "f:/source-wip/CS/include/iengine/fview.h" 2

struct iFrustumView;
struct iMeshWrapper;
class csFrustum;
class csFrustumContext;
class csObject;
class csOctreeNode;


const int iFrustumViewUserdata_VERSION = ((0 << 24) | (0 << 16) | 1); inline static scfInterfaceID iFrustumViewUserdata_scfGetID () { static scfInterfaceID ID = (scfInterfaceID)-1; if (ID == (scfInterfaceID)(-1)) ID = iSCF::SCF->GetInterfaceID ("iFrustumViewUserdata"); return ID; };




struct iFrustumViewUserdata : public iBase
{
};





typedef void (csFrustumViewObjectFunc)(iMeshWrapper* mesh,
  iFrustumView* lview, bool vis);







class csFrustumContext
{
private:





  csRef<iShadowBlockList> shadows;




  bool shared;


  bool mirror;





  csRef<csFrustum> light_frustum;

public:

  csFrustumContext () :
    shared (false),
    mirror (false)
  { }

  csFrustumContext& operator= (csFrustumContext const& c)
  {
    shadows = c.shadows;
    shared = c.shared;
    mirror = c.mirror;
    light_frustum = c.light_frustum;
    return *this;
  }


  iShadowBlockList* GetShadows () { return shadows; }

  void SetShadows (iShadowBlockList* shad, bool sh = true)
  {
    shadows = shad;
    shared = sh;
  }

  void SetNewShadows (csPtr<iShadowBlockList> shad, bool sh = false)
  {
    shadows = shad;
    shared = sh;
  }

  bool IsShared () { return shared; }


  void SetLightFrustum (csFrustum* lf) { light_frustum = lf; }

  void SetNewLightFrustum (csPtr<csFrustum> lf) { light_frustum = lf; }

  csFrustum* GetLightFrustum () { return light_frustum; }





  void SetMirrored (bool m) { mirror = m; }

  bool IsMirrored () { return mirror; }
};

const int iFrustumView_VERSION = ((0 << 24) | (4 << 16) | 1); inline static scfInterfaceID iFrustumView_scfGetID () { static scfInterfaceID ID = (scfInterfaceID)-1; if (ID == (scfInterfaceID)(-1)) ID = iSCF::SCF->GetInterfaceID ("iFrustumView"); return ID; };





struct iFrustumView : public iBase
{

  virtual csFrustumContext* GetFrustumContext () const = 0;







  virtual void CreateFrustumContext () = 0;




  virtual csFrustumContext* CopyFrustumContext () = 0;




  virtual void SetFrustumContext (csFrustumContext* ctxt) = 0;





  virtual void RestoreFrustumContext (csFrustumContext* original) = 0;


  virtual void SetObjectFunction (csFrustumViewObjectFunc* func) = 0;

  virtual void CallObjectFunction (iMeshWrapper* mesh, bool vis) = 0;


  virtual float GetRadius () const = 0;

  virtual float GetSquaredRadius () const = 0;

  virtual bool ThingShadowsEnabled () = 0;

  virtual bool CheckShadowMask (unsigned int mask) = 0;

  virtual bool CheckProcessMask (unsigned int mask) = 0;


  virtual void StartNewShadowBlock () = 0;


  virtual void SetUserdata (iFrustumViewUserdata* data) = 0;

  virtual iFrustumViewUserdata* GetUserdata () = 0;


  virtual csPtr<iShadowBlock> CreateShadowBlock () = 0;
};
# 31 "f:/source-wip/CS/include/iengine/light.h" 2

class csLight;
class csColor;
class csFlags;
struct iLight;
struct iSector;
struct iObject;
struct iCrossHalo;
struct iNovaHalo;
struct iFlareHalo;
# 84 "f:/source-wip/CS/include/iengine/light.h"
const int iLightCallback_VERSION = ((0 << 24) | (2 << 16) | 0); inline static scfInterfaceID iLightCallback_scfGetID () { static scfInterfaceID ID = (scfInterfaceID)-1; if (ID == (scfInterfaceID)(-1)) ID = iSCF::SCF->GetInterfaceID ("iLightCallback"); return ID; };






struct iLightCallback : public iBase
{




  virtual void OnColorChange (iLight* light, const csColor& newcolor) = 0;





  virtual void OnPositionChange (iLight* light, const csVector3& newpos) = 0;





  virtual void OnSectorChange (iLight* light, iSector* newsector) = 0;





  virtual void OnRadiusChange (iLight* light, float newradius) = 0;





  virtual void OnDestroy (iLight* light) = 0;
};


const int iLight_VERSION = ((0 << 24) | (0 << 16) | 9); inline static scfInterfaceID iLight_scfGetID () { static scfInterfaceID ID = (scfInterfaceID)-1; if (ID == (scfInterfaceID)(-1)) ID = iSCF::SCF->GetInterfaceID ("iLight"); return ID; };
# 151 "f:/source-wip/CS/include/iengine/light.h"
struct iLight : public iBase
{

  virtual csLight* GetPrivateObject () = 0;


  virtual const char* GetLightID () = 0;


  virtual iObject *QueryObject() = 0;


  virtual const csVector3& GetCenter () = 0;

  virtual void SetCenter (const csVector3& pos) = 0;


  virtual iSector *GetSector () = 0;

  virtual void SetSector (iSector* sector) = 0;



  virtual float GetRadius () = 0;

  virtual float GetSquaredRadius () = 0;

  virtual float GetInverseRadius () = 0;

  virtual void SetRadius (float r) = 0;



  virtual const csColor& GetColor () = 0;

  virtual void SetColor (const csColor& col) = 0;


  virtual bool IsDynamic () const = 0;



  virtual int GetAttenuation () = 0;
# 204 "f:/source-wip/CS/include/iengine/light.h"
  virtual void SetAttenuation (int a) = 0;
# 256 "f:/source-wip/CS/include/iengine/light.h"
  virtual iCrossHalo* CreateCrossHalo (float intensity, float cross) = 0;

  virtual iNovaHalo* CreateNovaHalo (int seed, int num_spokes,
        float roundness) = 0;

  virtual iFlareHalo* CreateFlareHalo () = 0;


  virtual float GetBrightnessAtDistance (float d) = 0;
# 274 "f:/source-wip/CS/include/iengine/light.h"
  virtual csFlags& GetFlags () = 0;





  virtual void SetLightCallback (iLightCallback* cb) = 0;




  virtual void RemoveLightCallback (iLightCallback* cb) = 0;


  virtual int GetLightCallbackCount () const = 0;


  virtual iLightCallback* GetLightCallback (int idx) const = 0;





  virtual uint32 GetLightNumber () const = 0;
};

const int iLightList_VERSION = ((0 << 24) | (0 << 16) | 2); inline static scfInterfaceID iLightList_scfGetID () { static scfInterfaceID ID = (scfInterfaceID)-1; if (ID == (scfInterfaceID)(-1)) ID = iSCF::SCF->GetInterfaceID ("iLightList"); return ID; };




struct iLightList : public iBase
{

  virtual int GetCount () const = 0;


  virtual iLight *Get (int n) const = 0;


  virtual int Add (iLight *obj) = 0;


  virtual bool Remove (iLight *obj) = 0;


  virtual bool Remove (int n) = 0;


  virtual void RemoveAll () = 0;


  virtual int Find (iLight *obj) const = 0;


  virtual iLight *FindByName (const char *Name) const = 0;


  virtual iLight *FindByID (const char* id) const = 0;
};

const int iLightingProcessData_VERSION = ((0 << 24) | (0 << 16) | 1); inline static scfInterfaceID iLightingProcessData_scfGetID () { static scfInterfaceID ID = (scfInterfaceID)-1; if (ID == (scfInterfaceID)(-1)) ID = iSCF::SCF->GetInterfaceID ("iLightingProcessData"); return ID; };






struct iLightingProcessData : public iBase
{




  virtual void FinalizeLighting () = 0;
};

const int iLightingProcessInfo_VERSION = ((0 << 24) | (0 << 16) | 2); inline static scfInterfaceID iLightingProcessInfo_scfGetID () { static scfInterfaceID ID = (scfInterfaceID)-1; if (ID == (scfInterfaceID)(-1)) ID = iSCF::SCF->GetInterfaceID ("iLightingProcessInfo"); return ID; };







struct iLightingProcessInfo : public iFrustumViewUserdata
{

  virtual iLight* GetLight () const = 0;


  virtual bool IsDynamic () const = 0;


  virtual void SetColor (const csColor& col) = 0;


  virtual const csColor& GetColor () const = 0;






  virtual void AttachUserdata (iLightingProcessData* userdata) = 0;




  virtual csPtr<iLightingProcessData> QueryUserdata (scfInterfaceID id,
        int version) = 0;






  virtual void FinalizeLighting () = 0;
};

const int iLightIterator_VERSION = ((0 << 24) | (1 << 16) | 0); inline static scfInterfaceID iLightIterator_scfGetID () { static scfInterfaceID ID = (scfInterfaceID)-1; if (ID == (scfInterfaceID)(-1)) ID = iSCF::SCF->GetInterfaceID ("iLightIterator"); return ID; };







struct iLightIterator : public iBase
{

  virtual bool HasNext () = 0;


  virtual iLight* Next () = 0;


  virtual iSector* GetLastSector () = 0;


  virtual void Reset () = 0;

};
# 26 "lightiter.cpp" 2
# 1 "f:/source-wip/CS/include/iengine/rview.h" 1
# 32 "f:/source-wip/CS/include/iengine/rview.h"
# 1 "f:/source-wip/CS/include/iengine/engine.h" 1
# 34 "f:/source-wip/CS/include/iengine/engine.h"
class csEngine;
class csVector3;
class csFrustum;
class csMatrix3;
class csColor;
class csBox3;
struct csTextureLayer;

struct iSector;
struct iFrustumView;
struct iSectorIterator;
struct iObjectIterator;
struct iLight;
struct iLightIterator;
struct iStatLight;
struct iDynLight;
struct iSprite;
struct iMeshObject;
struct iMeshObjectFactory;
struct iMeshWrapper;
struct iMeshFactoryWrapper;
struct iMeshObjectType;
struct iMaterial;
struct iMaterialWrapper;
struct iMaterialList;
struct iTextureWrapper;
struct iTextureHandle;
struct iTextureList;
struct iCameraPosition;
struct iCameraPositionList;
struct iRegion;
struct iGraphics3D;
struct iClipper2D;
struct iObject;
struct iObjectWatcher;
struct iCollection;
struct iCollectionList;
struct iDataBuffer;
struct iCamera;
struct iRenderView;
struct iSectorList;
struct iMeshList;
struct iMeshFactoryList;
struct iProgressMeter;
struct iRegionList;
struct iLoaderContext;
struct iCacheManager;
struct iSharedVariableList;
struct iRenderLoopManager;
struct iRenderLoop;
# 145 "f:/source-wip/CS/include/iengine/engine.h"
const int iEngine_VERSION = ((0 << 24) | (17 << 16) | 0); inline static scfInterfaceID iEngine_scfGetID () { static scfInterfaceID ID = (scfInterfaceID)-1; if (ID == (scfInterfaceID)(-1)) ID = iSCF::SCF->GetInterfaceID ("iEngine"); return ID; };






struct iEngine : public iBase
{

  virtual iObject *QueryObject() = 0;
# 165 "f:/source-wip/CS/include/iengine/engine.h"
  virtual bool Prepare (iProgressMeter* meter = 0) = 0;
# 174 "f:/source-wip/CS/include/iengine/engine.h"
  virtual void PrepareTextures () = 0;






  virtual void PrepareMeshes () = 0;







  virtual void ShineLights (iRegion* region = 0,
        iProgressMeter* meter = 0) = 0;





  virtual int GetTextureFormat () const = 0;





  virtual void SelectRegion (const char* name) = 0;




  virtual void SelectRegion (iRegion* region) = 0;




  virtual iRegion* GetCurrentRegion () const = 0;




  virtual void AddToCurrentRegion (iObject* obj) = 0;


  virtual void DeleteAll () = 0;
# 235 "f:/source-wip/CS/include/iengine/engine.h"
  virtual void RegisterRenderPriority (const char* name, long priority,
        int rendsort = 0, bool do_camera = false) = 0;

  virtual long GetRenderPriority (const char* name) const = 0;

  virtual void SetRenderPriorityCamera (long priority, bool do_camera) = 0;

  virtual bool GetRenderPriorityCamera (const char* name) const = 0;

  virtual bool GetRenderPriorityCamera (long priority) const = 0;

  virtual int GetRenderPrioritySorting (const char* name) const = 0;

  virtual int GetRenderPrioritySorting (long priority) const = 0;

  virtual long GetSkyRenderPriority () const = 0;

  virtual long GetWallRenderPriority () const = 0;

  virtual long GetObjectRenderPriority () const = 0;

  virtual long GetAlphaRenderPriority () const = 0;

  virtual void ClearRenderPriorities () = 0;

  virtual int GetRenderPriorityCount () const = 0;

  virtual const char* GetRenderPriorityName (long priority) const = 0;





  virtual csPtr<iMaterial> CreateBaseMaterial (iTextureWrapper* txt) = 0;






  virtual csPtr<iMaterial> CreateBaseMaterial (iTextureWrapper* txt,
        int num_layers, iTextureWrapper** wrappers, csTextureLayer* layers) = 0;


  virtual iTextureWrapper* CreateTexture (const char *name,
        const char *fileName, csColor *transp, int flags) = 0;

  virtual iTextureWrapper* CreateBlackTexture (const char *name,
        int w, int h, csColor *iTransp, int iFlags) = 0;


  virtual iMaterialWrapper* CreateMaterial (const char *name,
        iTextureWrapper* texture) = 0;



  virtual iSector *CreateSector (const char *name) = 0;
# 301 "f:/source-wip/CS/include/iengine/engine.h"
  virtual csPtr<iMeshWrapper> CreateSectorWallsMesh (iSector* sector,
      const char* name) = 0;







  virtual csPtr<iMeshWrapper> CreateThingMesh (iSector* sector,
        const char* name) = 0;


  virtual iSectorList* GetSectors () = 0;

  virtual iMeshFactoryList* GetMeshFactories () = 0;

  virtual iMeshList* GetMeshes () = 0;

  virtual iCollectionList* GetCollections () = 0;

  virtual iCameraPositionList* GetCameraPositions () = 0;

  virtual iTextureList* GetTextureList () const = 0;

  virtual iMaterialList* GetMaterialList () const = 0;

  virtual iSharedVariableList* GetVariableList () const = 0;

  virtual iRegionList* GetRegions () = 0;
# 342 "f:/source-wip/CS/include/iengine/engine.h"
  virtual iMaterialWrapper* FindMaterial (const char* name,
        iRegion* region = 0) = 0;
# 354 "f:/source-wip/CS/include/iengine/engine.h"
  virtual iTextureWrapper* FindTexture (const char* name,
        iRegion* region = 0) = 0;
# 366 "f:/source-wip/CS/include/iengine/engine.h"
  virtual iSector* FindSector (const char* name,
        iRegion* region = 0) = 0;
# 378 "f:/source-wip/CS/include/iengine/engine.h"
  virtual iMeshWrapper* FindMeshObject (const char* name,
        iRegion* region = 0) = 0;
# 390 "f:/source-wip/CS/include/iengine/engine.h"
  virtual iMeshFactoryWrapper* FindMeshFactory (const char* name,
        iRegion* region = 0) = 0;
# 402 "f:/source-wip/CS/include/iengine/engine.h"
  virtual iCameraPosition* FindCameraPosition (const char* name,
        iRegion* region = 0) = 0;
# 414 "f:/source-wip/CS/include/iengine/engine.h"
  virtual iCollection* FindCollection (const char* name,
        iRegion* region = 0) = 0;
# 430 "f:/source-wip/CS/include/iengine/engine.h"
  virtual void SetLightingCacheMode (int mode) = 0;

  virtual int GetLightingCacheMode () = 0;







  virtual void SetFastMeshThresshold (int th) = 0;

  virtual int GetFastMeshThresshold () const = 0;
# 454 "f:/source-wip/CS/include/iengine/engine.h"
  virtual void SetClearZBuf (bool yesno) = 0;




  virtual bool GetClearZBuf () const = 0;

  virtual bool GetDefaultClearZBuf () const = 0;
# 473 "f:/source-wip/CS/include/iengine/engine.h"
  virtual void SetClearScreen (bool yesno) = 0;




  virtual bool GetClearScreen () const = 0;

  virtual bool GetDefaultClearScreen () const = 0;





  virtual void SetMaxLightmapSize(int w, int h) = 0;

  virtual void GetMaxLightmapSize(int& w, int& h) = 0;

  virtual void GetDefaultMaxLightmapSize(int& w, int& h) = 0;

  virtual bool GetLightmapsRequirePO2 () const = 0;

  virtual int GetMaxLightmapAspectRatio () const = 0;
# 503 "f:/source-wip/CS/include/iengine/engine.h"
  virtual void ResetWorldSpecificSettings() = 0;





  virtual csPtr<iCamera> CreateCamera () = 0;




  virtual csPtr<iStatLight> CreateLight (const char* name, const csVector3& pos,
        float radius, const csColor& color, bool pseudoDyn) = 0;

  virtual iStatLight* FindLight (const char *Name, bool RegionOnly = false)
    const = 0;




  virtual iStatLight* FindLightID (const char* light_id) const = 0;




  virtual csPtr<iLightIterator> GetLightIterator (iRegion* region = 0) = 0;
# 538 "f:/source-wip/CS/include/iengine/engine.h"
  virtual csPtr<iDynLight> CreateDynLight (const csVector3& pos, float radius,
        const csColor& color) = 0;

  virtual void RemoveDynLight (iDynLight*) = 0;

  virtual iDynLight* GetFirstDynLight () const = 0;







  virtual int GetBeginDrawFlags () const = 0;




  virtual iClipper2D* GetTopLevelClipper () const = 0;
# 569 "f:/source-wip/CS/include/iengine/engine.h"
  virtual csPtr<iMeshFactoryWrapper> CreateMeshFactory (const char* classId,
        const char* name) = 0;





  virtual csPtr<iMeshFactoryWrapper> CreateMeshFactory (iMeshObjectFactory *,
        const char* name) = 0;





  virtual csPtr<iMeshFactoryWrapper> CreateMeshFactory (const char* name) = 0;







  virtual csPtr<iLoaderContext> CreateLoaderContext (
        iRegion* region = 0) = 0;





  virtual csPtr<iMeshFactoryWrapper> LoadMeshFactory (
        const char* name, const char* loaderClassId,
        iDataBuffer* input) = 0;
# 610 "f:/source-wip/CS/include/iengine/engine.h"
  virtual csPtr<iMeshWrapper> CreateMeshWrapper (iMeshFactoryWrapper* factory,
        const char* name, iSector* sector = 0,
        const csVector3& pos = csVector3(0, 0, 0)) = 0;




  virtual csPtr<iMeshWrapper> CreateMeshWrapper (iMeshObject*,
        const char* name, iSector* sector = 0,
        const csVector3& pos = csVector3(0, 0, 0)) = 0;
# 631 "f:/source-wip/CS/include/iengine/engine.h"
  virtual csPtr<iMeshWrapper> CreateMeshWrapper (const char* classid,
        const char* name, iSector* sector = 0,
        const csVector3& pos = csVector3(0, 0, 0)) = 0;




  virtual csPtr<iMeshWrapper> CreateMeshWrapper (const char* name) = 0;






  virtual csPtr<iMeshWrapper> LoadMeshWrapper (
        const char* name, const char* loaderClassId,
        iDataBuffer* input, iSector* sector, const csVector3& pos) = 0;
# 656 "f:/source-wip/CS/include/iengine/engine.h"
  virtual void Draw (iCamera* c, iClipper2D* clipper) = 0;






  virtual void SetContext (iTextureHandle* ctxt) = 0;

  virtual iTextureHandle *GetContext () const = 0;





  virtual void SetAmbientLight (const csColor &) = 0;

  virtual void GetAmbientLight (csColor &) const = 0;
# 694 "f:/source-wip/CS/include/iengine/engine.h"
  virtual int GetNearbyLights (iSector* sector, const csVector3& pos,
        uint32 flags, iLight** lights, int max_num_lights) = 0;
# 716 "f:/source-wip/CS/include/iengine/engine.h"
  virtual int GetNearbyLights (iSector* sector, const csBox3& box,
        uint32 flags, iLight** lights, int max_num_lights) = 0;






  virtual csPtr<iSectorIterator> GetNearbySectors (iSector* sector,
        const csVector3& pos, float radius) = 0;
# 736 "f:/source-wip/CS/include/iengine/engine.h"
  virtual csPtr<iObjectIterator> GetNearbyObjects (iSector* sector,
    const csVector3& pos, float radius, bool crossPortals = true ) = 0;
# 748 "f:/source-wip/CS/include/iengine/engine.h"
  virtual csPtr<iObjectIterator> GetVisibleObjects (iSector* sector,
    const csVector3& pos) = 0;
# 759 "f:/source-wip/CS/include/iengine/engine.h"
  virtual csPtr<iObjectIterator> GetVisibleObjects (iSector* sector,
    const csFrustum& frustum) = 0;
# 777 "f:/source-wip/CS/include/iengine/engine.h"
  virtual bool RemoveObject (iBase* object) = 0;
# 786 "f:/source-wip/CS/include/iengine/engine.h"
  virtual void SetCacheManager (iCacheManager* cache_mgr) = 0;




  virtual iCacheManager* GetCacheManager () = 0;


  virtual void GetDefaultAmbientLight (csColor &c) const = 0;







  virtual csPtr<iFrustumView> CreateFrustumView () = 0;





  virtual csPtr<iObjectWatcher> CreateObjectWatcher () = 0;







  virtual void WantToDie (iMeshWrapper* mesh) = 0;
# 850 "f:/source-wip/CS/include/iengine/engine.h"
};
# 33 "f:/source-wip/CS/include/iengine/rview.h" 2


# 1 "f:/source-wip/CS/include/ivideo/graph3d.h" 1
# 34 "f:/source-wip/CS/include/ivideo/graph3d.h"
# 1 "f:/source-wip/CS/include/csgeom/tri.h" 1
# 33 "f:/source-wip/CS/include/csgeom/tri.h"
struct csTriangle
{
  int a, b, c;


  csTriangle () {}


  csTriangle (int _a, int _b, int _c) : a(_a), b(_b), c(_c) {}


  csTriangle (const csTriangle& t)
  {
    a = t.a;
    b = t.b;
    c = t.c;
  }


  csTriangle& operator= (const csTriangle& t)
  {
    a = t.a;
    b = t.b;
    c = t.c;
    return *this;
  }


  void Set (int _a, int _b, int _c)
  {
    a = _a;
    b = _b;
    c = _c;
  }
};
# 35 "f:/source-wip/CS/include/ivideo/graph3d.h" 2
# 1 "f:/source-wip/CS/include/csutil/cscolor.h" 1
# 27 "f:/source-wip/CS/include/csutil/cscolor.h"
class csColor
{
public:

  float red;

  float green;

  float blue;

public:

  csColor () { }

  csColor (float r, float g, float b)
  { red = r; green = g; blue = b; }

  csColor (const csColor& c)
  { red = c.red; green = c.green; blue = c.blue; }

  void Set (float r, float g, float b)
  { red = r; green = g; blue = b; }

  void Clamp (float r, float g, float b)
  {
    if (red > r) red = r;
    if (green > g) green = g;
    if (blue > b) blue = b;
  }

  void ClampDown ()
  {
    if (red < 0) red = 0;
    if (green < 0) green = 0;
    if (blue < 0) blue = 0;
  }

  csColor& operator= (const csColor& c)
  { red = c.red; green = c.green; blue = c.blue; return *this; }

  csColor& operator*= (float f)
  { red *= f; green *= f; blue *= f; return *this; }

  csColor& operator+= (const csColor& c)
  { red += c.red; green += c.green; blue += c.blue; return *this; }

  csColor& operator-= (const csColor& c)
  { red -= c.red; green -= c.green; blue -= c.blue; return *this; }

  void Add (float r, float g, float b)
  { red += r; green += g; blue += b; }

  void Subtract (float r, float g, float b)
  { red -= r; green -= g; blue -= b; }
};


inline csColor operator* (const csColor& s, float f)
{ csColor c (s); c *= f; return c; }


inline csColor operator* (float f, const csColor& s)
{ csColor c (s); c *= f; return c; }


inline csColor operator+ (const csColor& s1, const csColor& s2)
{ csColor c (s1); c += s2; return c; }

inline csColor operator- (const csColor& s1, const csColor& s2)
{ csColor c (s1); c -= s2; return c; }
# 36 "f:/source-wip/CS/include/ivideo/graph3d.h" 2

class csMatrix3;
class csVector3;
class csVector2;
class csPlane3;
class csRect;
class csReversibleTransform;

struct iGraphics2D;
struct iPolygonTexture;
struct iPolygonBuffer;
struct iVertexBuffer;
struct iVertexBufferManager;
struct iTextureManager;
struct iTextureHandle;
struct iMaterialHandle;
struct iMaterial;
struct iClipper2D;
struct iHalo;
struct csRGBpixel;
struct csPixelFormat;
# 119 "f:/source-wip/CS/include/ivideo/graph3d.h"
class G3DFogInfo
{
public:

  float r, g, b;






  float intensity;
  float intensity2;
};


class G3DTexturePlane
{
public:

  csMatrix3* m_cam2tex;

  csVector3* v_cam2tex;
};


struct G3DPolygonDPFX
{

  int num;

  csVector2 vertices[100];

  float z[100];

  csVector2 texels[100];

  csColor colors[100];


  G3DFogInfo fog_info[100];

  bool use_fog;


  iMaterialHandle *mat_handle;

  uint mixmode;


  uint8 flat_color_r;
  uint8 flat_color_g;
  uint8 flat_color_b;




  G3DPolygonDPFX() {}
};


struct G3DPolygonDFP
{

  int num;

  csVector2 vertices[100];


  csPlane3 normal;
};


struct G3DPolygonDP : public G3DPolygonDFP
{

  G3DFogInfo fog_info[100];

  bool use_fog;


  iMaterialHandle* mat_handle;


  G3DTexturePlane plane;


  iPolygonTexture* poly_texture;


  bool do_fullbright;


  uint mixmode;


  float z_value;
};


typedef G3DPolygonDP G3DPolygonDPF;


enum csZBufMode
{


  CS_ZBUF_NONE = 0x00000000,

  CS_ZBUF_FILL = 0x00000001,

  CS_ZBUF_TEST = 0x00000002,

  CS_ZBUF_USE = 0x00000003,

  CS_ZBUF_FILLONLY = 0x00000004,

  CS_ZBUF_EQUAL = 0x00000005,

  CS_ZBUF_SPECIAL = 0x00000006
};


enum G3D_RENDERSTATEOPTION
{

  G3DRENDERSTATE_ZBUFFERMODE,

  G3DRENDERSTATE_DITHERENABLE,

  G3DRENDERSTATE_BILINEARMAPPINGENABLE,

  G3DRENDERSTATE_TRILINEARMAPPINGENABLE,

  G3DRENDERSTATE_TRANSPARENCYENABLE,

  G3DRENDERSTATE_MIPMAPENABLE,

  G3DRENDERSTATE_TEXTUREMAPPINGENABLE,

  G3DRENDERSTATE_LIGHTINGENABLE,

  G3DRENDERSTATE_INTERLACINGENABLE,

  G3DRENDERSTATE_MMXENABLE,

  G3DRENDERSTATE_INTERPOLATIONSTEP,

  G3DRENDERSTATE_MAXPOLYGONSTODRAW,

  G3DRENDERSTATE_GOURAUDENABLE,

  G3DRENDERSTATE_EDGES
};
# 287 "f:/source-wip/CS/include/ivideo/graph3d.h"
enum G3D_FOGMETHOD
{
  G3DFOGMETHOD_NONE = 0x00,
  G3DFOGMETHOD_ZBUFFER = 0x01,
  G3DFOGMETHOD_VERTEX = 0x02
};


struct csGraphics3DCaps
{
  bool CanClip;
  int minTexHeight, minTexWidth;
  int maxTexHeight, maxTexWidth;
  G3D_FOGMETHOD fog;
  bool NeedsPO2Maps;
  int MaxAspectRatio;
};
# 357 "f:/source-wip/CS/include/ivideo/graph3d.h"
struct G3DTriangleMesh
{
  enum
  {

    MAX_VERTEXPOOL = 2
  };


  int num_vertices_pool;


  int num_triangles;

  csTriangle* triangles;


  int clip_portal;

  int clip_plane;

  int clip_z_plane;


  bool use_vertex_color;


  bool do_fog;

  bool do_mirror;

  bool do_morph_texels;

  bool do_morph_colors;


  enum VertexMode
  {

    VM_WORLDSPACE,

    VM_VIEWSPACE
  };


  VertexMode vertex_mode;


  uint mixmode;
  float morph_factor;




  iVertexBuffer* buffers[MAX_VERTEXPOOL];
  iMaterialHandle* mat_handle;

  G3DFogInfo* vertex_fog;


};
# 429 "f:/source-wip/CS/include/ivideo/graph3d.h"
struct G3DPolygonMesh
{

  iPolygonBuffer* polybuf;


  bool do_fog;


  uint mixmode;


  int clip_portal;

  int clip_plane;

  int clip_z_plane;


  bool do_mirror;


  enum VertexMode
  {

    VM_WORLDSPACE,

    VM_VIEWSPACE
  };


  VertexMode vertex_mode;


  G3DFogInfo* vertex_fog;
};




struct csFog
{

  bool enabled;

  float density;

  float red;

  float green;

  float blue;
};

const int iGraphics3D_VERSION = ((5 << 24) | (1 << 16) | 0); inline static scfInterfaceID iGraphics3D_scfGetID () { static scfInterfaceID ID = (scfInterfaceID)-1; if (ID == (scfInterfaceID)(-1)) ID = iSCF::SCF->GetInterfaceID ("iGraphics3D"); return ID; };







struct iGraphics3D : public iBase
{

  virtual bool Open () = 0;

  virtual void Close () = 0;


  virtual iGraphics2D *GetDriver2D () = 0;


  virtual void SetDimensions (int width, int height) = 0;


  virtual int GetWidth () = 0;

  virtual int GetHeight () = 0;





  virtual csGraphics3DCaps *GetCaps () = 0;





  virtual void SetPerspectiveCenter (int x, int y) = 0;


  virtual void GetPerspectiveCenter (int& x, int& y) = 0;




  virtual void SetPerspectiveAspect (float aspect) = 0;


  virtual float GetPerspectiveAspect () = 0;





  virtual void SetObjectToCamera (csReversibleTransform* o2c) = 0;




  virtual const csReversibleTransform& GetObjectToCamera () = 0;






  virtual void SetClipper (iClipper2D* clipper, int cliptype) = 0;




  virtual iClipper2D* GetClipper () = 0;




  virtual int GetClipType () = 0;





  virtual void SetNearPlane (const csPlane3& pl) = 0;




  virtual void ResetNearPlane () = 0;




  virtual const csPlane3& GetNearPlane () = 0;




  virtual bool HasNearPlane () = 0;


  virtual uint32 *GetZBuffAt (int x, int y) = 0;


  virtual float GetZBuffValue (int x, int y) = 0;


  virtual bool BeginDraw (int DrawFlags) = 0;


  virtual void FinishDraw () = 0;






  virtual void Print (csRect *area) = 0;


  virtual bool SetRenderState (G3D_RENDERSTATEOPTION op, long val) = 0;


  virtual long GetRenderState (G3D_RENDERSTATEOPTION op) = 0;


  virtual void DrawPolygon (G3DPolygonDP& poly) = 0;







  virtual void DrawPolygonDebug (G3DPolygonDP& poly) = 0;
# 635 "f:/source-wip/CS/include/ivideo/graph3d.h"
  virtual void DrawPolygonFX (G3DPolygonDPFX& poly) = 0;




  virtual void DrawTriangleMesh (G3DTriangleMesh& mesh) = 0;




  virtual void DrawPolygonMesh (G3DPolygonMesh& mesh) = 0;
# 654 "f:/source-wip/CS/include/ivideo/graph3d.h"
  virtual void OpenFogObject (CS_ID id, csFog* fog) = 0;
# 667 "f:/source-wip/CS/include/ivideo/graph3d.h"
  virtual void DrawFogPolygon (CS_ID id, G3DPolygonDFP& poly, int fogtype) = 0;






  virtual void CloseFogObject (CS_ID id) = 0;
# 685 "f:/source-wip/CS/include/ivideo/graph3d.h"
  virtual void OpenPortal (G3DPolygonDFP* poly) = 0;




  virtual void ClosePortal () = 0;





  virtual void DrawLine (const csVector3& v1, const csVector3& v2,
    float fov, int color) = 0;


  virtual iHalo *CreateHalo (float iR, float iG, float iB,
    unsigned char *iAlpha, int iWidth, int iHeight) = 0;
# 717 "f:/source-wip/CS/include/ivideo/graph3d.h"
  virtual void DrawPixmap (iTextureHandle *hTex, int sx, int sy,
    int sw, int sh, int tx, int ty, int tw, int th, uint8 Alpha = 0) = 0;


  virtual iTextureManager *GetTextureManager () = 0;


  virtual void DumpCache () = 0;


  virtual void ClearCache () = 0;






  virtual void RemoveFromCache (iPolygonTexture* poly_texture) = 0;





  virtual iVertexBufferManager* GetVertexBufferManager () = 0;





  virtual bool IsLightmapOK (iPolygonTexture* poly_texture) = 0;
# 761 "f:/source-wip/CS/include/ivideo/graph3d.h"
  virtual void SetRenderTarget (iTextureHandle* handle,
        bool persistent = false) = 0;




  virtual iTextureHandle* GetRenderTarget () const = 0;
};
# 36 "f:/source-wip/CS/include/iengine/rview.h" 2






struct csFog;

struct iEngine;
struct iClipper2D;
struct iGraphics2D;
struct iGraphics3D;
struct iCamera;
struct iSector;
struct iPolygon3D;
class csRenderView;
class csReversibleTransform;
class csVector3;
class csSphere;
# 63 "f:/source-wip/CS/include/iengine/rview.h"
class csFogInfo
{
public:

  csFogInfo* next;


  csPlane3 incoming_plane;

  csPlane3 outgoing_plane;




  bool has_incoming_plane;






  bool has_outgoing_plane;



  csFog* fog;

};





class csRenderContextFrustum
{
  class csRenderContext;
  friend class csRenderContext;

private:
  int ref_count;

  ~csRenderContextFrustum () { }

public:




  csVector3 frustum[4];

  csRenderContextFrustum () : ref_count (1) { }
  void IncRef () { ref_count++; }
  void DecRef () { --ref_count; if (ref_count <= 0) delete this; }
};







class csRenderContext
{
  friend class csRenderView;

private:



  void* rcdata;

public:

  iCamera* icamera;

  iClipper2D* iview;

  csRenderContextFrustum* iview_frustum;


  iPolygon3D* portal_polygon;

  iSector* previous_sector;

  iSector* this_sector;





  csPlane3 clip_plane;
# 165 "f:/source-wip/CS/include/iengine/rview.h"
  bool do_clip_plane;
# 175 "f:/source-wip/CS/include/iengine/rview.h"
  bool do_clip_frustum;






  csFogInfo* fog_info;





  bool added_fog_info;







  int draw_rec_level;
};

const int iRenderView_VERSION = ((0 << 24) | (4 << 16) | 0); inline static scfInterfaceID iRenderView_scfGetID () { static scfInterfaceID ID = (scfInterfaceID)-1; if (ID == (scfInterfaceID)(-1)) ID = iSCF::SCF->GetInterfaceID ("iRenderView"); return ID; };





struct iRenderView : public iBase
{

  virtual csRenderContext* GetRenderContext () = 0;







  virtual void CreateRenderContext () = 0;





  virtual void RestoreRenderContext (csRenderContext* original) = 0;





  virtual iCamera* CreateNewCamera () = 0;


  virtual iEngine* GetEngine () = 0;

  virtual iGraphics2D* GetGraphics2D () = 0;

  virtual iGraphics3D* GetGraphics3D () = 0;

  virtual void SetFrustum (float lx, float rx, float ty, float by) = 0;

  virtual void GetFrustum (float& lx, float& rx, float& ty, float& by) = 0;

  virtual csRenderContextFrustum* GetTopFrustum () = 0;






  virtual iClipper2D* GetClipper () = 0;

  virtual void SetClipper (iClipper2D* clip) = 0;






  virtual bool IsClipperRequired () = 0;






  virtual bool GetClipPlane (csPlane3& pl) = 0;

  virtual csPlane3& GetClipPlane () = 0;



  virtual void SetClipPlane (const csPlane3& pl) = 0;

  virtual void UseClipPlane (bool u) = 0;

  virtual void UseClipFrustum (bool u) = 0;







  virtual csFogInfo* GetFirstFogInfo () = 0;



  virtual void SetFirstFogInfo (csFogInfo* fi) = 0;



  virtual bool AddedFogInfo () = 0;



  virtual void ResetFogInfo () = 0;




  virtual iCamera* GetCamera () = 0;




  virtual void CalculateFogPolygon (G3DPolygonDP& poly) = 0;



  virtual void CalculateFogPolygon (G3DPolygonDPFX& poly) = 0;






  virtual void CalculateFogMesh (const csTransform& tr_o2c,
        G3DTriangleMesh& mesh) = 0;
# 325 "f:/source-wip/CS/include/iengine/rview.h"
  virtual void CalculateFogMesh (const csTransform &tr_o2c,
    G3DPolygonMesh &mesh) = 0;
# 335 "f:/source-wip/CS/include/iengine/rview.h"
  virtual bool TestBSphere (const csReversibleTransform& o2c,
        const csSphere& sphere) = 0;
# 345 "f:/source-wip/CS/include/iengine/rview.h"
  virtual bool ClipBSphere (const csReversibleTransform& o2c,
        const csSphere& sphere,
        int& clip_portal, int& clip_plane, int& clip_z_plane) = 0;
# 356 "f:/source-wip/CS/include/iengine/rview.h"
  virtual bool ClipBBox (const csBox2& sbox, const csBox3& cbox,
        int& clip_portal, int& clip_plane, int& clip_z_plane) = 0;




  virtual iSector* GetThisSector () = 0;




  virtual void SetThisSector (iSector* s) = 0;




  virtual iSector* GetPreviousSector () = 0;




  virtual void SetPreviousSector (iSector* s) = 0;




  virtual iPolygon3D* GetPortalPolygon () = 0;




  virtual void SetPortalPolygon (iPolygon3D* poly) = 0;




  virtual int GetRenderRecursionLevel () = 0;



  virtual void SetRenderRecursionLevel (int rec) = 0;




  virtual void AttachRenderContextData (void* key, iBase* data) = 0;



  virtual iBase* FindRenderContextData (void* key) = 0;




  virtual void DeleteRenderContextData (void* key) = 0;





  virtual iCamera* GetOriginalCamera () const = 0;
};
# 27 "lightiter.cpp" 2
# 1 "f:/source-wip/CS/include/iengine/sector.h" 1
# 31 "f:/source-wip/CS/include/iengine/sector.h"
# 1 "f:/source-wip/CS/include/iutil/object.h" 1
# 29 "f:/source-wip/CS/include/iutil/object.h"
struct iObjectIterator;
# 57 "f:/source-wip/CS/include/iutil/object.h"
const int iObject_VERSION = ((0 << 24) | (3 << 16) | 0); inline static scfInterfaceID iObject_scfGetID () { static scfInterfaceID ID = (scfInterfaceID)-1; if (ID == (scfInterfaceID)(-1)) ID = iSCF::SCF->GetInterfaceID ("iObject"); return ID; };




struct iObject : public iBase
{

  virtual void SetName (const char *iName) = 0;


  virtual const char *GetName () const = 0;


  virtual CS_ID GetID () const = 0;





  virtual void SetObjectParent (iObject *obj) = 0;


  virtual iObject* GetObjectParent () const = 0;


  virtual void ObjAdd (iObject *obj) = 0;


  virtual void ObjRemove (iObject *obj) = 0;


  virtual void ObjRemoveAll () = 0;


  virtual void ObjAddChildren (iObject *Parent) = 0;
# 104 "f:/source-wip/CS/include/iutil/object.h"
  virtual void* GetChild (int iInterfaceID, int iVersion,
    const char *Name = 0, bool FirstName = false) const = 0;


  virtual iObject *GetChild (const char *Name) const = 0;





  virtual csPtr<iObjectIterator> GetIterator () = 0;


  virtual void ObjReleaseOld (iObject *obj) = 0;
};


const int iObjectIterator_VERSION = ((0 << 24) | (1 << 16) | 0); inline static scfInterfaceID iObjectIterator_scfGetID () { static scfInterfaceID ID = (scfInterfaceID)-1; if (ID == (scfInterfaceID)(-1)) ID = iSCF::SCF->GetInterfaceID ("iObjectIterator"); return ID; };
# 130 "f:/source-wip/CS/include/iutil/object.h"
struct iObjectIterator : public iBase
{

  virtual iObject* Next () = 0;


  virtual void Reset () = 0;


  virtual iObject* GetParentObj () const = 0;


  virtual bool HasNext () const = 0;







  virtual iObject* FindName (const char* name) = 0;
};


const int iDataObject_VERSION = ((0 << 24) | (0 << 16) | 1); inline static scfInterfaceID iDataObject_scfGetID () { static scfInterfaceID ID = (scfInterfaceID)-1; if (ID == (scfInterfaceID)(-1)) ID = iSCF::SCF->GetInterfaceID ("iDataObject"); return ID; };


struct iDataObject : public iBase
{

  virtual iObject *QueryObject () = 0;


  virtual void* GetData () = 0;
};
# 32 "f:/source-wip/CS/include/iengine/sector.h" 2

class csVector3;
class csSector;
class csColor;
class csBox3;
class csReversibleTransform;
struct iMeshWrapper;
struct iMeshList;
struct iLightList;
struct iLight;
struct iThing;
struct iStatLight;
struct iVisibilityCuller;
struct iVisibilityObject;
struct iObject;
struct csFog;
struct iGraphics3D;
struct iPolygon3D;
struct iRenderView;
struct iFrustumView;
struct iSector;
class csRenderMesh;

const int iSectorCallback_VERSION = ((0 << 24) | (0 << 16) | 1); inline static scfInterfaceID iSectorCallback_scfGetID () { static scfInterfaceID ID = (scfInterfaceID)-1; if (ID == (scfInterfaceID)(-1)) ID = iSCF::SCF->GetInterfaceID ("iSectorCallback"); return ID; };






struct iSectorCallback : public iBase
{




  virtual void Traverse (iSector* sector, iBase* context) = 0;
};
# 100 "f:/source-wip/CS/include/iengine/sector.h"
const int iSector_VERSION = ((0 << 24) | (5 << 16) | 2); inline static scfInterfaceID iSector_scfGetID () { static scfInterfaceID ID = (scfInterfaceID)-1; if (ID == (scfInterfaceID)(-1)) ID = iSCF::SCF->GetInterfaceID ("iSector"); return ID; };
# 111 "f:/source-wip/CS/include/iengine/sector.h"
struct iSector : public iBase
{

  virtual csSector *GetPrivateObject () = 0;

  virtual iObject *QueryObject() = 0;



  virtual bool HasFog () const = 0;

  virtual csFog *GetFog () const = 0;

  virtual void SetFog (float density, const csColor& color) = 0;

  virtual void DisableFog () = 0;



  virtual iMeshList* GetMeshes () = 0;





  virtual iLightList* GetLights () = 0;


  virtual void ShineLights () = 0;

  virtual void ShineLights (iMeshWrapper*) = 0;


  virtual void SetDynamicAmbientLight(const csColor& color) = 0;


  virtual csColor GetDynamicAmbientLight() const = 0;






  virtual void CalculateSectorBBox (csBox3& bbox,
    bool do_meshes) const = 0;






  virtual bool SetVisibilityCullerPlugin (const char *Name) = 0;





  virtual iVisibilityCuller* GetVisibilityCuller () = 0;




  virtual int GetRecLevel () const = 0;
# 182 "f:/source-wip/CS/include/iengine/sector.h"
  virtual iPolygon3D* HitBeam (const csVector3& start, const csVector3& end,
    csVector3& isect) = 0;







  virtual iMeshWrapper* HitBeam (const csVector3& start, const csVector3& end,
    csVector3& intersect, iPolygon3D** polygonPtr, bool accurate = false) = 0;
# 212 "f:/source-wip/CS/include/iengine/sector.h"
  virtual iSector* FollowSegment (csReversibleTransform& t,
    csVector3& new_position, bool& mirror, bool only_portals = false) = 0;


  virtual void Draw (iRenderView* rview) = 0;
# 254 "f:/source-wip/CS/include/iengine/sector.h"
  virtual void SetSectorCallback (iSectorCallback* cb) = 0;




  virtual void RemoveSectorCallback (iSectorCallback* cb) = 0;


  virtual int GetSectorCallbackCount () const = 0;


  virtual iSectorCallback* GetSectorCallback (int idx) const = 0;


  virtual void CheckFrustum (iFrustumView* lview) = 0;
};


const int iSectorList_VERSION = ((0 << 24) | (0 << 16) | 2); inline static scfInterfaceID iSectorList_scfGetID () { static scfInterfaceID ID = (scfInterfaceID)-1; if (ID == (scfInterfaceID)(-1)) ID = iSCF::SCF->GetInterfaceID ("iSectorList"); return ID; };




struct iSectorList : public iBase
{

  virtual int GetCount () const = 0;


  virtual iSector *Get (int n) const = 0;


  virtual int Add (iSector *obj) = 0;


  virtual bool Remove (iSector *obj) = 0;


  virtual bool Remove (int n) = 0;


  virtual void RemoveAll () = 0;


  virtual int Find (iSector *obj) const = 0;


  virtual iSector *FindByName (const char *Name) const = 0;
};

const int iSectorIterator_VERSION = ((0 << 24) | (1 << 16) | 0); inline static scfInterfaceID iSectorIterator_scfGetID () { static scfInterfaceID ID = (scfInterfaceID)-1; if (ID == (scfInterfaceID)(-1)) ID = iSCF::SCF->GetInterfaceID ("iSectorIterator"); return ID; };





struct iSectorIterator : public iBase
{

  virtual bool HasNext () = 0;


  virtual iSector* Next () = 0;





  virtual const csVector3& GetLastPosition () = 0;


  virtual void Reset () = 0;
};
# 28 "lightiter.cpp" 2
# 1 "f:/source-wip/CS/include/iengine/mesh.h" 1
# 30 "f:/source-wip/CS/include/iengine/mesh.h"
# 1 "f:/source-wip/CS/include/iutil/eventh.h" 1
# 29 "f:/source-wip/CS/include/iutil/eventh.h"
struct iEvent;

const int iEventHandler_VERSION = ((0 << 24) | (0 << 16) | 1); inline static scfInterfaceID iEventHandler_scfGetID () { static scfInterfaceID ID = (scfInterfaceID)-1; if (ID == (scfInterfaceID)(-1)) ID = iSCF::SCF->GetInterfaceID ("iEventHandler"); return ID; };




struct iEventHandler : public iBase
{
# 50 "f:/source-wip/CS/include/iutil/eventh.h"
  virtual bool HandleEvent (iEvent&) = 0;
};
# 31 "f:/source-wip/CS/include/iengine/mesh.h" 2
# 1 "f:/source-wip/CS/include/iutil/comp.h" 1
# 23 "f:/source-wip/CS/include/iutil/comp.h"
struct iObjectRegistry;
# 32 "f:/source-wip/CS/include/iutil/comp.h"
const int iComponent_VERSION = ((0 << 24) | (0 << 16) | 1); inline static scfInterfaceID iComponent_scfGetID () { static scfInterfaceID ID = (scfInterfaceID)-1; if (ID == (scfInterfaceID)(-1)) ID = iSCF::SCF->GetInterfaceID ("iComponent"); return ID; };




struct iComponent : public iBase
{

  virtual bool Initialize (iObjectRegistry*) = 0;
};
# 32 "f:/source-wip/CS/include/iengine/mesh.h" 2


struct iMeshObject;
struct iCamera;
struct iMeshObjectFactory;
struct iMeshWrapper;
struct iMeshList;
struct iMeshFactoryList;
class csMeshWrapper;
class csMeshFactoryWrapper;
struct iMeshFactoryWrapper;
struct iRenderView;
struct iMovable;
struct iLight;
struct iLightingInfo;
struct iShadowReceiver;
struct iObject;
class csFlags;
# 105 "f:/source-wip/CS/include/iengine/mesh.h"
const int iMeshDrawCallback_VERSION = ((0 << 24) | (0 << 16) | 1); inline static scfInterfaceID iMeshDrawCallback_scfGetID () { static scfInterfaceID ID = (scfInterfaceID)-1; if (ID == (scfInterfaceID)(-1)) ID = iSCF::SCF->GetInterfaceID ("iMeshDrawCallback"); return ID; };
# 115 "f:/source-wip/CS/include/iengine/mesh.h"
struct iMeshDrawCallback : public iBase
{




  virtual bool BeforeDrawing (iMeshWrapper* spr, iRenderView* rview) = 0;
};


const int iMeshWrapper_VERSION = ((0 << 24) | (5 << 16) | 0); inline static scfInterfaceID iMeshWrapper_scfGetID () { static scfInterfaceID ID = (scfInterfaceID)-1; if (ID == (scfInterfaceID)(-1)) ID = iSCF::SCF->GetInterfaceID ("iMeshWrapper"); return ID; };
# 139 "f:/source-wip/CS/include/iengine/mesh.h"
struct iMeshWrapper : public iBase
{





  virtual iObject *QueryObject () = 0;


  virtual iMeshObject* GetMeshObject () const = 0;

  virtual void SetMeshObject (iMeshObject*) = 0;







  virtual iLightingInfo* GetLightingInfo () const = 0;







  virtual iShadowReceiver* GetShadowReceiver () const = 0;





  virtual uint GetVisibilityNumber () const = 0;


  virtual iMeshFactoryWrapper *GetFactory () const = 0;

  virtual void SetFactory (iMeshFactoryWrapper* factory) = 0;
# 188 "f:/source-wip/CS/include/iengine/mesh.h"
  virtual void DeferUpdateLighting (int flags, int num_lights) = 0;
# 202 "f:/source-wip/CS/include/iengine/mesh.h"
  virtual void UpdateLighting (iLight** lights, int num_lights) = 0;
# 211 "f:/source-wip/CS/include/iengine/mesh.h"
  virtual iMovable* GetMovable () const = 0;
# 230 "f:/source-wip/CS/include/iengine/mesh.h"
  virtual void PlaceMesh () = 0;
# 241 "f:/source-wip/CS/include/iengine/mesh.h"
  virtual int HitBeamBBox (const csVector3& start, const csVector3& end,
                csVector3& isect, float* pr) = 0;





  virtual bool HitBeamOutline (const csVector3& start,
        const csVector3& end, csVector3& isect, float* pr) = 0;






  virtual bool HitBeamObject (const csVector3& start, const csVector3& end,
        csVector3& isect, float* pr) = 0;




  virtual bool HitBeam (const csVector3& start, const csVector3& end,
        csVector3& isect, float* pr) = 0;
# 273 "f:/source-wip/CS/include/iengine/mesh.h"
  virtual void SetDrawCallback (iMeshDrawCallback* cb) = 0;




  virtual void RemoveDrawCallback (iMeshDrawCallback* cb) = 0;


  virtual int GetDrawCallbackCount () const = 0;


  virtual iMeshDrawCallback* GetDrawCallback (int idx) const = 0;
# 302 "f:/source-wip/CS/include/iengine/mesh.h"
  virtual void SetRenderPriority (long rp) = 0;



  virtual long GetRenderPriority () const = 0;
# 321 "f:/source-wip/CS/include/iengine/mesh.h"
  virtual csFlags& GetFlags () = 0;
# 333 "f:/source-wip/CS/include/iengine/mesh.h"
  virtual void SetZBufMode (csZBufMode mode) = 0;



  virtual csZBufMode GetZBufMode () const = 0;
# 353 "f:/source-wip/CS/include/iengine/mesh.h"
  virtual void HardTransform (const csReversibleTransform& t) = 0;






  virtual void GetWorldBoundingBox (csBox3& cbox) = 0;






  virtual void GetTransformedBoundingBox (const csReversibleTransform& trans,
        csBox3& cbox) = 0;






  virtual float GetScreenBoundingBox (iCamera* camera, csBox2& sbox,
        csBox3& cbox) = 0;






  virtual iMeshList* GetChildren () = 0;





  virtual iMeshWrapper* GetParentContainer () = 0;




  virtual void SetParentContainer (iMeshWrapper *) = 0;


  virtual void GetRadius (csVector3& rad, csVector3& cent) const = 0;







  virtual void Draw (iRenderView* rview) = 0;
# 434 "f:/source-wip/CS/include/iengine/mesh.h"
};

const int iMeshFactoryWrapper_VERSION = ((0 << 24) | (1 << 16) | 6); inline static scfInterfaceID iMeshFactoryWrapper_scfGetID () { static scfInterfaceID ID = (scfInterfaceID)-1; if (ID == (scfInterfaceID)(-1)) ID = iSCF::SCF->GetInterfaceID ("iMeshFactoryWrapper"); return ID; };
# 449 "f:/source-wip/CS/include/iengine/mesh.h"
struct iMeshFactoryWrapper : public iBase
{

  virtual iObject *QueryObject () = 0;

  virtual iMeshObjectFactory* GetMeshObjectFactory () const = 0;

  virtual void SetMeshObjectFactory (iMeshObjectFactory* fact) = 0;
# 466 "f:/source-wip/CS/include/iengine/mesh.h"
  virtual void HardTransform (const csReversibleTransform& t) = 0;




  virtual iMeshWrapper* CreateMeshWrapper () = 0;





  virtual iMeshFactoryWrapper* GetParentContainer () const = 0;




  virtual void SetParentContainer (iMeshFactoryWrapper *p) = 0;




  virtual iMeshFactoryList* GetChildren () = 0;




  virtual csReversibleTransform& GetTransform () = 0;




  virtual void SetTransform (const csReversibleTransform& tr) = 0;
};

const int iMeshList_VERSION = ((0 << 24) | (0 << 16) | 1); inline static scfInterfaceID iMeshList_scfGetID () { static scfInterfaceID ID = (scfInterfaceID)-1; if (ID == (scfInterfaceID)(-1)) ID = iSCF::SCF->GetInterfaceID ("iMeshList"); return ID; };




struct iMeshList : public iBase
{

  virtual int GetCount () const = 0;


  virtual iMeshWrapper *Get (int n) const = 0;


  virtual int Add (iMeshWrapper *obj) = 0;


  virtual bool Remove (iMeshWrapper *obj) = 0;


  virtual bool Remove (int n) = 0;


  virtual void RemoveAll () = 0;


  virtual int Find (iMeshWrapper *obj) const = 0;






  virtual iMeshWrapper *FindByName (const char *Name) const = 0;
};

const int iMeshFactoryList_VERSION = ((0 << 24) | (0 << 16) | 1); inline static scfInterfaceID iMeshFactoryList_scfGetID () { static scfInterfaceID ID = (scfInterfaceID)-1; if (ID == (scfInterfaceID)(-1)) ID = iSCF::SCF->GetInterfaceID ("iMeshFactoryList"); return ID; };




struct iMeshFactoryList : public iBase
{

  virtual int GetCount () const = 0;


  virtual iMeshFactoryWrapper *Get (int n) const = 0;


  virtual int Add (iMeshFactoryWrapper *obj) = 0;


  virtual bool Remove (iMeshFactoryWrapper *obj) = 0;


  virtual bool Remove (int n) = 0;


  virtual void RemoveAll () = 0;


  virtual int Find (iMeshFactoryWrapper *obj) const = 0;


  virtual iMeshFactoryWrapper *FindByName (const char *Name) const = 0;
};
# 29 "lightiter.cpp" 2
# 1 "f:/source-wip/CS/include/iengine/material.h" 1
# 30 "f:/source-wip/CS/include/iengine/material.h"
class csMaterialWrapper;
struct iMaterial;
struct iMaterialHandle;
struct iTextureManager;
struct iTextureWrapper;
struct iObject;

const int iMaterialWrapper_VERSION = ((0 << 24) | (0 << 16) | 4); inline static scfInterfaceID iMaterialWrapper_scfGetID () { static scfInterfaceID ID = (scfInterfaceID)-1; if (ID == (scfInterfaceID)(-1)) ID = iSCF::SCF->GetInterfaceID ("iMaterialWrapper"); return ID; };
# 46 "f:/source-wip/CS/include/iengine/material.h"
struct iMaterialWrapper : public iBase
{

  virtual iObject *QueryObject() = 0;


  virtual iMaterialWrapper *Clone () const = 0;





  virtual void SetMaterialHandle (iMaterialHandle *mat) = 0;

  virtual iMaterialHandle* GetMaterialHandle () = 0;





  virtual void SetMaterial (iMaterial* material) = 0;

  virtual iMaterial* GetMaterial () = 0;


  virtual void Register (iTextureManager *txtmng) = 0;






  virtual void Visit () = 0;
};

const int iMaterialEngine_VERSION = ((0 << 24) | (0 << 16) | 1); inline static scfInterfaceID iMaterialEngine_scfGetID () { static scfInterfaceID ID = (scfInterfaceID)-1; if (ID == (scfInterfaceID)(-1)) ID = iSCF::SCF->GetInterfaceID ("iMaterialEngine"); return ID; };
# 91 "f:/source-wip/CS/include/iengine/material.h"
struct iMaterialEngine : public iBase
{



  virtual iTextureWrapper *GetTextureWrapper () = 0;




  virtual iTextureWrapper* GetTextureWrapper (int idx) = 0;
};

const int iMaterialList_VERSION = ((0 << 24) | (0 << 16) | 1); inline static scfInterfaceID iMaterialList_scfGetID () { static scfInterfaceID ID = (scfInterfaceID)-1; if (ID == (scfInterfaceID)(-1)) ID = iSCF::SCF->GetInterfaceID ("iMaterialList"); return ID; };




struct iMaterialList : public iBase
{

  virtual iMaterialWrapper* NewMaterial (iMaterial* material) = 0;





  virtual iMaterialWrapper *NewMaterial (iMaterialHandle *ith) = 0;


  virtual int GetCount () const = 0;


  virtual iMaterialWrapper *Get (int n) const = 0;


  virtual int Add (iMaterialWrapper *obj) = 0;


  virtual bool Remove (iMaterialWrapper *obj) = 0;


  virtual bool Remove (int n) = 0;


  virtual void RemoveAll () = 0;


  virtual int Find (iMaterialWrapper *obj) const = 0;


  virtual iMaterialWrapper *FindByName (const char *Name) const = 0;
};
# 30 "lightiter.cpp" 2

# 1 "lightiter.h" 1
# 24 "lightiter.h"
# 1 "f:/source-wip/CS/include/csutil/csstring.h" 1
# 22 "f:/source-wip/CS/include/csutil/csstring.h"
# 1 "d:/util/mingw/include/stdarg.h" 1





# 1 "d:/util/mingw/include/stdarg.h" 1 3
# 7 "d:/util/mingw/include/stdarg.h" 2
# 23 "f:/source-wip/CS/include/csutil/csstring.h" 2
# 1 "d:/util/mingw/include/ctype.h" 1
# 36 "d:/util/mingw/include/ctype.h"
# 1 "d:/util/mingw/include/stddef.h" 1





# 1 "d:/util/mingw/include/stddef.h" 1 3





# 1 "d:/util/mingw/lib/gcc-lib/mingw32/3.2.3/include/stddef.h" 1 3
# 7 "d:/util/mingw/include/stddef.h" 2 3
# 7 "d:/util/mingw/include/stddef.h" 2
# 37 "d:/util/mingw/include/ctype.h" 2
# 59 "d:/util/mingw/include/ctype.h"
extern "C" {


 int isalnum(int);
 int isalpha(int);
 int iscntrl(int);
 int isdigit(int);
 int isgraph(int);
 int islower(int);
 int isprint(int);
 int ispunct(int);
 int isspace(int);
 int isupper(int);
 int isxdigit(int);


 int _isctype (int, int);



 int tolower(int);
 int toupper(int);
# 92 "d:/util/mingw/include/ctype.h"
 int _tolower(int);
 int _toupper(int);
# 120 "d:/util/mingw/include/ctype.h"
 unsigned short _ctype[];

  unsigned short* _pctype;
# 187 "d:/util/mingw/include/ctype.h"
typedef wchar_t wctype_t;



 int iswalnum(wint_t);
 int iswalpha(wint_t);
 int iswascii(wint_t);
 int iswcntrl(wint_t);
 int iswctype(wint_t, wctype_t);
 int is_wctype(wint_t, wctype_t);
 int iswdigit(wint_t);
 int iswgraph(wint_t);
 int iswlower(wint_t);
 int iswprint(wint_t);
 int iswpunct(wint_t);
 int iswspace(wint_t);
 int iswupper(wint_t);
 int iswxdigit(wint_t);

 wchar_t towlower(wchar_t);
 wchar_t towupper(wchar_t);

 int isleadbyte (int);
# 231 "d:/util/mingw/include/ctype.h"
int __isascii (int);
int __toascii (int);
int __iscsymf (int);
int __iscsym (int);
# 245 "d:/util/mingw/include/ctype.h"
int isascii (int);
int toascii (int);
int iscsymf (int);
int iscsym (int);





}
# 24 "f:/source-wip/CS/include/csutil/csstring.h" 2
# 1 "f:/source-wip/CS/include/csutil/snprintf.h" 1



# 1 "d:/util/mingw/include/stdarg.h" 1





# 1 "d:/util/mingw/include/stdarg.h" 1 3
# 7 "d:/util/mingw/include/stdarg.h" 2
# 5 "f:/source-wip/CS/include/csutil/snprintf.h" 2





extern int cs_snprintf(char *, size_t, const char *, ...)
    ;
extern int cs_vsnprintf(char *, size_t, const char *, va_list)
    ;
# 25 "f:/source-wip/CS/include/csutil/csstring.h" 2





class csString
{
protected:


  enum { DEFAULT_GROW_BY = 64 };


  char *Data;

  size_t Size;

  size_t MaxSize;

  size_t GrowBy;

  bool GrowExponentially;




  void ExpandIfNeeded(size_t NewSize);

public:







  void SetCapacity (size_t NewSize);


  size_t GetCapacity() const
  { return MaxSize; }
# 74 "f:/source-wip/CS/include/csutil/csstring.h"
  void SetGrowsBy(size_t);


  size_t GetGrowsBy() const
  { return GrowBy; }





  void SetGrowsExponentially(bool b)
  { GrowExponentially = b; }


  bool GetGrowsExponentially() const
  { return GrowExponentially; }


  void Free ();


  csString &Truncate (size_t iLen);


  csString &Reclaim ();


  csString& Clear ()
  { return Truncate (0); }





  char* GetData () const
  { return Data; }


  size_t Length () const
  { return Size; }


  bool IsEmpty () const
  { return (Size == 0); }


  char& operator [] (size_t n)
  {
    ;
    return Data [n];
  }


  char operator [] (size_t n) const
  {
    ;
    return Data[n];
  }





  void SetAt (size_t n, const char c)
  {
    ;
    Data [n] = c;
  }


  char GetAt (size_t n) const
  {
    ;
    return Data [n];
  }


  csString& DeleteAt (size_t iPos, size_t iCount = 1);


  csString& Insert (size_t iPos, const csString&);


  csString& Insert (size_t iPos, const char);


  csString& Overwrite (size_t iPos, const csString&);






  csString& Append (const char*, size_t iCount = (size_t)-1);





  csString& Append (const csString &iStr, size_t iCount = (size_t)-1);


  csString& Append (char c)
  { char s[2]; s[0] = c; s[1] = '\0'; return Append(s); }

  csString &Append (unsigned char c)
  { return Append(char(c)); }





  void SubString (csString& sub, size_t start, size_t len);





  size_t FindFirst (const char c, size_t p = (size_t)-1);




  size_t FindLast (const char c, size_t p = (size_t)-1);



  csString& Append(short n) { char s[32]; cs_snprintf(s, 32, "%hd", n); return Append(s); }
  csString& Append(unsigned short n) { char s[32]; cs_snprintf(s, 32, "%hu", n); return Append(s); }
  csString& Append(int n) { char s[32]; cs_snprintf(s, 32, "%d", n); return Append(s); }
  csString& Append(unsigned int n) { char s[32]; cs_snprintf(s, 32, "%u", n); return Append(s); }
  csString& Append(long n) { char s[32]; cs_snprintf(s, 32, "%ld", n); return Append(s); }
  csString& Append(unsigned long n) { char s[32]; cs_snprintf(s, 32, "%lu", n); return Append(s); }
  csString& Append(float n) { char s[64]; cs_snprintf(s, 64, "%g", n); return Append(s); }
  csString& Append(double n) { char s[64]; cs_snprintf(s, 64, "%g", n); return Append(s); }




  csString& Append (bool b) { return Append (b ? "1" : "0"); }







  csString& Replace (const csString& iStr, size_t iCount = (size_t)-1)
  {
    Size = 0;
    return Append (iStr, iCount);
  }






  csString& Replace (const char* iStr, size_t iCount = (size_t)-1)
  {
    Size = 0;
    return Append (iStr, iCount);
  }



  csString& Replace (char s) { Size = 0; return Append(s); }
  csString& Replace (unsigned char s) { Size = 0; return Append(s); }
  csString& Replace (short s) { Size = 0; return Append(s); }
  csString& Replace (unsigned short s) { Size = 0; return Append(s); }
  csString& Replace (int s) { Size = 0; return Append(s); }
  csString& Replace (unsigned int s) { Size = 0; return Append(s); }
  csString& Replace (long s) { Size = 0; return Append(s); }
  csString& Replace (unsigned long s) { Size = 0; return Append(s); }
  csString& Replace (float s) { Size = 0; return Append(s); }
  csString& Replace (double s) { Size = 0; return Append(s); }

  csString& Replace (bool s) { Size = 0; return Append(s); }




  bool Compare (const csString& iStr) const
  {
    if (&iStr == this)
      return true;
    size_t const n = iStr.Length();
    if (Size != n)
      return false;
    if (Size == 0 && n == 0)
      return true;
    return (memcmp (Data, iStr.GetData (), Size) == 0);
  }


  bool Compare (const char* iStr) const
  { return (strcmp (Data ? Data : "", iStr) == 0); }


  bool CompareNoCase (const csString& iStr) const
  {
    if (&iStr == this)
      return true;
    size_t const n = iStr.Length();
    if (Size != n)
      return false;
    if (Size == 0 && n == 0)
      return true;
    return (strncasecmp (Data, iStr.GetData (), Size) == 0);
  }


  bool CompareNoCase (const char* iStr) const
  { return (strncasecmp (Data ? Data : "", iStr, Size) == 0); }


  csString () : Data (0), Size (0), MaxSize (0), GrowBy (DEFAULT_GROW_BY),
    GrowExponentially (false) {}


  csString (size_t iLength) : Data (0), Size (0), MaxSize (0),
    GrowBy (DEFAULT_GROW_BY), GrowExponentially(false)
  { SetCapacity (iLength); }


  csString (const csString& copy) : Data (0), Size (0), MaxSize (0),
    GrowBy (DEFAULT_GROW_BY), GrowExponentially(false)
  { Append (copy); }


  csString (const char* copy) : Data (0), Size (0), MaxSize (0),
    GrowBy (DEFAULT_GROW_BY), GrowExponentially(false)
  { Append (copy); }


  csString (char c) : Data (0), Size (0), MaxSize (0),
    GrowBy (DEFAULT_GROW_BY), GrowExponentially(false)
  { Append (c); }


  csString (unsigned char c) : Data(0), Size (0), MaxSize (0),
    GrowBy (DEFAULT_GROW_BY), GrowExponentially(false)
  { Append ((char) c); }


  virtual ~csString ();


  csString Clone () const
  { return csString (*this); }


  csString& LTrim();


  csString& RTrim();


  csString& Trim();





  csString& Collapse();






  csString& Format(const char *format, ...) ;






  csString& FormatV(const char *format, va_list args);




  static csString Format (short v);
  static csString Format (unsigned short v);
  static csString Format (int v);
  static csString Format (unsigned int v);
  static csString Format (long v);
  static csString Format (unsigned long v);
  static csString Format (float v);
  static csString Format (double v);




  static csString Format (short v, int width, int prec=0);
  static csString Format (unsigned short v, int width, int prec=0);
  static csString Format (int v, int width, int prec=0);
  static csString Format (unsigned int v, int width, int prec=0);
  static csString Format (long v, int width, int prec=0);
  static csString Format (unsigned long v, int width, int prec=0);




  static csString Format (float v, int width, int prec=6);
  static csString Format (double v, int width, int prec=6);



  csString& PadLeft (size_t iNewSize, char iChar=' ');


  csString AsPadLeft (size_t iNewSize, char iChar=' ');




  static csString PadLeft (const csString& v, size_t iNewSize, char iChar=' ');
  static csString PadLeft (const char* v, size_t iNewSize, char iChar=' ');
  static csString PadLeft (char v, size_t iNewSize, char iChar=' ');
  static csString PadLeft (unsigned char v, size_t iNewSize, char iChar=' ');
  static csString PadLeft (short v, size_t iNewSize, char iChar=' ');
  static csString PadLeft (unsigned short v, size_t iNewSize, char iChar=' ');
  static csString PadLeft (int v, size_t iNewSize, char iChar=' ');
  static csString PadLeft (unsigned int v, size_t iNewSize, char iChar=' ');
  static csString PadLeft (long v, size_t iNewSize, char iChar=' ');
  static csString PadLeft (unsigned long v, size_t iNewSize, char iChar=' ');
  static csString PadLeft (float v, size_t iNewSize, char iChar=' ');
  static csString PadLeft (double v, size_t iNewSize, char iChar=' ');

  static csString PadLeft (bool v, size_t iNewSize, char iChar=' ');




  csString& PadRight (size_t iNewSize, char iChar=' ');


  csString AsPadRight (size_t iNewSize, char iChar=' ');




  static csString PadRight (const csString& v, size_t iNewSize, char iChar=' ');
  static csString PadRight (const char* v, size_t iNewSize, char iChar=' ');
  static csString PadRight (char v, size_t iNewSize, char iChar=' ');
  static csString PadRight (unsigned char v, size_t iNewSize, char iChar=' ');
  static csString PadRight (short v, size_t iNewSize, char iChar=' ');
  static csString PadRight (unsigned short v, size_t iNewSize, char iChar=' ');
  static csString PadRight (int v, size_t iNewSize, char iChar=' ');
  static csString PadRight (unsigned int v, size_t iNewSize, char iChar=' ');
  static csString PadRight (long v, size_t iNewSize, char iChar=' ');
  static csString PadRight (unsigned long v, size_t iNewSize, char iChar=' ');
  static csString PadRight (float v, size_t iNewSize, char iChar=' ');
  static csString PadRight (double v, size_t iNewSize, char iChar=' ');

  static csString PadRight (bool v, size_t iNewSize, char iChar=' ');




  csString& PadCenter (size_t iNewSize, char iChar=' ');


  csString AsPadCenter (size_t iNewSize, char iChar=' ');




  static csString PadCenter (const csString& v, size_t iNewSize, char iChar=' ');
  static csString PadCenter (const char* v, size_t iNewSize, char iChar=' ');
  static csString PadCenter (char v, size_t iNewSize, char iChar=' ');
  static csString PadCenter (unsigned char v, size_t iNewSize, char iChar=' ');
  static csString PadCenter (short v, size_t iNewSize, char iChar=' ');
  static csString PadCenter (unsigned short v, size_t iNewSize, char iChar=' ');
  static csString PadCenter (int v, size_t iNewSize, char iChar=' ');
  static csString PadCenter (unsigned int v, size_t iNewSize, char iChar=' ');
  static csString PadCenter (long v, size_t iNewSize, char iChar=' ');
  static csString PadCenter (unsigned long v, size_t iNewSize, char iChar=' ');
  static csString PadCenter (float v, size_t iNewSize, char iChar=' ');
  static csString PadCenter (double v, size_t iNewSize, char iChar=' ');

  static csString PadCenter (bool v, size_t iNewSize, char iChar=' ');






  const csString& operator = (const csString& s) { return Replace (s); }
  const csString& operator = (const char* s) { return Replace (s); }
  const csString& operator = (char s) { return Replace (s); }
  const csString& operator = (unsigned char s) { return Replace (s); }
  const csString& operator = (short s) { return Replace (s); }
  const csString& operator = (unsigned short s) { return Replace (s); }
  const csString& operator = (int s) { return Replace (s); }
  const csString& operator = (unsigned int s) { return Replace (s); }
  const csString& operator = (long s) { return Replace (s); }
  const csString& operator = (unsigned long s) { return Replace (s); }
  const csString& operator = (float s) { return Replace (s); }
  const csString& operator = (double s) { return Replace (s); }

  const csString& operator = (bool s) { return Replace (s); }




  csString &operator += (const csString& s) { return Append (s); }
  csString &operator += (const char* s) { return Append (s); }
  csString &operator += (char s) { return Append (s); }
  csString &operator += (unsigned char s) { return Append (s); }
  csString &operator += (short s) { return Append (s); }
  csString &operator += (unsigned short s) { return Append (s); }
  csString &operator += (int s) { return Append (s); }
  csString &operator += (unsigned int s) { return Append (s); }
  csString &operator += (long s) { return Append (s); };
  csString &operator += (unsigned long s) { return Append (s); }
  csString &operator += (float s) { return Append (s); }
  csString &operator += (double s) { return Append (s); }

  csString &operator += (bool s) { return Append (s); }




  const csString& operator + (const csString &iStr) const
  { return Clone ().Append (iStr); }


  operator const char* () const
  { return Data; }


  bool operator == (const csString& iStr) const
  { return Compare (iStr); }
  bool operator == (const char* iStr) const
  { return Compare (iStr); }
  bool operator != (const csString& iStr) const
  { return !Compare (iStr); }
  bool operator != (const char* iStr) const
  { return !Compare (iStr); }
# 524 "f:/source-wip/CS/include/csutil/csstring.h"
  char* Detach ()
  { char *d = Data; Data = 0; Size = 0; MaxSize = 0; return d; }

  void strlwr()
  {
    for (char *p = Data; *p; p++)
      *p = (char) tolower (*p);
  }
};


inline csString operator + (const char* iStr1, const csString &iStr2)
{
  return csString (iStr1).Append (iStr2);
}


inline csString operator + (const csString &iStr1, const char* iStr2)
{
  return iStr1.Clone ().Append (iStr2);
}




inline csString &operator << (csString &s, const csString& v) { return s.Append (v); }
inline csString &operator << (csString &s, const char* v) { return s.Append (v); }
inline csString &operator << (csString &s, char v) { return s.Append (v); }
inline csString &operator << (csString &s, unsigned char v) { return s.Append (v); }
inline csString &operator << (csString &s, short v) { return s.Append (v); }
inline csString &operator << (csString &s, unsigned short v) { return s.Append (v); }
inline csString &operator << (csString &s, int v) { return s.Append (v); }
inline csString &operator << (csString &s, unsigned int v) { return s.Append (v); }
inline csString &operator << (csString &s, long v) { return s.Append (v); };
inline csString &operator << (csString &s, unsigned long v) { return s.Append (v); }
inline csString &operator << (csString &s, float v) { return s.Append (v); }
inline csString &operator << (csString &s, double v) { return s.Append (v); }

inline csString &operator << (csString &s, bool v) { return s.Append (v); }
# 25 "lightiter.h" 2
# 1 "f:/source-wip/CS/include/iengine/renderloop.h" 1
# 32 "f:/source-wip/CS/include/iengine/renderloop.h"
# 1 "f:/source-wip/CS/include/ivideo/rendersteps/icontainer.h" 1
# 26 "f:/source-wip/CS/include/ivideo/rendersteps/icontainer.h"
struct iRenderStep;

const int iRenderStepContainer_VERSION = ((0 << 24) | (0 << 16) | 1); inline static scfInterfaceID iRenderStepContainer_scfGetID () { static scfInterfaceID ID = (scfInterfaceID)-1; if (ID == (scfInterfaceID)(-1)) ID = iSCF::SCF->GetInterfaceID ("iRenderStepContainer"); return ID; };





struct iRenderStepContainer : public iBase
{
  virtual int AddStep (iRenderStep* step) = 0;
  virtual int GetStepCount () = 0;
};
# 33 "f:/source-wip/CS/include/iengine/renderloop.h" 2

struct iCamera;
struct iClipper2D;
struct iSector;
struct iRenderStep;
class csRenderView;






const int iRenderLoop_VERSION = ((0 << 24) | (0 << 16) | 3); inline static scfInterfaceID iRenderLoop_scfGetID () { static scfInterfaceID ID = (scfInterfaceID)-1; if (ID == (scfInterfaceID)(-1)) ID = iSCF::SCF->GetInterfaceID ("iRenderLoop"); return ID; };






struct iRenderLoop : public iRenderStepContainer
{
  virtual void Draw (iCamera* c, iClipper2D* clipper) = 0;
};

const int iRenderLoopManager_VERSION = ((0 << 24) | (0 << 16) | 2); inline static scfInterfaceID iRenderLoopManager_scfGetID () { static scfInterfaceID ID = (scfInterfaceID)-1; if (ID == (scfInterfaceID)(-1)) ID = iSCF::SCF->GetInterfaceID ("iRenderLoopManager"); return ID; };







struct iRenderLoopManager : public iBase
{




  virtual csPtr<iRenderLoop> Create () = 0;
# 82 "f:/source-wip/CS/include/iengine/renderloop.h"
  virtual bool Register (const char* name, iRenderLoop* loop) = 0;






  virtual iRenderLoop* Retrieve (const char* name) = 0;





  virtual const char* GetName (iRenderLoop* loop) = 0;





  virtual bool Unregister (iRenderLoop* loop) = 0;
};
# 26 "lightiter.h" 2
# 1 "f:/source-wip/CS/include/ivideo/rendersteps/irenderstep.h" 1
# 23 "f:/source-wip/CS/include/ivideo/rendersteps/irenderstep.h"
struct iRenderView;
struct iSector;

const int iRenderStep_VERSION = ((0 << 24) | (0 << 16) | 1); inline static scfInterfaceID iRenderStep_scfGetID () { static scfInterfaceID ID = (scfInterfaceID)-1; if (ID == (scfInterfaceID)(-1)) ID = iSCF::SCF->GetInterfaceID ("iRenderStep"); return ID; };

struct iRenderStep : public iBase
{
  virtual void Perform (iRenderView* rview, iSector* sector) = 0;
};
# 27 "lightiter.h" 2
# 1 "f:/source-wip/CS/include/ivideo/rendersteps/ilightiter.h" 1
# 26 "f:/source-wip/CS/include/ivideo/rendersteps/ilightiter.h"
const int iLightIterRenderStep_VERSION = ((0 << 24) | (0 << 16) | 1); inline static scfInterfaceID iLightIterRenderStep_scfGetID () { static scfInterfaceID ID = (scfInterfaceID)-1; if (ID == (scfInterfaceID)(-1)) ID = iSCF::SCF->GetInterfaceID ("iLightIterRenderStep"); return ID; };

struct iLightIterRenderStep : public iBase
{
};

const int iLightRenderStep_VERSION = ((0 << 24) | (0 << 16) | 1); inline static scfInterfaceID iLightRenderStep_scfGetID () { static scfInterfaceID ID = (scfInterfaceID)-1; if (ID == (scfInterfaceID)(-1)) ID = iSCF::SCF->GetInterfaceID ("iLightRenderStep"); return ID; };

struct iLightRenderStep : public iBase
{
  virtual void Perform (iRenderView* rview, iSector* sector,
    iLight* light) = 0;
};
# 28 "lightiter.h" 2
# 1 "f:/source-wip/CS/include/ivideo/shader/shader.h" 1
# 24 "f:/source-wip/CS/include/ivideo/shader/shader.h"
# 1 "f:/source-wip/CS/include/csgeom/vector4.h" 1
# 37 "f:/source-wip/CS/include/csgeom/vector4.h"
class csVector4;





class csDVector4
{
public:

  double x;

  double y;

  double z;

  double w;






  csDVector4 () {}






  csDVector4 (double m) : x(m), y(m), z(m), w(m) {}


  csDVector4 (double ix, double iy, double iz = 0, double iw = 1) { x = ix; y = iy; z = iz; w = iw;}


  csDVector4 (const csDVector4& v) { x = v.x; y = v.y; z = v.z; w = v.w; }


  csDVector4 (const csVector4&);


  csDVector4 (const csDVector3& v) { x = v.x; y = v.y; z = v.z; w = 1.0; }


  inline friend
  csDVector4 operator+ (const csDVector4& v1, const csDVector4& v2)
  { return csDVector4(v1.x+v2.x, v1.y+v2.y, v1.z+v2.z, v1.w+v2.w); }


  inline friend
  csDVector4 operator- (const csDVector4& v1, const csDVector4& v2)
  { return csDVector4(v1.x-v2.x, v1.y-v2.y, v1.z-v2.z, v1.w-v2.w); }


  inline friend double operator* (const csDVector4& v1, const csDVector4& v2)
  { return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z + v1.w*v2.w; }


  inline friend csDVector4 operator% (const csDVector4& v1, const csDVector4& v2)
  {

    return csDVector4 ( (v1.x*v2.y - v1.y*v2.x) + (v1.x*v2.z - v1.z*v2.x) + (v1.y*v2.z - v1.z*v2.y),
                        (v1.z*v2.y - v1.y*v2.z) + (v1.y*v2.w - v1.w*v2.y) + (v1.z*v2.w - v1.w*v2.z),
                        (v1.x*v2.z - v1.z-v2.x) + (v1.w*v2.x - v1.x*v2.w) + (v1.z*v2.w - v1.w*v2.z),
                        (v1.y*v1.x - v1.x*v2.y) + (v1.w*v2.x - v1.x*v2.w) + (v1.w*v2.y - v1.y*v2.w) );
  }


  void Cross (const csDVector4 & v1, const csDVector4 & v2)
  {
    x = (v1.x*v2.y - v1.y*v2.x) + (v1.x*v2.z - v1.z*v2.x) + (v1.y*v2.z - v1.z*v2.y);
    y = (v1.z*v2.y - v1.y*v2.z) + (v1.y*v2.w - v1.w*v2.y) + (v1.z*v2.w - v1.w*v2.z);
    z = (v1.x*v2.z - v1.z-v2.x) + (v1.w*v2.x - v1.x*v2.w) + (v1.z*v2.w - v1.w*v2.z);
    w = (v1.y*v1.x - v1.x*v2.y) + (v1.w*v2.x - v1.x*v2.w) + (v1.w*v2.y - v1.y*v2.w);
  }


  inline friend csDVector4 operator* (const csDVector4& v, double f)
  { return csDVector4(v.x*f, v.y*f, v.z*f, v.w*f); }


  inline friend csDVector4 operator* (double f, const csDVector4& v)
  { return csDVector4(v.x*f, v.y*f, v.z*f, v.w*f); }


  inline friend csDVector4 operator/ (const csDVector4& v, double f)
  { f = 1.0f/f; return csDVector4(v.x*f, v.y*f, v.z*f, v.w*f); }


  inline friend bool operator== (const csDVector4& v1, const csDVector4& v2)
  { return v1.x==v2.x && v1.y==v2.y && v1.z==v2.z && v1.w == v2.w; }


  inline friend bool operator!= (const csDVector4& v1, const csDVector4& v2)
  { return v1.x!=v2.x || v1.y!=v2.y || v1.z!=v2.z || v1.w!=v2.w; }


  inline friend
  csDVector4 operator>> (const csDVector4& v1, const csDVector4& v2)
  { return v2*(v1*v2)/(v2*v2); }


  inline friend
  csDVector4 operator<< (const csDVector4& v1, const csDVector4& v2)
  { return v1*(v1*v2)/(v1*v1); }


  inline friend bool operator< (const csDVector4& v, double f)
  { return ((v.x)<0?-(v.x):(v.x))<f && ((v.y)<0?-(v.y):(v.y))<f && ((v.z)<0?-(v.z):(v.z))<f && ((v.w)<0?-(v.w):(v.w))<f; }


  inline friend bool operator> (double f, const csDVector4& v)
  { return ((v.x)<0?-(v.x):(v.x))<f && ((v.y)<0?-(v.y):(v.y))<f && ((v.z)<0?-(v.z):(v.z))<f && ((v.w)<0?-(v.w):(v.w))<f; }


  inline double operator[](int n) const {return !n?x:n&1?y:n&2?z:w;}


  inline double & operator[](int n){return !n?x:n&1?y:n&2?z:w;}


  inline csDVector4& operator+= (const csDVector4& v)
  {
    x += v.x;
    y += v.y;
    z += v.z;
    w += v.w;

    return *this;
  }


  inline csDVector4& operator-= (const csDVector4& v)
  {
    x -= v.x;
    y -= v.y;
    z -= v.z;
    w -= v.w;

    return *this;
  }


  inline csDVector4& operator*= (double f)
  { x *= f; y *= f; z *= f; w *= f; return *this; }


  inline csDVector4& operator/= (double f)
  { x /= f; y /= f; z /= f; w /= f; return *this; }


  inline csDVector4 operator+ () const { return *this; }


  inline csDVector4 operator- () const { return csDVector4(-x,-y,-z,-w); }


  inline void Set (double sx, double sy, double sz, double sw) { x = sx; y = sy; z = sz; w = sw; }


  double Norm () const;


  double SquaredNorm () const;






  csDVector4 Unit () const { return (*this)/(this->Norm()); }


  inline static double Norm (const csDVector4& v) { return v.Norm(); }


  inline static csDVector4 Unit (const csDVector4& v) { return v.Unit(); }


  void Normalize();
};





class csVector4
{
public:

  float x;

  float y;

  float z;

  float w;






  csVector4 () {}






  csVector4 (float m) : x(m), y(m), z(m), w(m) {}


  csVector4 (float ix, float iy, float iz = 0, float iw = 1) : x(ix), y(iy), z(iz), w(iw) {}


  csVector4 (const csVector4& v) : x(v.x), y(v.y), z(v.z), w(v.w) {}


  csVector4 (const csDVector4 &v);


  csVector4 (const csVector3 &v) : x(v.x), y(v.y), z(v.z), w(1.0f) {}


  inline friend csVector4 operator+ (const csVector4& v1, const csVector4& v2)
  { return csVector4(v1.x+v2.x, v1.y+v2.y, v1.z+v2.z, v1.w+v2.w); }


  inline friend csDVector4 operator+ (const csDVector4& v1, const csVector4& v2)
  { return csDVector4(v1.x+v2.x, v1.y+v2.y, v1.z+v2.z, v1.w+v2.w); }


  inline friend csDVector4 operator+ (const csVector4& v1, const csDVector4& v2)
  { return csDVector4(v1.x+v2.x, v1.y+v2.y, v1.z+v2.z, v1.w+v2.w); }


  inline friend csVector4 operator- (const csVector4& v1, const csVector4& v2)
  { return csVector4(v1.x-v2.x, v1.y-v2.y, v1.z-v2.z, v1.w-v2.w); }


  inline friend csDVector4 operator- (const csVector4& v1, const csDVector4& v2)
  { return csDVector4(v1.x-v2.x, v1.y-v2.y, v1.z-v2.z, v1.w-v2.w); }


  inline friend csDVector4 operator- (const csDVector4& v1, const csVector4& v2)
  { return csDVector4(v1.x-v2.x, v1.y-v2.y, v1.z-v2.z, v1.w-v2.w); }



  inline friend float operator* (const csVector4& v1, const csVector4& v2)
  { return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z + v1.w*v2.w; }


  inline friend csVector4 operator% (const csVector4& v1, const csVector4& v2)
  {
    return csVector4 ( (v1.x*v2.y - v1.y*v2.x) + (v1.x*v2.z - v1.z*v2.x) + (v1.y*v2.z - v1.z*v2.y),
                        (v1.z*v2.y - v1.y*v2.z) + (v1.y*v2.w - v1.w*v2.y) + (v1.z*v2.w - v1.w*v2.z),
                        (v1.x*v2.z - v1.z*v2.x) + (v1.w*v2.x - v1.x*v2.w) + (v1.z*v2.w - v1.w*v2.z),
                        (v1.y*v1.x - v1.x*v2.y) + (v1.w*v2.x - v1.x*v2.w) + (v1.w*v2.y - v1.y*v2.w) );
  }


  void Cross (const csVector4 & v1, const csVector4 & v2)
  {
    x = (v1.x*v2.y - v1.y*v2.x) + (v1.x*v2.z - v1.z*v2.x) + (v1.y*v2.z - v1.z*v2.y);
    y = (v1.z*v2.y - v1.y*v2.z) + (v1.y*v2.w - v1.w*v2.y) + (v1.z*v2.w - v1.w*v2.z);
    z = (v1.x*v2.z - v1.z*v2.x) + (v1.w*v2.x - v1.x*v2.w) + (v1.z*v2.w - v1.w*v2.z);
    w = (v1.y*v1.x - v1.x*v2.y) + (v1.w*v2.x - v1.x*v2.w) + (v1.w*v2.y - v1.y*v2.w);
  }


  inline friend csVector4 operator* (const csVector4& v, float f)
  { return csVector4(v.x*f, v.y*f, v.z*f, v.w*f); }


  inline friend csVector4 operator* (float f, const csVector4& v)
  { return csVector4(v.x*f, v.y*f, v.z*f, v.w*f); }


  inline friend csDVector4 operator* (const csVector4& v, double f)
  { return csDVector4(v) * f; }


  inline friend csDVector4 operator* (double f, const csVector4& v)
  { return csDVector4(v) * f; }


  inline friend csVector4 operator* (const csVector4& v, int f)
  { return v * (float)f; }


  inline friend csVector4 operator* (int f, const csVector4& v)
  { return v * (float)f; }


  inline friend csVector4 operator/ (const csVector4& v, float f)
  { f = 1.0f/f; return csVector4(v.x*f, v.y*f, v.z*f, v.w*f); }


  inline friend csDVector4 operator/ (const csVector4& v, double f)
  { return csDVector4(v) / f; }


  inline friend csVector4 operator/ (const csVector4& v, int f)
  { return v / (float)f; }


  inline friend bool operator== (const csVector4& v1, const csVector4& v2)
  { return v1.x==v2.x && v1.y==v2.y && v1.z==v2.z && v1.w==v2.w; }


  inline friend bool operator!= (const csVector4& v1, const csVector4& v2)
  { return v1.x!=v2.x || v1.y!=v2.y || v1.z!=v2.z || v1.w!=v2.w; }


  inline friend csVector4 operator>> (const csVector4& v1, const csVector4& v2)
  { return v2*(v1*v2)/(v2*v2); }


  inline friend csVector4 operator<< (const csVector4& v1, const csVector4& v2)
  { return v1*(v1*v2)/(v1*v1); }


  inline friend bool operator< (const csVector4& v, float f)
  { return ((v.x)<0?-(v.x):(v.x))<f && ((v.y)<0?-(v.y):(v.y))<f && ((v.z)<0?-(v.z):(v.z))<f && ((v.w)<0?-(v.w):(v.w))<f; }


  inline friend bool operator> (float f, const csVector4& v)
  { return ((v.x)<0?-(v.x):(v.x))<f && ((v.y)<0?-(v.y):(v.y))<f && ((v.z)<0?-(v.z):(v.z))<f && ((v.w)<0?-(v.w):(v.w))<f; }


  inline float operator[] (int n) const { return !n?x:n&1?y:n&2?z:w; }


  inline float & operator[] (int n) { return !n?x:n&1?y:n&2?z:w; }


  inline csVector4& operator+= (const csVector4& v)
  {
    x += v.x;
    y += v.y;
    z += v.z;
    w += v.w;

    return *this;
  }


  inline csVector4& operator-= (const csVector4& v)
  {
    x -= v.x;
    y -= v.y;
    z -= v.z;
    w -= v.w;

    return *this;
  }


  inline csVector4& operator*= (float f)
  { x *= f; y *= f; z *= f; w *= f; return *this; }


  inline csVector4& operator/= (float f)
  { f = 1.0f / f; x *= f; y *= f; z *= f; w *= f; return *this; }


  inline csVector4 operator+ () const { return *this; }


  inline csVector4 operator- () const { return csVector4(-x,-y,-z, -w); }


  inline void Set (float sx, float sy, float sz, float sw) { x = sx; y = sy; z = sz; w = sw; }


  inline void Set (const csVector4& v) { x = v.x; y = v.y; z = v.z; w = v.w; }


  float Norm () const;


  float SquaredNorm () const
  { return x * x + y * y + z * z + w * w; }






  csVector4 Unit () const { return (*this)/(this->Norm()); }


  inline static float Norm (const csVector4& v) { return v.Norm(); }


  inline static csVector4 Unit (const csVector4& v) { return v.Unit(); }


  void Normalize ();


  inline bool IsZero (float precision = 0.000001f) const
  { return (((x)<0?-(x):(x)) < precision) && (((y)<0?-(y):(y)) < precision)
            && (((z)<0?-(z):(z)) < precision) && (((w)<0?-(w):(w)) < precision);
  }
};
# 25 "f:/source-wip/CS/include/ivideo/shader/shader.h" 2

# 1 "f:/source-wip/CS/include/csutil/refarr.h" 1
# 28 "f:/source-wip/CS/include/csutil/refarr.h"
template <class T>
class csRefArray
{
private:
  int count, limit, threshold;
  csRef<T>* root;

public:
  typedef int ArrayCompareFunction (T* item1, T* item2);
  typedef int ArrayCompareKeyFunction (T* item, void* key);





  csRefArray (int ilimit = 0, int ithreshold = 0)
  {
    count = 0;
    limit = (ilimit > 0 ? ilimit : 0);
    threshold = (ithreshold > 0 ? ithreshold : 16);
    if (limit != 0)
      root = (csRef<T>*)calloc (limit, sizeof(csRef<T>));
    else
      root = 0;
  }







  void TransferTo (csRefArray<T>& destination)
  {
    destination.DeleteAll ();
    destination.root = root;
    destination.count = count;
    destination.limit = limit;
    destination.threshold = threshold;
    root = 0;
    limit = count = 0;
  }




  void DeleteAll ()
  {
    if (root)
    {
      int i;
      for (i = 0 ; i < limit ; i++)
        root[i] = 0;
      free (root);
      root = 0;
      limit = count = 0;
    }
  }




  ~csRefArray ()
  {
    DeleteAll ();
  }


  void SetLength (int n)
  {

    int i;
    for (i = n ; i < count ; i++) root[i] = 0;

    int old_count = count;
    count = n;

    if (n > limit || (limit > threshold && n < limit - threshold))
    {
      n = ((n + threshold - 1) / threshold ) * threshold;
      if (!n)
      {
        DeleteAll ();
      }
      else if (root == 0)
        root = (csRef<T>*)calloc (n, sizeof(csRef<T>));
      else
      {
        csRef<T>* newroot = (csRef<T>*)calloc (n, sizeof(csRef<T>));
        memcpy (newroot, root, old_count * sizeof (csRef<T>));
        free (root);
        root = newroot;
      }
      limit = n;
    }
  }


  int Length () const
  {
    return count;
  }


  int Limit () const
  {
    return limit;
  }


  const csRef<T>& Get (int n) const
  {
    ;
    return root[n];
  }


  const csRef<T>& operator [] (int n) const
  {
    ;
    return root[n];
  }


  csRef<T>& operator [] (int n)
  {
    ;
    if (n >= count)
      SetLength (n + 1);
    return root[n];
  }


  int Find (T* which) const
  {
    int i;
    for (i = 0 ; i < Length () ; i++)
      if (root[i] == which)
        return i;
    return -1;
  }


  int Push (T* what)
  {
    SetLength (count + 1);
    root [count - 1] = what;
    return (count - 1);
  }


  int PushSmart (T* what)
  {
    int n = Find (what);
    return (n == -1) ? Push (what) : n;
  }


  csPtr<T> Pop ()
  {
    csRef<T> ret = root [count - 1];
    SetLength (count - 1);
    return csPtr<T> (ret);
  }


  T* Top () const
  {
    return root [count - 1];
  }


  bool Delete (int n)
  {
    if (n >= 0 && n < count)
    {
      root[n] = 0;
      const int ncount = count - 1;
      const int nmove = ncount - n;
      if (nmove > 0)
      {
        memmove (&root [n], &root [n + 1], nmove * sizeof (csRef<T>));


        if (root[ncount])
          root[ncount]->IncRef ();
      }
      SetLength (ncount);
      return true;
    }
    else
      return false;
  }


  bool Delete (T* item)
  {
    int n = Find (item);
    if (n == -1) return false;
    else return Delete (n);
  }


  bool Insert (int n, T* item)
  {
    if (n <= count)
    {
      SetLength (count + 1);
      const int nmove = (count - n - 1);
      if (nmove > 0)
      {
        memmove (&root [n + 1], &root [n], nmove * sizeof (csRef<T>));




        if (root[n])
          root[n]->IncRef ();
      }
      root [n] = item;
      return true;
    }
    else
     return false;
  }




  int FindSortedKey (void* key, ArrayCompareKeyFunction* comparekey) const
  {
    int l = 0, r = Length () - 1;
    while (l <= r)
    {
      int m = (l + r) / 2;
      int cmp = comparekey (root [m], key);

      if (cmp == 0)
        return m;
      else if (cmp < 0)
        l = m + 1;
      else
        r = m - 1;
    }
    return -1;
  }





  int InsertSorted (T* item, ArrayCompareFunction* compare)
  {
    int m = 0, l = 0, r = Length () - 1;
    while (l <= r)
    {
      m = (l + r) / 2;
      int cmp = compare (root [m], item);

      if (cmp == 0)
      {
        Insert (++m, item);
        return m;
      }
      else if (cmp < 0)
        l = m + 1;
      else
        r = m - 1;
    }
    if (r == m)
      m++;
    Insert (m, item);
    return m;
  }


  void QuickSort (ArrayCompareFunction* compare)
  {
    if (count > 0)
      QuickSort (0, count - 1, compare);
  }


  void QuickSort (int Left, int Right, ArrayCompareFunction* compare)
  {
  recurse:
    int i = Left, j = Right;
    int x = (Left + Right) / 2;
    do
    {
      while ((i != x) && (compare (root[i], root[x]) < 0))
        i++;
      while ((j != x) && (compare (root[j], root[x]) > 0))
        j--;
      if (i < j)
      {
        csRef<T> swap;
        swap = root[i];
        root[i] = root[j];
        root[j] = swap;
        if (x == i)
          x = j;
        else if (x == j)
          x = i;
      }
      if (i <= j)
      {
        i++;
        if (j > Left)
          j--;
      }
    } while (i <= j);

    if (j - Left < Right - i)
    {
      if (Left < j)
        QuickSort (Left, j, compare);
      if (i < Right)
      {
        Left = i;
        goto recurse;
      }
    }
    else
    {
      if (i < Right)
        QuickSort (i, Right, compare);
      if (Left < j)
      {
        Right = j;
        goto recurse;
      }
    }
  }

};
# 27 "f:/source-wip/CS/include/ivideo/shader/shader.h" 2





struct iString;
struct iDataBuffer;
struct iDocumentNode;
class csSymbolTable;
struct iMaterial;

struct iShaderManager;
struct iShaderRenderInterface;
struct iShader;
struct iShaderWrapper;
struct iShaderVariable;
struct iShaderTechnique;
struct iShaderPass;
struct iShaderProgram;
struct iShaderProgramPlugin;

const int iShaderManager_VERSION = ((0 << 24) | (1 << 16) | 0); inline static scfInterfaceID iShaderManager_scfGetID () { static scfInterfaceID ID = (scfInterfaceID)-1; if (ID == (scfInterfaceID)(-1)) ID = iSCF::SCF->GetInterfaceID ("iShaderManager"); return ID; };



struct iShaderManager : iBase
{

  virtual csPtr<iShader> CreateShader() = 0;

  virtual iShader* GetShader(const char* name) = 0;

  virtual csPtr<iShaderWrapper> CreateWrapper(iShader* shader) = 0;

  virtual const csRefArray<iShaderWrapper> &GetShaders () = 0;


  virtual csPtr<iShaderVariable> CreateVariable(const char* name) const = 0;

  virtual bool AddVariable(iShaderVariable* variable) = 0;

  virtual iShaderVariable* GetVariable(int namehash) = 0;

  virtual csBasicVector GetAllVariableNames() const = 0;

  virtual csSymbolTable* GetSymbolTable() = 0;


  virtual csPtr<iShaderProgram> CreateShaderProgram(const char* type) = 0;
};

const int iShaderRenderInterface_VERSION = ((0 << 24) | (0 << 16) | 1); inline static scfInterfaceID iShaderRenderInterface_scfGetID () { static scfInterfaceID ID = (scfInterfaceID)-1; if (ID == (scfInterfaceID)(-1)) ID = iSCF::SCF->GetInterfaceID ("iShaderRenderInterface"); return ID; };
struct iShaderRenderInterface : iBase
{

  virtual void* GetPrivateObject(const char* name) = 0;
};

const int iShader_VERSION = ((0 << 24) | (0 << 16) | 1); inline static scfInterfaceID iShader_scfGetID () { static scfInterfaceID ID = (scfInterfaceID)-1; if (ID == (scfInterfaceID)(-1)) ID = iSCF::SCF->GetInterfaceID ("iShader"); return ID; };



struct iShader : iBase
{

  virtual void SetName(const char* name) = 0;

  virtual const char* GetName() = 0;


  virtual csPtr<iShaderTechnique> CreateTechnique() = 0;

  virtual int GetTechniqueCount() const = 0;

  virtual iShaderTechnique* GetTechnique(int technique) = 0;

  virtual iShaderTechnique* GetBestTechnique() = 0;


  virtual bool AddVariable(iShaderVariable* variable) = 0;

  virtual iShaderVariable* GetVariable(int namehash) = 0;

  virtual csBasicVector GetAllVariableNames() const = 0;

  virtual csSymbolTable* GetSymbolTable() = 0;


  virtual bool IsValid() const = 0;


  virtual bool Load(iDataBuffer* program) = 0;


  virtual bool Load(iDocumentNode* node) = 0;


  virtual bool Prepare() = 0;
};

const int iShaderWrapper_VERSION = ((0 << 24) | (0 << 16) | 1); inline static scfInterfaceID iShaderWrapper_scfGetID () { static scfInterfaceID ID = (scfInterfaceID)-1; if (ID == (scfInterfaceID)(-1)) ID = iSCF::SCF->GetInterfaceID ("iShaderWrapper"); return ID; };




struct iShaderWrapper : iBase
{

  virtual iShader* GetShader() = 0;


  virtual void SelectMaterial(iMaterial* mat) = 0;


  virtual csSymbolTable* GetSymbolTable() = 0;
};




const int iShaderVariable_VERSION = ((0 << 24) | (0 << 16) | 1); inline static scfInterfaceID iShaderVariable_scfGetID () { static scfInterfaceID ID = (scfInterfaceID)-1; if (ID == (scfInterfaceID)(-1)) ID = iSCF::SCF->GetInterfaceID ("iShaderVariable"); return ID; };
struct iShaderVariable : iBase
{
  enum VariableType
  {
    INT = 1,
    STRING,
    FLOAT,
    VECTOR3,
    VECTOR4
  };

  virtual VariableType GetType() const = 0;
  virtual void SetType(VariableType) = 0;

  virtual int GetHash() const = 0;

  virtual void SetName(const char*) = 0;
  virtual const char* GetName() const = 0;
  virtual bool GetValue(int& value) const = 0;
  virtual bool GetValue(float& value) const = 0;
  virtual bool GetValue(iString*& value) const = 0;
  virtual bool GetValue(csVector3& value) const = 0;
  virtual bool GetValue(csVector4& value) const = 0;
  virtual bool SetValue(int value) = 0;
  virtual bool SetValue(float value) = 0;
  virtual bool SetValue(iString* value) = 0;
  virtual bool SetValue(const csVector3 &value) = 0;
  virtual bool SetValue(const csVector4 &value) = 0;
};

const int iShaderTechnique_VERSION = ((0 << 24) | (0 << 16) | 1); inline static scfInterfaceID iShaderTechnique_scfGetID () { static scfInterfaceID ID = (scfInterfaceID)-1; if (ID == (scfInterfaceID)(-1)) ID = iSCF::SCF->GetInterfaceID ("iShaderTechnique"); return ID; };



struct iShaderTechnique : iBase
{




  virtual int GetPriority() const = 0;


  virtual void SetPriority(int priority) = 0;


  virtual csPtr<iShaderPass> CreatePass() = 0;

  virtual int GetPassCount() const = 0;

  virtual iShaderPass* GetPass( int pass ) = 0;


  virtual bool IsValid() const = 0;


  virtual bool Load(iDataBuffer* program) = 0;


  virtual bool Load(iDocumentNode* node) = 0;


  virtual bool Prepare() = 0;


  virtual bool AddVariable(iShaderVariable* variable) = 0;

  virtual iShaderVariable* GetVariable(int namehash) = 0;

  virtual csBasicVector GetAllVariableNames() const = 0;

  virtual csSymbolTable* GetSymbolTable() = 0;
};

const int iShaderPass_VERSION = ((0 << 24) | (0 << 16) | 1); inline static scfInterfaceID iShaderPass_scfGetID () { static scfInterfaceID ID = (scfInterfaceID)-1; if (ID == (scfInterfaceID)(-1)) ID = iSCF::SCF->GetInterfaceID ("iShaderPass"); return ID; };



struct iShaderPass : iBase
{

  virtual void AddStreamMapping (csStringID name, csVertexAttrib attribute) = 0;

  virtual csStringID GetStreamMapping (csVertexAttrib attribute) const = 0;


  virtual void AddTextureMapping (const char* name, int unit) = 0;

  virtual void AddTextureMapping (int layer, int unit) = 0;

  virtual int GetTextureMappingAsLayer (int unit) const = 0;

  virtual iTextureHandle* GetTextureMappingAsDirect (int unit) = 0;


  virtual uint GetMixmodeOverride () const = 0;


  virtual iShaderProgram* GetVertexProgram() = 0;


  virtual void SetVertexProgram(iShaderProgram* program) = 0;


  virtual iShaderProgram* GetFragmentProgram() = 0;


  virtual void SetFragmentProgram(iShaderProgram* program) = 0;


  virtual bool IsValid() const = 0;


  virtual void Activate(csRenderMesh* mesh) = 0;


  virtual void Deactivate() = 0;


  virtual void SetupState (csRenderMesh* mesh) = 0;


  virtual void ResetState () = 0;


  virtual bool AddVariable(iShaderVariable* variable) = 0;

  virtual iShaderVariable* GetVariable(int namehash) = 0;

  virtual csBasicVector GetAllVariableNames() const = 0;

  virtual csSymbolTable* GetSymbolTable() = 0;


  virtual bool Load(iDataBuffer* program) = 0;


  virtual bool Load(iDocumentNode* node) = 0;


  virtual bool Prepare() = 0;
};

const int iShaderProgram_VERSION = ((0 << 24) | (0 << 16) | 1); inline static scfInterfaceID iShaderProgram_scfGetID () { static scfInterfaceID ID = (scfInterfaceID)-1; if (ID == (scfInterfaceID)(-1)) ID = iSCF::SCF->GetInterfaceID ("iShaderProgram"); return ID; };




struct iShaderProgram : iBase
{

  virtual csPtr<iString> GetProgramID() = 0;


  virtual void Activate(iShaderPass* current, csRenderMesh* mesh) = 0;


  virtual void Deactivate(iShaderPass* current) = 0;


  virtual void SetupState (iShaderPass* current, csRenderMesh* mesh) = 0;


  virtual void ResetState () = 0;




  virtual bool GetProperty(const char* name, iString* string) = 0;
  virtual bool GetProperty(const char* name, int* string) = 0;
  virtual bool GetProperty(const char* name, csVector3* string) = 0;





  virtual bool SetProperty(const char* name, iString* string) = 0;
  virtual bool SetProperty(const char* name, int* string) = 0;
  virtual bool SetProperty(const char* name, csVector3* string) = 0;



  virtual bool AddVariable(iShaderVariable* variable) = 0;

  virtual iShaderVariable* GetVariable(int namehash) = 0;

  virtual csBasicVector GetAllVariableNames() = 0;

  virtual csSymbolTable* GetSymbolTable() = 0;


  virtual bool IsValid() = 0;


  virtual bool Load(iDataBuffer* program) = 0;


  virtual bool Load(iDocumentNode* node) = 0;



  virtual bool Prepare() = 0;
};

const int iShaderProgramPlugin_VERSION = ((0 << 24) | (0 << 16) | 1); inline static scfInterfaceID iShaderProgramPlugin_scfGetID () { static scfInterfaceID ID = (scfInterfaceID)-1; if (ID == (scfInterfaceID)(-1)) ID = iSCF::SCF->GetInterfaceID ("iShaderProgramPlugin"); return ID; };
struct iShaderProgramPlugin : iBase
{
  virtual csPtr<iShaderProgram> CreateProgram(const char* type) = 0;
  virtual bool SupportType(const char* type) = 0;
  virtual void Open() = 0;
};
# 29 "lightiter.h" 2

# 1 "../common/basesteptype.h" 1
# 25 "../common/basesteptype.h"
# 1 "f:/source-wip/CS/include/iutil/objreg.h" 1
# 41 "f:/source-wip/CS/include/iutil/objreg.h"
struct iObjectRegistryIterator;

const int iObjectRegistry_VERSION = ((0 << 24) | (0 << 16) | 4); inline static scfInterfaceID iObjectRegistry_scfGetID () { static scfInterfaceID ID = (scfInterfaceID)-1; if (ID == (scfInterfaceID)(-1)) ID = iSCF::SCF->GetInterfaceID ("iObjectRegistry"); return ID; };





struct iObjectRegistry : public iBase
{

  inline static iObjectRegistry* __ImplicitPtrCast (iObjectRegistry* ptr) { return ptr; };




  virtual void Clear () = 0;
# 71 "f:/source-wip/CS/include/iutil/objreg.h"
  virtual bool Register (iBase*, char const* tag = 0) = 0;
# 80 "f:/source-wip/CS/include/iutil/objreg.h"
  virtual void Unregister (iBase*, char const* tag = 0) = 0;





  virtual iBase* Get (char const* tag) = 0;
# 95 "f:/source-wip/CS/include/iutil/objreg.h"
  virtual iBase* Get (char const* tag, scfInterfaceID id, int version) = 0;







  virtual csPtr<iObjectRegistryIterator> Get (
        scfInterfaceID id, int version) = 0;







  virtual csPtr<iObjectRegistryIterator> Get () = 0;
};

const int iObjectRegistryIterator_VERSION = ((0 << 24) | (1 << 16) | 0); inline static scfInterfaceID iObjectRegistryIterator_scfGetID () { static scfInterfaceID ID = (scfInterfaceID)-1; if (ID == (scfInterfaceID)(-1)) ID = iSCF::SCF->GetInterfaceID ("iObjectRegistryIterator"); return ID; };





struct iObjectRegistryIterator : public iBase
{




  virtual bool Reset () = 0;




  virtual const char* GetCurrentTag () = 0;




  virtual bool HasNext () = 0;




  virtual iBase* Next () = 0;
};
# 26 "../common/basesteptype.h" 2
# 1 "f:/source-wip/CS/include/ivideo/rendersteps/irsfact.h" 1
# 33 "f:/source-wip/CS/include/ivideo/rendersteps/irsfact.h"
struct iRenderStep;

const int iRenderStepFactory_VERSION = ((0 << 24) | (0 << 16) | 1); inline static scfInterfaceID iRenderStepFactory_scfGetID () { static scfInterfaceID ID = (scfInterfaceID)-1; if (ID == (scfInterfaceID)(-1)) ID = iSCF::SCF->GetInterfaceID ("iRenderStepFactory"); return ID; };




struct iRenderStepFactory : public iBase
{



  virtual csPtr<iRenderStep> Create () = 0;
};

const int iRenderStepType_VERSION = ((0 << 24) | (0 << 16) | 1); inline static scfInterfaceID iRenderStepType_scfGetID () { static scfInterfaceID ID = (scfInterfaceID)-1; if (ID == (scfInterfaceID)(-1)) ID = iSCF::SCF->GetInterfaceID ("iRenderStepType"); return ID; };





struct iRenderStepType : public iBase
{



  virtual csPtr<iRenderStepFactory> NewFactory() = 0;
};
# 27 "../common/basesteptype.h" 2

class csBaseRenderStepType : public iComponent, public iRenderStepType
{
protected:
  csRef<iObjectRegistry> object_reg;
public:
  int scfRefCount; public: iBase *scfParent; virtual void IncRef (); virtual void DecRef (); virtual int GetRefCount (); virtual void *QueryInterface (scfInterfaceID iInterfaceID, int iVersion);

  csBaseRenderStepType (iBase *p);
  virtual ~csBaseRenderStepType ();

  virtual bool Initialize(iObjectRegistry *object_reg);
};
# 31 "lightiter.h" 2
# 1 "../common/basesteploader.h" 1
# 26 "../common/basesteploader.h"
# 1 "f:/source-wip/CS/include/imap/reader.h" 1
# 28 "f:/source-wip/CS/include/imap/reader.h"
struct iLoaderContext;
struct iDocumentNode;

const int iLoaderPlugin_VERSION = ((0 << 24) | (2 << 16) | 1); inline static scfInterfaceID iLoaderPlugin_scfGetID () { static scfInterfaceID ID = (scfInterfaceID)-1; if (ID == (scfInterfaceID)(-1)) ID = iSCF::SCF->GetInterfaceID ("iLoaderPlugin"); return ID; };




struct iLoaderPlugin : public iBase
{

  virtual csPtr<iBase> Parse (iDocumentNode* node, iLoaderContext* ldr_context,
        iBase* context) = 0;
};

const int iBinaryLoaderPlugin_VERSION = ((0 << 24) | (0 << 16) | 1); inline static scfInterfaceID iBinaryLoaderPlugin_scfGetID () { static scfInterfaceID ID = (scfInterfaceID)-1; if (ID == (scfInterfaceID)(-1)) ID = iSCF::SCF->GetInterfaceID ("iBinaryLoaderPlugin"); return ID; };




struct iBinaryLoaderPlugin : public iBase
{

  virtual csPtr<iBase> Parse (void* data, iLoaderContext* ldr_context,
        iBase* context) = 0;
};
# 27 "../common/basesteploader.h" 2
# 1 "f:/source-wip/CS/include/imap/services.h" 1
# 29 "f:/source-wip/CS/include/imap/services.h"
class csMatrix3;
class csVector3;
class csVector2;
class csVector;
class csColor;
class csBox3;
class csGradient;
struct iPolygon3DStatic;
struct iEngine;
struct iSector;
struct iMaterialWrapper;
struct iThingFactoryState;
struct iLoaderContext;
struct iDocumentNode;
# 56 "f:/source-wip/CS/include/imap/services.h"
const int iSyntaxService_VERSION = ((1 << 24) | (3 << 16) | 0); inline static scfInterfaceID iSyntaxService_scfGetID () { static scfInterfaceID ID = (scfInterfaceID)-1; if (ID == (scfInterfaceID)(-1)) ID = iSCF::SCF->GetInterfaceID ("iSyntaxService"); return ID; };





struct iSyntaxService : public iBase
{
# 78 "f:/source-wip/CS/include/imap/services.h"
  virtual bool ParseBool (iDocumentNode* node, bool& result,
        bool def_result) = 0;




  virtual bool ParseMatrix (iDocumentNode* node, csMatrix3 &m) = 0;




  virtual bool ParseVector (iDocumentNode* node, csVector3 &v) = 0;




  virtual bool ParseBox (iDocumentNode* node, csBox3 &v) = 0;




  virtual bool ParseColor (iDocumentNode* node, csColor &c) = 0;




  virtual bool ParseMixmode (iDocumentNode* node, uint &mixmode) = 0;
# 127 "f:/source-wip/CS/include/imap/services.h"
  virtual bool ParseTextureMapping (iDocumentNode* node,
                             const csVector3* vref, uint &texspec,
                             csVector3 &tx_orig, csVector3 &tx1,
                             csVector3 &tx2, csVector3 &len,
                             csMatrix3 &tx_m, csVector3 &tx_v,
                             csVector2 &uv_shift,
                             int &idx1, csVector2 &uv1,
                             int &idx2, csVector2 &uv2,
                             int &idx3, csVector2 &uv3,
                             const char *polyname) = 0;





  virtual bool ParsePortal (iDocumentNode* node, iLoaderContext* ldr_context,
                           iPolygon3DStatic* poly3d,
                           csVector &flags, bool &mirror,
                           bool& warp, int& msv,
                           csMatrix3 &m, csVector3 &before,
                           csVector3 &after) = 0;





  virtual bool ParsePoly3d (iDocumentNode* node,
                            iLoaderContext* ldr_context,
                            iEngine* engine, iPolygon3DStatic* poly3d,
                            float default_texlen,
                            iThingFactoryState* thing_fact_state,
                            int vt_offset) = 0;




  virtual bool ParseGradient (iDocumentNode* node,
                              csGradient& gradient) = 0;




  virtual void ReportError (const char* msgid, iDocumentNode* errornode,
        const char* msg, ...) = 0;





  virtual void ReportBadToken (iDocumentNode* badtokennode) = 0;




  virtual void Report (const char* msgid, int severity,
    iDocumentNode* errornode, const char* msg, ...) = 0;
};
# 28 "../common/basesteploader.h" 2


class csBaseRenderStepLoader : public iLoaderPlugin, public iComponent
{
protected:
  csRef<iObjectRegistry> object_reg;
  csRef<iSyntaxService> synldr;
public:
  int scfRefCount; public: iBase *scfParent; virtual void IncRef (); virtual void DecRef (); virtual int GetRefCount (); virtual void *QueryInterface (scfInterfaceID iInterfaceID, int iVersion);

  csBaseRenderStepLoader (iBase *p);
  virtual ~csBaseRenderStepLoader ();

  virtual bool Initialize(iObjectRegistry *object_reg);

  virtual csPtr<iBase> Parse (iDocumentNode* node, iLoaderContext* ldr_context,
        iBase* context) = 0;
};
# 32 "lightiter.h" 2
# 1 "../common/parserenderstep.h" 1
# 24 "../common/parserenderstep.h"
# 1 "f:/source-wip/CS/include/iutil/plugin.h" 1
# 33 "f:/source-wip/CS/include/iutil/plugin.h"
struct iComponent;
# 55 "f:/source-wip/CS/include/iutil/plugin.h"
const int iPluginIterator_VERSION = ((0 << 24) | (0 << 16) | 1); inline static scfInterfaceID iPluginIterator_scfGetID () { static scfInterfaceID ID = (scfInterfaceID)-1; if (ID == (scfInterfaceID)(-1)) ID = iSCF::SCF->GetInterfaceID ("iPluginIterator"); return ID; };




struct iPluginIterator : public iBase
{

  virtual bool HasNext () = 0;

  virtual iBase* Next () = 0;
};
# 75 "f:/source-wip/CS/include/iutil/plugin.h"
const int iPluginManager_VERSION = ((0 << 24) | (2 << 16) | 0); inline static scfInterfaceID iPluginManager_scfGetID () { static scfInterfaceID ID = (scfInterfaceID)-1; if (ID == (scfInterfaceID)(-1)) ID = iSCF::SCF->GetInterfaceID ("iPluginManager"); return ID; };





struct iPluginManager : public iBase
{





  virtual iBase *LoadPlugin (const char *classID,
    const char *iInterface = 0, int iVersion = 0, bool init = true) = 0;
# 98 "f:/source-wip/CS/include/iutil/plugin.h"
  virtual iBase *QueryPlugin (const char *iInterface, int iVersion) = 0;

  virtual iBase *QueryPlugin (const char* classID,
        const char *iInterface, int iVersion) = 0;

  virtual bool UnloadPlugin (iComponent *obj) = 0;

  virtual bool RegisterPlugin (const char *classID, iComponent *obj) = 0;






  virtual csPtr<iPluginIterator> GetPlugins () = 0;

  virtual void Clear () = 0;







  virtual void QueryOptions (iComponent* object) = 0;
};
# 25 "../common/parserenderstep.h" 2

class csRenderStepParser
{
  csRef<iObjectRegistry> object_reg;
  csRef<iSyntaxService> synldr;
  csRef<iPluginManager> plugmgr;

  csStringHash tokens;

# 1 "f:/source-wip/CS/include/cstool/tokenlist.h" 1
# 88 "f:/source-wip/CS/include/cstool/tokenlist.h"
enum {
# 1 "f:/source-wip/CS/plugins/video/render3d/renderloop/common/parserenderstep.tok" 1
XMLTOKEN_STEP,
# 90 "f:/source-wip/CS/include/cstool/tokenlist.h" 2
};





static void init_token_table(csStringHash& t)
{
  csString s;
# 1 "f:/source-wip/CS/plugins/video/render3d/renderloop/common/parserenderstep.tok" 1
s = "STEP"; s.strlwr(); t.Register(s, XMLTOKEN_STEP);
# 100 "f:/source-wip/CS/include/cstool/tokenlist.h" 2
}
# 35 "../common/parserenderstep.h" 2

public:
  bool Initialize(iObjectRegistry *object_reg);

  csPtr<iRenderStep> Parse (iObjectRegistry* object_reg,
    iDocumentNode* node);
  bool ParseRenderSteps (iRenderStepContainer* container,
    iDocumentNode* node);
};
# 33 "lightiter.h" 2

class csLightIterRSType : public csBaseRenderStepType
{
public:
  csLightIterRSType (iBase* p);

  virtual csPtr<iRenderStepFactory> NewFactory();
};

class csLightIterRSLoader : public csBaseRenderStepLoader
{
  csRenderStepParser rsp;

  csStringHash tokens;



public:
  csLightIterRSLoader (iBase* p);

  virtual bool Initialize (iObjectRegistry* object_reg);

  virtual csPtr<iBase> Parse (iDocumentNode* node,
    iLoaderContext* ldr_context,
    iBase* context);
};

class csLightIterRenderStepFactory : public iRenderStepFactory
{
  csRef<iObjectRegistry> object_reg;
public:
  int scfRefCount; public: iBase *scfParent; virtual void IncRef (); virtual void DecRef (); virtual int GetRefCount (); virtual void *QueryInterface (scfInterfaceID iInterfaceID, int iVersion);

  csLightIterRenderStepFactory (iObjectRegistry* object_reg);

  virtual csPtr<iRenderStep> Create ();
};

class csLightIterRenderStep : public iRenderStep,
                              public iLightIterRenderStep,
                              public iRenderStepContainer
{
private:
  csRefArray<iLightRenderStep> steps;
public:
  int scfRefCount; public: iBase *scfParent; virtual void IncRef (); virtual void DecRef (); virtual int GetRefCount (); virtual void *QueryInterface (scfInterfaceID iInterfaceID, int iVersion);

  csLightIterRenderStep (iObjectRegistry* object_reg);

  virtual void Perform (iRenderView* rview, iSector* sector);

  virtual int AddStep (iRenderStep* step);
  virtual int GetStepCount ();
};
# 32 "lightiter.cpp" 2

static inline void csLightIterRSType_scfUnitInitialize(iSCF* SCF) { iSCF::SCF = SCF; } extern "C" void csLightIterRSType_scfInitialize(iSCF* SCF) { csLightIterRSType_scfUnitInitialize(SCF); } void cs_static_var_cleanup (void (*p)()); extern "C" void csLightIterRSType_scfFinalize() { cs_static_var_cleanup (0); iSCF::SCF = 0; } extern "C" void* csLightIterRSType_Create(iBase *iParent) { void *ret = new csLightIterRSType (iParent); ; return ret; };
static inline void csLightIterRSLoader_scfUnitInitialize(iSCF* SCF) { iSCF::SCF = SCF; } extern "C" void csLightIterRSLoader_scfInitialize(iSCF* SCF) { csLightIterRSLoader_scfUnitInitialize(SCF); } void cs_static_var_cleanup (void (*p)()); extern "C" void csLightIterRSLoader_scfFinalize() { cs_static_var_cleanup (0); iSCF::SCF = 0; } extern "C" void* csLightIterRSLoader_Create(iBase *iParent) { void *ret = new csLightIterRSLoader (iParent); ; return ret; };



csLightIterRSType::csLightIterRSType (iBase* p) : csBaseRenderStepType (p)
{
}

csPtr<iRenderStepFactory> csLightIterRSType::NewFactory()
{
  return csPtr<iRenderStepFactory>
    (new csLightIterRenderStepFactory (object_reg));
}



csLightIterRSLoader::csLightIterRSLoader (iBase* p) : csBaseRenderStepLoader (p)
{
  init_token_table (tokens);
}

bool csLightIterRSLoader::Initialize (iObjectRegistry* object_reg)
{
  if (csBaseRenderStepLoader::Initialize (object_reg))
  {
    return rsp.Initialize (object_reg);
  }
  else
  {
    return false;
  }
}

csPtr<iBase> csLightIterRSLoader::Parse (iDocumentNode* node,
                                       iLoaderContext* ldr_context,
                                       iBase* context)
{
  csRef<iLightIterRenderStep> step =
    new csLightIterRenderStep (object_reg);

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    csStringID id = tokens.Request (child->GetValue ());
    switch (id)
    {
      default:
        if (synldr) synldr->ReportBadToken (child);
        return 0;
    }
  }

  return csPtr<iBase> (step);
}



void csLightIterRenderStepFactory::IncRef () { ; scfRefCount++; } void csLightIterRenderStepFactory::DecRef () { if (scfRefCount == 1) { ; if (scfParent) scfParent->DecRef (); delete this; return; } scfRefCount--; } int csLightIterRenderStepFactory::GetRefCount () { return scfRefCount; } void *csLightIterRenderStepFactory::QueryInterface (scfInterfaceID iInterfaceID, int iVersion) { ;;
  static scfInterfaceID iRenderStepFactory_scfID = (scfInterfaceID)-1; if (iRenderStepFactory_scfID == (scfInterfaceID)-1) iRenderStepFactory_scfID = iSCF::SCF->GetInterfaceID ("iRenderStepFactory"); if (iInterfaceID == iRenderStepFactory_scfID && scfCompatibleVersion (iVersion, iRenderStepFactory_VERSION)) { (this)->IncRef (); return (static_cast<iRenderStepFactory*>(this)); };
return scfParent->QueryInterface (iInterfaceID, iVersion); };

csLightIterRenderStepFactory::csLightIterRenderStepFactory (
  iObjectRegistry* object_reg)
{
  scfRefCount = 1; scfParent = 0; if (scfParent) scfParent->IncRef();;

  csLightIterRenderStepFactory::object_reg = object_reg;
}

csPtr<iRenderStep> csLightIterRenderStepFactory::Create ()
{
  return csPtr<iRenderStep>
    (new csLightIterRenderStep (object_reg));
}



void csLightIterRenderStep::IncRef () { ; scfRefCount++; } void csLightIterRenderStep::DecRef () { if (scfRefCount == 1) { ; if (scfParent) scfParent->DecRef (); delete this; return; } scfRefCount--; } int csLightIterRenderStep::GetRefCount () { return scfRefCount; } void *csLightIterRenderStep::QueryInterface (scfInterfaceID iInterfaceID, int iVersion) { ;;
  static scfInterfaceID iRenderStep_scfID = (scfInterfaceID)-1; if (iRenderStep_scfID == (scfInterfaceID)-1) iRenderStep_scfID = iSCF::SCF->GetInterfaceID ("iRenderStep"); if (iInterfaceID == iRenderStep_scfID && scfCompatibleVersion (iVersion, iRenderStep_VERSION)) { (this)->IncRef (); return (static_cast<iRenderStep*>(this)); };
  static scfInterfaceID iRenderStepContainer_scfID = (scfInterfaceID)-1; if (iRenderStepContainer_scfID == (scfInterfaceID)-1) iRenderStepContainer_scfID = iSCF::SCF->GetInterfaceID ("iRenderStepContainer"); if (iInterfaceID == iRenderStepContainer_scfID && scfCompatibleVersion (iVersion, iRenderStepContainer_VERSION)) { (this)->IncRef (); return (static_cast<iRenderStepContainer*>(this)); };
  static scfInterfaceID iLightIterRenderStep_scfID = (scfInterfaceID)-1; if (iLightIterRenderStep_scfID == (scfInterfaceID)-1) iLightIterRenderStep_scfID = iSCF::SCF->GetInterfaceID ("iLightIterRenderStep"); if (iInterfaceID == iLightIterRenderStep_scfID && scfCompatibleVersion (iVersion, iLightIterRenderStep_VERSION)) { (this)->IncRef (); return (static_cast<iLightIterRenderStep*>(this)); };
return scfParent->QueryInterface (iInterfaceID, iVersion); };

csLightIterRenderStep::csLightIterRenderStep (
  iObjectRegistry* object_reg)
{
  scfRefCount = 1; scfParent = 0; if (scfParent) scfParent->IncRef();;
}

void csLightIterRenderStep::Perform (iRenderView* rview, iSector* sector)
{
  iGraphics3D* r3d = rview->GetGraphics3D();

  r3d->SetLightParameter (0, 2,
    csVector3 (0, 0, 0));

  iLightList* lights = sector->GetLights();

  int nlights = lights->GetCount();

  while (nlights-- > 0)
  {
    iLight* light = lights->Get (nlights);
    const csVector3 lightPos = light->GetCenter ();





    csReversibleTransform camTransR =
      rview->GetCamera()->GetTransform();
    r3d->SetObjectToCamera (&camTransR);

    const csColor& color = light->GetColor ();
    r3d->SetLightParameter (0, 1,
    csVector3 (color.red, color.green, color.blue));

    r3d->SetLightParameter (0, 3,
      light->GetAttenuationVector ());
    r3d->SetLightParameter (0, 0,
      lightPos);

    csSphere lightSphere (lightPos, light->GetInfluenceRadius ());
    if (rview->TestBSphere (camTransR, lightSphere))
    {
      int i;
      for (i = 0; i < steps.Length(); i++)
      {
        steps[i]->Perform (rview, sector, light);
      }
    }
  }
}

int csLightIterRenderStep::AddStep (iRenderStep* step)
{
  csRef<iLightRenderStep> lrs =
    csPtr<iLightRenderStep> ((iLightRenderStep *)(step)->QueryInterface ( iLightRenderStep_scfGetID (), iLightRenderStep_VERSION));
  if (!lrs) return -1;
  return steps.Push (lrs);
}

int csLightIterRenderStep::GetStepCount ()
{
  return steps.Length();
}
