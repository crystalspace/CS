
#ifndef __CS_3DS2LEV_H__
#define __CS_3DS2LEV_H__

#include <lib3ds/types.h>

#define  FLAG_VERBOSE       0x0001    /* Verbose mode on                  */
#define  FLAG_VERYVERBOSE   0x0002    /* Very verbose mode on             */
#define  FLAG_OVERWR        0x0004    /* Don't ask for file overwrite     */
#define  FLAG_LIGHTING      0x0008    /* Lighting enabled for polygons    */
#define  FLAG_CENTRE        0x0010    /* Centre objects                   */
#define  FLAG_MODEL         0x0400    /* Only output one model from 3ds   */
#define  FLAG_SPRITE        0x0800    /* Output 3D sprite instead level   */
#define  FLAG_LIST          0x1000    /* List all objects in this 3ds     */
#define  FLAG_SWAP_V        0x2000    /* Texture origin lower left    */
#define	 FLAG_COMBINEFACES  0x4000    /* combine triangles to polygions */
#define  FLAG_REMOVEDOUBLEVERTICES 0x10000 /* remove doubled vertices */

extern int flags;

#endif // __CS_3DS2LEV_H__

