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

#include "config.h"
#include "exception.h"
#include "main.h"
#include "pengine.h"
#include "vehicle.h"

///
/// @brief Constructor without functionality
/// @param [in] app Used by load function to change a few settings
///
PConfig::PConfig(MainApp *app) :
    mainapp(app),
    rootelem(NULL),
    cfgfilename("")
{
}

///
/// @brief Load configurations from files
/// @todo Since C++11 introduced default members initializers, the defaults could
///  be set in the class declaration directly rather than in this function.
///
void PConfig::loadConfig()
{
  PUtil::outLog() << "Loading game configuration" << std::endl;

  // Set defaults
  cfg_playername = "Player";
  cfg_copydefplayers = true;

  cfg_video_cx = 640;
  cfg_video_cy = 480;
  cfg_video_fullscreen = false;

  cfg_drivingassist = 1.0f;
  cfg_enable_sound = true;
  cfg_enable_codriversigns = true;
  cfg_skip_saves = 5;
  cfg_volume_engine = 0.33f;
  cfg_volume_sfx = 1.0f;
  cfg_volume_codriver = 1.0f;
  cfg_anisotropy = 1.0f;
  cfg_foliage = true;
  cfg_roadsigns = true;
  cfg_weather = true;
  cfg_speed_unit = mph;
  cfg_snowflaketype = SnowFlakeType::point;
  cfg_dirteffect = true;
  cfg_enable_fps = false;
  cfg_enable_ghost = false;

  cfg_datadirs.clear();

  hud_speedo_start_deg = MPH_ZERO_DEG;
  hud_speedo_mps_deg_mult = MPS_MPH_DEG_MULT;
  hud_speedo_mps_speed_mult = MPS_MPH_SPEED_MULT;

  ctrl.action_name[ActionForward] = std::string("forward");
  ctrl.action_name[ActionBack] = std::string("back");
  ctrl.action_name[ActionLeft] = std::string("left");
  ctrl.action_name[ActionRight] = std::string("right");
  ctrl.action_name[ActionHandbrake] = std::string("handbrake");
  ctrl.action_name[ActionRecover] = std::string("recover");
  ctrl.action_name[ActionRecoverAtCheckpoint] = std::string("recoveratcheckpoint");
  ctrl.action_name[ActionCamMode] = std::string("cammode");
  ctrl.action_name[ActionCamLeft] = std::string("camleft");
  ctrl.action_name[ActionCamRight] = std::string("camright");
  ctrl.action_name[ActionShowMap] = std::string("showmap");
  ctrl.action_name[ActionPauseRace] = std::string("pauserace");
  ctrl.action_name[ActionShowUi] = std::string("showui");
  ctrl.action_name[ActionShowCheckpoint] = std::string("showcheckpoint");
  ctrl.action_name[ActionNext] = std::string("next");

  for (int i = 0; i < ActionCount; i++) {
    ctrl.map[i].type = UserControl::TypeUnassigned;
    ctrl.map[i].value = 0.0f;
  }

  // Do config file management
  cfgfilename = "trigger-rally-" PACKAGE_VERSION ".config";

  if (!PHYSFS_exists(cfgfilename.c_str())) {
#ifdef UNIX
    const std::vector<std::string> cfghidingplaces {
        "/usr/share/games/trigger-rally/"
    };

    for (const std::string &cfgpath: cfghidingplaces)
        if (PHYSFS_mount(cfgpath.c_str(), NULL, 1) == 0)
        {
            PUtil::outLog() << "Failed to add PhysFS search directory \"" <<
                cfgpath << "\"\nPhysFS: " << physfs_getErrorString() << std::endl;
        }
#endif
    PUtil::outLog() << "No user config file, copying over defaults" << std::endl;

    std::string cfgdefaults = "trigger-rally.config.defs";

    if (!PUtil::copyFile(cfgdefaults, cfgfilename)) {

      PUtil::outLog() << "Couldn't create user config file. Proceeding with defaults." << std::endl;

      cfgfilename = cfgdefaults;
    }
  }

  // Load actual settings from file
  rootelem = PUtil::loadRootElement(xmlfile, cfgfilename, "config");
  if (!rootelem) {
    PUtil::outLog() << "Error: Couldn't load configuration file" << std::endl;
#if TINYXML2_MAJOR_VERSION >= 6
    PUtil::outLog() << "TinyXML: " << xmlfile.ErrorStr() << std::endl;
#else
    PUtil::outLog() << "TinyXML: " << xmlfile.GetErrorStr1() << ' ' << xmlfile.GetErrorStr2() << std::endl;
#endif
    PUtil::outLog() << "Your data paths are probably not set up correctly" << std::endl;
    throw MakePException ("Boink");
  }

  const char *val;

  for (XMLElement *walk = rootelem->FirstChildElement();
    walk; walk = walk->NextSiblingElement()) {

    if (strcmp(walk->Value(), "player") == 0)
    {
        val = walk->Attribute("name");

        if (val != nullptr)
        {
            cfg_playername = val;
            mainapp->best_times.setPlayerName(val);
        }

        val = walk->Attribute("copydefplayers");

        if (val != nullptr && std::string(val) == "no")
            cfg_copydefplayers = false;
        else
            cfg_copydefplayers = true;

        val = walk->Attribute("skipsaves");

        if (val != nullptr)
            cfg_skip_saves = std::stol(val);

        mainapp->best_times.setSkipSaves(cfg_skip_saves);
    }
    else
    if (!strcmp(walk->Value(), "video")) {

        val = walk->Attribute("automatic");

        if (val != nullptr && std::string(val) == "yes")
          mainapp->automaticVideoMode(true);
        else
          mainapp->automaticVideoMode(false);

      val = walk->Attribute("width");
      if (val) cfg_video_cx = atoi(val);

      val = walk->Attribute("height");
      if (val) cfg_video_cy = atoi(val);

      val = walk->Attribute("fullscreen");
      if (val) {
        if (!strcmp(val, "yes"))
          cfg_video_fullscreen = true;
        else if (!strcmp(val, "no"))
          cfg_video_fullscreen = false;
      }

      val = walk->Attribute("requirergb");
      if (val) {
        if (!strcmp(val, "yes"))
          mainapp->requireRGB(true);
        else if (!strcmp(val, "no"))
          mainapp->requireRGB(false);
      }

      val = walk->Attribute("requirealpha");
      if (val) {
        if (!strcmp(val, "yes"))
          mainapp->requireAlpha(true);
        else if (!strcmp(val, "no"))
          mainapp->requireAlpha(false);
      }

      val = walk->Attribute("requiredepth");
      if (val) {
        if (!strcmp(val, "yes"))
          mainapp->requireDepth(true);
        else if (!strcmp(val, "no"))
          mainapp->requireDepth(false);
      }

      val = walk->Attribute("requirestencil");
      if (val) {
        if (!strcmp(val, "yes"))
          mainapp->requireStencil(true);
        else if (!strcmp(val, "no"))
          mainapp->requireStencil(false);
      }

      val = walk->Attribute("stereo");
      if (val) {
        if (!strcmp(val, "none"))
          mainapp->setStereoMode(PApp::StereoNone);
        else if (!strcmp(val, "quadbuffer"))
          mainapp->setStereoMode(PApp::StereoQuadBuffer);
        else if (!strcmp(val, "red-blue"))
          mainapp->setStereoMode(PApp::StereoRedBlue);
        else if (!strcmp(val, "red-green"))
          mainapp->setStereoMode(PApp::StereoRedGreen);
        else if (!strcmp(val, "red-cyan"))
          mainapp->setStereoMode(PApp::StereoRedCyan);
        else if (!strcmp(val, "yellow-blue"))
          mainapp->setStereoMode(PApp::StereoYellowBlue);
      }

      float sepMult = 1.0f;
      val = walk->Attribute("stereoswapeyes");
      if (val && !strcmp(val, "yes"))
        sepMult = -1.0f;

      val = walk->Attribute("stereoeyeseparation");
      if (val) {
        mainapp->setStereoEyeSeperation(atof(val) * sepMult);
      }
    }
    else
    if (!strcmp(walk->Value(), "audio"))
    {
        val = walk->Attribute("enginevolume");

        if (val != nullptr)
            cfg_volume_engine = atof(val);

        val = walk->Attribute("sfxvolume");

        if (val != nullptr)
            cfg_volume_sfx = atof(val);

        val = walk->Attribute("codrivervolume");

        if (val != nullptr)
            cfg_volume_codriver = atof(val);
    }
    else
    if (!strcmp(walk->Value(), "graphics"))
    {
        val = walk->Attribute("anisotropy");

        if (val)
        {
            if (!strcmp(val, "off"))
            {
                cfg_anisotropy = 0.0f;
            }
            else
            if (!strcmp(val, "max"))
            {
                glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &cfg_anisotropy);
            }
            else // TODO: listen to the user, but don't trust him
            {
                cfg_anisotropy = atof(val);
                CLAMP_LOWER(cfg_anisotropy, 1.0f);
            }
        }

        val = walk->Attribute("foliage");

        if (val)
        {
            if (!strcmp(val, "no"))
                cfg_foliage = false;
            else // "yes"
                cfg_foliage = true;
        }

        val = walk->Attribute("roadsigns");

        if (val != nullptr)
        {
            if (strcmp(val, "no") == 0)
                cfg_roadsigns = false;
            else // yes
                cfg_roadsigns = true;
        }

        val = walk->Attribute("weather");

        if (val)
        {
            if (!strcmp(val, "no"))
                cfg_weather = false;
            else // "yes"
                cfg_weather = true;
        }

        val = walk->Attribute("snowflaketype");

        if (val)
        {
            if (!strcmp(val, "square"))
                cfg_snowflaketype = SnowFlakeType::square;
            else
            if (!strcmp(val, "textured"))
                cfg_snowflaketype = SnowFlakeType::textured;
            else // default
                cfg_snowflaketype = SnowFlakeType::point;
        }

        val = walk->Attribute("dirteffect");

        if (val)
        {
            if (!strcmp(val, "yes"))
                cfg_dirteffect = true;
            else
                cfg_dirteffect = false;
        }
    }
    else
    if (!strcmp(walk->Value(), "datadirectory"))
    {
        for (XMLElement *walk2 = walk->FirstChildElement(); walk2; walk2 = walk2->NextSiblingElement())
            if (!strcmp(walk2->Value(), "data"))
                cfg_datadirs.push_back(walk2->Attribute("path"));
    }
    else if (!strcmp(walk->Value(), "parameters")) {

      val = walk->Attribute("drivingassist");
      if (val) cfg_drivingassist = atof(val);

      val = walk->Attribute("enablesound");
      if (val) {
        if (!strcmp(val, "yes"))
          cfg_enable_sound = true;
        else if (!strcmp(val, "no"))
          cfg_enable_sound = false;
      }

        val = walk->Attribute("enablecodriversigns");

        if (val != nullptr)
        {
            if (strcmp(val, "yes") == 0)
                cfg_enable_codriversigns = true;
            else
            if (strcmp(val, "no") == 0)
                cfg_enable_codriversigns = false;
        }

      val = walk->Attribute("speedunit");
      if (val) {
        if (!strcmp(val, "mph")) {
            cfg_speed_unit = mph;
            hud_speedo_start_deg = MPH_ZERO_DEG;
            hud_speedo_mps_deg_mult = MPS_MPH_DEG_MULT;
            hud_speedo_mps_speed_mult = MPS_MPH_SPEED_MULT;
          }
        else if (!strcmp(val, "kph")) {
           cfg_speed_unit = kph;
           hud_speedo_start_deg = KPH_ZERO_DEG;
           hud_speedo_mps_deg_mult = MPS_KPH_DEG_MULT;
           hud_speedo_mps_speed_mult = MPS_KPH_SPEED_MULT;
         }
      }

      val = walk->Attribute("enablefps");
      if (val) {
        if (!strcmp(val, "yes"))
          cfg_enable_fps = true;
        else if (!strcmp(val, "no"))
          cfg_enable_fps = false;
      }

      val = walk->Attribute("enableghost");
      if (val) {
        if (!strcmp(val, "yes"))
          cfg_enable_ghost= true;
        else if (!strcmp(val, "no"))
          cfg_enable_ghost = false;
      }

      val = walk->Attribute("codriver");

      if (val != nullptr)
        cfg_codrivername = val;

      val = walk->Attribute("codriversigns");

        if (val != nullptr)
            cfg_codriversigns = val;

        val = walk->Attribute("codriversignslife");

        if (val != nullptr)
            cfg_codriveruserconfig.life = std::stof(val);

        val = walk->Attribute("codriversignsposx");

        if (val != nullptr)
            cfg_codriveruserconfig.posx = std::stof(val);

        val = walk->Attribute("codriversignsposy");

        if (val != nullptr)
            cfg_codriveruserconfig.posy = std::stof(val);

        val = walk->Attribute("codriversignsscale");

        if (val != nullptr)
            cfg_codriveruserconfig.scale = std::stof(val);

    } else if (!strcmp(walk->Value(), "controls")) {

      for (XMLElement *walk2 = walk->FirstChildElement();
        walk2; walk2 = walk2->NextSiblingElement()) {

        if (!strcmp(walk2->Value(), "keyboard")) {

          val = walk2->Attribute("enable");
          if (val && !strcmp(val, "no"))
            continue;

          for (XMLElement *walk3 = walk2->FirstChildElement();
            walk3; walk3 = walk3->NextSiblingElement()) {

            if (!strcmp(walk3->Value(), "key")) {

              val = walk3->Attribute("action");

              int a;
              for (a = 0; a < ActionCount; a++)
                if (ctrl.action_name[a] == val) break;

              if (a >= ActionCount) {
                PUtil::outLog() << "Config ctrls: Unknown action \"" << val << "\"" << std::endl;
                continue;
              }
              /*
              // TODO: implement string to keycode mapping
              val = walk3->Attribute("code");
              if (!val) {
                PUtil::outLog() << "Config ctrls: Key has no code" << std::endl;
                continue;
              }
              */

              val = walk3->Attribute("id");

              if (!val)
              {
                  PUtil::outLog() << "Config ctrls: Key has no ID" << std::endl;
                  continue;
              }

              ctrl.map[a].type = UserControl::TypeKey;
              //ctrl.map[a].key.sym = (SDLKey) atoi(val);
              ctrl.map[a].key.sym = SDL_GetKeyFromName(val);
            }
          }

        } else if (!strcmp(walk2->Value(), "joystick")) {

          val = walk2->Attribute("enable");
          if (val && !strcmp(val, "no"))
            continue;

          for (XMLElement *walk3 = walk2->FirstChildElement();
            walk3; walk3 = walk3->NextSiblingElement()) {

            if (!strcmp(walk3->Value(), "button")) {

              val = walk3->Attribute("action");

              int a;
              for (a = 0; a < ActionCount; a++)
                if (ctrl.action_name[a] == val) break;

              if (a >= ActionCount) {
                PUtil::outLog() << "Config ctrls: Unknown action \"" << val << "\"" << std::endl;
                continue;
              }

              val = walk3->Attribute("index");
              if (!val) {
                PUtil::outLog() << "Config ctrls: Joy button has no index" << std::endl;
                continue;
              }

              ctrl.map[a].type = UserControl::TypeJoyButton;
              ctrl.map[a].joybutton.button = atoi(val);

            } else if (!strcmp(walk3->Value(), "axis")) {

              val = walk3->Attribute("action");

              int a;
              for (a = 0; a < ActionCount; a++)
                if (ctrl.action_name[a] == val) break;

              if (a >= ActionCount) {
                PUtil::outLog() << "Config ctrls: Unknown action \"" << val << "\"" << std::endl;
                continue;
              }

              val = walk3->Attribute("index");
              if (!val) {
                PUtil::outLog() << "Config ctrls: Joy axis has no index" << std::endl;
                continue;
              }

              int index = atoi(val);

              bool positive;

              val = walk3->Attribute("direction");
              if (!val) {
                PUtil::outLog() << "Config ctrls: Joy axis has no direction" << std::endl;
                continue;
              }
              if (!strcmp(val, "+"))
                positive = true;
              else if (!strcmp(val, "-"))
                positive = false;
              else {
                PUtil::outLog() << "Config ctrls: Joy axis direction \"" << val <<
                  "\" is neither \"+\" nor \"-\"" << std::endl;
                continue;
              }

              ctrl.map[a].type = UserControl::TypeJoyAxis;
              ctrl.map[a].joyaxis.axis = index;
              ctrl.map[a].joyaxis.sign = positive ? 1.0f : -1.0f;
              ctrl.map[a].joyaxis.deadzone = 0.0f;
              ctrl.map[a].joyaxis.maxrange = 1.0f;

              val = walk3->Attribute("deadzone");
              if (val) ctrl.map[a].joyaxis.deadzone = atof(val);

              val = walk3->Attribute("maxrange");
              if (val) ctrl.map[a].joyaxis.maxrange = atof(val);
            }
          }
        }
      }
    }
  }
}

