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

#include "control.h"
#include "menu.h"
#include "render.h"
#include <algorithm>
#include <cctype>

///
/// @brief Constructor of controls menu
/// @param[in] parent   Game menu used to add labels to
/// @param[in] config   Configuration data of game from XML file
///
PControl::PControl(Gui &parent, PConfig &config) :
    parent(parent),
    cfg(config),
    pos(0),
    activepos(-1)
{
  controls.push_back({ PConfig::ActionForward, "forward" });
  controls.push_back({ PConfig::ActionBack, "back" });
  controls.push_back({ PConfig::ActionLeft, "left" });
  controls.push_back({ PConfig::ActionRight, "right" });
  controls.push_back({ PConfig::ActionHandbrake, "handbrake" });
  controls.push_back({ PConfig::ActionRecover, "reset car" });
  controls.push_back({ PConfig::ActionRecoverAtCheckpoint, "reset on road" });
  controls.push_back({ PConfig::ActionCamMode, "toggle camera" });
  controls.push_back({ PConfig::ActionCamRight, "view right" });
  controls.push_back({ PConfig::ActionCamLeft, "view left" });
  controls.push_back({ PConfig::ActionShowMap, "show map" });
  controls.push_back({ PConfig::ActionPauseRace, "pause race" });
  controls.push_back({ PConfig::ActionShowUi, "show UI" });
  controls.push_back({ PConfig::ActionShowCheckpoint, "show checkpoints" });
}

///
/// @brief Renders controls menu
///
void PControl::render()
{
  pos = 0;
  parent.makeClickable(
    parent.addLabel(10.0f, 10.0f, "back", PTEXT_HZA_LEFT | PTEXT_VTA_BOTTOM, 40.0f),
    AA_RELOAD_ALL, 0);
  parent.addLabel(80.0f, 520.0f, "Controls", PTEXT_HZA_LEFT | PTEXT_VTA_CENTER, 30.0f, LabelStyle::Header);
  parent.addLabel(340.0f, 520.0f, "keyboard keys", PTEXT_HZA_LEFT | PTEXT_VTA_CENTER, 22.0f, LabelStyle::Weak);
  parent.addGraphic(330.0f, 70.0f, 380.0f, 430.0f, nullptr, GraphicStyle::Image);

  for (unsigned int i = 0; i < controls.size(); ++i) {
    addControl(controls[i], (int)i == activepos);
  }
}

///
/// @brief Called when an control item is clicked
/// @param[in] index    Position of item on screen
///
void PControl::select(int index)
{
  if (activepos == index)
    activepos = -1;
  else
    activepos = index;
}

///
/// @brief Handles key presses in menu
/// @param[in] ke   Event of pressed key
/// @return Key was handled or not
///
bool PControl::handleKey(const SDL_KeyboardEvent &ke)
{
  if (activepos >= 0 && activepos < (int)controls.size()) {
    if (ke.keysym.sym != SDLK_ESCAPE) {
      unassignKey(ke.keysym.sym);
      cfg.getCtrl().map[controls[activepos].action].type = PConfig::UserControl::TypeKey;
      cfg.getCtrl().map[controls[activepos].action].key.sym = ke.keysym.sym;
    }
    activepos = -1;
    return true;
  }
  return false;
}

///
/// @brief Render a single control item
/// @param[in] control  Data of control item
///
void PControl::addControl(Control &control, bool active)
{
  std::string keyname = "PRESS KEY";

  if (!active) {
    SDL_Keycode keycode = cfg.getCtrl().map[control.action].key.sym;

    if (keycode == SDLK_UNKNOWN)
      keyname = "NOT ASSIGNED";
    else {
      keyname = SDL_GetKeyName(cfg.getCtrl().map[control.action].key.sym);
      transform(keyname.begin(), keyname.end(), keyname.begin(), ::tolower);
    }
  }

  parent.addLabel(80.0f, 490.0f - (float)pos * 30.0f,
      control.text, PTEXT_HZA_LEFT | PTEXT_VTA_TOP, 22.0f, LabelStyle::Regular);
  parent.makeClickable(
      parent.addLabel(340.0f, 490.0f - (float)pos * 30.0f, keyname,
          PTEXT_HZA_LEFT | PTEXT_VTA_TOP, 22.0f, LabelStyle::Regular),
      AA_PICK_CTRL, pos);
  ++pos;
}

///
/// @brief Un-assign key from all actions
/// @param[in] keycode  Key code to un-assign
///
void PControl::unassignKey(SDL_Keycode keycode)
{
  for (Control control: controls) {
    if (cfg.getCtrl().map[control.action].key.sym == keycode) {
      cfg.getCtrl().map[control.action].type = PConfig::UserControl::TypeUnassigned;
      cfg.getCtrl().map[control.action].key.sym = SDLK_UNKNOWN;
    }
  }
}
