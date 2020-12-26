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

#ifndef DAMAGEH
#define DAMAGEH

#include "vmath.h"
#include <vector>

class PReferenceFrame;
struct vehicle_clip_s;

/// @brief Damage indicator in racing UI
///
class PDamage {
public:
  ///
  /// @brief Side of the vehicle colliding with object
  ///
  enum DamageSide {
    DamageFrontLeft = 0,
    DamageFrontRight,
    DamageRearLeft,
    DamageRearRight,
    DamageSideSize
  };

  void setClip(const std::vector<vehicle_clip_s> &clip);
  void addDamage(const vec3f &crashpoint, float increment, PReferenceFrame &ref_world);
  float getDamage(DamageSide damageside);
  float getDamage(const vec3f &position);

private:
  // Damage of each vehicle side
  float damage[DamageSide::DamageSideSize];
  // Center position of each vehicle side
  vec3f center[DamageSide::DamageSideSize];
  // Indicator flashes on impact
  bool flash[DamageSide::DamageSideSize];
};

#endif
