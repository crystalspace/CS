#ifndef SPACETIMEOBJECT_H
#define SPACETIMEOBJECT_H

// hideously inefficient collision detection/response algorithm
// just wanted to see some stuff bouncing around for now. 
#include "csphyzik/ctcat.h"
#include "cstool/collider.h"
#include "ivaria/collider.h"
#include "isys/system.h"

class csEngine;
class csMeshWrapper;
class ctRigidBody;
class ctCollidingContact;
class ctWorld;
class ctLameCollisionCatastrophe;

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
  friend class ctLameCollisionCatastrophe;

  static csRigidSpaceTimeObj *space_time_continuum[ MAX_SPACE_TIME_NUM ];
  static long continuum_end;

  static void evolve_system ( real t1, real t2, 
			      ctWorld *time_world, csEngine *space_engine );

  csColliderWrapper *col;
  csMeshWrapper *sprt;
  ctVector3 prev_pos;
  ctRigidBody *rb;
  
  csCollisionPair cd_contact[MAX_COL_PER_STO];
  int num_collisions;

  ctCollidingContact *contact;

  csRigidSpaceTimeObj( iCollideSystem* cdsys, csMeshWrapper *psprt, ctRigidBody *prb );

protected:
  static void update_space();
  static real collision_check();
  static void collision_response();

};

class ctLameCollisionCatastrophe : public ctCatastropheManager
{
public: 
  // check for a catastrophe and return a real indicating the "magnitude"
  // of the worst ( bigger number ) catastrophe.  Return 0 for no catastrophe
  virtual real check_catastrophe();
  
  // take care of the catastrophe so that when integrated forward that
  // catasrophe will not exist.
  virtual void handle_catastrophe();
};

#endif
