
// vehicle.cpp

// Copyright 2004-2006 Jasmine Langridge, jas@jareiko.net
// License: GPL version 2 (see included gpl.txt)

#include "psim.h"

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
  proper_name = "Vehicle";
  proper_class = "Unknown";

  pstat_weightkg = "N/A";
  pstat_enginebhp = "N/A";
  pstat_wheeldrive = "N/A";
  pstat_handling = "N/A";

  mass = 1.0;
  dims = vec3f(1.0,1.0,1.0);

  wheelscale = 1.0;
  wheelmodel = nullptr;

  ctrlrate.setDefaultRates();

  param.speed = 0.0;
  param.turnspeed = vec3f::zero();
  param.turnspeed_a = 1.0;
  param.turnspeed_b = 0.0;
  param.drag = vec3f::zero();
  param.angdrag = 0.0;
  param.lift = vec2f::zero();
  param.fineffect = vec2f::zero();

  float allscale = 1.0;

  float drive_total = 0.0f;

  wheel_speed_multiplier = 0.0f;

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

	// stats not used by simulation displayed to user
    if (!strcmp(walk->Value(), "pstats")) {
        val = walk->Attribute("enginebhp");
        if (val != nullptr) pstat_enginebhp = val;

        val = walk->Attribute("wheeldrive");
        if (val != nullptr) pstat_wheeldrive = val;

        val = walk->Attribute("handling");
        if (val != nullptr) pstat_handling = val;

	// generic params
    } else if (!strcmp(walk->Value(), "genparams")) {

      val = walk->Attribute("mass");
      if (val)
	  {
		  mass = atof(val);
		  std::stringstream ss;
		  ss << mass;
		  pstat_weightkg = ss.str();
      }
      val = walk->Attribute("dimensions");
      if (val) {
        sscanf(val, "%f , %f , %f", &dims.x, &dims.y, &dims.z);
        dims *= allscale;
      }

      val = walk->Attribute("wheelscale");
      if (val) wheelscale = atof(val);

      val = walk->Attribute("wheelmodel");
      if (val) wheelmodel = ssModel.loadModel(PUtil::assemblePath(val, filename));

	// params about controls
    } else if (!strcmp(walk->Value(), "ctrlparams")) {

      val = walk->Attribute("speed");
      if (val) param.speed = atof(val);

      val = walk->Attribute("turnspeed");
      if (val) sscanf(val, "%f , %f , %f", &param.turnspeed.x, &param.turnspeed.y, &param.turnspeed.z);

      val = walk->Attribute("drag");
      if (val) sscanf(val, "%f , %f , %f", &param.drag.x, &param.drag.y, &param.drag.z);

      val = walk->Attribute("angdrag");
      if (val) param.angdrag = atof(val);

      val = walk->Attribute("lift");
      if (val) sscanf(val, "%f , %f", &param.lift.x, &param.lift.y);


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

      val = walk->Attribute("orientation");
      if (val) {
        quatf ori;
        // note: w first, as per usual mathematical notation
        if (sscanf(val, "%f , %f , %f , %f", &ori.w, &ori.x, &ori.y, &ori.z) == 4)
          vtp->ref_local.setOrientation(ori);
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

          if (false) ;
          else if (!strcmp(val, "body")) vc.type = v_clip_type::body;
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

          vtw.radius = 1.0f;
          vtw.drive = 0.0f;
          vtw.steer = 0.0f;
          vtw.brake1 = 0.0f;
          vtw.brake2 = 0.0f;

          vtw.force = 0.0f;
          vtw.dampening = 0.0f;

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



//TNL_IMPLEMENT_NETOBJECT(PVehicle);

///
/// @brief Constructor of the PVehicle class
/// @param sim_parent = the simulation where the vehicle stays
/// @param _type = the type of vehicle the new vehicle is
///
PVehicle::PVehicle(PSim &sim_parent, PVehicleType *_type) :
  sim(sim_parent), type(_type), iengine(&_type->engine)
{
  // body is the vehicle rigid body
  body = sim.createRigidBody();

  // vehicle mass is approximately meant to be cuboid
  body->setMassCuboid(type->mass, type->dims);

  // set control
  state.setZero();
  ctrl.setZero();

  forwardspeed = 0.0f;

  blade_ang1 = 0.0;

  // set starting checkpoints
  nextcp = 0;
  nextcdcp = 0;

  // set lap
  currentlap = 1;

  wheel_angvel = 0.0f;

  reset_trigger_time = 0.0f;

  reset_time = 0.0f;

  crunch_level = 0.0f;
  crunch_level_prev = 0.0f;

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

  // set a reset time again
  reset_time = 3.0f;

  crunch_level = 0.0f;
  crunch_level_prev = 0.0f;

  // stop all the wheels
  for (unsigned int i=0; i<part.size(); i++) {
    for (unsigned int j=0; j<part[i].wheel.size(); j++) {
      part[i].wheel[j].spin_vel = 0.0f;
      part[i].wheel[j].spin_pos = 0.0f;
      part[i].wheel[j].ride_vel = 0.0f;
      part[i].wheel[j].ride_pos = 0.0f;
      part[i].wheel[j].turn_pos = 0.0f;
      part[i].wheel[j].skidding = 0.0f;
      part[i].wheel[j].dirtthrow = 0.0f;
    }
  }

  // stop
  forwardspeed = 0.0f;
  wheel_angvel = 0.0f;
  wheel_speed = 0.0f;

  // reset engine
  iengine.doReset();

  // control state
  state.setZero();
}

///
/// @brief reset the vehicle to a specific point (used when the 'recover' button is pressed)
/// @param pos = the position where the vehicle has to be set
/// @param ori = the orientation the vehicle has to be set
/// @todo this share much code with doReset(), maybe we should do something about it?
///
void PVehicle::doReset2(const vec3f &pos, const quatf &ori)
{
  // some time has to pass before reset
  if (reset_time != 0.0f) return;

  // the reset position
  reset_pos = pos;

  // set the new orientation
  quatf temp = ori; // FIXME: laziness and fear of breaking copy-pasted code

  if (body->ori.dot(temp) < 0.0f) temp = temp * -1.0f;

  reset_ori = temp;

  // set a reset time again
  reset_time = 3.0f;

  crunch_level = 0.0f;
  crunch_level_prev = 0.0f;

  // stop all the wheels
  for (unsigned int i=0; i<part.size(); i++) {
    for (unsigned int j=0; j<part[i].wheel.size(); j++) {
      part[i].wheel[j].spin_vel = 0.0f;
      part[i].wheel[j].spin_pos = 0.0f;
      part[i].wheel[j].ride_vel = 0.0f;
      part[i].wheel[j].ride_pos = 0.0f;
      part[i].wheel[j].turn_pos = 0.0f;
      part[i].wheel[j].skidding = 0.0f;
      part[i].wheel[j].dirtthrow = 0.0f;
    }
  }

  // stop
  forwardspeed = 0.0f;
  wheel_angvel = 0.0f;
  wheel_speed = 0.0f;

  // reset engine
  iengine.doReset();

  state.setZero();
}

///
/// @brief Physic simulation for the single vehicle
/// @param delta = timeslice to compute
///
void PVehicle::tick(float delta)
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
  //vec3f pos = body->getPosition();
  vec3f linvel = body->getLinearVel();
  mat44f orimatt = body->getInverseOrientationMatrix();
  vec3f angvel = body->getAngularVel();

  if (orimatt.row[2].z <= 0.1f) {
    reset_trigger_time += delta;

    if (reset_trigger_time >= 4.0f)
      doReset();
  } else
    reset_trigger_time = 0.0f;

  vec3f loclinvel = body->getWorldToLocVector(linvel);
  vec3f locangvel = body->getWorldToLocVector(angvel);
  //vec3f locangvel = body->getLocToWorldVector(angvel);
  //vec3f locangvel = angvel;

  // check for resetting (if the vehicle has been flipped or something)
  if (reset_time != 0.0f) {
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

  // angular drag
  body->addTorque(angvel.modulate(angvel) * -type->param.angdrag);

  // linear drag
  vec3f frc = -vec3f(
    SQUARED(loclinvel.x) * type->param.drag.x,
    SQUARED(loclinvel.y) * type->param.drag.y,
    SQUARED(loclinvel.z) * type->param.drag.z);

  // lift
  frc += -vec3f(
    loclinvel.x * type->param.lift.x * loclinvel.y,
    0.0,
    loclinvel.z * type->param.lift.y * loclinvel.y);


  // VEHICLE TYPE POINT

  // vehicle-specific code
  switch (type->coretype) {
  default: break;

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

  case v_core_type::car:
    break;
  }

  body->addLocForce(frc);

  vec3f forwarddir = makevec3f(body->getInverseOrientationMatrix().row[1]);
  //vec3f rightdir = makevec3f(body->getInverseOrientationMatrix().row[0]);

  // handle engine (output torque, change gear if needed...)
  iengine.tick(delta, state.throttle, wheel_angvel);

  // Output engine power
  float drivetorque = iengine.getOutputTorque();

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
              // how much the fast the clip is going down (times own clip dampen capabilities)
              - surfvel.z * type->part[i].clip[j].dampening;

            // if the clip pushes against the ground
            if (perpforce > 0.0f) {
              vec2f friction = vec2f(-surfvel.x, -surfvel.y) * 10000.0f;

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
      const TerrainType mf_tt = sim.getTerrain()->getRoadSurface(wheel.ref_world.getPosition());
      const float mf_coef     = PUtil::decideFrictionCoef(mf_tt);
      const float mf_resis    = PUtil::decideResistance(mf_tt);

      // where the wheel might touch the ground
      vec3f wclip = wheel.getLowestPoint(typewheel);

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
      // is tensed up and the proper suspension force
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

        // suspension can't get more strecthed than the 70% of the wheel radius
        float maxdepth = typewheel.radius * 0.7f;

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
          vec2f friction = vec2f(-surfvel.x, -surfvel.y) * 10000.0f;

          // max friction available proportional to the pressure of the wheel to the ground and ground own friction coefficent
          float maxfriction = perpforce * mf_coef;

          float testfriction = perpforce * 1.0f;

          // the length of the friction
          float leng = friction.length();

          // if there is some friction, and it is bigger than testfriction
          if (leng > 0.0f && leng > testfriction)
            // friction will be put equal to maxfriction with a little gain
            friction *= (maxfriction / leng) + 0.02f;

          // the force of the wheel
          frc += (
              // the perpendicular force along the normal
		      tci.normal * perpforce +
		      // laterally
              surf_right * friction.x +
              // in the forward direction
              surf_forward * friction.y);

          // update wheel spin velocity
          wheel.spin_vel -= (friction.y * typewheel.radius) * 0.1f * delta;

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
          wheel.dirtthrowpos = wheel.ref_world.getPosition() +
            downward * typewheel.radius;
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
            vec3f wclip = part[i].wheel[j].getLowestPoint(type->part[i].wheel[j]);

            PTerrain::ContactInfo tci;

            tci.pos.x = wclip.x;
            tci.pos.y = wclip.y;
            sim.getTerrain()->getContactInfo(tci);

#define MAX_HEIGHT 1.5f
            if (wclip.z - tci.pos.z <= MAX_HEIGHT)
                return true;
#undef MAX_HEIGHT
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

      quatf turnang, spinang;
      turnang.fromZAngle(part[i].wheel[j].turn_pos);
      spinang.fromXAngle(part[i].wheel[j].spin_pos);

      part[i].wheel[j].ref_world.ori = spinang * turnang * part[i].ref_world.ori;

      part[i].wheel[j].ref_world.updateMatrices();
    }
  }
}

///
/// @brief Get the lowest point of the wheel, where it would touch the ground
/// @todo calc wclip along wheel plane instead of just straight down to prevent unrealistic behaviour
///
vec3f PVehicleWheel::getLowestPoint(const PVehicleTypeWheel& typewheel)
{
	vec3f wclip = ref_world.getPosition();

	wclip.z -= typewheel.radius - INTERP(bumplast, bumpnext, bumptravel);

	return wclip;
}
