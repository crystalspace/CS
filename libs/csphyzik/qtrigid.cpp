/*
    Dynamics/Kinematics modeling and simulation library.
    Copyright (C) 1999 by Michael Alexander Ewert and Noah Gibbs

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#include <assert.h>
#include "cssysdef.h"
#include "csphyzik/ctvector.h"
#include "csphyzik/ctmatrix.h"
#include "csphyzik/qtrigid.h"
#include "csphyzik/ctquat.h"
#include "csphyzik/qtrbconn.h"
#include "csphyzik/phyzent.h"

ctQuatRigidBody::ctQuatRigidBody(ctVector3 x, real M)
{
  mass = M;
  pos = x;
  mom[0] = mom[1] = mom[2] = 0.0;
  quat.r = 1.0; quat.x = quat.y = quat.z = 0.0;
  ang[0] = ang[1] = ang[2] = 0.0;

  F[0] = F[1] = F[2] = 0.0;
  T[0] = T[1] = T[2] = 0.0;

  precalculated = false;
}

int ctQuatRigidBody::get_state(const float *sa)
{
  pos[0] = sa[0];
  pos[1] = sa[1];
  pos[2] = sa[2];
  mom[0] = sa[3];
  mom[1] = sa[4];
  mom[2] = sa[5];
  quat.r = sa[6];
  quat.x = sa[7];
  quat.y = sa[8];
  quat.z = sa[9];
  ang[0] = sa[10];
  ang[1] = sa[11];
  ang[2] = sa[12];

  return get_state_size ();
}

int ctQuatRigidBody::set_state(real *sa)
{
  sa[0]  = pos[0];
  sa[1]  = pos[1];
  sa[2]  = pos[2];
  sa[3]  = mom[0];
  sa[4]  = mom[1];
  sa[5]  = mom[2];
  sa[6]  = quat.r;
  sa[7]  = quat.x;
  sa[8]  = quat.y;
  sa[9]  = quat.z;
  sa[10] = ang[0];
  sa[11] = ang[1];
  sa[12] = ang[2];

  precalculated = false;

  return get_state_size ();
}

int ctQuatRigidBody::set_delta_state(real *sa)
{
  Precalculate ();  // just in case

  sa[0] = mom[0] / mass;
  sa[1] = mom[1] / mass;
  sa[2] = mom[2] / mass;
  sa[3] = F[0] / mass;
  sa[4] = F[1] / mass;
  sa[5] = F[2] / mass;

  ctQuaternion qdot (get_omega () * 0.5);
  qdot *= quat;
  sa[6] = qdot.r;
  sa[7] = qdot.x;
  sa[8] = qdot.y;
  sa[9] = qdot.z;

  sa[10] = T[0];
  sa[11] = T[1];
  sa[12] = T[2];
  return get_state_size ();
}

ctQuatRigidBodyConnector *ctQuatRigidBody::new_connector(ctVector3 offs)
{
  ctQuatRigidBodyConnector *ret = new ctQuatRigidBodyConnector(this, offs);
  assert(ret);
  return ret;
}

void ctQuatRigidBody::Precalculate ()
{
  if(precalculated) return;
  quat.Normalize();
  R = quat.to_matrix();
  Iinv = R * Ibodyinv * R.get_transpose ();
  omega = Iinv * ang;
  precalculated = true;
}
