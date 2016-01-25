//
// Copyright (C) 2016 Andrei Bondor, ab396356@users.sourceforge.net
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

#include <iterator>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace
{
// how many seconds until the codriver signs start fading
const float signlife = 3.0f;

// scale of the codriver signs
const float signscale = 0.2f;
}

///
/// @brief Draws the codriver signs for the driver.
///
class PCodriverSigns
{
public:

    PCodriverSigns() = delete;

    PCodriverSigns(const std::unordered_map<std::string, PTexture *> &signs):
        signs(signs)
    {
    }

    ///
    /// @brief Sets the current codriver signs.
    /// @param [in] notes       Original notes.
    /// @param time             Codriver checkpoint time.
    ///
    void set(const std::string &notes, float time)
    {
        std::stringstream ssnotes(notes);
        std::istream_iterator<std::string> itnotes_begin(ssnotes);
        std::istream_iterator<std::string> itnotes_end;
        std::vector<std::string> vnotes(itnotes_begin, itnotes_end);

        cpsigns.clear();
        cptime = time;

        while (!vnotes.empty())
        {
            PTexture *cptex = nullptr;
            std::string tempnote;
            auto cut_end = vnotes.cbegin();

            for (auto ci = vnotes.cbegin(); ci != vnotes.cend(); ++ci)
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

            vnotes.erase(vnotes.cbegin(), cut_end + 1);
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

        if (coursetime - cptime < signlife)
        {
            alpha = 1.0f;
        }
        else
        if (coursetime - cptime < signlife + 1.0f)
        {
            alpha = (signlife + 1.0f) - (coursetime - cptime);
        }
        else
        {
            cpsigns.clear();
            return;
        }

        glPushMatrix();
        glLoadIdentity();
        glColor4f(1.0f, 1.0f, 1.0f, alpha);
        glTranslatef(-0.5f * cpsigns.size() * 2 * signscale + signscale, 0.45f, 0.0f);
        glScalef(signscale, signscale, 1.0f);

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

    std::unordered_map<std::string, PTexture *> signs;
    std::vector<PTexture *> cpsigns;
    float cptime;
};

