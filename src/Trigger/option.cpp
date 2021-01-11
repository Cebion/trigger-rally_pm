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
#include "menu.h"
#include "option.h"
#include "render.h"
#include <algorithm>

///
/// @brief Constructor initializing data of options displayed in menu
/// @param[in] parent   Game menu used to add labels to
/// @param[in] config   Configuration data of game from XML file
///
POption::POption(Gui &parent, PConfig &config) :
    parent(parent),
    cfg(config),
    pos(0)
{
  //options.push_back({ OptionPlayerName, "Player name",
  //  { "_A-z 0-9" }, 0 });
  options.push_back({ OptionEngineVolume, "Engine volume",
    { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10" }, 0 });
  options.push_back({ OptionSfxVolume, "SFX volume",
    { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10" }, 0 });
  options.push_back({ OptionCodriverVolume, "Codriver volume",
    { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10" }, 0 });
  options.push_back({ OptionCodriverVoice, "Codriver voice",
    { "off", "ab", "paula", "tim" }, 0 });
  options.push_back({ OptionCodriverSigns, "Codriver signs",
    { "off", "abon", "glossy", "plain", "white" }, 0 });
  options.push_back({ OptionTextureQuality, "Texture quality",
    { "off", "1", "2", "4", "8", "16", "32" }, 0 });
  options.push_back({ OptionSnowflakes, "Snowflakes",
    { "off", "point", "square", "textured" }, 0 });
  options.push_back({ OptionSpeedUnits, "Speed units",
    { "kmh", "mph" }, 0 });
  options.push_back({ OptionGhostCars, "Ghost cars",
    { "off", "on" }, 0 });
  options.push_back({ OptionDisplayFps, "Display FPS",
    { "off", "on" }, 0 });
  options.push_back({ OptionFoliage, "Foliage",
    { "off", "on" }, 0 });
  options.push_back({ OptionRoadsigns, "Roadsigns",
    { "off", "on" }, 0 });
  options.push_back({ OptionDirtEffect, "Dirt effect",
    { "off", "on" }, 0 });
}

///
/// @brief Renders option menu
///
void POption::render()
{
  pos = 0;

  parent.makeClickable(
    parent.addLabel(10.0f, 10.0f, "back", PTEXT_HZA_LEFT | PTEXT_VTA_BOTTOM, 40.0f),
    AA_RELOAD_ALL, 0);
  parent.addLabel(80.0f, 520.0f, "Options", PTEXT_HZA_LEFT | PTEXT_VTA_CENTER, 30.0f, LabelStyle::Header);

  for (Option option : options) {
    addOption(option);
  }
}

///
/// @brief Called when an option is clicked
/// @param[in] index    Lower word holds row, higher word holds index
///
void POption::select(int index)
{
  unsigned int row = index & 0xFFFF;
  unsigned int column = (index >> 16) & 0xFFFF;

  options[row].select = column;

  switch (options[row].id) {
  case OptionEngineVolume:
    cfg.setVolumeEngine(column * 0.1f);
    break;
  case OptionSfxVolume:
    cfg.setVolumeSfx(column * 0.1f);
    break;
  case OptionCodriverVolume:
    cfg.setVolumeCodriver(column * 0.1f);
    break;
  case OptionCodriverVoice:
    if (!column)
      cfg.setCodrivername("mime");
    else
      cfg.setCodrivername(options[row].values[column]);
    break;
  case OptionCodriverSigns:
    if (column) {
      cfg.setEnableCodriversigns(true);
      cfg.setCodriversigns(options[row].values[column]);
    }
    else {
      cfg.setEnableCodriversigns(false);
    }
    break;
  case OptionTextureQuality:
    if (column)
      cfg.setAnisotropy(pow(2, column - 1));
    else
      cfg.setAnisotropy(0.0f);
    break;
  case OptionSnowflakes:
    if (column) {
      cfg.setWeather(true);
      cfg.setSnowflaketype((PConfig::SnowFlakeType)(column - 1));
    }
    else
      cfg.setWeather(false);
    break;
  case OptionSpeedUnits:
    if (!column)
      cfg.setSpeedUnit(PConfig::Speedunit::kph);
    else
      cfg.setSpeedUnit(PConfig::Speedunit::mph);
    break;
  case OptionGhostCars:
    cfg.setEnableGhost(column);
    break;
  case OptionDisplayFps:
    cfg.setEnableFps(column);
    break;
  case OptionFoliage:
    cfg.setFoliage(column);
    break;
  case OptionRoadsigns:
    cfg.setRoadsigns(column);
    break;
  case OptionDirtEffect:
    cfg.setDirteffect(column);
    break;
  default:
    break;
  }
}

///
/// @brief Render a single option row
/// @param [in] option  Data of option item
///
void POption::addOption(Option &option)
{
  unsigned int cursor = 0;

  updateSelect(option);

  parent.addLabel(80.0f, 490.0f - (float)pos * 30.0f,
      option.text, PTEXT_HZA_LEFT | PTEXT_VTA_TOP, 22.0f, LabelStyle::Regular);

  for (unsigned int i = 0; i < option.values.size(); ++i) {
    bool select = (i == option.select);

    parent.makeSelectable(
        parent.addLabel(340.0f + (float)cursor * 44.0f / 3.0f, 490.0f - (float)pos * 30.0f,
            option.values[i], PTEXT_HZA_LEFT | PTEXT_VTA_TOP, 22.0f, LabelStyle::Weak),
        AA_PICK_OPT, pos | (i << 16), select);
    cursor += option.values[i].length() + 1;
  }

  ++pos;
}

///
/// @brief Retrieves configuration data to select active option
/// @param [in] option  Data of option item
///
void POption::updateSelect(Option &option)
{
  switch(option.id) {
  case OptionEngineVolume:
    option.select = round(cfg.getVolumeEngine() * 10.0f);
    break;
  case OptionSfxVolume:
    option.select = round(cfg.getVolumeSfx() * 10.0f);
    break;
  case OptionCodriverVolume:
    option.select = round(cfg.getVolumeCodriver() * 10.0f);
    break;
  case OptionCodriverVoice:
    if (cfg.getCodrivername() == "mime")
      option.select = 0;
    else
      option.select = findStringPos(cfg.getCodrivername(), option.values);
    break;
  case OptionCodriverSigns:
    if (cfg.getEnableCodriversigns())
      option.select = findStringPos(cfg.getCodriversigns(), option.values);
    else
      option.select = 0;
    break;
  case OptionTextureQuality: {
    GLfloat anisotropy = cfg.getAnisotropy();
    if (anisotropy == 1.0f)
      option.select = 1;
    else if (anisotropy == 2.0f)
      option.select = 2;
    else if (anisotropy == 4.0f)
      option.select = 3;
    else if (anisotropy == 8.0f)
      option.select = 4;
    else if (anisotropy == 16.0f)
      option.select = 5;
    else if (anisotropy == 32.0f)
      option.select = 6;
    else
      option.select = 0;
    break;
  }
  case OptionSnowflakes:
    if (cfg.getWeather())
      option.select = cfg.getSnowflaketype() + 1;
    else
      option.select = 0;
    break;
  case OptionSpeedUnits:
    if (cfg.getSpeedUnit() == PConfig::Speedunit::mph)
      option.select = 1;
    else
      option.select = 0;
    break;
  case OptionGhostCars:
    option.select = cfg.getEnableGhost();
    break;
  case OptionDisplayFps:
    option.select = cfg.getEnableFps();
    break;
  case OptionFoliage:
    option.select = cfg.getFoliage();
    break;
  case OptionRoadsigns:
    option.select = cfg.getRoadsigns();
    break;
  case OptionDirtEffect:
    option.select = cfg.getDirteffect();
    break;
  default:
    break;
  }
}

///
/// @brief Return index of string in string vector
/// @param [in] str String to find
/// @param [in] vec Vector of strings
/// @return Index of string or index 0 if not found
///
unsigned int POption::findStringPos(const std::string &str, std::vector<std::string> &vec)
{
  std::vector<std::string>::iterator it = std::find(vec.begin(), vec.end(), str);
  if (it == vec.end())
    it = vec.begin();
  return std::distance(vec.begin(), it);
}
