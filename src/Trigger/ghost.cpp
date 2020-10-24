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

#include <limits>
#include <physfs.h>
#include <ostream>
#include "ghost.h"
#include "pengine.h"
#include "physfs_utils.h"
#include "psim.h"
#include "vehicle.h"

///
/// @brief Writes a GhostWheel object to an output stream.
/// @param [in,out] os  Output stream
/// @param [in] gw      Ghost wheel to be written
/// @returns The output stream
///
inline std::ostream & operator << (std::ostream &os, const PGhost::GhostWheel &gw)
{
  os << ',' << gw.pos.x << ',';
  os << gw.pos.y << ',';
  os << gw.pos.z << ',';
  os << gw.ori.x << ',';
  os << gw.ori.y << ',';
  os << gw.ori.z << ',';
  os << gw.ori.w;
  return os;
}

///
/// @brief Reads a GhostData object from an input stream.
/// @param [in,out] is  Input stream.
/// @param [out] gd     Ghost data to be read
/// @returns The input stream
///
inline std::istream & operator >> (std::istream &is, PGhost::GhostData &gd)
{
  std::string line;
  std::string cell;
  std::vector<std::string> result;
  std::istringstream iss;

  std::getline(is, line);
  if (line.empty())
    return is;
  iss = std::istringstream(line);
  while (std::getline(iss, cell, ','))
    result.push_back(cell);
  try {
    gd.time = std::stof(result[0]);
    gd.pos.x = std::stof(result[1]);
    gd.pos.y = std::stof(result[2]);
    gd.pos.z = std::stof(result[3]);
    gd.ori.x = std::stof(result[4]);
    gd.ori.y = std::stof(result[5]);
    gd.ori.z = std::stof(result[6]);
    gd.ori.w = std::stof(result[7]);

    gd.wheel.clear();
    for (unsigned int i = 8; i < result.size(); i = i + 7) {
      PGhost::GhostWheel gw;
      gw.pos.x = std::stof(result[i]);
      gw.pos.y = std::stof(result[i + 1]);
      gw.pos.z = std::stof(result[i + 2]);
      gw.ori.x = std::stof(result[i + 3]);
      gw.ori.y = std::stof(result[i + 4]);
      gw.ori.z = std::stof(result[i + 5]);
      gw.ori.w = std::stof(result[i + 6]);
      gd.wheel.push_back(gw);
    }
  }
  catch (...) {
    PUtil::outLog() << "Invalid data format in *.ghost file." << std::endl;
  }
  return is;
}

///
/// @brief Writes a GhostData object to an output stream.
/// @param [in,out] os  Output stream
/// @param [in] gd      Ghost data to be written
/// @returns The output stream
///
inline std::ostream & operator << (std::ostream &os, const PGhost::GhostData &gd)
{
  os << gd.time << ',';
  os << gd.pos.x << ',';
  os << gd.pos.y << ',';
  os << gd.pos.z << ',';
  os << gd.ori.x << ',';
  os << gd.ori.y << ',';
  os << gd.ori.z << ',';
  os << gd.ori.w;
  for (unsigned int i = 0; i < gd.wheel.size(); ++i) {
    os << gd.wheel[i];
  }
  os << std::endl;
  return os;
}

///
/// @brief Constructs a ghost object.
/// @param [in] sampletime  Time in seconds after which ghost is sampled
///
PGhost::PGhost(float sampletime) :
    mapname(""),
    vehiclename(""),
    sampletime(sampletime),
    lastsample(std::numeric_limits<float>::lowest()),
    recordeddata(std::vector<GhostData>()),
    replaydata(std::vector<GhostData>()),
    racetime(0.0f),
    replayvehicle(""),
    replaytime(std::numeric_limits<float>::max())
{
}

