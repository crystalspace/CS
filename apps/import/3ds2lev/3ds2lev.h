
#ifndef __CS_3DS2LEV_H__
#define __CS_3DS2LEV_H__

#include <lib3ds/types.h>

#define  FLAG_VERBOSE       0x0001    /* Verbose mode on                  */
#define  FLAG_VERYVERBOSE   0x0002    /* Very verbose mode on             */
#define  FLAG_OVERWR        0x0004    /* Don't ask for file overwrite     */
#define  FLAG_LIGHTING      0x0008    /* Lighting enabled for polygons    */
#define  FLAG_CENTRE        0x0010    /* Centre objects                   */
#define  FLAG_SCALE         0x0020    /* Scale objects                    */
#define  FLAG_RELOCATE      0x0040    /* Relocate objects                 */
#define  FLAG_NORMDUP       0x0200    /* Don't remove duplicated vertices */
#define  FLAG_MODEL         0x0400    /* Only output one model from 3ds   */
#define  FLAG_SPRITE        0x0800    /* Output 3D sprite instead level   */
#define  FLAG_LIST          0x1000    /* List all objects in this 3ds     */
#define  FLAG_SWAP_V        0x2000    /* Texture origin lower left    */


typedef unsigned char   byte;
typedef unsigned short  word;
typedef unsigned long   dword;
typedef float           float32;


class cs3ds2LevConverter
{

public:

  /// constructor
  cs3ds2LevConverter();

  /// destructor
  ~cs3ds2LevConverter();

  // loads a 3ds file
  Lib3dsFile * LoadFile( char *filename );
};

#endif // __CS_3DS2LEV_H__
