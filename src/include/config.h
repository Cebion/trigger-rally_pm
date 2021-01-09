//
// Copyright (C) 2021 Martin Scherer
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

#pragma once

#include "codriver.h"
#include <list>
#include <string>

class MainApp;

///
/// @brief Loading and storing of configuration settings
///
class PConfig {
public:
  /// Unit of vehicle speed
  enum Speedunit {
    mph,        ///< Miles per hour
    kph         ///< Kilometers per hour
  };
  /// Snow flake style
  enum SnowFlakeType {
    point,      ///< Point shape
    square,     ///< Square shape
    textured    ///< Textured snow flake
  };

  struct UserControl {
    enum {
      TypeUnassigned,
      TypeKey,
      TypeJoyButton,
      TypeJoyAxis
    } type;
    union {
      struct {
        SDL_Keycode sym;
      } key;
      struct {
        int button;
      } joybutton;
      struct {
        int axis;
        float sign;
        float deadzone;
        float maxrange;
      } joyaxis; // more like half-axis, really
    };
    // from 0.0 to 1.0 depending on activation level
    float value;
  };

  enum Action {
    ActionForward,
    ActionBack,
    ActionLeft,
    ActionRight,
    ActionHandbrake,
    ActionRecover,
    ActionRecoverAtCheckpoint,
    ActionCamMode,
    ActionCamLeft,
    ActionCamRight,
    ActionShowMap,
    ActionShowUi,
    ActionShowCheckpoint,
    ActionPauseRace,
    ActionNext,
    ActionCount
  };

  struct Control {
    std::string action_name[ActionCount];
    UserControl map[ActionCount];
  };

  PConfig(MainApp *app);
  void loadConfig();
  void storeConfig();

  bool getFoliage() const;
  bool getRoadsigns() const;
  bool getWeather() const;
  int getVideoCx() const;
  int getVideoCy() const;
  bool getVideoFullscreen() const;
  const std::list<std::string> &getDatadirs() const;
  bool getCopydefplayers() const;
  const std::string &getPlayername() const;
  struct Control &getCtrl();
  float getVolumeCodriver() const;
  const PCodriverUserConfig &getCodriveruserconfig() const;
  bool getEnableCodriversigns() const;
  const std::string &getCodriversigns() const;
  bool getEnableSound() const;
  const std::string &getCodrivername() const;
  bool getDirteffect() const;
  bool getEnableGhost() const;
  float getDrivingassist() const;
  float getVolumeEngine() const;
  float getVolumeSfx() const;
  SnowFlakeType getSnowflaketype() const;
  bool getEnableFps() const;
  float getHudSpeedoMpsSpeedMult() const;
  Speedunit getSpeedUnit() const;
  GLfloat getAnisotropy() const;

  void setFoliage(bool foliage);
  void setRoadsigns(bool roadsigns);
  void setWeather(bool weather);
  void setVolumeCodriver(float volumeCodriver);
  void setEnableCodriversigns(bool enableCodriversigns);
  void setCodriversigns(const std::string &codriversigns);
  void setCodrivername(const std::string &codrivername);
  void setDirteffect(bool dirteffect);
  void setEnableGhost(bool enableGhost);
  void setVolumeEngine(float volumeEngine);
  void setVolumeSfx(float volumeSfx);
  void setSnowflaketype(SnowFlakeType snowflaketype);
  void setEnableFps(bool enableFps);
  void setSpeedUnit(Speedunit speedUnit);
  void setAnisotropy(GLfloat anisotropy);

private:
  PConfig();
  PConfig(const PConfig&);
  PConfig& operator=(const PConfig&);

  MainApp *mainapp;         ///< Used to some change settings in load function

  XMLDocument xmlfile;      ///< In-memory copy of config file
  XMLElement *rootelem;     ///< Root element of XML config file
  std::string cfgfilename;  ///< Path to XML configuration file

  // Config settings
  std::string cfg_playername;
  bool cfg_copydefplayers;

  int cfg_video_cx, cfg_video_cy;
  bool cfg_video_fullscreen;

  float cfg_drivingassist;
  bool cfg_enable_sound;
  bool cfg_enable_codriversigns;
  bool cfg_enable_fps;
  bool cfg_enable_ghost;

  long int cfg_skip_saves;

  /// Basic volume control.
  float cfg_volume_engine       = 0.33f;    ///< Engine.
  float cfg_volume_sfx          = 1.00f;    ///< Sound effects (wind, gear change, crash, skid, etc.)
  float cfg_volume_codriver     = 1.00f;    ///< Codriver voice.

  /// Search paths for the data directory, as read from the configuration.
  std::list<std::string> cfg_datadirs;

  /// Name of the codriver whose words to load.
  /// Must be a valid directory in /data/sounds/codriver/.
  std::string cfg_codrivername;

  /// Name of the codriver icon set to load.
  /// Must be a valid directory in /data/textures/CodriverSigns/.
  std::string cfg_codriversigns;

  /// User settings for codriver signs: position, scale, fade start time.
  PCodriverUserConfig cfg_codriveruserconfig;

  Speedunit cfg_speed_unit;
  float hud_speedo_start_deg;
  float hud_speedo_mps_deg_mult;
  float hud_speedo_mps_speed_mult;

  SnowFlakeType cfg_snowflaketype = SnowFlakeType::point;

  bool cfg_dirteffect = true;

  GLfloat cfg_anisotropy = 1.0f;    ///< Anisotropic filter quality.
  bool cfg_foliage = true;          ///< Foliage on/off flag.
  bool cfg_roadsigns = true;        ///< Road signs on/off flag.
  bool cfg_weather = true;          ///< Weather on/off flag.

  struct Control ctrl;
};
