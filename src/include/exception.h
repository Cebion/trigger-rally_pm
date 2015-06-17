//
// Copyright (C) 2004-2006 Jasmine Langridge, ja-reiko@users.sourceforge.net
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

#include <exception>
#include <string>

class PException: public std::exception
{
public:

    PException(const std::string &text):
        text(text)
    {
    }

    const char * what() const noexcept
    {
        return text.c_str();
    }

private:

    std::string text;
};

/// Helper macros to convert `__LINE__` into a string literal.
#define MAKESTRING2(x)          #x
#define MAKESTRING(x)           MAKESTRING2(x)

// Helper macro to use when throwing a `PException`.
#define MakePException(text)    PException(std::string() + text + std::string(" at " __FILE__ ":" MAKESTRING(__LINE__)))

