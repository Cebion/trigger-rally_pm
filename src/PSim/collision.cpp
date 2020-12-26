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

#include "collision.h"

///
/// @brief Calculates AABB box from clip positions in world coordinates
/// @param clip = vector of vehicle clips
/// @param ref_world = to calculate world coordinates
///
PCollision::PCollision(const std::vector<vehicle_clip_s> &clip, PReferenceFrame &ref_world) :
    crashpoint(vec3f::zero())
{
  boxmin = vec3f(
    std::numeric_limits<float>::max(),
    std::numeric_limits<float>::max(),
    std::numeric_limits<float>::max());
  boxmax = vec3f(
    std::numeric_limits<float>::lowest(),
    std::numeric_limits<float>::lowest(),
    std::numeric_limits<float>::lowest());

  for (unsigned int i = 0; i < clip.size(); ++i) {
    const vec3f &lcliptop = clip[i].pt;
    vec3f lclipbottom = lcliptop;
    vec3f wcliptop = vec3f::zero();
    vec3f wclipbottom = vec3f::zero();

    lclipbottom.z = 0.0f;
    wcliptop = ref_world.getLocToWorldPoint(lcliptop);
    wclipbottom = ref_world.getLocToWorldPoint(lclipbottom);

    calcmin(wcliptop);
    calcmin(wclipbottom);
    calcmax(wcliptop);
    calcmax(wclipbottom);
  }
}

///
/// @brief Returns objects inside AABB box
/// @param foliage = vector of world objects
/// @retval vector of inside objects
///
const std::vector<PTerrainFoliage> PCollision::checkContact(const std::vector<PTerrainFoliage> *foliage) const
{
  std::vector<PTerrainFoliage> contact;

  for (std::vector<PTerrainFoliage>::const_iterator iter = foliage->begin(); iter != foliage->end(); ++iter) {
    const vec3f &foliagemin = iter->pos;
    vec3f foliagemax = iter->pos;

    foliagemax.z += iter->scale;
    if (foliagemin.x <= boxmax.x && foliagemax.x >= boxmin.x &&
        foliagemin.y <= boxmax.y && foliagemax.y >= boxmin.y &&
        foliagemin.z <= boxmax.z && foliagemax.z >= boxmin.z) {
      contact.push_back(*iter);
    }
  }
  return contact;
}

///
/// @brief Prevents vehicle to be stuck with world objects 
/// @param body = position of vehicle
/// @param contact = position of world object
/// @param diff = distance moved by vehicle
/// @retval true if vehicle drives moves towards object
///
bool PCollision::towardsContact(const vec3f &body, const vec3f &contact, const vec3f &diff) const
{
  vec3f dist1 = body - contact;
  vec3f dist2 = body + diff - contact;
  return (dist1.length() > dist2.length());
}

///
/// @brief Calculate position to apply crash force
/// @param body = position of vehicle
/// @param foliage = crash object
/// @retval position to apply crash force
///
const vec3f &PCollision::getCrashPoint(const vec3f &body, const PTerrainFoliage &foliage)
{
  // Either take vehicle center Z position or highest point of object
  crashpoint = foliage.pos;
  crashpoint.z = MIN(body.z, foliage.pos.z + foliage.scale);
  return crashpoint;
}

///
/// @brief Calculates 3D minimum position of AABB box
/// @param a = vector to compare to minimum
///
void PCollision::calcmin(const vec3f &a)
{
  boxmin.x = MIN(boxmin.x, a.x);
  boxmin.y = MIN(boxmin.y, a.y);
  boxmin.z = MIN(boxmin.z, a.z);
}

///
/// @brief Calculates 3D maximum position of AABB box
/// @param a = vector to compare to maximum
///
void PCollision::calcmax(const vec3f &a)
{
  boxmax.x = MAX(boxmax.x, a.x);
  boxmax.y = MAX(boxmax.y, a.y);
  boxmax.z = MAX(boxmax.z, a.z);
}
