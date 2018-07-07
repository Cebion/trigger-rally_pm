
// psim.h [psim]

// Copyright 2004-2006 Jasmine Langridge, jas@jareiko.net
// License: GPL version 2 (see included gpl.txt)

//
// This file contains basic classes the physic engine uses
//

#include <pengine.h>


class PSim;
class PRigidBody;

class PVehicleType;
class PVehicleTypePart;
class PVehicle;

///
/// @brief Store a Reference oriented point (stores coordinates and orientation)
/// @details It is the base class for more advanced classes such as PRigidBody
///
class PReferenceFrame {
public:
  // position
  vec3f pos;
  // orientation
  quatf ori;
  // orientation matrix, and its inverse
  // not directly modify them, but modify ori then run UpdateMatrices()
  // @todo maybe make them private/protected?
  mat44f ori_mat, ori_mat_inv;

public:
  PReferenceFrame() : pos(vec3f::zero()), ori(quatf::identity()) {
    updateMatrices();
  }

  // Update the orientation matrices from quaternion orientation (ori)
  void updateMatrices() {
    ori.normalize();
    ori_mat = ori.getMatrix();
    ori_mat_inv = ori_mat.transpose();
  }

  // Position
  void setPosition(const vec3f &_pos) { pos = _pos; }
  vec3f getPosition() { return pos; }

  // Orientation
  void setOrientation(const quatf &_ori) { ori = _ori; }
  quatf getOrientation() { return ori; }
  mat44f getOrientationMatrix() { return ori_mat; }
  mat44f getInverseOrientationMatrix() { return ori_mat_inv; }

  // Change coordinate reference (From local to world system and viceversa)
  vec3f getLocToWorldVector(const vec3f &pt) {
    return ori_mat.transform1(pt);
  }
  vec3f getWorldToLocVector(const vec3f &pt) {
    return ori_mat.transform2(pt);
  }
  vec3f getLocToWorldPoint(const vec3f &pt) {
    return pos + ori_mat.transform1(pt);
  }
  vec3f getWorldToLocPoint(const vec3f &pt) {
    return ori_mat.transform2(pt - pos);
  }
};

///
/// @brief A Rigid body with mass, position, velocity, and angular mass, position, and velocity
/// @todo: intelligent friction calculation
///
class PRigidBody : public PReferenceFrame {
private:

  // Class that stores simulation informations (such as gravity force intensity)
  PSim &sim;

  // mass and 1/mass
  float mass, mass_inv;
  // inertial tensor (angular mass) and 1/angmass (usually just approximately computed)
  vec3f angmass, angmass_inv;

  // linear and angular velocity
  vec3f linvel;
  vec3f angvel;

  // the force and the torque accumuled during a slice of time
  vec3f accum_force;
  vec3f accum_torque;

public:
  PRigidBody(PSim &sim_parent);
  ~PRigidBody();

public:
  void setMassCuboid(float _mass, const vec3f &dim);

  void setLinearVel(const vec3f &vel) { linvel = vel; }
  const vec3f &getLinearVel() { return linvel; }

  void setAngularVel(const vec3f &vel) { angvel = vel; }
  const vec3f &getAngularVel() { return angvel; }

  void addForce(const vec3f &frc);
  void addLocForce(const vec3f &frc);
  void addForceAtPoint(const vec3f &frc, const vec3f &pt);
  void addLocForceAtPoint(const vec3f &frc, const vec3f &pt);
  void addForceAtLocPoint(const vec3f &frc, const vec3f &pt);
  void addLocForceAtLocPoint(const vec3f &frc, const vec3f &pt);

  void addTorque(const vec3f &trq);
  void addLocTorque(const vec3f &trq);

  vec3f getLinearVelAtPoint(const vec3f &pt);
  vec3f getLinearVelAtLocPoint(const vec3f &pt);

  // Step the simulation delta seconds
  void tick(float delta);

  friend class PSim;
};




#include "vehicle.h"

///
/// @brief class that stores information about a physic simulation instance
///
class PSim {
private:

  // the terrain class
  PTerrain *terrain;

  // the various types of vehicles
  PResourceList<PVehicleType> vtypelist;

  // the various bodyes inside the simulation
  std::vector<PRigidBody *> body;

  // the various vehicles inside the simulation
  std::vector<PVehicle *> vehicle;

  // the gravity vector
  vec3f gravity;
  
public:
  PSim();
  ~PSim();

  inline void setTerrain(PTerrain *new_terrain) { terrain = new_terrain; }
  inline PTerrain *getTerrain() { return terrain; }

  inline void setGravity(const vec3f &new_gravity) { gravity = new_gravity; }

  PVehicleType *loadVehicleType(const std::string &filename, PSSModel &ssModel);

  PRigidBody *createRigidBody();

  PVehicle *createVehicle(XMLElement *element, const std::string &filepath, PSSModel &ssModel);
  PVehicle *createVehicle(const std::string &type, const vec3f &pos, const quatf &ori, const std::string &filepath, PSSModel &ssModel);
  PVehicle *createVehicle(PVehicleType *type, const vec3f &pos, const quatf &ori /* , PSSModel &ssModel */);

  // Remove all bodies and vehicles
  void clear();

  // Step the simulation delta seconds
  void tick(float delta);

  friend class PRigidBody;
  friend class PVehicle;
};
