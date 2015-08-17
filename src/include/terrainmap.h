//
// Copyright (C) 2015 Andrei Bondor, ab396356@users.sourceforge.net
// Copyright (C) 2015 Onsemeliot, onsemeliot@gmail.com
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

enum class TerrainType
{
    Unknown,
    RacingTarmac,
    CountryRoad,
    SlipperyRoad,
    Ice,
    DeepSnow,
    FirmSand,
    LooseSand,
    PressedEarth,
    LooseEarth,
    MoistEarth,
    Mud,
    FineGravel,
    RoughGravel,
    Grass,
    DeepWater
};

#define RGB rgbcolor

///
/// @brief X-macro describing various terrain types.
/// @details
///
///   Name              RgbColor                Friction    Resistance
///
#define TERRAINMAP_MATERIALS                                        \
    X(RacingTarmac,     RGB( 80,  60, 100),     1.50,       0.00)   \
    X(CountryRoad,      RGB(140,  80, 180),     1.20,       0.00)   \
    X(SlipperyRoad,     RGB(200, 140, 255),     0.50,       0.00)   \
    X(Ice,              RGB(180, 220, 255),     0.20,       0.00)   \
    X(DeepSnow,         RGB(255, 255, 255),     0.30,       0.50)   \
    X(FirmSand,         RGB(255, 220,   0),     0.90,       0.05)   \
    X(LooseSand,        RGB(255, 255, 140),     0.80,       0.30)   \
    X(PressedEarth,     RGB(255, 200, 200),     1.00,       0.05)   \
    X(LooseEarth,       RGB(255, 140, 140),     0.80,       0.15)   \
    X(MoistEarth,       RGB(255,   0,   0),     1.10,       0.15)   \
    X(Mud,              RGB(160,   0,   0),     0.80,       0.25)   \
    X(FineGravel,       RGB(140, 140, 140),     0.90,       0.10)   \
    X(RoughGravel,      RGB(  0,   0,   0),     0.80,       0.05)   \
    X(Grass,            RGB(  0, 180,   0),     0.40,       0.10)   \
    X(DeepWater,        RGB(  0,   0, 255),     0.20,       0.50)
