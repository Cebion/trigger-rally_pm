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

#ifndef GHOSTH
#define GHOSTH

#include <string>
#include <vector>
#include "vmath.h"

class PSSModel;
struct PVehiclePart;

///
/// @brief Recording and play back of ghost vehicles
///
class PGhost {
public:
  ///
  /// @brief Wheels of the ghost
  ///
  struct GhostWheel {
    // Position in world coordinates
    vec3f pos;
    // Orientation of the wheel
    quatf ori;
  };

  ///
  /// @brief State of the ghost vehicle
  ///
  struct GhostData {
    // Time stamp in seconds in game time
    float time;
    // Position in world coordinates
    vec3f pos;
    // Orientation of the vehicle
    quatf ori;
    // Status of the wheels
    std::vector<GhostWheel> wheel;
  };

  PGhost(float sampletime);
  void recordStart(const std::string &map, const std::string &vehicle);
  void recordSample(float delta, const PVehiclePart &part);
  void recordStop(float time);
  bool getReplayData(GhostData &data, std::string &vehicle) const;

private:
  PGhost();
  PGhost(const PGhost&);
  PGhost& operator=(const PGhost&);

  // Map related to the ghost car
  std::string mapname;
  // Name of the vehicle for recording
  std::string vehiclename;
  // Minimum time after which a sample is taken
  float sampletime;
  // To check if time since last has passed
  float lastsample;
  // Recorded data samples
  std::vector<GhostData> recordeddata;
  // Replay data samples
  std::vector<GhostData> replaydata;
  // Accumulated race time
  float racetime;
  // Ghost vehicle for replay
  std::string replayvehicle;
  // Total race time of replay vehicle
  float replaytime;
};

#endif
