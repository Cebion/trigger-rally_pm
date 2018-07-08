
// vehicle.h [psim]

// Copyright 2004-2006 Jasmine Langridge, jas@jareiko.net
// License: GPL version 2 (see included gpl.txt)

//
// This file contains definitions and classes related to Vehicles
//

#include "engine.h"

// vehicle core types
enum class v_core_type{
	car,
	tank,
	helicopter,
	plane,
	hovercraft
};


// vehicle clip point types
enum class v_clip_type{
	body,
	drive_left,
	drive_right,
	hover
};

// MPS = metres per second
// KPH = kilometres per hour
// MPH = miles per hour

#define MPS_TO_MPH(x) ((x) * 2.23693629f) // thanks Google!
#define MPS_TO_KPH(x) ((x) * 3.6f)

// Starting position in degrees, measured counter-clockwise from the x-axis.
#define MPH_ZERO_DEG 210
#define KPH_ZERO_DEG 220

// Degrees to rotate the speedo needle for each unit of speed
#define DEG_PER_MPH 1.5f
#define DEG_PER_KPH 1.0f

// Multiplier for mps to speed in some unit
const float MPS_MPH_SPEED_MULT = 2.23693629f;
const float MPS_KPH_SPEED_MULT = 3.6f;

// Multiplier for mps to degs on the speedo dial
const float MPS_MPH_DEG_MULT = MPS_MPH_SPEED_MULT * DEG_PER_MPH;
const float MPS_KPH_DEG_MULT = MPS_KPH_SPEED_MULT * DEG_PER_KPH;

///
/// @brief class which contains the control status of a vehicle
///
struct v_control_s {
  // shared
  float throttle;
  float brake1,brake2;
  vec3f turn;
  vec2f aim;

  // helicopter
  float collective;

  // -- utility --

  void setZero() {
    throttle = 0.0f;
    brake1 = 0.0f;
    brake2 = 0.0f;
    turn = vec3f::zero();
    aim = vec2f::zero();
    collective = 0.0f;
  }
  
  void setDefaultRates() {
    throttle = 10.0f;
    brake1 = 10.0f;
    brake2 = 10.0f;
    turn = vec3f(10.0f,10.0f,10.0f);
    aim = vec2f(10.0f,10.0f);
    collective = 10.0f;
  }
  
  ///
  /// @brief ensure all the values are within range
  ///
  void clamp() {
    CLAMP(throttle, -1.0f, 1.0f);
    CLAMP(brake1, 0.0f, 1.0f);
    CLAMP(brake2, 0.0f, 1.0f);
    CLAMP(turn.x, -1.0f, 1.0f);
    CLAMP(turn.y, -1.0f, 1.0f);
    CLAMP(turn.z, -1.0f, 1.0f);
    CLAMP(aim.x, -1.0f, 1.0f);
    CLAMP(aim.y, -1.0f, 1.0f);
    CLAMP(collective, -1.0f, 1.0f);
  }
};

// @todo Why call the same class in two ways?
typedef v_control_s v_state_s;


struct vehicle_clip_s {
  vec3f pt;
  v_clip_type type;
  float force, dampening;
};

///
/// @brief stores a type of wheel and its stats
///
struct PVehicleTypeWheel {
  // its position
  vec3f pt;
  float radius;
  // performance
  float drive, steer, brake1, brake2;
  // suspension proper resistance force
  float force;
  float dampening;
  
  // frictions value of the wheel with the ground (around 0.03)
  float friction;
};

///
/// @brief stores a type of part
///
struct PVehicleTypePart {
  std::string name, parentname;
  int parent;
  
  PReferenceFrame ref_local;
  
  std::vector<vehicle_clip_s> clip;
  
  std::vector<PVehicleTypeWheel> wheel;
  
  std::vector<PReferenceFrame> flame;
  
  float scale;
  PModel *model;
};

///
/// @brief class which store a model (type) of vehicle: e.g. name, specifications
///
class PVehicleType : public PResource {
public:

  // Statistics and stuff displayed to the user, not actually used in the simulation
  std::string proper_name;
  std::string proper_class;  // class name (i.e. "WRC")
  
  // how many wheels drive
  std::string pstat_wheeldrive;
  
  // contains 100 times the average wheel roadholding in human readable form
  std::string pstat_roadholding;
  
  // contains the engine max power in human readable form
  std::string pstat_enginepower;
  
  // type of vehicle (usually car)
  v_core_type coretype;
  
  // mass
  float mass;
  
  // dimensions (aproximated as a cuboid)
  vec3f dims;
  
  // parts which compose the vehicle
  std::vector<PVehicleTypePart> part;
  
  // wheel scale
  float wheelscale;
  
  // wheel model
  PModel *wheelmodel;
  
  // Car statistics (powercurve, gears...)
  PEngine engine;
  
  float inverse_drive_total;
  
  float wheel_speed_multiplier;
  
