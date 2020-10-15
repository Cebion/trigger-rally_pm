
// vehicle.cpp

// Copyright 2004-2006 Jasmine Langridge, jas@jareiko.net
// Copyright 2018      Emanuele Sorce,    emanuele.sorce@hotmail.com
// License: GPL version 2 (see included gpl.txt)

#include "psim.h"
#include "collision.h"

// default vehicle type values
#define DEF_VEHICLE_NAME "Vehicle"
#define DEF_VEHICLE_CLASS "Unknown"

#define DEF_VEHICLE_MASS 1
#define DEF_VEHICLE_DIMS vec3f(1,1,1)
#define DEF_VEHICLE_WHEELSCALE 1

#define DEF_VEHICLE_SPEED 0
#define DEF_VEHICLE_TURNSPEED vec3f::zero()
#define DEF_VEHICLE_TURNSPEED_A 1
#define DEF_VEHICLE_TURNSPEED_B 0

#define DEF_VEHICLE_DRAG vec3f(1,1,1)
#define DEF_VEHICLE_ANGDRAG 1
#define DEF_VEHICLE_LIFT 1
#define DEF_VEHICLE_FINEFFECT vec2f::zero()

#define DEF_WHEEL_RADIUS 1
#define DEF_WHEEL_DRIVE 0
#define DEF_WHEEL_STEER 0
#define DEF_WHEEL_BRAKE1 0
#define DEF_WHEEL_BRAKE2 0
#define DEF_WHEEL_FORCE 0
#define DEF_WHEEL_DAMPENING 0
#define DEF_WHEEL_FRICTION 0.02

// When a car is reset, how much time to wait
#define VEHICLE_RESET_TIME 3
// When a car is upside down, how much time to wait before resetting
#define VEHICLE_UPSIDEDOWN_RESET_TIME 4

// This is a coefficent used to get the friction of a wheel or a clip with the ground
#define FRICTION_MAGIC_COEFF_CLIP 10000
#define FRICTION_MAGIC_COEFF_WHEEL (FRICTION_MAGIC_COEFF_CLIP * 50)

// How much of the wheel radius can the suspension be compressed
#define MAX_SUSPENSION_DEPTH_COEFF 0.7

// How much the vehicle sinks on the ground, besed of terrain resistance
#define SINK_COEFF 0.26

// How fast the wheel rotation velocity match with the velocity of the ground below
#define WHEEL_SPIN_VEL_UPDATE_RATIO 0.1

// under what distance from the ground the wheel will create a dust trail
#define MAX_DUST_TRAIL_HEIGHT 0.05

///
/// @brief PVehicleWheel constructor
///
PVehicleWheel::PVehicleWheel()
{
	reset();
}

/// @brief reset wheel status
void PVehicleWheel::reset()
{
	ride_pos = 0;
	ride_vel = 0;
	spin_pos = 0;
	spin_vel = 0;
	turn_pos = 0;
	bumplast = 0;
	bumpnext = 0;
	bumptravel = 0;
	skidding = 0;
	dirtthrow = 0;
	dirtthrowpos = vec3f::zero();
	dirtthrowvec = vec3f::zero();
}