///
/// @brief Store configuration to XML file
///
void PConfig::storeConfig()
{
  XMLPrinter printer;
  PHYSFS_file *pfile = PHYSFS_openWrite(cfgfilename.c_str());

  if (pfile == nullptr) {
    PUtil::outLog() << "Load failed: PhysFS: " << physfs_getErrorString() << std::endl;
    return;
  }

  PUtil::outLog() << "Storing game configuration" << std::endl;

  for (XMLElement *walk = rootelem->FirstChildElement(); walk; walk = walk->NextSiblingElement()) {
    if (!strcmp(walk->Value(), "audio")) {
      walk->SetAttribute("enginevolume", cfg_volume_engine);
      walk->SetAttribute("sfxvolume", cfg_volume_sfx);
      walk->SetAttribute("codrivervolume", cfg_volume_codriver);
    }
    else if (!strcmp(walk->Value(), "graphics")) {
      if (cfg_anisotropy == 0.0f)
        walk->SetAttribute("anisotropy", "off");
      else
        walk->SetAttribute("anisotropy", std::to_string((int)cfg_anisotropy).c_str());

      if (cfg_foliage)
        walk->SetAttribute("foliage", "yes");
      else
        walk->SetAttribute("foliage", "no");

      if (cfg_roadsigns)
        walk->SetAttribute("roadsigns", "yes");
      else
        walk->SetAttribute("roadsigns", "no");

      if (cfg_weather)
        walk->SetAttribute("weather", "yes");
      else
        walk->SetAttribute("weather", "no");

      switch(cfg_snowflaketype) {
      case SnowFlakeType::point:
        walk->SetAttribute("snowflaketype", "point");
        break;
      case SnowFlakeType::square:
        walk->SetAttribute("snowflaketype", "quare");
        break;
      case SnowFlakeType::textured:
        walk->SetAttribute("snowflaketype", "textured");
        break;
      default:
        break;
      }

      if (cfg_dirteffect)
        walk->SetAttribute("dirteffect", "yes");
      else
        walk->SetAttribute("dirteffect", "no");
    }
    else if (!strcmp(walk->Value(), "parameters")) {
      if (cfg_enable_sound)
        walk->SetAttribute("enablesound", "yes");
      else
        walk->SetAttribute("enablesound", "no");

      if (cfg_enable_codriversigns)
        walk->SetAttribute("enablecodriversigns", "yes");
      else
        walk->SetAttribute("enablecodriversigns", "no");

      if (cfg_speed_unit == mph)
        walk->SetAttribute("speedunit", "mph");
      else
        walk->SetAttribute("speedunit", "kph");

      if (cfg_enable_fps)
        walk->SetAttribute("enablefps", "yes");
      else
        walk->SetAttribute("enablefps", "no");

      if (cfg_enable_ghost)
        walk->SetAttribute("enableghost", "yes");
      else
        walk->SetAttribute("enableghost", "no");

      walk->SetAttribute("codriver", cfg_codrivername.c_str());

      walk->SetAttribute("codriversigns", cfg_codriversigns.c_str());
    }
    else if (!strcmp(walk->Value(), "controls")) {
      for (XMLElement *walk2 = walk->FirstChildElement(); walk2; walk2 = walk2->NextSiblingElement()) {
        if (!strcmp(walk2->Value(), "keyboard")) {
          for (XMLElement *walk3 = walk2->FirstChildElement(); walk3; walk3 = walk3->NextSiblingElement()) {
            if (!strcmp(walk3->Value(), "key")) {
              const char *val = walk3->Attribute("action");
              unsigned int i = 0;

              while (i < ActionCount) {
                if (ctrl.action_name[i] == val && ctrl.map[i].type == UserControl::TypeKey) {
                  walk3->SetAttribute("id", SDL_GetKeyName(ctrl.map[i].key.sym));
                  break;
                }
                ++i;
              }

              if (i >= ActionCount) {
                PUtil::outLog() << "Config ctrls: Unknown action \"" << val << "\"" << std::endl;
                continue;
              }
            }
          }
        }
      }
    }
  }

  xmlfile.Print(&printer);
  physfs_write(pfile, printer.CStr(), sizeof(char), printer.CStrSize() - 1);
  PHYSFS_close(pfile);
}