///
/// @brief Start recording the next race for later replay
/// @param [in] map     Name of the map to store ghost data
/// @param [in] vehicle Name of the vehicle on the race
///
void PGhost::recordStart(const std::string &map, const std::string &vehicle)
{
  PHYSFS_File *physfile = nullptr;
  std::string readdata = "";
  std::istringstream inputstream;
  GhostData data;
  std::string replaytimestring;

  mapname = map + ".ghost";
  replace(mapname.begin(), mapname.end(), '/', '_');

  vehiclename = vehicle;
  lastsample = std::numeric_limits<float>::lowest();
  recordeddata.clear();
  replaydata.clear();
  racetime = 0.0f;
  replayvehicle = "";
  replaytime = std::numeric_limits<float>::max();

  if (PHYSFS_isInit() == 0)
    return;

  physfile = PHYSFS_openRead(mapname.c_str());
  if (physfile == nullptr)
    return;

  readdata = std::string(PHYSFS_fileLength(physfile), '\0');
  physfs_read(physfile, &readdata.front(), sizeof(char), readdata.size());

  inputstream = std::istringstream(readdata);
  std::getline(inputstream, replayvehicle, ',');
  std::getline(inputstream, replaytimestring);
  try {
    replaytime = std::stof(replaytimestring);

    while (inputstream >> data)
      replaydata.push_back(data);
    PHYSFS_close(physfile);
  }
  catch (...) {
    PUtil::outLog() << "Invalid data format in *.ghost file." << std::endl;
  }
}

///
/// @brief Samples the state of the vehicle if sample time is passed.
/// @param [in] delta   Duration of the time slice
/// @param [in] part    Vehicle data
///
void PGhost::recordSample(float delta, const PVehiclePart &part)
{
  bool firstsample = (racetime == 0.0f);

  racetime += delta;
  if (firstsample || racetime >= lastsample + sampletime) {
    GhostData data;

    lastsample = racetime;
    data.time = racetime;
    data.pos = part.ref_world.pos;
    data.ori = part.ref_world.ori;
    for (unsigned int i = 0; i < part.wheel.size(); ++i) {
      GhostWheel wheel;

      wheel.pos = part.wheel[i].ref_world.pos;
      wheel.ori = part.wheel[i].ref_world.ori;
      data.wheel.push_back(wheel);
    }
    recordeddata.push_back(data);
  }
}

///
/// @brief Store ghost at end of the race if best time achieved.
/// @param [in] time    Time achieved during race in seconds
///
void PGhost::recordStop(float time)
{
  if (time <= replaytime) {
    PHYSFS_File *physfile = nullptr;
    std::ostringstream outputstream;

    if (mapname.empty())
      return;
    if (PHYSFS_isInit() == 0)
      return;

    physfile = PHYSFS_openWrite(mapname.c_str());
    if (physfile == nullptr)
      return;

    outputstream << vehiclename << ',' << time << std::endl;
    for (const GhostData &data: recordeddata)
      outputstream << data;

    physfs_write(physfile, outputstream.str().data(), sizeof(char), outputstream.str().size());
    PHYSFS_close(physfile);
  }
}

///
/// @brief Get ghost state based on current race time.
/// @param [out] data       Ghost position and orientation
/// @param [out] vehicle    Name of the ghost vehicle
/// @returns Data returned in output arguments is valid
///
bool PGhost::getReplayData(GhostData &data, std::string &vehicle) const
{
  vehicle = replayvehicle;

  for(unsigned int i = 0; i < replaydata.size(); ++i) {
    if (i == replaydata.size()-1 && racetime >= replaydata[i].time) {
      data = replaydata[i];
      return true;
    }
    else if (racetime >= replaydata[i].time && racetime < replaydata[i+1].time) {
      float interptime = 0.0f;

      if (replaydata[i+1].time - replaydata[i].time != 0.0f)
        interptime = (racetime - replaydata[i].time) / (replaydata[i+1].time - replaydata[i].time);

      data.time = racetime;
      data.pos = INTERP(replaydata[i].pos, replaydata[i+1].pos, interptime);
      data.ori = INTERP(replaydata[i].ori, replaydata[i+1].ori, interptime);

      for (unsigned int j = 0; j < replaydata[i].wheel.size(); ++j) {
        GhostWheel wheel;

        wheel.pos = INTERP(replaydata[i].wheel[j].pos, replaydata[i+1].wheel[j].pos, interptime);
        wheel.ori = INTERP(replaydata[i].wheel[j].ori, replaydata[i+1].wheel[j].ori, interptime);
        data.wheel.push_back(wheel);
      }
      return true;
    }
  }
  return false;
}