///
/// @brief load a vehicle type from a file
///
bool PVehicleType::load(const std::string &filename, PSSModel &ssModel)
{
  if (PUtil::isDebugLevel(DEBUGLEVEL_TEST))
    PUtil::outLog() << "Loading vehicle type \"" << filename << "\"\n";

  name = filename;

  unload();

  // defaults
  proper_name = DEF_VEHICLE_NAME;
  proper_class = DEF_VEHICLE_CLASS;

  mass = DEF_VEHICLE_MASS;
  dims = DEF_VEHICLE_DIMS;

  wheelscale = DEF_VEHICLE_WHEELSCALE;
  wheelmodel = nullptr;

  ctrlrate.setDefaultRates();

  param.speed = DEF_VEHICLE_SPEED;
  param.turnspeed = DEF_VEHICLE_TURNSPEED;
  param.turnspeed_a = DEF_VEHICLE_TURNSPEED_A;
  param.turnspeed_b = DEF_VEHICLE_TURNSPEED_B;
  param.fineffect = DEF_VEHICLE_FINEFFECT;
  
	driving_wheels_num = 0;

	// the scale of the values in the file
	float allscale = 1.0;

  float drive_total = 0.0f;
  
	// temp values that store the respective .vehicle coefficents
	// not stored in param struct because will be done calculation on them
	vec3f drag = DEF_VEHICLE_DRAG;
	float angdrag = DEF_VEHICLE_ANGDRAG;
	float lift = DEF_VEHICLE_LIFT;

  wheel_speed_multiplier = 0.0f;

	// if the dimensions will be specificed in the file or just assumed using the 3D model
	bool custom_dims = false;

  // Read stats from file

  XMLDocument xmlfile;
  XMLElement *rootelem = PUtil::loadRootElement(xmlfile, filename, "vehicle");
  if (!rootelem) {
    PUtil::outLog() << "Load failed: TinyXML error\n";
    return false;
  }

  const char *val;

  val = rootelem->Attribute("name");
  if (val)
		proper_name = val;

  val = rootelem->Attribute("class");
  if (val)
    proper_class = val;

  val = rootelem->Attribute("allscale");
  if (val)
		allscale = atof(val);

  // vehicle type
  val = rootelem->Attribute("type");
  if (!val || !strlen(val)) {
    if (PUtil::isDebugLevel(DEBUGLEVEL_TEST))
      PUtil::outLog() << "Warning: <vehicle> element without type attribute\n";
    return false;
  }

  if (!strcmp(val, "car"))
	  coretype = v_core_type::car;
  else if (!strcmp(val, "tank"))
	  coretype = v_core_type::tank;
  else if (!strcmp(val, "helicopter"))
	  coretype = v_core_type::helicopter;
  else if (!strcmp(val, "plane"))
	  coretype = v_core_type::plane;
  else if (!strcmp(val, "hovercraft"))
	  coretype = v_core_type::hovercraft;
  else {
    if (PUtil::isDebugLevel(DEBUGLEVEL_TEST))
      PUtil::outLog() << "Error: <vehicle> has unrecognised type \"" << val << "\"\n";
    return false;
  }

  for (XMLElement *walk = rootelem->FirstChildElement();
    walk; walk = walk->NextSiblingElement()) {

	// generic params
	if (!strcmp(walk->Value(), "genparams")) {

		val = walk->Attribute("mass");
		if (val)
			mass = atof(val);
	  
		val = walk->Attribute("dimensions");
		if (val) {
			custom_dims = true;
			sscanf(val, "%f , %f , %f", &dims.x, &dims.y, &dims.z);
			dims *= allscale;
		}

      val = walk->Attribute("wheelscale");
      if (val) wheelscale = atof(val);

      val = walk->Attribute("wheelmodel");
      if (val) wheelmodel = ssModel.loadModel(PUtil::assemblePath(val, filename));

	// params
    } else if (!strcmp(walk->Value(), "ctrlparams")) {

      val = walk->Attribute("speed");
      if (val) param.speed = atof(val);

      val = walk->Attribute("turnspeed");
      if (val) sscanf(val, "%f , %f , %f", &param.turnspeed.x, &param.turnspeed.y, &param.turnspeed.z);

      val = walk->Attribute("drag");
      if (val) sscanf(val, "%f , %f , %f", &drag.x, &drag.y, &drag.z);

      val = walk->Attribute("angdrag");
      if (val) angdrag = atof(val);

      val = walk->Attribute("lift");
      //if (val) sscanf(val, "%f , %f", &param.lift.x, &param.lift.y);
	  if(val) sscanf(val, "%f", &lift);

      val = walk->Attribute("speedrate");
      if (val) ctrlrate.throttle = atof(val);

      val = walk->Attribute("turnspeedrate");
      if (val) sscanf(val, "%f , %f , %f", &ctrlrate.turn.x, &ctrlrate.turn.y, &ctrlrate.turn.z);

      val = walk->Attribute("turnspeedcoefficients");
      if (val) sscanf(val, "%f , %f", &param.turnspeed_a, &param.turnspeed_b);

      val = walk->Attribute("fineffect");
      if (val) sscanf(val, "%f , %f", &param.fineffect.x, &param.fineffect.y);

	// engine performance
    } else if (!strcmp(walk->Value(), "drivesystem")) {

      for (XMLElement *walk2 = walk->FirstChildElement();
        walk2; walk2 = walk2->NextSiblingElement()) {
        if (!strcmp(walk2->Value(), "engine")) {

          float powerscale = 1.0f;

          val = walk2->Attribute("powerscale");
          if (val) powerscale = atof(val);

          for (XMLElement *walk3 = walk2->FirstChildElement();
            walk3; walk3 = walk3->NextSiblingElement()) {

            if (!strcmp(walk3->Value(), "powerpoint")) {

              float in_rpm, in_power;

              val = walk3->Attribute("rpm");
              if (!val) {
                PUtil::outLog() << "Warning: failed to read engine RPM value\n";
                continue;
              }
              in_rpm = atof(val);

              val = walk3->Attribute("power");
              if (!val) {
                PUtil::outLog() << "Warning: failed to read engine power value\n";
                continue;
              }
              in_power = atof(val);

              engine.addPowerCurvePoint(in_rpm, in_power * powerscale);
            }
          }

        } else if (!strcmp(walk2->Value(), "gearbox")) {

          for (XMLElement *walk3 = walk2->FirstChildElement();
            walk3; walk3 = walk3->NextSiblingElement()) {

            if (!strcmp(walk3->Value(), "gear")) {

              val = walk3->Attribute("absolute");
              if (val) {
                engine.addGear(atof(val));
              } else {
                val = walk3->Attribute("relative");
                if (!val) {
                  PUtil::outLog() << "Warning: gear has neither absolute nor relative value\n";
                  continue;
                }

                if (!engine.hasGears()) {
                  PUtil::outLog() << "Warning: first gear cannot use relative value\n";
                  continue;
                }

                engine.addGear(engine.getLastGearRatio() * atof(val));
              }
            }
          }

        }
      }
	// parts
    } else if (!strcmp(walk->Value(), "part")) {
      part.push_back(PVehicleTypePart());
      PVehicleTypePart *vtp = &part.back();

      vtp->parent = -1;
      //vtp->ref_local.setPosition(vec3f::zero());
      //vtp->ref_local.setOrientation(vec3f::zero());
      vtp->model = nullptr;
      vtp->scale = 1.0;

      val = walk->Attribute("name");
      if (val) vtp->name = val;

      val = walk->Attribute("parent");
      if (val) vtp->parentname = val;

      val = walk->Attribute("pos");
      if (val) {
        vec3f pos;
        if (sscanf(val, "%f , %f , %f", &pos.x, &pos.y, &pos.z) == 3)
          vtp->ref_local.setPosition(pos * allscale);
      }
	  
	  val = walk->Attribute("render_pos");
      if (val) {
        vec3f render_pos;
        if (sscanf(val, "%f , %f , %f", &render_pos.x, &render_pos.y, &render_pos.z) == 3)
          vtp->render_ref_local.setPosition(render_pos * allscale);
      }
	  else {
		  // render pos is optional, it will fallback to the normal pos if missing
		  vtp->render_ref_local = vtp->ref_local;
	  }

      val = walk->Attribute("orientation");
      if (val) {
        quatf ori;
        // note: w first, as per usual mathematical notation
        if (sscanf(val, "%f , %f , %f , %f", &ori.w, &ori.x, &ori.y, &ori.z) == 4)
          vtp->ref_local.setOrientation(ori);
		  vtp->render_ref_local.setOrientation(ori);
      }

      val = walk->Attribute("scale");
      if (val) vtp->scale = atof(val);

      val = walk->Attribute("model");
      if (val) vtp->model = ssModel.loadModel(PUtil::assemblePath(val, filename));

      for (XMLElement *walk2 = walk->FirstChildElement();
        walk2; walk2 = walk2->NextSiblingElement()) {
        if (!strcmp(walk2->Value(), "clip")) {
          vehicle_clip_s vc;

          vc.force = 0.0f;
          vc.dampening = 0.0f;

          val = walk2->Attribute("type");
          if (!val || !strlen(val)) {
            if (PUtil::isDebugLevel(DEBUGLEVEL_TEST))
              PUtil::outLog() << "Warning: <clip> element without type attribute\n";
            continue;
          }

          if      (!strcmp(val, "body")) vc.type = v_clip_type::body;
          else if (!strcmp(val, "drive-left")) vc.type = v_clip_type::drive_left;
          else if (!strcmp(val, "drive-right")) vc.type = v_clip_type::drive_right;
          else if (!strcmp(val, "hover")) vc.type = v_clip_type::hover;
          else {
            if (PUtil::isDebugLevel(DEBUGLEVEL_TEST))
              PUtil::outLog() << "Warning: <clip> has unrecognised type \"" << val << "\"\n";
            continue;
          }

          val = walk2->Attribute("pos");
          if (!val) {
            if (PUtil::isDebugLevel(DEBUGLEVEL_TEST))
              PUtil::outLog() << "Warning: <clip> has no pos attribute\n";
            continue;
          }
          sscanf(val, "%f , %f , %f", &vc.pt.x, &vc.pt.y, &vc.pt.z);
          vc.pt *= allscale;

          val = walk2->Attribute("force");
          if (val) vc.force = atof(val);

          val = walk2->Attribute("dampening");
          if (val) vc.dampening = atof(val);

          vtp->clip.push_back(vc);
        } else if (!strcmp(walk2->Value(), "wheel")) {
          PVehicleTypeWheel vtw;
          
          vtw.radius = DEF_WHEEL_RADIUS;
          vtw.drive = DEF_WHEEL_DRIVE;
          vtw.steer = DEF_WHEEL_STEER;
          vtw.brake1 = DEF_WHEEL_BRAKE1;
          vtw.brake2 = DEF_WHEEL_BRAKE2;
          vtw.force = DEF_WHEEL_FORCE;
          vtw.dampening = DEF_WHEEL_DAMPENING;
          vtw.friction = DEF_WHEEL_FRICTION;

          val = walk2->Attribute("pos");
          if (!val) {
            if (PUtil::isDebugLevel(DEBUGLEVEL_TEST))
              PUtil::outLog() << "Warning: <wheel> has no pos attribute\n";
            continue;
          }
          sscanf(val, "%f , %f , %f", &vtw.pt.x, &vtw.pt.y, &vtw.pt.z);
          vtw.pt *= allscale;

          val = walk2->Attribute("radius");
          if (val) vtw.radius = atof(val);

          val = walk2->Attribute("drive");
          if (val) vtw.drive = atof(val);

          val = walk2->Attribute("steer");
          if (val) vtw.steer = atof(val);

          val = walk2->Attribute("brake1");
          if (val) vtw.brake1 = atof(val);

          val = walk2->Attribute("brake2");
          if (val) vtw.brake2 = atof(val);

          val = walk2->Attribute("force");
          if (val) vtw.force = atof(val);

          val = walk2->Attribute("dampening");
          if (val) vtw.dampening = atof(val);

          val = walk2->Attribute("friction");
          if (val) vtw.friction = atof(val);

          vtp->wheel.push_back(vtw);
          drive_total += vtw.drive;
          wheel_speed_multiplier += 1.0f;
        } else if (!strcmp(walk2->Value(), "jetflame")) {
          vtp->flame.push_back(PReferenceFrame());

          val = walk2->Attribute("pos");
          if (val) {
            vec3f pos;
            if (sscanf(val, "%f , %f , %f", &pos.x, &pos.y, &pos.z) == 3)
              vtp->flame.back().setPosition(pos * allscale);
          }

          val = walk2->Attribute("ori");
          if (val) {
            quatf ori;
            if (sscanf(val, "%f , %f , %f , %f", &ori.w, &ori.x, &ori.y, &ori.z) == 4)
              vtp->flame.back().setOrientation(ori);
          }
        }
      }

      vtp->ref_local.updateMatrices();
    }
  }

  for (unsigned int i=0; i<part.size(); ++i) {
    if (part[i].parentname.length() > 0) {
      unsigned int j;
      for (j=0; j<part.size(); ++j) {
        if (i == j) continue;
        if (part[i].parentname == part[j].name) {
          part[i].parent = j;
          break;
        }
      }
      if (j >= part.size() &&
        PUtil::isDebugLevel(DEBUGLEVEL_TEST))
        PUtil::outLog() << "Warning: part \"" << part[i].name <<
          "\" references non-existant parent \"" << part[i].parentname << "\"\n";
    }
  }
  
	// compute road holding average
	// and count number of driving wheels in the meantime
	float road_holding = 0;
	unsigned wheel_num = 0;
	for(unsigned int i=0; i!=part.size(); i++)
	{
		for(unsigned int j=0; j!=part[i].wheel.size(); j++)
		{
			++wheel_num;
			road_holding += part[i].wheel[j].friction;
		
			// count number of drving wheels
			driving_wheels_num += part[i].wheel[j].drive;
		}
	}
	road_holding /= wheel_num;
	pstat_roadholding = std::to_string(road_holding * 100);
	
	// avoid 0 as number of driving wheels
	// it's to avoid division by 0 - but won't affect physic
	if (driving_wheels_num == 0)
		driving_wheels_num = 1;
	
  // remove trailing spaces/points
  while(1)
  {
	if(pstat_roadholding.back() == '0')
	{
		pstat_roadholding.pop_back();
		continue;
	}
	else if(pstat_roadholding.back() == '.')
		pstat_roadholding.pop_back();
	break;
  }
  
  // get pstat of engine
  pstat_enginepower = std::to_string(engine.getHorsePower());
  // remove decimals
  while(1)
  {
	if(pstat_enginepower.back() == '.')
	{
		pstat_enginepower.pop_back();
		break;
	}
	pstat_enginepower.pop_back();
  }
  
  // assign pstat of driving wheels
  std::vector<float> wheels_drive;
  for(unsigned int i=0; i!=part.size(); i++)
  {
	for(unsigned int j=0; j!=part[i].wheel.size(); j++)
	{
		wheels_drive.push_back(part[i].wheel[j].drive);
	}
  }
  // standard four wheels layout
  if(wheels_drive.size() == 4)
  {
	// 4x4
	if
	(
		wheels_drive[0] > 0 &&
		wheels_drive[1] > 0 &&
		wheels_drive[2] > 0 &&
		wheels_drive[3] > 0
	)
		pstat_wheeldrive = "4WD";
	// forward
	else if
	(
		wheels_drive[0] > 0 &&
		wheels_drive[1] > 0 &&
		wheels_drive[2] == 0 &&
		wheels_drive[3] == 0
	)
		pstat_wheeldrive = "FWD";
	// backward
	else if
	(
		wheels_drive[0] == 0 &&
		wheels_drive[1] == 0 &&
		wheels_drive[2] > 0 &&
		wheels_drive[3] > 0
	)
		pstat_wheeldrive = "RWD";
	//
	else
		pstat_wheeldrive = "non standard layout";
  }
  else
  {
	// count how many wheels drive
	unsigned int wd = 0;
	for(unsigned int i=0; i!=wheels_drive.size(); i++)
	{
		if(wheels_drive[i] > 0)
			++wd;
	}
	pstat_wheeldrive = std::to_string(wd) + " driving out of " + std::to_string(wheels_drive.size());
  }
  
	// Use model of the first part to compute dimensions
	if(!custom_dims){
		std::pair<vec3f,vec3f> extents = part[0].model->getExtents();
		dims.x = (extents.second.x - extents.first.x) * part[0].scale;
		dims.y = (extents.second.y - extents.first.y) * part[0].scale;
		dims.z = (extents.second.z - extents.first.z) * part[0].scale;
	}
	
	// linear drag coefficent
	// @todo: when in reverse drag is different!
	// @todo: most of these calculation could be done once and not done each tick
	// HACK looks like that for some scaling issue, cars are littler than what they should be
	//    this affects drag calculation. We use a constant to resize areas accordingly
	// @todo: solve the real problem, that is that cars are underscaled
	const float drag_hack_area = 1.777777;
	// We use the formula:
	//
	// F = cd x p x u^2 x A x 1/2
	//
	// Where:
	// F is the drag force we're looking for
	// cd is the drag coefficent, typical of each car.
	//    It is different for each model (0.20 to 0.45), we assume it's on average 0.30
	//    then we apply the car type variation coefficent.
	const float drag_coefficent_front = 0.3 * drag.y;
	//    For the lateral and vertical sides, since cars are not built to be aerodynamic there we use custom consts
	const float drag_coefficent_side = 0.8 * drag.x;
	const float drag_coefficent_bottom = 0.9 * drag.z;
	// p is the fluid density (for air = ~1.2 Kg/m3)
	const float air_density = 1.2;
	// u is the linear velocity in the direction
	// A is the reference area.
	//     We use the relevant dimensions of the car as area, and we apply coefficent to adjust
	const float drag_reference_area_front = dims.x * dims.z * drag_hack_area * 0.9;
	const float drag_reference_area_side = dims.y * dims.z * drag_hack_area * 0.75;
	const float drag_reference_area_bottom = dims.x * dims.y * drag_hack_area * 0.97;

	drag_coeff = vec3f(
		drag_coefficent_side * air_density * drag_reference_area_side * 0.5,
		drag_coefficent_front * air_density * drag_reference_area_front * 0.5,
		drag_coefficent_bottom * air_density * drag_reference_area_bottom * 0.5 );
    
	// angular drag
	// This is an empiric formula... there is no clear easy right way to do it
	ang_drag_coeff = vec3f(
		62 * (dims.y + dims.z) * angdrag,
		62 * (dims.x + dims.z) * angdrag,
		62 * (dims.y + dims.x) * angdrag );
	
	// lift (downforce)
	// Its formula is similar to the drag one 
	//
	// L = 0.5 x W x h x F x p x V^2
	//
	// Where:
	// L = result downforce
	// W = wingspan
	const float wingspan = dims.x;
	// h = wing cord
	const float chord = dims.y;
	// F = lift coefficent - we assume an average rally car has just a bit of downforce
	const float F = -0.02 * lift;
	// p = air density
	// V = is the speed in the forward direction
	// and we also apply the area hack (see above)
	lift_coeff = 0.5 * wingspan * chord * F * air_density * drag_hack_area;
	
  // assign inverse_drive_total
  if (drive_total > 0.0f)
    inverse_drive_total = 1.0f / drive_total;
  else
    inverse_drive_total = 0.0f;

  if (wheel_speed_multiplier > 0.0f)
    wheel_speed_multiplier = 1.0f / wheel_speed_multiplier;
  
  return true;
}

