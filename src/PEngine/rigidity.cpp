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

#include "pengine.h"
#include "rigidity.h"

///
/// @brief Loads rigidity.xml file and stores the content in a map
///
PRigidity::PRigidity()
{
  XMLDocument xmlfile;
  XMLElement *rootelem = PUtil::loadRootElement(xmlfile, "rigidity.xml", "rigiditymap");

  if (!rootelem) {
    PUtil::outLog() << "Could not load file \"rigidity.xml\"." << std::endl;
    return;
  }

  for (XMLElement *walk = rootelem->FirstChildElement();
      walk; walk = walk->NextSiblingElement()) {
    if (strcmp(walk->Value(), "rigidity") == 0) {
      std::string sprite;
      float value;
      const char *val;

      val = walk->Attribute("sprite");
      if (val == nullptr)
        continue;
      sprite = val;

      val = walk->Attribute("value");
      if (val == nullptr)
        continue;
      value = std::stof(val);

      rigiditymap[sprite] = value;
    }
  }
}

///
/// @brief Returns rigidity value of a sprite
/// @param sprite = path to the sprite file
/// @retval rigidity value
///
float PRigidity::getRigidity(std::string sprite) const
{
  std::map<std::string, float>::const_iterator pos = rigiditymap.find(sprite);

  if (pos == rigiditymap.end())
    return 0.0f;
  return pos->second;
}