  // Vehicle dinamic specification
  struct {
    // shared
    float speed;
    vec3f turnspeed;
    float turnspeed_a, turnspeed_b; // turnspeed = a + b * speed
    vec3f drag;
    float angdrag; // angular drag
    vec2f lift; // x = fin lift (hz), y = wing lift (vt)
    vec2f fineffect; // x = rudder/fin (hz), y = tail (vt)
  } param;

  // vehicle specific control specifications
  v_control_s ctrlrate;

public:
  PVehicleType() { }
  ~PVehicleType() { unload(); }

public:
  bool load(const std::string &filename, PSSModel &ssModel);
  void unload();
};


///
/// @brief Class representing a wheel of a vehicle
///
struct PVehicleWheel {

	// suspension position
	float ride_pos;
	// suspension position changing velocity
	float ride_vel;
	// driving axis rotation
	float spin_pos, spin_vel;
	// steering axis rotation
	float turn_pos;
  
	// his reference position in the world
	PReferenceFrame ref_world;
	
	// the reference position in the world of the lowest point of the wheel (the one touching the ground)
	PReferenceFrame ref_world_lowest_point;
  
	float skidding, dirtthrow;
	// where the dust trail starts and its velocity
	vec3f dirtthrowpos, dirtthrowvec;
  
    // bump travel is the current velocity from bumplast to bumpnext
	float bumplast, bumpnext, bumptravel;
  
	PVehicleWheel();
	
	// reset wheel to default
	void reset();
	
	// get the lowest point of the wheel (the one that will touch the ground)
	vec3f getLowestPoint();
};

///
/// @brief a vehicle point part, can have wheels attached
///
struct PVehiclePart {

  // ref_local is initted from vehicle type, but may change per-vehicle

  // reference points in the local and world system
  PReferenceFrame ref_local, ref_world;
  
  std::vector<PVehicleWheel> wheel;
};

///
/// @brief store a vehicle instance
///
class PVehicle {
  
public:
  // physic simulation information
  PSim &sim;
  
  // the type of Vehicle and of his part
  PVehicleType *type;
  
  // the reference rigid body of the vehicle
  // contains datas such as position, orientation, velocity, mass ...
  PRigidBody *body;
  
  // the part which compose the vehicle
  std::vector<PVehiclePart> part;
  
  // current control state (eg. brakes, turn)
  v_state_s state;
  
  // engine instance
  PEngineInstance iengine;
  
  // helicopter-specific
  float blade_ang1;
  
  // next checkpoint
  int nextcp;
  // next codriver checkpoint
  int nextcdcp;
  // current lap, counted from 1
  int currentlap;
  
  // for vehicle resetting, after being flipped
  float reset_trigger_time;
  vec3f reset_pos;
  quatf reset_ori;
  float reset_time;
  
  // for body crash/impact noises
  float crunch_level, crunch_level_prev;
  
  // current controls situation (eg. brakes, turn)
  v_control_s ctrl;
  
  float forwardspeed;
  float wheel_angvel;
  float wheel_speed;
  
  // how much noise for the drifting of the wheels. 0 means none
  float skid_level;
  
  // when a vehicle starts going offroad this time is set (to know how much time has spent offroad)
  float offroadtime_begin;
  // when a vehicle stops going offroad this time is set (to know how much time has spent offroad)
  float offroadtime_end;
  // total time offroad
  float offroadtime_total;
  
  PVehicle(PSim &sim_parent, PVehicleType *_type);
  //~PVehicle() { unload(); } // body unloaded by sim
  
public:
  PRigidBody &getBody() { return *body; }
  
  /*
  // NetObject stuff
  void performScopeQuery(GhostConnection *connection);
  U32 packUpdate(GhostConnection *connection, U32 updateMask, BitStream *stream);
  void unpackUpdate(GhostConnection *connection, BitStream *stream);
  */
  
  // simulate for 'delta' seconds
  void tick(const float& delta);
  
  // check if a wheel touches the ground so that can have a dust trail
  bool canHaveDustTrail();
  
  // update world reference of parts and wheels
  void updateParts();
  
  // reset the car in place
  void doReset();
  // reset the car to a custom position and orientation
  void doReset(const vec3f &pos, const quatf &ori);
  
  float getEngineRPM() { return iengine.getEngineRPM(); }
  int getCurrentGear() { return iengine.getCurrentGear(); }
  bool getFlagGearChange() { return iengine.getFlagGearChange(); }
  float getCrashNoiseLevel() {
    if (crunch_level > crunch_level_prev) {
      float tmp = crunch_level - crunch_level_prev;
      crunch_level_prev = crunch_level;
      return tmp;
    } else {
      return 0.0f;
    }
  }
  float getWheelSpeed() { return wheel_speed; }
  float getSkidLevel() { return skid_level; }

private:
	// subroutines that reset and stops status of the car
	// it's more low level that the doReset() one
	void reset();
};
