#ifndef PHYZTYPE_H
#define PHYZTYPE_H

const double M_PER_WORLDUNIT = 1.0;

#define DEFAULT_AIR_RESISTANCE	0.25L /**M_PER_WORLDUNIT */  // 2.5 is ok for bigger stuff
#define DEFAULT_JOINT_FRICTION	0.04 //40.0L //40.0L  // 40 is nice //!me increase to 500 for blow-up condition

// physical constants
#define PHYZ_CONSTANT_G		6.67e-11 * M_PER_WORLDUNIT * M_PER_WORLDUNIT


#define MIN_REAL 0.000000001
#define MAX_REAL 100000000.0

typedef double real;
typedef double coord;
typedef float angle;

/*class csMatrix3;
class csVector3;

#define ctVector3 csVector3
#define ctMatrix3 csMatrix3
*/

//#define ctVector3 ctVector3
//#define ctMatrix3 ctMatrix3

#endif
