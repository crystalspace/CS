/*
    You can change these macros to suit your own needs
    For a description of what each macro does, see mk/user.mak
*/
#ifndef __VOLATILE_H__
#define __VOLATILE_H__

  #define OS_WIN32
  #define PROC_INTEL
#if defined(__BORLANDC__)
  #define COMP_BC
  #define __NEED_GENERIC_ISDIR
#else
  #define COMP_VC
#endif
  #define DO_SOUND
  #define DO_GIF
  #define DO_BMP
  #define DO_TGA
  #define DO_PNG
  #define DO_JPG
  #define DO_AIFF
  #define DO_IFF
  #define DO_WAV
  #define DO_AU
  #define DO_MMX
  #define ZLIB_DLL
  #define DO_DINPUT_KEYBOARD // undefine this if you have problems with keyboard handling

  //#ifdef _DEBUG
    //Right now, Inline assembler doesn' work any more on MSVC, 
    //This needs to be examined further. Thomas Hieber. 07/17/1999
    #define NO_ASSEMBLER
  //#endif

#endif // __VOLATILE_H__
