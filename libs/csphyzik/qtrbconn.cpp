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

#include "csphyzik/qtrbconn.h"
#include "csphyzik/qtrigid.h"

ctQuatRigidBodyConnector::ctQuatRigidBodyConnector(ctQuatRigidBody *rb,
						   ctVector3 offset)
  : rigid(rb), r(offset) {}

ctVector3 ctQuatRigidBodyConnector::pos() {
  ctVector3 pt = rigid->get_orientation().Rotate(r);
  pt += rigid->get_center();
  return pt;
}

ctVector3 ctQuatRigidBodyConnector::vel() {
  ctVector3 pt = rigid->get_orientation().Rotate(r);
  ctVector3 av;
  av.cross(rigid->get_omega(), pt);
  return av + rigid->get_linear_velocity();
}

void ctQuatRigidBodyConnector::apply_force(ctVector3 F) {
  ctVector3 torque;

  torque.cross(r, F);
  rigid->apply_T(torque);
  rigid->apply_F(F);
}
