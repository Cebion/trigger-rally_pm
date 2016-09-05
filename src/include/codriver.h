//
// Copyright (C) 2015-2016 Andrei Bondor, ab396356@users.sourceforge.net
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

#include <cstddef>
#include <forward_list>
#include <iterator>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

// maximum number of characters for a note
// e.g.: "hard left over jump" has 19 characters
//
// NOTE: this isn't a hard limit and it can be exceeded by the game safely
//  but at the cost of one or more memory reallocations
const std::size_t note_maxlength = 128;

struct PCodriverUserConfig
{
    float life  = 3.00f;    ///< How many seconds until the codriver signs start fading.
    float scale = 0.20f;    ///< Scale of the codriver signs.
    float posx  = 0.00f;    ///< X scale centering.
    float posy  = 0.45f;    ///< Y scale centering.
};

///
/// @brief Draws the codriver signs for the driver.
///
class PCodriverSigns
{
public:

    PCodriverSigns() = delete;

    PCodriverSigns(
        const std::unordered_map<std::string, PTexture *> &signs,
        const PCodriverUserConfig &uc
        ):
        signs(signs),
        uc(uc)
    {
        cpsigns.reserve(8); // FIXME: magic number
        tempnote.reserve(note_maxlength);
    }

    ///
    /// @brief Sets the current codriver signs.
    /// @param [in] notes       Original notes.
    /// @param time             Codriver checkpoint time.
    ///
    void set(const std::string &notes, float time)
    {
        std::istringstream ssnotes(notes);
        std::istream_iterator<std::string> itnotes_begin(ssnotes);
        std::istream_iterator<std::string> itnotes_end;
        std::forward_list<std::string> flnotes(itnotes_begin, itnotes_end);

        cpsigns.clear();
        cptime = time;

        while (!flnotes.empty())
        {
            PTexture *cptex = nullptr;
            auto cut_end = flnotes.cbegin();

            tempnote.clear();

            for (auto ci = flnotes.cbegin(); ci != flnotes.cend(); ++ci)
            {
                tempnote += *ci;

                if (signs.count(tempnote) != 0)
                {
                    cptex = signs.at(tempnote);
                    cut_end = ci;
                }
            }

            if (cptex != nullptr)
                cpsigns.push_back(cptex);

            flnotes.erase_after(flnotes.cbefore_begin(), std::next(cut_end));
        }
    }

    ///
    /// @brief Draws the current codriver signs.
    /// @param coursetime       The current time of the course.
    ///
    void render(float coursetime)
    {
        if (cpsigns.empty())
            return;

        float alpha;

        if (coursetime - cptime < uc.life)
        {
            alpha = 1.0f;
        }
        else
        if (coursetime - cptime < uc.life + 1.0f)
        {
            alpha = (uc.life + 1.0f) - (coursetime - cptime);
        }
        else
        {
            cpsigns.clear();
            return;
        }

        glPushMatrix();
        glLoadIdentity();
        glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
        glTranslatef(uc.posx, uc.posy, 0.0f);
        glScalef(uc.scale, uc.scale, 1.0f);
        glTranslatef(-0.5f * cpsigns.size() * 2 + 1.0f, 0.0f, 0.0f);
        glColor4f(1.0f, 1.0f, 1.0f, alpha);

        for (PTexture *cptex: cpsigns)
        {
            cptex->bind();
            glBegin(GL_QUADS);
            glTexCoord2f(   1.0f,   1.0f);
            glVertex2f(     1.0f,   1.0f);
            glTexCoord2f(   0.0f,   1.0f);
            glVertex2f(    -1.0f,   1.0f);
            glTexCoord2f(   0.0f,   0.0f);
            glVertex2f(    -1.0f,  -1.0f);
            glTexCoord2f(   1.0f,   0.0f);
            glVertex2f(     1.0f,  -1.0f);
            glEnd();
            glTranslatef(2.0f, 0.0f, 0.0f);
        }

        glPopMatrix();
    }

private:

    std::unordered_map<std::string, PTexture *> signs; ///< MainApp::tex_codriversigns
    PCodriverUserConfig uc; ///< User configuration.
    std::vector<PTexture *> cpsigns;
    std::string tempnote; ///< Temporary string for performance.
    float cptime;
};

///
/// @brief Gives a voice to the codriver.
/// @todo Re-code without making use of C++11 threads to be in line with
///  the rest of the game logic, which is implemented with `tick()` functions.
///
class PCodriverVoice
{
public:

    PCodriverVoice() = delete;

    PCodriverVoice(const std::unordered_map<std::string, PAudioSample *> &words, float volume):
        words(words),
        volume(volume)
    {
        tempnote.reserve(note_maxlength);
    }

    void say(const std::string &notes)
    {
        if (words.empty())
            return;

        std::vector<PAudioSample *> cpwords;
        std::istringstream ssnotes(notes);
        std::istream_iterator<std::string> itnotes_begin(ssnotes);
        std::istream_iterator<std::string> itnotes_end;
        std::forward_list<std::string> flnotes(itnotes_begin, itnotes_end);

        cpwords.reserve(note_maxlength);

        while (!flnotes.empty())
        {
            PAudioSample *cpaud = nullptr;
            auto cut_end = flnotes.cbegin();

            tempnote.clear();

            for (auto ci = flnotes.cbegin(); ci != flnotes.cend(); ++ci)
            {
                tempnote += *ci;

                if (words.count(tempnote) != 0)
                {
                    cpaud = words.at(tempnote);
                    cut_end = ci;
                }
            }

            if (cpaud != nullptr)
                cpwords.push_back(cpaud);

            flnotes.erase_after(flnotes.cbefore_begin(), std::next(cut_end));
        }

        std::thread(&PCodriverVoice::asyncSay, this, cpwords).detach();
    }

private:

    void asyncSay(std::vector<PAudioSample *> cpwords) const
    {
        for (auto w: cpwords)
        {
            PAudioInstance voice(w);

            voice.setGain(volume);
            voice.play();

            while (voice.isPlaying())
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    std::unordered_map<std::string, PAudioSample *> words; ///< MainApp::aud_codriverwords
    std::string tempnote; ///< Temporary string for performance.
    float volume;
};