bool PConfig::getFoliage() const
{
  return cfg_foliage;
}

bool PConfig::getRoadsigns() const
{
  return cfg_roadsigns;
}

bool PConfig::getWeather() const
{
  return cfg_weather;
}

int PConfig::getVideoCx() const
{
  return cfg_video_cx;
}

int PConfig::getVideoCy() const
{
  return cfg_video_cy;
}

bool PConfig::getVideoFullscreen() const
{
  return cfg_video_fullscreen;
}

const std::list<std::string> &PConfig::getDatadirs() const
{
  return cfg_datadirs;
}

bool PConfig::getCopydefplayers() const
{
  return cfg_copydefplayers;
}

const std::string &PConfig::getPlayername() const
{
  return cfg_playername;
}

struct PConfig::Control &PConfig::getCtrl()
{
  return ctrl;
}

float PConfig::getVolumeCodriver() const
{
  return cfg_volume_codriver;
}

const PCodriverUserConfig &PConfig::getCodriveruserconfig() const
{
  return cfg_codriveruserconfig;
}

bool PConfig::getEnableCodriversigns() const
{
  return cfg_enable_codriversigns;
}

const std::string &PConfig::getCodriversigns() const
{
  return cfg_codriversigns;
}

bool PConfig::getEnableSound() const
{
  return cfg_enable_sound;
}

