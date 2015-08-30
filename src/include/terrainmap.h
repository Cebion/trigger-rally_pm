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
    Shrubbery,
    DeepWater
};

#define RGB     rgbcolor
#define DI      dirtinfo

///
/// @brief X-macro describing various terrain types.
/// @details
///
///   Name              RgbColor                Friction    Resistance      DirtInfo
///
#define TERRAINMAP_MATERIALS                                                                        \
    X(RacingTarmac,     RGB( 80,  60, 100),     1.50,       0.00,           DI(0.00, 0.10,  6.0))   \
    X(CountryRoad,      RGB(140,  80, 180),     1.20,       0.00,           DI(0.00, 0.20,  6.0))   \
    X(SlipperyRoad,     RGB(200, 140, 255),     0.50,       0.00,           DI(0.00, 0.30,  6.0))   \
    X(Ice,              RGB(180, 220, 255),     0.20,       0.00,           DI(0.00, 0.10,  6.0))   \
    X(DeepSnow,         RGB(255, 255, 255),     0.30,       0.50,           DI(0.00, 0.80,  1.0))   \
    X(FirmSand,         RGB(255, 220,   0),     1.00,       0.05,           DI(0.50, 2.60,  1.5))   \
    X(LooseSand,        RGB(255, 255, 140),     0.75,       0.55,           DI(0.50, 3.00,  1.5))   \
    X(PressedEarth,     RGB(255, 200, 200),     0.90,       0.05,           DI(0.00, 0.35,  3.0))   \
    X(LooseEarth,       RGB(255, 140, 140),     0.70,       0.25,           DI(0.00, 1.00,  2.0))   \
    X(MoistEarth,       RGB(255,   0,   0),     1.10,       0.15,           DI(0.00, 0.75,  7.5))   \
    X(Mud,              RGB(160,   0,   0),     0.60,       0.40,           DI(0.00, 0.55,  6.0))   \
    X(FineGravel,       RGB(140, 140, 140),     0.90,       0.10,           DI(0.00, 1.25,  3.0))   \
    X(RoughGravel,      RGB(  0,   0,   0),     0.60,       0.20,           DI(0.00, 0.40,  5.0))   \
    X(Grass,            RGB(  0, 180,   0),     0.40,       0.10,           DI(0.00, 0.30,  6.5))   \
    X(Shrubbery,        RGB(  0,  90,   0),     0.40,       0.70,           DI(0.00, 0.50,  6.0))   \
    X(DeepWater,        RGB(  0,   0, 255),     0.20,       0.60,           DI(0.00, 0.50,  6.0))
