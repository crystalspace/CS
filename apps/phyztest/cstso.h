#ifndef SPACETIMEOBJECT_H
#define SPACETIMEOBJECT_H

// hideously inefficient collision detection/response algorithm
// just wanted to see some stuff bouncing around for now. 

#include "csengine/collider.h"

class csWorld;
class csSprite3D;
class ctRigidBody;
class ctCollidingContact;
class ctWorld;

#define MAX_SPACE_TIME_NUM 100
#define MAX_COL_PER_STO 50
enum sttype { ST_SPACETIME, ST_RIGID };

enum collisionresult { COL_DONOTHING, COL_SPHEREIMPULSE, COL_DIRECTIONAL_IMPULSE };

class csSpaceTimeObj
{
public:

	sttype what_type;



	csSpaceTimeObj();

};

class csRigidSpaceTimeObj : public csSpaceTimeObj
{
public:
	static csRigidSpaceTimeObj *space_time_continuum[ MAX_SPACE_TIME_NUM ];
	static long continuum_end;

  static void evolve_system( real t1, real t2, ctWorld *time_world, csWorld *space_world );

	csCollider *col;
	csSprite3D *sprt;
	ctRigidBody *rb;
  
  collision_pair cd_contact[MAX_COL_PER_STO];
  int num_collisions;

	ctCollidingContact *contact;

	csRigidSpaceTimeObj( csSprite3D *psprt, ctRigidBody *prb );

protected:
  static void update_space();
  static real collision_check( csWorld *space );
  static void collision_response( csWorld *space );


};

#endif
