//
// Copyright (C) 2020 Martin Scherer, martinscherer84@gmail.com
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
PCollision::PCollision(const std::vector<vehicle_clip_s> &clip, PReferenceFrame &ref_world)
{
  boxmin = vec2f(
    std::numeric_limits<float>::max(),
    std::numeric_limits<float>::max());
  boxmax = vec2f(
    std::numeric_limits<float>::lowest(),
    std::numeric_limits<float>::lowest());

  for (unsigned int i = 0; i < clip.size(); ++i) {
    vec3f wclip = ref_world.getLocToWorldPoint(clip[i].pt);

    calcmin(wclip);
    calcmax(wclip);
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
    if (iter->pos.x >= boxmin.x && iter->pos.x <= boxmax.x &&
        iter->pos.y >= boxmin.y && iter->pos.y <= boxmax.y) {
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
bool PCollision::towardsContact(vec3f body, vec3f contact, vec3f diff) const
{
  vec3f dist1 = body - contact;
  vec3f dist2 = body + diff - contact;
  return (dist1.length() > dist2.length());
}

///
/// @brief Calculates 2D minimum position of AABB box
/// @param a = vector to compare to minimum
///
void PCollision::calcmin(const vec3f &a)
{
  boxmin.x = MIN(boxmin.x, a.x);
  boxmin.y = MIN(boxmin.y, a.y);
}

///
/// @brief Calculates 2D maximum position of AABB box
/// @param a = vector to compare to maximum
///
void PCollision::calcmax(const vec3f &a)
{
  boxmax.x = MAX(boxmax.x, a.x);
  boxmax.y = MAX(boxmax.y, a.y);
}
