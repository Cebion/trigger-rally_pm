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

#include "damage.h"
#include "vehicle.h"
#include <limits>

///
/// @brief Calculate center position of each vehicle side
/// @param clip = vector of vehicle clips
///
void PDamage::setClip(const std::vector<vehicle_clip_s> &clip)
{
  vec3f clipmin = vec3f(
      std::numeric_limits<float>::max(),
      std::numeric_limits<float>::max(),
      std::numeric_limits<float>::max());
  vec3f clipmax = vec3f(
      std::numeric_limits<float>::lowest(),
      std::numeric_limits<float>::lowest(),
      std::numeric_limits<float>::lowest());

  for (unsigned int i = 0; i < clip.size(); ++i) {
    if (clip[i].pt.x < clipmin.x) clipmin.x = clip[i].pt.x;
    if (clip[i].pt.y < clipmin.y) clipmin.y = clip[i].pt.y;
    if (clip[i].pt.z < clipmin.z) clipmin.z = clip[i].pt.z;

    if (clip[i].pt.x > clipmax.x) clipmax.x = clip[i].pt.x;
    if (clip[i].pt.y > clipmax.y) clipmax.y = clip[i].pt.y;
    if (clip[i].pt.z > clipmax.z) clipmax.z = clip[i].pt.z;
  }
  for (unsigned int i = 0; i < DamageSide::DamageSideSize; ++i) {
    vec3f position = vec3f::zero();

    switch (i) {
    case DamageSide::DamageFrontLeft:
      position.x = clipmin.x;
      position.y = clipmax.y;
      position.z = 0.5f * clipmax.z;
      break;
    case DamageSide::DamageFrontRight:
      position.x = clipmax.x;
      position.y = clipmax.y;
      position.z = 0.5f * clipmax.z;
      break;
    case DamageSide::DamageRearLeft:
      position.x = clipmin.x;
      position.y = clipmin.y;
      position.z = 0.5f * clipmax.z;
      break;
    case DamageSide::DamageRearRight:
      position.x = clipmax.x;
      position.y = clipmin.y;
      position.z = 0.5f * clipmax.z;
      break;
    default:
      break;
    }

    center[i] = position;
    damage[i] = 0.0f;
    flash[i] = false;
  }
}

///
/// @brief Add damage to vehicle side closest to point of impact
/// @param crashpoint = point to which crash force is applied
/// @param increment = amount of damage to be added
/// @param ref_world = to calculate world coordinates
///
void PDamage::addDamage(const vec3f &crashpoint, float increment, PReferenceFrame &ref_world)
{
  float minimum = std::numeric_limits<float>::max();
  DamageSide damageside = DamageSide::DamageSideSize;

  for (unsigned int i = 0; i < DamageSide::DamageSideSize; ++i) {
    vec3f delta = crashpoint - ref_world.getLocToWorldPoint(center[i]);
    float distance = delta.length();

    if (distance < minimum) {
      minimum = distance;
      damageside = (DamageSide)i;
    }
  }
  if (damageside < DamageSide::DamageSideSize) {
    damage[damageside] += increment;
    flash[damageside] = true;
  }
}

///
/// @brief Return damage of a vehicle side
/// @param damageside = vehicle side
/// @retval damage (-1.0 to flash indicator)
///
float PDamage::getDamage(DamageSide damageside)
{
  if (damageside >= DamageSide::DamageSideSize)
    return 0.0f;
  if (flash[damageside]) {
    flash[damageside] = false;
    return -1.0;
  }
  return damage[damageside];
}

///
/// @brief Return damage closest to a local position
/// @param position = local position
/// @retval damage (0.0 to 1.0)
///
float PDamage::getDamage(const vec3f &position)
{
  float minimum = std::numeric_limits<float>::max();
  DamageSide damageside = DamageSide::DamageSideSize;

  for (unsigned int i = 0; i < DamageSide::DamageSideSize; ++i) {
    vec3f delta = position - center[i];
    float distance = delta.length();

    if (distance < minimum) {
      minimum = distance;
      damageside = (DamageSide)i;
    }
  }
  if (damageside >= DamageSide::DamageSideSize)
    return 0.0f;
  if (damage[damageside] > 1.0f)
    return 1.0f;
  return damage[damageside];
}
