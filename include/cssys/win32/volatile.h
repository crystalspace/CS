/*
    You can change these macros to suit your own needs
    For a description of what each macro does, see mk/user.mak
*/
#ifndef __VOLATILE_H__
#define __VOLATILE_H__

  #define OS_WIN32
  #define PROC_INTEL
  #define COMP_VC
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
  #undef DO_DINPUT_KEYBOARD // undefine this if you have problems with keyboard handling

  //#ifdef _DEBUG
    //Right now, Inline assembler doesn' work any more on MSVC, 
    //This needs to be examined further. Thomas Hieber. 07/17/1999
    #define NO_ASSEMBLER
  //#endif

#endif // __VOLATILE_H__