const std::string &PConfig::getCodrivername() const
{
  return cfg_codrivername;
}

bool PConfig::getDirteffect() const
{
  return cfg_dirteffect;
}

bool PConfig::getEnableGhost() const
{
  return cfg_enable_ghost;
}

float PConfig::getDrivingassist() const
{
  return cfg_drivingassist;
}

float PConfig::getVolumeEngine() const
{
  return cfg_volume_engine;
}

float PConfig::getVolumeSfx() const
{
  return cfg_volume_sfx;
}

PConfig::SnowFlakeType PConfig::getSnowflaketype() const
{
  return cfg_snowflaketype;
}

bool PConfig::getEnableFps() const
{
  return cfg_enable_fps;
}

float PConfig::getHudSpeedoMpsSpeedMult() const
{
  return hud_speedo_mps_speed_mult;
}

PConfig::Speedunit PConfig::getSpeedUnit() const
{
  return cfg_speed_unit;
}

GLfloat PConfig::getAnisotropy() const
{
  return cfg_anisotropy;
}

void PConfig::setFoliage(bool foliage)
{
  cfg_foliage = foliage;
}

void PConfig::setRoadsigns(bool roadsigns)
{
  cfg_roadsigns = roadsigns;
}

void PConfig::setWeather(bool weather)
{
  cfg_weather = weather;
}