///
/// @brief unload parts of the PVehicleType
///
void PVehicleType::unload()
{
  part.clear();
}

///
/// @brief Sets if the vehicle is locked 
///
void PVehicleType::setLocked(bool setLocked)
{
  locked = setLocked;
}

///
/// @brief Gets if the vehicle is locked 
///
bool PVehicleType::getLocked()
{
  return locked;
}

//TNL_IMPLEMENT_NETOBJECT(PVehicle);

///
/// @brief Constructor of the PVehicle class
/// @param sim_parent = the simulation where the vehicle stays
/// @param _type = the type of vehicle the new vehicle is
///
PVehicle::PVehicle(PSim &sim_parent, PVehicleType *_type) :
	sim(sim_parent),
	type(_type),
	body(sim.createRigidBody()),
	iengine(&type->engine),
	blade_ang1(0.0f),
	nextcp(0),
	nextcdcp(0),
	currentlap(1),
	reset_trigger_time(0.0f),
	reset_time(0.0f),
	crunch_level(0.0f),
	crunch_level_prev(0.0f),
	forwardspeed(0.0f),
	wheel_angvel(0.0f),
	offroadtime_begin(0),
	offroadtime_end(0),
	offroadtime_total(0)
{
	// vehicle mass is approximately meant to be cuboid
	body->setMassCuboid(type->mass, type->dims);

	// set control
	state.setZero();
	ctrl.setZero();

	// Set parts and wheels
	part.resize(type->part.size());
	for (unsigned int i=0; i<part.size(); i++) {
		part[i].ref_local = type->part[i].ref_local;

		part[i].wheel.resize(type->part[i].wheel.size());

		for (unsigned int j=0; j<part[i].wheel.size(); j++) {
			part[i].wheel[j].ref_world.setPosition(vec3f(0,0,1000000)); // FIXME
		}
	}

	updateParts();

	//mNetFlags.set(Ghostable);
}

