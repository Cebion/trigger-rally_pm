
// vehicle.h [psim]

// Copyright 2004-2006 Jasmine Langridge, jas@jareiko.net
// License: GPL version 2 (see included gpl.txt)

//
// This file contains definitions and classes related to Vehicles
//

// vehicle core types

#define VCTYPE_TANK         1
#define VCTYPE_HELICOPTER   2
#define VCTYPE_PLANE        3
#define VCTYPE_HOVERCRAFT   4
#define VCTYPE_CAR          5


// vehicle clip point types

#define VCLIP_BODY          10
#define VCLIP_DRIVE_LEFT    30
#define VCLIP_DRIVE_RIGHT   31
#define VCLIP_HOVER         40


// RPM = revolutions per minute
// RPS = radians per second

#define RPM_TO_RPS(x) ((x) * (PI / 30.0f))
#define RPS_TO_RPM(x) ((x) * (30.0f / PI))

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

///
/// @brief Datas about performances of a vehicle
///
class PDriveSystem {
private:

  // the curve graph of the power of the engine
  // x = radians per second
  // y = output power
  std::vector<vec2f> powercurve;
  
  // vector of gears
  // round ratio
  std::vector<float> gear;
  
  // standard time to change gear
  float gearch_first;
  
  // time to change gear if you change it just after another change
  float gearch_repeat;
  
  // engine minimum and maximum radians per second angular speed
  float minRPS, maxRPS;
  
protected:
  float getPowerAtRPS(float rps);
  
public:
  PDriveSystem() :
    gearch_first(0.4f),
    gearch_repeat(0.15f),
    minRPS(10000000.0f),
    maxRPS(0.0f) { }
  
  ///
  /// @brief Add a point to the power curve
  /// @param rpm   = at 'rpm' round per minute
  /// @param power = you get 'power' output power
  /// @todo overload function to allow to give angular velocity directly in radians per second?
  ///
  void addPowerCurvePoint(float rpm, float power) {
	// no power with the engine still
    if (rpm <= 0.0f) return;
    
    // round per minute >> radians per second
    float rps = RPM_TO_RPS(rpm);
    
    // add the point
    powercurve.push_back(vec2f(rps, power));
    
    // if the point is out of range, adapt max or min rps
    if (minRPS > rps) minRPS = rps;
    if (maxRPS < rps) maxRPS = rps;
  }
  
  ///
  /// @brief add a gear
  /// @param ratio = gear ratio
  ///
  void addGear(float ratio) {
    if (hasGears()) {
      if (ratio <= getLastGearRatio()) return;
    } else {
      if (ratio <= 0.0f) return;
    }
    
    // put in the ratio
    gear.push_back(ratio);
  }
  
  bool hasGears() { return !gear.empty(); }
  float getLastGearRatio() { return gear.back(); }
  
  friend class PDriveSystemInstance;
};

///
/// @brief data about current values of the vehicle, compare with PDriveSystem
///
class PDriveSystemInstance {
private:
  
  // reference with performance of the vehicle
  PDriveSystem *dsys;
  
  // current rps (always positive)
  float rps;
  
  // gear currently used
  int currentgear;
  
  // target gear relative (1 to go up, -1 to go down, 0 to stay)
  int targetgear_rel;
  
  // timing variable for gear changing
  float gearch;
  
  // if the car is going reverse, (rps will be always positive) 
  bool reverse;
  
  // current output engine torque
  float out_torque;
  
  // if the vehicle has changed gear in the past
  bool flag_gearchange;
  
public:
  PDriveSystemInstance(PDriveSystem *system) :
    dsys(system),
    currentgear(0),
    targetgear_rel(0),
    gearch(0.0f),
    reverse(false),
    out_torque(0.0f),
    flag_gearchange(false) { }
  
  // Simulation tick
  void tick(float delta, float throttle, float wheel_rps);
  
  // return current information
  float getOutputTorque() { return out_torque; }
  float getEngineRPS() { return rps; }
  float getEngineRPM() { return RPS_TO_RPM(rps); }
  
  // return current gear (reverse will out -1)
  int getCurrentGear() { return reverse ? -1 : currentgear; }
  
  bool getFlagGearChange() {
    bool ret = flag_gearchange;
    flag_gearchange = false;
    return ret;
  }
  
  ///
  /// @brief Reset the engine, used i.e. with the 'recover' key
  ///
  void doReset() {
    rps = dsys->minRPS;
    currentgear = 0;
    targetgear_rel = 0;
    gearch = 0.0f;
    out_torque = 0.0f;
  }
};

struct vehicle_clip_s {
  vec3f pt;
  int type;
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
  
  std::string pstat_weightkg;
  std::string pstat_enginebhp;
  std::string pstat_wheeldrive;
  std::string pstat_handling;
  
  // type of vehicle (usually VCTYPE_CAR)
  int coretype;
  
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
  PDriveSystem dsys;
  
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
  
  // his reference position
  PReferenceFrame ref_world;
  
  float skidding, dirtthrow;
  vec3f dirtthrowpos, dirtthrowvec;
  
  float bumplast, bumpnext, bumptravel;
  
  PVehicleWheel() {
    ride_pos = 0.0f;
    ride_vel = 0.0f;
    spin_pos = 0.0f;
    spin_vel = 0.0f;
    turn_pos = 0.0f;
    bumplast = 0.0f;
    bumpnext = 0.0f;
    bumptravel = 0.0f;
  }
};


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
//class PVehicle : public NetObject {
//typedef NetObject Parent;
  
public:
  // physic simulation information
  PSim &sim;
  
  // the type of Vehicle
  PVehicleType *type;
  
  // the reference rigid body of the vehicle
  // contains datas such as position, orientation, velocity, mass ...
  PRigidBody *body;
  
  // the part which compose the vehicle
  std::vector<PVehiclePart> part;
  
  // current control state (eg. brakes, turn)
  v_state_s state;
  
  // engine instance (current RPS, gear...)
  PDriveSystemInstance dsysi;
  
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
  
  // info
  float forwardspeed;
  float wheel_angvel;
  float wheel_speed;
  float skid_level;
  
  // when a vehicle starts going offroad this time is set (to know how much time has spent offroad)
  float offroadtime_begin   = 0.0f;
  // when a vehicle stops going offroad this time is set (to know how much time has spent offroad)
  float offroadtime_end     = 0.0f;
  // total time offroad
  float offroadtime_total   = 0.0f;
  
public:
/*  
  enum StateBits {
    InitialBit = BIT(0),
    KinematicsBit = BIT(1),
    ControlBit = BIT(2),
  };
*/
public:
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
  void tick(float delta);
  
  bool canHaveDustTrail();
  
  void updateParts();
  
  // reset the car when it get flipped or something
  void doReset();
  // reset the car when 'q' is pressed
  void doReset2(const vec3f &pos, const quatf &ori);
  
  float getEngineRPM() { return dsysi.getEngineRPM(); }
  int getCurrentGear() { return dsysi.getCurrentGear(); }
  bool getFlagGearChange() { return dsysi.getFlagGearChange(); }
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
};
