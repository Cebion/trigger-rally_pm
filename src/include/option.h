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

#include <string>
#include <vector>

class Gui;
class GuiWidget;
class PConfig;

///
/// @brief Options menu
///
class POption {
public:
  POption(Gui &parent, PConfig &config);

  void render();
  void select(int index);

private:
  enum OptionId {
    OptionPlayerName,
    OptionEngineVolume,
    OptionSfxVolume,
    OptionCodriverVolume,
    OptionCodriverVoice,
    OptionCodriverSigns,
    OptionTextureQuality,
    OptionSnowflakes,
    OptionSpeedUnits,
    OptionGhostCars,
    OptionDisplayFps,
    OptionFoliage,
    OptionRoadsigns,
    OptionDirtEffect,
    OptionMaxSize
  };
  /// Data of one option item
  struct Option {
    OptionId id;                        ///< Identifier of option from enum
    std::string text;                   ///< Text displayed in options menu
    std::vector<std::string> values;    ///< Options that can be selected in menu
    unsigned int select;                ///< Index of option currently selected
  };

  POption();
  POption(const POption&);
  POption& operator=(const POption&);

  void addOption(Option &option);
  void updateSelect(Option &option);
  unsigned int findStringPos(const std::string &str, std::vector<std::string> &vec);

  Gui &parent;                  ///< Holds functions to add labels
  PConfig &cfg;                 ///< Configuration data of the game
  unsigned int pos;             ///< Index of of each option row
  std::vector<Option> options;  ///< Data of each option item
};