///
/// @brief reset the vehicle anywhere it is (used when it get flipped or something)
///
void PVehicle::doReset()
{
  // some time has to pass before reset
  if (reset_time != 0.0f) return;

  // the reset position
  reset_pos = body->pos + vec3f(0.0f, 0.0f, 2.0f);

  // set the new orientation
  vec3f forw = makevec3f(body->getOrientationMatrix().row[0]);
  float forwangle = atan2(forw.y, forw.x);

  quatf temp;
  temp.fromZAngle(forwangle);

  if (body->ori.dot(temp) < 0.0f) temp = temp * -1.0f;

  reset_ori = temp;

  reset();
}

///
/// @brief reset the vehicle to a specific point (used when the 'recover' button is pressed)
/// @param pos = the position where the vehicle has to be set
/// @param ori = the orientation the vehicle has to be set
///
void PVehicle::doReset(const vec3f &pos, const quatf &ori)
{
  // some time has to pass before reset
  if (reset_time != 0.0f) return;

  // the reset position
  reset_pos = pos;

  // set the new orientation
  quatf temp = ori; // FIXME: laziness and fear of breaking copy-pasted code

  if (body->ori.dot(temp) < 0.0f) temp = temp * -1.0f;

  reset_ori = temp;

  reset();
}

void PVehicle::reset()
{
	// set a reset time
	reset_time = VEHICLE_RESET_TIME;

	// reset noise level
	crunch_level = 0;
	crunch_level_prev = 0;

	// stop all the wheels
	for (unsigned int i=0; i<part.size(); i++) {
		for (unsigned int j=0; j<part[i].wheel.size(); j++) {
			part[i].wheel[j].reset();
		}
	}

	// stop
	forwardspeed = 0;
	wheel_angvel = 0;
	wheel_speed = 0;
	skid_level = 0;

	// reset engine
	iengine.doReset();

	// reset controls
	state.setZero();
}

