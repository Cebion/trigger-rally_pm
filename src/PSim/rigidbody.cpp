
// rigidbody.cpp [psim]

// Copyright 2004-2006 Jasmine Langridge, jas@jareiko.net
// License: GPL version 2 (see included gpl.txt)

//
// PRigidbody class functions
// Here there are functions that manage the rigid bodies and related physic
//

#include "psim.h"

///
/// @brief initialize PRigidBody
///
PRigidBody::PRigidBody(PSim &sim_parent):
	PReferenceFrame(),
	sim(sim_parent),
	mass(1.0),
	mass_inv(1.0),
	angmass(vec3f(1.0,1.0,1.0)),
	angmass_inv(vec3f(1.0,1.0,1.0)),
	linvel(vec3f::zero()),
	angvel(vec3f::zero()),
	accum_force(vec3f::zero()),
	accum_torque(vec3f::zero())
{}

PRigidBody::~PRigidBody()
{}

///
/// @brief set a new mass as cuboid then update the angular mass
/// @param _mass  = new mass
/// @param rad    = the size of the cuboid
///
void PRigidBody::setMassCuboid(float _mass, const vec3f &rad)
{
  // if there is no mass or a dimension is zero or less return
  if (mass <= 0.0 ||
    rad.x <= 0.0 ||
    rad.y <= 0.0 ||
    rad.z <= 0.0) return;

  // set mass
  mass = _mass;
  mass_inv = 1.0 / mass;

  // set angular mass
  angmass = vec3f(rad.y*rad.z, rad.z*rad.x, rad.x*rad.y) * (mass * 0.4);

  angmass_inv.x = 1.0 / angmass.x;
  angmass_inv.y = 1.0 / angmass.y;
  angmass_inv.z = 1.0 / angmass.z;
}

///
/// @brief Add a force to the rigid body
/// @param frc = force in the world system
///
void PRigidBody::addForce(const vec3f &frc)
{
  accum_force += frc;
}

///
/// @brief Add a force to the rigid body
/// @param frc = force in the local system
///
void PRigidBody::addLocForce(const vec3f &frc)
{
  addForce(getLocToWorldVector(frc));
}

///
/// @brief Add a force to the rigid body
/// @param frc = force in the world system
/// @param pt = point where the force is applied in the world system
///
void PRigidBody::addForceAtPoint(const vec3f &frc, const vec3f &pt)
{
  accum_force += frc;

  vec3f wdiff = pt - pos;

  accum_torque += frc ^ wdiff;
  //accum_torque -= wdiff ^ frc;
}

///
/// @brief Add a force to the rigid body
/// @param frc = force in the local system
/// @param pt = point where the force is applied in the world system
///
void PRigidBody::addLocForceAtPoint(const vec3f &frc, const vec3f &pt)
{
  addForceAtPoint(getLocToWorldVector(frc), pt);
}

///
/// @brief Add a force to the rigid body
/// @param frc = force in the world system
/// @param pt = point where the force is applied in the local system
///
void PRigidBody::addForceAtLocPoint(const vec3f &frc, const vec3f &pt)
{
  addForceAtPoint(frc, getLocToWorldPoint(pt));
}

///
/// @brief Add a force to the rigid body
/// @param frc = force in the local system
/// @param pt = point where the force is applied in the local system
///
void PRigidBody::addLocForceAtLocPoint(const vec3f &frc, const vec3f &pt)
{
  addForceAtPoint(getLocToWorldVector(frc), getLocToWorldPoint(pt));
}

///
/// @brief Add a torque to the rigid body
/// @param trq = force in the world system
///
void PRigidBody::addTorque(const vec3f &trq)
{
  accum_torque += trq;
}

///
/// @brief Add a torque to the rigid body
/// @param trq = force in the local system
///
void PRigidBody::addLocTorque(const vec3f &trq)
{
  addTorque(getLocToWorldVector(trq));
}

///
/// @brief get the linear velocity of a point
/// @details it's linear velocity is derived from rigid body's linear velocity plus his angular velocity cross product with the distance from the rotation center
/// @param pt = point in the world system
///
vec3f PRigidBody::getLinearVelAtPoint(const vec3f &pt)
{
  vec3f usept = pt - pos;
  return (linvel + (usept ^ angvel));
}

/// @brief get the linear velocity of a point
/// @details it's linear velocity is derived from rigid body's linear velocity plus his angular velocity cross product with the distance from the rotation center
/// @param pt = point in the local system
///
vec3f PRigidBody::getLinearVelAtLocPoint(const vec3f &pt)
{
  return getLinearVelAtPoint(getLocToWorldPoint(pt));
}

// Uncomment this to prevent rigid body velocity and angular velocity go too high (see PRigidBody::tick() )
//#define CLAMPVEL

///
/// @brief Apply forces and torque accumulated, update the body accordingly
/// @param delta = the time slice to compute
///
void PRigidBody::tick(float delta)
{
  // update the linear velocity of the body
  linvel += (accum_force * mass_inv + sim.gravity) * delta;

#ifdef CLAMPVEL
  // Keep linvel inside a range of values
  CLAMP(linvel.x, -20.0, 20.0);
  CLAMP(linvel.y, -20.0, 20.0);
  CLAMP(linvel.z, -20.0, 20.0);
#endif

  // update the position of the body
  pos += linvel * delta;

#if 0
  mat44f ori_mat2;
  ori_mat2.assemble(
    vec3f(ori_mat.row[0][0]*0.5+0.5, ori_mat.row[0][1]*0.5+0.5, ori_mat.row[0][2]*0.5+0.5),
    vec3f(ori_mat.row[1][0]*0.5+0.5, ori_mat.row[1][1]*0.5+0.5, ori_mat.row[1][2]*0.5+0.5),
    vec3f(ori_mat.row[2][0]*0.5+0.5, ori_mat.row[2][1]*0.5+0.5, ori_mat.row[2][2]*0.5+0.5));

  vec3f angmass_inv_world = ori_mat2.transform2(angmass_inv);
#else
  vec3f angmass_inv_world = angmass_inv;
#endif

  vec3f ang_accel = vec3f(
    accum_torque.x * angmass_inv_world.x,
    accum_torque.y * angmass_inv_world.y,
    accum_torque.z * angmass_inv_world.z);

  // update the angular velocity
  angvel += ang_accel * delta;

#ifdef CLAMPVEL
  // keep angular velocity inside a range a value
  CLAMP(angvel.x, -20.0, 20.0);
  CLAMP(angvel.y, -20.0, 20.0);
  CLAMP(angvel.z, -20.0, 20.0);
#endif

  // update the orientation
  quatf angdelta;
  angdelta.fromThreeAxisAngle(angvel * delta);
  ori = ori * angdelta;
  //ori = angdelta * ori;

  //PULLTOWARD(linvel, vec3f::zero(), delta * 0.1);
  //PULLTOWARD(angvel, vec3f::zero(), delta * 0.1);

  // resetting
  accum_force = vec3f::zero();
  accum_torque = vec3f::zero();

  PReferenceFrame::updateMatrices();
}




