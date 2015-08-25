//
// Copyright (C) 2015 Andrei Bondor, ab396356@users.sourceforge.net
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

#include <queue>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

enum class CodriverWords
{
    Left,
    Right,
    Flat,
    Easy,
    Medium,
    Chicane,
    Square,
    Hard,
    Hairpin,
    Over,
    Into,
    Jump,
    Finish,
    Dont,
    Cut,
    Long,
    Tarmac,
    Gravel,
    Dirt,
    Mud,
    Sand,
    Ice,
    Snow,
    Water,
    Grass,
    NUMBER_OF_WORDS
};

///
/// @brief X-macro describing codriver words.
/// @details
///
///   Word          Note
///
#define CODRIVERVOICE_PARAMETERS    \
    X(Left,         "left"      )   \
    X(Right,        "right"     )   \
    X(Flat,         "flat"      )   \
    X(Easy,         "easy"      )   \
    X(Medium,       "medium"    )   \
    X(Chicane,      "chicane"   )   \
    X(Square,       "square"    )   \
    X(Hard,         "hard"      )   \
    X(Hairpin,      "hairpin"   )   \
    X(Over,         "over"      )   \
    X(Into,         "into"      )   \
    X(Jump,         "jump"      )   \
    X(Finish,       "finish"    )   \
    X(Dont,         "dont"      )   \
    X(Cut,          "cut"       )   \
    X(Long,         "long"      )   \
    X(Tarmac,       "tarmac"    )   \
    X(Gravel,       "gravel"    )   \
    X(Dirt,         "dirt"      )   \
    X(Mud,          "mud"       )   \
    X(Sand,         "sand"      )   \
    X(Ice,          "ice"       )   \
    X(Snow,         "snow"      )   \
    X(Water,        "water"     )   \
    X(Grass,        "grass"     )

///
/// @brief Gives a voice to the codriver.
///
class PCodriverVoice
{
public:

    PCodriverVoice() = delete;

    PCodriverVoice(const std::vector<PAudioSample *> &words, float volume):
        words(words),
        volume(volume)
    {
    }

    void say(const std::string &notes) const
    {
        if (words.empty() || words.size() != static_cast<unsigned int> (CodriverWords::NUMBER_OF_WORDS))
            return;

        std::thread(&PCodriverVoice::asyncSay, this, notes).detach();
    }

private:

    void asyncSay(const std::string &notes) const
    {
        std::string cw; // Current Word
        std::stringstream ssnotes(notes);
        std::queue<PAudioInstance> speak;

        while (ssnotes >> cw)
        {
#define X(Word, Note)   if (cw == Note) speak.emplace(words[static_cast<unsigned int> (CodriverWords::Word)]); else
            CODRIVERVOICE_PARAMETERS (void)0;
#undef X
        }

        while (!speak.empty())
        {
            speak.front().setGain(volume);
            speak.front().play();

            while (speak.front().isPlaying());

            speak.pop();
        }
    }

    std::vector<PAudioSample *> words;
    float volume;
};

