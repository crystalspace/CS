#ifndef CT_COLLIDING_H
#define CT_COLLIDING_H

#include "csphyzik/phyztype.h"

class ctPhysicalEntity;


class ctCollidingContact
{
public:

  ctCollidingContact(){
    body_a = NULL;
    body_b = NULL;
    restitution = 1.0;
  }

  ctPhysicalEntity *body_a;  // body a.  one body involved in the contact
  // set body_b to NULL if it is an immovable object. e.g. the ground.
  // don't set body_a to NULL however.
  ctPhysicalEntity *body_b;  // body b.  other body involved in the contact

  ctVector3 contact_p;  // point of contact in world coords

  ctVector3 n;   // collision normal

  real restitution;  // coefficent of restitution.  what % of v bounces back. +ve

};

#endif