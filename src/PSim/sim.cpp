
// psim.cpp [psim]

// Copyright 2004-2006 Jasmine Langridge, jas@jareiko.net
// License: GPL version 2 (see included gpl.txt)

//
// Physic related functions
// Among others here we have creation of PVehicles and PRigidBody, loading of PVehicleType and simulation tick
//

#include "psim.h"


///
/// @brief constructor
///
PSim::PSim() : terrain(nullptr), gravity(vec3f::zero())
{
}

///
/// @brief destructor
///
PSim::~PSim()
{
  clear();
}

///
/// @brief Load a vehicle type from a file
/// @param filename = name of the file to load from
/// @param ssModel = the PSSModel of the vehicle type
/// @retval the newly loaded vehicle type, or nullptr if failed to load
///
PVehicleType *PSim::loadVehicleType(const std::string &filename, PSSModel &ssModel)
{
  PVehicleType *vtype = vtypelist.find(filename);
  if (!vtype) {
    vtype = new PVehicleType();
    if (!vtype->load(filename, ssModel)) {
      if (PUtil::isDebugLevel(DEBUGLEVEL_ENDUSER))
        PUtil::outLog() << "Failed to load " << filename << "\n";
      return nullptr;
    }
    vtypelist.add(vtype);
  }
  return vtype;
}

///
/// @brief Put a new rigid body in the body vector
/// @retval the pointer to the new rigid body
///
PRigidBody *PSim::createRigidBody()
{
  PRigidBody *newbody = new PRigidBody(*this);

  body.push_back(newbody);

  return newbody;
}

///
/// @brief Create a new vehicle and put it in the vehicle vector
/// @retval the pointer to the newly created vehicle, nullptr if problems occurred
///
PVehicle *PSim::createVehicle(XMLElement *element, const std::string &filepath, PSSModel &ssModel)
{
  const char *val;

  const char *type = element->Attribute("type");
  if (!type) {
    PUtil::outLog() << "Vehicle has no type\n";
    return nullptr;
  }

  vec3f pos = vec3f::zero();

  val = element->Attribute("pos");
  if (val) sscanf(val, "%f , %f , %f", &pos.x, &pos.y, &pos.z);

  quatf ori = quatf::identity();

  val = element->Attribute("ori");
  if (val) sscanf(val, "%f , %f , %f , %f", &ori.w, &ori.x, &ori.y, &ori.z);

  return createVehicle(type, pos, ori, filepath, ssModel);
}

///
/// @brief Create a new vehicle and put it in the vehicle vector
/// @retval the pointer to the newly created vehicle, nullptr if problems occurred
///
PVehicle *PSim::createVehicle(const std::string &type, const vec3f &pos, const quatf &ori, const std::string &filepath, PSSModel &ssModel)
{
  PVehicleType *vtype = loadVehicleType(PUtil::assemblePath(type, filepath), ssModel);

  return createVehicle(vtype, pos, ori /*, ssModel */);
}

///
/// @brief Create a new vehicle and put it in the vehicle vector
/// @retval the pointer to the newly created vehicle, nullptr if problems occurred
///
PVehicle *PSim::createVehicle(PVehicleType *type, const vec3f &pos, const quatf &ori /*, PSSModel &ssModel */)
{
  if (!type) return nullptr;

  PVehicle *newvehicle = new PVehicle(*this, type);

  vec3f vpos = pos;
  if (terrain) vpos.z += terrain->getHeight(vpos.x, vpos.y);
  newvehicle->getBody().setPosition(vpos);

  newvehicle->getBody().setOrientation(ori);
  newvehicle->getBody().updateMatrices();

  newvehicle->updateParts();

  vehicle.push_back(newvehicle);
  return newvehicle;
}

///
/// @brief Clear from the simulation any rigid body, vehicle and vehicle types
///
void PSim::clear()
{
  // clear bodies
  for (unsigned int i=0; i<body.size(); ++i)
    delete body[i];
  body.clear();

  // clear vehicles
  for (unsigned int i=0; i<vehicle.size(); ++i)
    delete vehicle[i];
  vehicle.clear();

  // clear vehicle types
  vtypelist.clear();
}

///
/// @brief Calls the vehicles and bodies ticks and update the parts of the vehicles
/// @param delta = how much time to compute
///
void PSim::tick(float delta)
{
	if (delta <= 0.0) return;

	/*
	 * old code that uses variable step size
	 * 
	// Find a new timeslice similar to 0.005 so we can do an int number (num) of equal ticks in the delta interval
	float timeslice = 0.005;
	int num = (int)(delta / timeslice) + 1;
	timeslice = delta / (float)num;
	// do 'num' ticks each of 'timeslice' length
	for (int timestep=0; timestep<num; ++timestep) {
	*/
	
	// size of a step
	const float timeslice = 0.004;

	// how much time remains to simulate
	static float t = 0;
	t += delta;
	
	while(t >= timeslice)
	{
		// update time
		t -= timeslice;
		
		// tick for vehicles
		for (unsigned int i=0; i<vehicle.size(); ++i)
			vehicle[i]->tick(timeslice);
		
		// tick for rigid bodies
		for (unsigned int i=0; i<body.size(); ++i)
			body[i]->tick(timeslice);
		
		// Update vehicles parts
		for (unsigned int i=0; i<vehicle.size(); ++i)
			vehicle[i]->updateParts();
	}
}
