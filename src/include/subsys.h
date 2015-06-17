//
// Copyright (C) 2004-2006 Jasmine Langridge, ja-reiko@users.sourceforge.net
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

class PResource
{
protected:

    std::string name;

public:

    const std::string & getName() const
    {
        return name;
    }
};

template <class T>
class PResourceList
{
private:

    std::vector<T *> reslist;

public:

    ~PResourceList()
    {
        clear();
    }

    T * add(T *newresource)
    {
        reslist.push_back(newresource);
        return newresource;
    }

    T * find(const std::string &name)
    {
        for (T *res: reslist)
            if (name == res->getName())
                return res;

        return nullptr;
    }

    void clear()
    {
        for (T *res: reslist)
            delete res;

        reslist.clear();
    }
};

class PSubsystem
{
protected:

    PApp &app;

public:

    PSubsystem(PApp &parentApp):
        app(parentApp)
    {
    }

    virtual ~PSubsystem()
    {
    }

    virtual void tick(float delta, const vec3f &eyepos, const mat44f &eyeori, const vec3f &eyevel)
    {
        UNREFERENCED_PARAMETER(delta);
        UNREFERENCED_PARAMETER(eyepos);
        UNREFERENCED_PARAMETER(eyeori);
        UNREFERENCED_PARAMETER(eyevel);
    }
};

