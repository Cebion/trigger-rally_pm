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

#include "config.h"
#include <SDL2/SDL.h>
#include <string>
#include <vector>

class Gui;
class PConfig;

///
/// @brief Controls menu
///
class PControl {
public:
  PControl(Gui &parent, PConfig &config);

  void render();
  void select(int index);
  bool handleKey(const SDL_KeyboardEvent &ke);

private:
  /// Data of one control item
  struct Control {
    PConfig::Action action; ///< Action associated with a key
    std::string text;       ///< Text displayed in control menu
  };

  PControl();
  PControl(const PControl&);
  PControl& operator=(const PControl&);

  void addControl(Control &control, bool active);
  void unassignKey(SDL_Keycode keycode);

  Gui &parent;                      ///< Holds functions to add visual elements
  PConfig &cfg;                     ///< Configuration data of the game
  int pos;                          ///< Index of of each option row
  int activepos;                    ///< Index of item waiting for key
  std::vector<Control> controls;    ///< Data of each control item
};
