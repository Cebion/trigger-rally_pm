//
// Copyright (C) 2020 Martin Scherer
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

#ifndef COLLISIONH
#define COLLISIONH

#include "psim.h"
#include "vehicle.h"

///
/// @brief Handling of collisions with world objects
///
class PCollision {
public:
  PCollision(const std::vector<vehicle_clip_s> &clip, PReferenceFrame &ref_world);
  const std::vector<PTerrainFoliage> checkContact(const std::vector<PTerrainFoliage> *foliage) const;
  bool towardsContact(const vec3f &body, const vec3f &contact, const vec3f &diff) const;
  const vec3f &getCrashPoint(const vec3f &body, const PTerrainFoliage &foliage);

private:
  PCollision();
  PCollision(const PCollision&);
  PCollision& operator=(const PCollision&);

  void calcmin(const vec3f &a);
  void calcmax(const vec3f &a);

  // 3D minimum position of AABB box
  vec3f boxmin;
  // 3D maximum position of AABB box
  vec3f boxmax;
  // Position to apply crash force
  vec3f crashpoint;
};

#endif
