/*
    Dynamics/Kinematics modeling and simulation library.
    Copyright (C) 2000 by Michael Alexander Ewert and Noah Gibbs

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

#include "cssysdef.h"
#include "csphyzik/qtrbconn.h"
#include "csphyzik/qtrigid.h"
#include "csphyzik/phyzent.h"

ctQuatRigidBodyConnector::ctQuatRigidBodyConnector(ctQuatRigidBody *rb,
						   ctVector3 offset)
  : rigid(rb), r(offset) {}

ctVector3 ctQuatRigidBodyConnector::pos ()
{
  ctVector3 pt = rigid->get_orientation().Rotate(r);
  pt += rigid->get_center();
  return pt;
}

ctVector3 ctQuatRigidBodyConnector::vel ()
{
  ctVector3 pt = rigid->get_orientation ().Rotate (r);
  ctVector3 av;
  av.Cross(rigid->get_omega (), pt);
  return av + rigid->get_linear_velocity ();
}

void ctQuatRigidBodyConnector::apply_force(ctVector3 F)
{
  ctVector3 torque;

  torque.Cross (r, F);
  rigid->apply_T (torque);
  rigid->apply_F (F);
}

int ctQuatRigidBodyConnector::num_state_vars ()
{
  return rigid->get_state_size ();
}

int ctQuatRigidBodyConnector::state_index(int i)
{
  return rigid->get_state_offset () + i;
}

// Returns the derivative of the position wrt any state var
// pos = q * (0,r) * qconj + center
ctVector3 ctQuatRigidBodyConnector::dp (int i)
{
  if (i < 0)
    return ctVector3(0.0, 0.0, 0.0);

  else if (i < 3)
  {
    // derivative wrt center
    switch (i)
    {
      case 0:
	return ctVector3(1.0, 0.0, 0.0);
      case 1:
	return ctVector3(0.0, 1.0, 0.0);
      case 2:
	return ctVector3(0.0, 0.0, 1.0);
      default:
	assert(0 && "Should never get here in switch in dp(int i)");
    }

    assert(0 && "Should never get here after switch in dp(int i)");
  }
  else if (i < 6)
  {
    // derivative wrt linear momentum
    return ctVector3(0.0, 0.0, 0.0);
  }
  else if (i < 10)
  {
    // Derivative wrt quaternion
    // Center term vanishes, so just d (q * (0,r) * qconj) / d q
    int offs = i - 6;  // 0, 1, 2 or 3

    ctVector3 retval;
    ctQuaternion quat (rigid->get_orientation ());
    // Memo to self:  never work this stuff out by hand again.  Ugh.
    if (offs == 0)
    {
      // dpos/dr
      retval[0] = (r[0] * 2.0 * quat.r
		   - r[1] * 2.0 * quat.z
		   + r[2] * 2.0 * quat.y);
      retval[1] = (r[0] * 2.0 * quat.z
		   + r[1] * 2.0 * quat.r
		   - r[2] * 2.0 * quat.x);
      retval[2] = (-r[0] * 2.0 * quat.y
		   + r[1] * 2.0 * quat.x
		   + r[2] * 2.0 * quat.r);
    }
    else
    {
      // dpos/dx, dpos/dy, dpos/dz
      ctVector3 qvec(quat.x, quat.y, quat.z);
      offs--; // Now 0, 1, or 2 for x, y or z
      int cur1 = (offs + 1) % 3;
      int cur2 = (cur1 + 1) % 3;

      retval[offs] = (r[offs] * 2.0 * qvec[offs]
		      + r[cur1] * 2.0 * qvec[cur1]
		      + r[cur2] * 2.0 * qvec[cur2]);
      retval[cur1] = (r[offs] * 2.0 * qvec[cur1]
		      + r[cur1] * -2.0 * qvec[offs]
		      + r[cur2] * -2.0 * quat.r);
      retval[cur2] = (r[offs] * 2.0 * qvec[cur2]
		      + r[cur1] * 2.0 * quat.r
		      + r[cur2] * -2.0 * qvec[offs]);
    }
    return retval;
  }
  else if (i < 13)
    // Derivative wrt angular momentum
    return ctVector3 (0.0, 0.0, 0.0);
  else
    return ctVector3 (0.0, 0.0, 0.0);

  assert(0 && "Should never get here in dp(int i)");
  return ctVector3(0.0, 0.0, 0.0);
}

// Returns the derivative of point velocity wrt any state var
ctVector3 ctQuatRigidBodyConnector::dpdot(int i)
{
  // Derivative of
  if (i<0)
    return ctVector3(0.0, 0.0, 0.0);
  if (i<3)
  {
    // Deriv of vel wrt center location
    return ctVector3(0.0, 0.0, 0.0);
  }
  else if (i<6)
  {
    // Deriv of vel wrt center linear momentum
    if (i==3) return ctVector3(1.0/rigid->get_mass(), 0.0, 0.0);
    if (i==4) return ctVector3(0.0, 1.0/rigid->get_mass(), 0.0);
    if (i==5) return ctVector3(0.0, 0.0, 1.0/rigid->get_mass());
    assert(0 && "Shouldn't get here calcing dpdot() wrt momentum");
  }
  else if (i<10)
  {
    // Deriv of vel wrt quat orientation
    // vel = omega x quat(r)
    // omega = Iinv * ang_mom
    // Iinv = R * Ibodyinv * R^T

    return ctVector3(0.0, 0.0, 0.0);
  }
  else if (i < 13)
  {
    // Deriv of vel wrt angular momentum
    // vel = omega x quat(r)
    // omega = Iinv * ang_mom
    ctVector3 qrot(rigid->get_orientation().Rotate(r));
    ctMatrix3 inertia(rigid->get_Iinv());

    // Build df/da matrix
    int offs;
    ctMatrix3 dfda;
    for (offs=0; offs<3; offs++)
    {
      int cur1 = (offs + 1) % 3;
      int cur2 = (cur1 + 1) % 3;

      dfda[offs][offs] =  0.0;
      dfda[offs][cur1] = -qrot[cur2];
      dfda[offs][cur2] =  qrot[cur1];
    }
    ctMatrix3 deriv (dfda * inertia.get_transpose ());
    offs = i - 10;  // 0, 1, 2 for x, y, or z of ang momentum
    return ctVector3 (deriv[offs][0], deriv[offs][1], deriv[offs][2]);
  }
  else
    return ctVector3(0.0, 0.0, 0.0);

  assert(0
	 && "Should never get this far calculating wrt angmom in dp(int i)");

  return ctVector3(0.0, 0.0, 0.0);
}
