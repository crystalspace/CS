/*
    You can change these macros to suit your own needs
    For a description of what each macro does, see mk/user.mak
*/
#ifndef __VOLATILE_H__
#define __VOLATILE_H__

#define OS_WINNT

#define PROC_INTEL

#if defined(__BORLANDC__)
#	define COMP_BC
#	define __NEED_GENERIC_ISDIR
#else
#	define COMP_GCC
#endif


// #ifdef _DEBUG
// Right now, Inline assembler doesn' work any more on MSVC, 
//  This needs to be examined further. Thomas Hieber. 07/17/1999
//    #define NO_ASSEMBLER
//#endif

#endif // __VOLATILE_H__