///
/// @brief Physic simulation for the single vehicle
/// @param delta = timeslice to compute
///
void PVehicle::tick(const float& delta)
{
  // ensure control values are in valid range
  ctrl.clamp();

  // handle crunch noise level
  PULLTOWARD(crunch_level_prev, crunch_level, delta * 5.0f);
  PULLTOWARD(crunch_level, 0.0f, delta * 5.0f);

  // smooth out control values
  PULLTOWARD(state.throttle, ctrl.throttle, type->ctrlrate.throttle * delta);
  PULLTOWARD(state.brake1, ctrl.brake1, type->ctrlrate.brake1 * delta);
  PULLTOWARD(state.brake2, ctrl.brake2, type->ctrlrate.brake2 * delta);
  PULLTOWARD(state.turn.x, ctrl.turn.x, type->ctrlrate.turn.x * delta);
  PULLTOWARD(state.turn.y, ctrl.turn.y, type->ctrlrate.turn.y * delta);
  PULLTOWARD(state.turn.z, ctrl.turn.z, type->ctrlrate.turn.z * delta);
  //PULLTOWARD(state.aim.x, ctrl.aim.x, type->ctrlrate.aim.x * delta);
  //PULLTOWARD(state.aim.y, ctrl.aim.y, type->ctrlrate.aim.y * delta);
  PULLTOWARD(state.collective, ctrl.collective, type->ctrlrate.collective * delta);

  // prepare some useful data
  vec3f linvel = body->getLinearVel();
  mat44f orimatt = body->getInverseOrientationMatrix();
  vec3f angvel = body->getAngularVel();
  vec3f loclinvel = body->getWorldToLocVector(linvel);
  vec3f locangvel = body->getWorldToLocVector(angvel);

  // if car is upside down, wait some seconds then reset
  if (orimatt.row[2].z <= 0.1f) {
    reset_trigger_time += delta;

    if (reset_trigger_time >= VEHICLE_UPSIDEDOWN_RESET_TIME)
      doReset();
  } else
    reset_trigger_time = 0.0f;

  // check for resetting to do
  if (reset_time > 0.0f) {
	// move toward reset position and orientation
    PULLTOWARD(body->pos, reset_pos, delta * 2.0f);
    PULLTOWARD(body->ori, reset_ori, delta * 2.0f);

    // stop
    body->setLinearVel(vec3f::zero());
    body->setAngularVel(vec3f::zero());

    body->updateMatrices();

    reset_time -= delta;
    if (reset_time <= 0.0f)
      reset_time = -2.0f;

    return;

  } else {
    reset_time += delta;
    if (reset_time > 0.0f)
      reset_time = 0.0f;
  }

  forwardspeed = loclinvel.y;

  // body turn control
  vec3f desiredturn = vec3f(
    state.turn.x * type->param.turnspeed.x,
    state.turn.y * type->param.turnspeed.y,
    state.turn.z * type->param.turnspeed.z);
  body->addLocTorque(desiredturn * type->param.turnspeed_a);
  body->addLocTorque((desiredturn - locangvel) * (type->param.turnspeed_b * loclinvel.y));

  // fin effect (torque due to drag)
  body->addLocTorque(vec3f(-loclinvel.z * type->param.fineffect.y, 0.0, loclinvel.x * type->param.fineffect.x));
	
	// linear drag
	vec3f frc = -vec3f(
		loclinvel.x * fabsf(loclinvel.x) * type->drag_coeff.x,
		loclinvel.y * fabsf(loclinvel.y) * type->drag_coeff.y,
		loclinvel.z * fabsf(loclinvel.z) * type->drag_coeff.z );
  
	// angular drag
	body->addLocTorque( -vec3f(
		locangvel.x * fabsf(locangvel.x) * type->ang_drag_coeff.x,
		locangvel.y * fabsf(locangvel.y) * type->ang_drag_coeff.y,
		locangvel.z * fabsf(locangvel.z) * type->ang_drag_coeff.z
		)
	);
  
	// lift
	//frc += -vec3f(
	//  loclinvel.x * type->param.lift.x * loclinvel.y,
	//  0.0,
	//  loclinvel.z * type->param.lift.y * loclinvel.y);
	frc += vec3f(0, 0, SQUARED(forwardspeed) * type->lift_coeff);

  // VEHICLE TYPE POINT

  // vehicle-specific code
  switch (type->coretype) {

  case v_core_type::tank:
    if (part.size() >= 3) {
      state.aim.x += ctrl.aim.x * delta * 0.5;
      if (state.aim.x < -PI) state.aim.x += 2.0*PI;
      if (state.aim.x >= PI) state.aim.x -= 2.0*PI;
      state.aim.y += ctrl.aim.y * delta * 0.5;
      CLAMP(state.aim.y, 0.0, 0.5);

      part[1].ref_local.ori.fromThreeAxisAngle(
        vec3f(0.0,0.0,-state.aim.x));

      part[2].ref_local.ori.fromThreeAxisAngle(
        vec3f(-state.aim.y,0.0,0.0));

      part[1].ref_local.updateMatrices();
      part[2].ref_local.updateMatrices();
    }
    break;

  case v_core_type::helicopter:
    break;

  case v_core_type::plane:
    {
      frc.y += state.throttle * type->param.speed;
    }
    break;

  case v_core_type::hovercraft:
    {
      blade_ang1 = fmod(blade_ang1 + delta * 50.0 * state.throttle, 2.0*PI);

      if (part.size() >= 4) {
        state.aim.x += ctrl.aim.x * delta * 0.5;
        if (state.aim.x < -PI) state.aim.x += 2.0*PI;
        if (state.aim.x >= PI) state.aim.x -= 2.0*PI;
        state.aim.y += ctrl.aim.y * delta * 0.5;
        CLAMP(state.aim.y, 0.0, 0.5);

        part[1].ref_local.ori.fromThreeAxisAngle(vec3f(0.0, blade_ang1, 0.0));

        part[2].ref_local.ori.fromThreeAxisAngle(vec3f(0.0, 0.0, state.turn.z * -0.5));

        part[1].ref_local.updateMatrices();
        part[2].ref_local.updateMatrices();
      }

      frc.y += state.throttle * type->param.speed;
    }
    break;

  default:
    break;
  }

  body->addLocForce(frc);

  vec3f forwarddir = makevec3f(body->getInverseOrientationMatrix().row[1]);
  //vec3f rightdir = makevec3f(body->getInverseOrientationMatrix().row[0]);

  // handle engine (output torque, change gear if needed...)
  iengine.tick(delta, state.throttle, wheel_angvel);

  // Output engine power delivered to each wheel
  // the power will be shared equally to each wheel
  // TODO: ABS and differential should be implemented to share the power between wheels better (26-2-2020)
  float drivetorque = iengine.getOutputTorque() / type->driving_wheels_num;

  float turnfactor = state.turn.z;// /
    //(1.0f + fabsf(wheel_angvel) / 70.0f);

  wheel_angvel = 0.0f;

  wheel_speed = 0.0f;

  skid_level = 0.0f;

  // the parts
  for (unsigned int i=0; i<part.size(); ++i) {
    // the clips of the part
    for (unsigned int j=0; j<type->part[i].clip.size(); ++j) {

      // the local clip coordinate
      vec3f lclip = type->part[i].clip[j].pt;
      // the world clip coordinate
      vec3f wclip = part[i].ref_world.getLocToWorldPoint(lclip);

      // where the clip *might* touch the ground
      PTerrain::ContactInfo tci;
      tci.pos.x = wclip.x;
      tci.pos.y = wclip.y;
      sim.getTerrain()->getContactInfo(tci);

      // if the clip hovers let it hover
      if (type->part[i].clip[j].type == v_clip_type::hover) {
        if (tci.pos.z < 40.3) {
          tci.pos.z = 40.3;
          tci.normal = vec3f(0,0,1);
        }
      }

      // if the clip touches the ground
      if (wclip.z <= tci.pos.z) {

        // how much the clip enters the groud along the normal
        float depth = (tci.pos - wclip) * tci.normal;

        // clip velocity
        vec3f ptvel = body->getLinearVelAtPoint(wclip);

        vec3f frc = vec3f::zero();

        switch (type->part[i].clip[j].type) {
        default:
		case v_clip_type::body:
          {
            #if 0
            frc += vec3f(0.0, 0.0, type->part[i].clip[j].force);

            frc += ptvel * -type->part[i].clip[j].dampening;

            frc *= depth;
            #else
            vec3f rightdir;
            if (tci.normal.x > 0.5f)
              rightdir = vec3f(0.0f, 1.0f, 0.0f);
            else
              rightdir = vec3f(1.0f, 0.0f, 0.0f);

            //float testval = tci.normal * rightdir;

            // forward direction
            vec3f surf_forward = tci.normal ^ rightdir;
            surf_forward.normalize();

            // lateral right dircetion
            vec3f surf_right = surf_forward ^ tci.normal;
            surf_right.normalize();

            // velocity along directions
            vec3f surfvel(
              ptvel * surf_right, // lateral
              ptvel * surf_forward, // forward
              ptvel * tci.normal); // normal (perpendicular the ground)

            // how much force the clip put against the ground
            float perpforce =
			  // how much the clip is below surface (times own clip force)
              depth * type->part[i].clip[j].force
              // how much fast the clip is going down (times own clip dampen capabilities)
              - surfvel.z * type->part[i].clip[j].dampening;

            // if the clip pushes against the ground
            if (perpforce > 0.0f) {
              vec2f friction = vec2f(-surfvel.x, -surfvel.y) * FRICTION_MAGIC_COEFF_CLIP;

              float maxfriction = perpforce * 0.9f;
              float testfriction = perpforce * 1.2f;

              float leng = friction.length();

              if (leng > 0.0f && leng > testfriction)
                friction *= (maxfriction / leng);

              frc += (tci.normal * perpforce +
                  surf_right * friction.x +
                  surf_forward * friction.y);

              CLAMP_LOWER(crunch_level, perpforce * 0.00001f);
            }
            #endif
          } break;

        case v_clip_type::drive_left:
          {
            frc += vec3f(0.0, 0.0, type->part[i].clip[j].force);

            vec3f drivevec = forwarddir *
              (state.throttle * type->param.speed +
              state.turn.z * type->param.turnspeed.z);

            vec3f relvel = drivevec - tci.normal * (drivevec * tci.normal);

            frc += (ptvel - relvel) * -type->part[i].clip[j].dampening;

            frc *= depth;
          } break;

        case v_clip_type::drive_right:
          {
            frc += vec3f(0.0, 0.0, type->part[i].clip[j].force);

            vec3f drivevec = forwarddir *
              (state.throttle * type->param.speed -
              state.turn.z * type->param.turnspeed.z);

            vec3f relvel = drivevec - tci.normal * (drivevec * tci.normal);

            frc += (ptvel - relvel) * -type->part[i].clip[j].dampening;

            frc *= depth;
          } break;

        case v_clip_type::hover:
          {
            float surfvelz = ptvel * tci.normal;

            float perpfrc = type->part[i].clip[j].force;
            if (surfvelz < 0.0) perpfrc += surfvelz * -type->part[i].clip[j].dampening;

            frc += (tci.normal * perpfrc) * depth;
          } break;
        }

        body->addForceAtPoint(frc, wclip);
      }
    }

    // The wheels
    for (unsigned int j=0; j<type->part[i].wheel.size(); ++j) {

      PVehicleWheel &wheel = part[i].wheel[j];
      PVehicleTypeWheel &typewheel = type->part[i].wheel[j];

      // terrain information
      const TerrainType mf_tt = sim.getTerrain()->getRoadSurface(wheel.ref_world_lowest_point.getPosition());
      const float mf_coef     = PUtil::decideFrictionCoef(mf_tt);
      const float mf_resis    = PUtil::decideResistance(mf_tt);

      // where the wheel might touch the ground
      vec3f wclip = wheel.getLowestPoint();

      wheel.spin_vel += drivetorque * typewheel.drive * delta * (1.0f - mf_resis);

      // brakes affects wheel spin velocity
      float desiredchange = (state.brake1 * typewheel.brake1 +
        state.brake2 * typewheel.brake2) * delta;
      if (wheel.spin_vel > desiredchange)
        wheel.spin_vel -= desiredchange;
      else if (wheel.spin_vel < -desiredchange)
        wheel.spin_vel += desiredchange;
      else
        wheel.spin_vel = 0.0f;

      // update wheel spin position
      wheel.spin_pos += wheel.spin_vel * delta;

      // update wheel spin velocity for the ground resistance
      wheel.spin_vel -= wheel.spin_vel * mf_resis * delta;

      // update wheel position respect steering axis
      wheel.turn_pos = turnfactor * typewheel.steer;

      wheel.dirtthrow = 0.0f;

      // the suspension force is proportional to how much the suspension
      // is tensed up and the suspension elasticity of the type of wheel
      float suspension_force = wheel.ride_pos * typewheel.force;

      // update suspension velocity
      wheel.ride_vel +=
        (-suspension_force -
        wheel.ride_vel * typewheel.dampening) * 0.02 * delta;

      // update suspension position
      wheel.ride_pos += wheel.ride_vel * delta;

      // tci = the terrain point that shares the vertical with wclip
      PTerrain::ContactInfo tci;
      tci.pos.x = wclip.x;
      tci.pos.y = wclip.y;

      sim.getTerrain()->getContactInfo(tci);
	  
	  // apply a bit of sinking depending of the material
	  tci.pos.z -= SINK_COEFF * mf_resis;

      // further interaction only if the wheel touches the ground
      if (wclip.z <= tci.pos.z) {

        // bump velocity is proportional to the wheel spin velocity
        wheel.bumptravel += fabsf(wheel.spin_vel) * 0.6f * delta;

        // assign a new random bump if the bump velocity is too high
        if (wheel.bumptravel >= 1.0f) {
          wheel.bumplast = wheel.bumpnext;
          wheel.bumptravel -= (int)wheel.bumptravel;

          wheel.bumpnext = randm11 * rand01 * typewheel.radius * 0.1f;
        }

        // how much wheel is below the ground along the normal
        float depth = (tci.pos - wclip) * tci.normal;

        // wheel velocity
        vec3f ptvel = body->getLinearVelAtPoint(wclip);

        vec3f frc = vec3f::zero();

        vec3f rightdir = makevec3f(wheel.ref_world.getInverseOrientationMatrix().row[0]);

        //float testval = tci.normal * rightdir;

		// the forward direction
        vec3f surf_forward = tci.normal ^ rightdir;
        surf_forward.normalize();

        // the right side direction
        vec3f surf_right = surf_forward ^ tci.normal;
        surf_right.normalize();

        // add wheel rotation speed to ptvel
        // direction * wheel spin vel * wheel radius * own terrain resistance
        ptvel += surf_forward * (-wheel.spin_vel * typewheel.radius) * (1.0f - mf_resis);

        // velocity along these three directions
        vec3f surfvel(
          ptvel * surf_right,
          ptvel * surf_forward,
          ptvel * tci.normal);

        // with how much force the wheel pushes the ground
        float perpforce = suspension_force;

        // if the wheel has a velocity toward the bottom (the normal respect the ground right below, it can be inclinated)
        if (surfvel.z < 0.0f)
          // the perpendicular force get absorbed by the suspension
          perpforce -= surfvel.z * typewheel.dampening;

        // suspension get pressed to keep the wheel above ground
        wheel.ride_pos += depth;

        // suspension can't get more strecthed than the MAX_SUSPENSION_DEPTH_COEFF of the wheel radius
        float maxdepth = typewheel.radius * MAX_SUSPENSION_DEPTH_COEFF;

        // if the suspension get pressed too much
        if (wheel.ride_pos > maxdepth) {
		  // how much is overpressed
          float overdepth = wheel.ride_pos - maxdepth;
          // suspension will be pressed down to its maximum value
          wheel.ride_pos = maxdepth;
          // the force will be instead transferred directly to the ground
          perpforce -= overdepth * surfvel.z * typewheel.dampening * 5.0f;
        }

        if (wheel.ride_vel < -surfvel.z)
          wheel.ride_vel = -surfvel.z;

        // further interaction only if the wheel pushes the ground
        if (perpforce > 0.0f) {

		  // proportional to the actual velocity right and forward
		  //vec2f friction = vec2f(-surfvel.x, -surfvel.y) * 10000.0f;
		  vec2f friction = vec2f(-surfvel.x, -surfvel.y) * typewheel.friction * FRICTION_MAGIC_COEFF_WHEEL;

          // max friction available proportional to the pressure of the wheel to the ground and ground own friction coefficent
          float maxfriction = perpforce * mf_coef;

          float testfriction = perpforce * 1.0f;

          // the intensity of the friction
          float leng = friction.length();

          // if there is some friction, and it is bigger than testfriction
          if (leng > 0.0f && leng > testfriction)
            // friction will be put equal to maxfriction with a little gain
            friction *= (maxfriction / leng) + typewheel.friction;
            //friction *= (maxfriction / leng) + 0.02f;

          // the force of the wheel
          frc += (
              // the perpendicular force along the normal
		      tci.normal * perpforce +
		      // laterally
              surf_right * friction.x +
              // in the forward direction
              surf_forward * friction.y);

          // update wheel spin velocity
          wheel.spin_vel -= (friction.y * typewheel.radius) * delta * WHEEL_SPIN_VEL_UPDATE_RATIO;

          //wheel.turn_vel -= friction.x * 1.0f * delta;

          // apply the force
          body->addForceAtPoint(frc, wclip);

          wheel.dirtthrow = leng / maxfriction;
          skid_level += wheel.dirtthrow;

          // downward direction
          vec3f downward = surf_forward ^ rightdir;
          downward.normalize();

          if (wheel.spin_vel > 0.0f)
            downward += surf_forward * -0.3f;
          else
            downward += surf_forward * 0.3f;
          downward.normalize();

          // where to throw dirt
          wheel.dirtthrowpos = wheel.ref_world_lowest_point.getPosition();
          // how much fast throw
          wheel.dirtthrowvec =
            body->getLinearVelAtPoint(wheel.dirtthrowpos) +
            (downward ^ rightdir) * (wheel.spin_vel * typewheel.radius);
        }
      }

      //wheel.spin_vel /= 1.0f + delta * 0.6f;

      wheel.spin_pos = fmodf(wheel.spin_pos, PI*2.0f);

      wheel_angvel += wheel.spin_vel * typewheel.drive;

      wheel_speed += wheel.spin_vel * typewheel.radius;
    }

    // Calculate collisions with world objects
    PCollision collision(type->part[i].clip, part[i].ref_world);
    const std::vector<PTerrainFoliage> *foliage = sim.getTerrain()->getFoliageAtPos(body->pos);

    if (foliage) {
      const std::vector<PTerrainFoliage> contact = collision.checkContact(foliage);

      for (unsigned int i = 0; i < contact.size(); ++i) {
        vec3f crashforce = vec3f::zero();
        vec3f ptvel = body->getLinearVelAtPoint(body->pos);

        // Prevent that vehicle gets stuck in an object
        if (collision.towardsContact(body->pos, contact[i].pos, ptvel * delta)) {
          const float crashthreshold = 0.025f;
          vec3f crashpoint = collision.getCrashPoint(body->pos, contact[i]);

          ptvel.x = -ptvel.x * contact[i].rigidity;
          ptvel.y = -ptvel.y * contact[i].rigidity;
          ptvel.z = 0.0f;

          // apply crash force [N] at center of object in X/Y direction (F=v*m/t)
          if (delta != 0.0f)
            crashforce = ptvel * type->mass / delta;
          body->addForceAtPoint(crashforce, crashpoint);

          // Trigger crash sound or amplify gravel sound
          if (contact[i].rigidity > crashthreshold)
            CLAMP_LOWER(crunch_level, crashforce.length() * 0.00001f);
          else
            skid_level += crashforce.length();
        }
      }
    }
  }

  wheel_angvel *= type->inverse_drive_total;

  wheel_speed *= type->wheel_speed_multiplier;

  skid_level *= type->wheel_speed_multiplier;
}

