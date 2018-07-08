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

#include "engine.h"

void PEngine::addPowerCurvePoint(const float& rpm, const float& power)
{
	// no power with the engine still
	if (rpm <= 0.0f) return;
    
	// round per minute >> radians per second
	float rps = RPM_TO_RPS(rpm);
    
	// add the point
	powercurve.push_back(vec2f(rps, power));
    
	// if the point is out of range, adapt max or min rps
	if (minRPS > rps)
		minRPS = rps;
	else if (maxRPS < rps)
		maxRPS = rps;
}

void PEngine::addGear(const float& ratio)
{
	if (hasGears()) {
		if (ratio <= getLastGearRatio()) return;
	} else {
		if (ratio <= 0.0f) return;
	}

	// put in the ratio
	gear.push_back(ratio);
}

float PEngine::getHorsePower()
{
	float bhp = 0;
	
	const float k = 4.75e-6;
	
	// return point with higher output
	for(unsigned int i=0; i!=powercurve.size(); i++)
		bhp = MAX(bhp, powercurve[i][1] * RPS_TO_RPM(powercurve[i][0]) * k);
	
	return bhp;
}

///
/// @brief Get the engine power output at a rps
/// @param rps = radians per second
/// @retval output power
///
float PEngine::getPowerAtRPS(float rps)
{
	unsigned int p;
	float power;

	// find which curve points rps lies between
	for (p = 0; p < powercurve.size() && powercurve[p].x < rps; p++);

	if (p == 0) {
		// to the left of the graph
		power = powercurve[0].y * (rps / powercurve[0].x);
	} else if (p < powercurve.size()) {
		// on the graph
		power = powercurve[p-1].y + (powercurve[p].y - powercurve[p-1].y) *
			( (rps - powercurve[p-1].x) / (powercurve[p].x - powercurve[p-1].x) );
	} else {
		// to the right of the graph
		power = powercurve[p-1].y + (0.0f - powercurve[p-1].y) *
			( (rps - powercurve[p-1].x) / (powercurve.back().x - powercurve[p-1].x) );
	}

	return power;
}

///
/// @brief engine simulation tick. Decide if change gear, compute output torque
/// @param delta = timeslice to compute
/// @param wheel_rps = current rps of the wheel
/// @param thottle = input throttle
///
void PEngineInstance::tick(float delta, float throttle, float wheel_rps)
{
	// convert the rps of the wheel to the actual engine rps
	// multiplying it for the inverse of the current gear ratio
	rps = wheel_rps / engine->gear[currentgear];

	bool wasreverse = reverse;

	// check if the throttle will going reverse or not
	reverse = (throttle < 0.0f);

	// if reverse changes, there is a gear change
	if (wasreverse != reverse) flag_gearchange = true;

	// rps and throttle will be set here always positive
	if (reverse)
	{
		rps *= -1.0f;
		throttle *= -1.0f;
	}

	CLAMP_UPPER(throttle, 1.0f);

	// engine rps have to be in the range
	CLAMP(rps, engine->minRPS, engine->maxRPS);

	if (reverse)
		currentgear = 0;

	// final output engine torque
	out_torque = engine->getPowerAtRPS(rps) / (engine->gear[currentgear] * rps);

	// Change gear only if it's not reversed
	if (!reverse)
	{
		// store if we should change gear (0 no, 1 go up, -1 go down)
		int newtarget_rel = 0;

		// if it's not last gear (we can go up)
		if (currentgear < (int)engine->gear.size()-1)
		{
			// nextrate = rps if the gear was the next one
			float nextrate = rps * engine->gear[currentgear] / engine->gear[currentgear+1];
			// nextrate has to be in the rps range
			CLAMP(nextrate, engine->minRPS, engine->maxRPS);
			// final output engine torque if the gear was the next one
			float nexttorque = engine->getPowerAtRPS(nextrate) / (engine->gear[currentgear+1] * nextrate);
			// if going up we gain torque
			if (nexttorque > out_torque)
				// do it
				newtarget_rel = 1;
		}

		// if the gear is not reverse and we haven't yet decided to go up
		if (currentgear > 0 && newtarget_rel == 0)
		{
			// nextrate = rps if the gear was the previous one
			float nextrate = rps * engine->gear[currentgear] / engine->gear[currentgear-1];
			// nextrate has to be in the rps range
			CLAMP(nextrate, engine->minRPS, engine->maxRPS);
			// final output engine torque if the gear was the previous one
			float nexttorque = engine->getPowerAtRPS(nextrate) / (engine->gear[currentgear-1] * nextrate);
			// if going down we gain gear
			if (nexttorque > out_torque)
				// do it
				newtarget_rel = -1;
		}

		// if we are going to change gear and targetgear_rel is updated
		if (newtarget_rel != 0 && newtarget_rel == targetgear_rel)
		{
			// if has passed enought time
			if ((gearch -= delta) <= 0.0f)
			{
				// the rps with the new gear
				float nextrate = rps * engine->gear[currentgear] / engine->gear[currentgear + targetgear_rel];
				CLAMP(nextrate, engine->minRPS, engine->maxRPS);
				// final output torque with the new gear
				out_torque = engine->getPowerAtRPS(nextrate) / (engine->gear[currentgear + targetgear_rel] * nextrate);
				// change gear
				currentgear += targetgear_rel;
				// set gearch
				gearch = engine->gearch_repeat;
				// there is a gear change
				flag_gearchange = true;
			}
		} else {
			// update targetgear_rel, and set gearch
			gearch = engine->gearch_first;
			targetgear_rel = newtarget_rel;
		}
	}

	// output torque is proportional to throttle
	out_torque *= throttle;

	// if reverse the output torque is obviously reversed
	if (reverse)
		out_torque *= -1.0;

	// transimission and rolling energy dispersion and resistance
	out_torque -= wheel_rps * 0.1f;
}
