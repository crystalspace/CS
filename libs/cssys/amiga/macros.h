/*
**      $VER: macros.h 0.1 (19.5.97)
**
**
*/

#ifndef MACROS_H
#define MACROS_H

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif /* EXEC_TYPES_H */

/*
** macros for function definitions and declarations
**
*/

#ifdef __GNUC__
#define REG(xn, parm) parm __asm(#xn)
#define REGARGS __regargs
#define STDARGS __stdargs
#define SAVEDS __saveds
#define ALIGNED __attribute__ ((aligned(4))
#define FAR
#define CHIP
#else /* of __GNUC__ */

#ifdef __SASC
#define REG(xn, parm) register __ ## xn parm
#define REGARGS __asm
#define SAVEDS __saveds
#define ALIGNED __aligned
#define STDARGS __stdargs
#define FAR __far
#define CHIP __chip
#else /* of __SASC */

#ifdef _DCC
#define REG(xn, parm) __ ## xn parm
#define REGARGS
#define SAVEDS __geta4
#define FAR __far
#define CHIP __chip
#endif /* _DCC */

#endif /* __SASC */

#endif /* __GNUC__ */


#endif /* MACROS_H */
