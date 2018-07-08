//
// Copyright (C) 2004-2006 Jasmine Langridge, ja-reiko@users.sourceforge.net
// Copyright (C) 2017 Emanuele Sorce, emanuele.sorce@hotmail.com
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

#ifndef ENGINEH
#define ENGINEH

#include <vector>
#include "vmath.h"

// RPM = revolutions per minute
// RPS = radians per second

#define RPM_TO_RPS(x) ((x) * (PI / 30.0f))
#define RPS_TO_RPM(x) ((x) * (30.0f / PI))

///
/// @brief Datas about performances of an engine
///
class PEngine {

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
	PEngine():
		gearch_first(0.4f),
		gearch_repeat(0.15f),
		minRPS(10000000.0f),
		maxRPS(0.0f) { }
  
	///
	/// @brief Add a point to the power curve
	/// @param rpm   = at 'rpm' round per minute
	/// @param power = you get 'power' output power
	/// @todo function to allow to give angular velocity directly in radians per second?
	///
	void addPowerCurvePoint(const float& rpm, const float& power);
  
	///
	/// @brief add a gear
	/// @param ratio = gear ratio
	///
	void addGear(const float& ratio);
  
	bool hasGears() { return !gear.empty(); }
	float getLastGearRatio() { return gear.back(); }
  
	// get engine horse power
	float getHorsePower();
  
	friend class PEngineInstance;
};

///
/// @brief current status of the engine
///
class PEngineInstance {
  
	// reference with performance of the engine
	PEngine *engine;
  
	// current rps (always positive)
	float rps;
  
	// gear currently used
	int currentgear;
  
	// target gear relative (1 to go up, -1 to go down, 0 to stay)
	int targetgear_rel;
  
	// timing variable for gear changing
	float gearch;
  
	// if the car is going reverse (rps will be always positive) 
	bool reverse;
  
	// current output engine torque
	float out_torque;
  
	// if the vehicle has changed gear recently
	bool flag_gearchange;
  
public:
	PEngineInstance(PEngine *neweng) :
		engine(neweng),
		currentgear(0),
		targetgear_rel(0),
		gearch(0.0f),
		reverse(false),
		out_torque(0.0f),
		flag_gearchange(false) { }

	// Simulation tick
	void tick(float delta, float throttle, float wheel_rps);
  
	// return current information
	float getOutputTorque() 
	{ 
		return out_torque;
	}
	float getEngineRPS() 
	{ 
		return rps;
	}
	float getEngineRPM() {
		return RPS_TO_RPM(rps);
	}
  
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
		rps = engine->minRPS;
		currentgear = 0;
		targetgear_rel = 0;
		gearch = 0.0f;
		out_torque = 0.0f;
	}
};

#endif