///
/// @brief Checks if vehicle can have a dust trail. If at least one wheel touches the ground
/// @todo Use a dynamic height?
///
bool PVehicle::canHaveDustTrail()
{
    for (unsigned int i=0; i<part.size(); ++i)
    {
        for (unsigned int j=0; j<type->part[i].wheel.size(); ++j)
        {
            vec3f wclip = part[i].wheel[j].getLowestPoint();

            PTerrain::ContactInfo tci;

            tci.pos.x = wclip.x;
            tci.pos.y = wclip.y;
            sim.getTerrain()->getContactInfo(tci);

            if (wclip.z - tci.pos.z <= MAX_DUST_TRAIL_HEIGHT)
                return true;
        }
    }

    return false;
}

///
/// @brief update parts world coord and orientation
/// @details If the local system moved, local coordinates and orientation are unchanged in the local system but not in the world system
///
void PVehicle::updateParts()
{
  for (unsigned int i=0; i<part.size(); ++i) {
    PReferenceFrame *parent;
    if (type->part[i].parent > -1)
      parent = &part[type->part[i].parent].ref_world;
    else
      parent = body;

    part[i].ref_world.ori = part[i].ref_local.ori * parent->ori;

    part[i].ref_world.updateMatrices();

    part[i].ref_world.pos = parent->pos +
      parent->getOrientationMatrix().transform1(part[i].ref_local.pos);

    for (unsigned int j=0; j<part[i].wheel.size(); j++) {
      vec3f locpos = type->part[i].wheel[j].pt +
            vec3f(0.0f, 0.0f, part[i].wheel[j].ride_pos);

      part[i].wheel[j].ref_world.setPosition(part[i].ref_world.getLocToWorldPoint(locpos));
	  part[i].wheel[j].ref_world_lowest_point.setPosition(part[i].ref_world.getLocToWorldPoint(locpos - vec3f(0,0,type->part[i].wheel[j].radius)));

      quatf turnang, spinang;
      turnang.fromZAngle(part[i].wheel[j].turn_pos);
      spinang.fromXAngle(part[i].wheel[j].spin_pos);

      part[i].wheel[j].ref_world.ori = spinang * turnang * part[i].ref_world.ori;

      part[i].wheel[j].ref_world.updateMatrices();
	  part[i].wheel[j].ref_world_lowest_point.updateMatrices();
    }
  }
}

///
/// @brief Get the lowest point of the wheel, where it would touch the ground
///
vec3f PVehicleWheel::getLowestPoint()
{
	vec3f wclip = ref_world_lowest_point.getPosition();
	
	wclip.z += INTERP(bumplast, bumpnext, bumptravel);
	
	return wclip;
}