void PConfig::setVolumeCodriver(float volumeCodriver)
{
  cfg_volume_codriver = volumeCodriver;
}

void PConfig::setEnableCodriversigns(bool enableCodriversigns)
{
  cfg_enable_codriversigns = enableCodriversigns;
}

void PConfig::setCodriversigns(const std::string &codriversigns)
{
  cfg_codriversigns = codriversigns;
}

void PConfig::setCodrivername(const std::string &codrivername)
{
  cfg_codrivername = codrivername;
}

void PConfig::setDirteffect(bool dirteffect)
{
  cfg_dirteffect = dirteffect;
}

void PConfig::setEnableGhost(bool enableGhost)
{
  cfg_enable_ghost = enableGhost;
}

void PConfig::setVolumeEngine(float volumeEngine)
{
  cfg_volume_engine = volumeEngine;
}

void PConfig::setVolumeSfx(float volumeSfx)
{
  cfg_volume_sfx = volumeSfx;
}

void PConfig::setSnowflaketype(SnowFlakeType snowflaketype)
{
  cfg_snowflaketype = snowflaketype;
}
void PConfig::setEnableFps(bool enableFps)
{
  cfg_enable_fps = enableFps;
}

void PConfig::setSpeedUnit(Speedunit speedUnit)
{
  cfg_speed_unit = speedUnit;
}

void PConfig::setAnisotropy(GLfloat anisotropy)
{
  cfg_anisotropy = anisotropy;
}
