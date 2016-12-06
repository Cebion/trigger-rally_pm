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

#include <algorithm>
#include <functional>
#include <iomanip>
#include <istream>
#include <limits>
#include <ostream>
#include <regex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <physfs.h>

#define GETLINE_SKIP_EMPTY_LINES(InputStream, String)   if (true) { \
    while (std::getline(InputStream, String)) {                     \
        if (!String.empty())                                        \
            break;                                                  \
    }                                                               \
    if (String.empty())                                             \
        return InputStream;                                         \
} else (void)0


// TODO: remove duplicate code
#define GETLINE_SKIP_EMPTY_LINES_B(InputStream, String) if (true) { \
    while (std::getline(InputStream, String)) {                     \
        if (!String.empty())                                        \
            break;                                                  \
    }                                                               \
    if (String.empty())                                             \
        return static_cast<bool> (InputStream);                     \
} else (void)0

///
/// @brief Basic structure to load and save race results.
///
struct RaceData
{
    std::string playername;     ///< e.g. "Andrei"
    std::string mapname;        ///< e.g. "/maps/jumpy/jumpy.level"
    std::string carname;        ///< e.g. "FMC Fox"
    std::string carclass;       ///< e.g. "Super500"
    float totaltime;            ///< e.g. "102.2"
    float maxspeed;             ///< e.g. "212.0"

    RaceData() = default;

    ///
    /// @note This exists because the player's name isn't read from file.
    ///
    explicit RaceData(const std::string &playername):
        playername(playername)
    {
    }

    RaceData(
        const std::string &playername,
        const std::string &mapname,
        const std::string &carname,
        const std::string &carclass,
        float totaltime,
        float maxspeed
        ):
        playername(playername),
        mapname(mapname),
        carname(carname),
        carclass(carclass),
        totaltime(totaltime),
        maxspeed(maxspeed)
    {
    }
};

///
/// @brief Reads a RaceData object from an input stream.
/// @note The player name is intentionally omitted.
/// @todo Each call to `std::getline()` should be checked for success.
/// @param [in,out] is      Input stream.
/// @param [out] rd         Race data to be read.
/// @returns The input stream.
///
inline std::istream & operator >> (std::istream &is, RaceData &rd)
{
    std::string ts; // Temporary String

    GETLINE_SKIP_EMPTY_LINES(is, rd.mapname);
    GETLINE_SKIP_EMPTY_LINES(is, rd.carname);
    GETLINE_SKIP_EMPTY_LINES(is, rd.carclass);
    GETLINE_SKIP_EMPTY_LINES(is, ts);
    rd.totaltime = std::stof(ts);
    GETLINE_SKIP_EMPTY_LINES(is, ts);
    rd.maxspeed = std::stof(ts);
    return is;
}

///
/// @brief Writes a RaceData object to an output stream.
/// @todo Each call to `std::operator<<()` should be checked for success.
/// @todo Decide if should call `std::ostream::flush()`.
/// @param [in,out] os      Output stream.
/// @param [in] rd          Race data to be written.
/// @returns The output stream.
///
inline std::ostream & operator << (std::ostream &os, const RaceData &rd)
{
    os << rd.mapname << '\n';
    os << rd.carname << '\n';
    os << rd.carclass << '\n';
    os << rd.totaltime << '\n';
    os << rd.maxspeed << "\n\n";
    return os;
}

enum class HISCORE1_SORT
{
    BY_TOTALTIME_ASC,
    BY_TOTALTIME_DESC,
    BY_MAXSPEED_ASC,
    BY_MAXSPEED_DESC,
    BY_PLAYERNAME_ASC,
    BY_PLAYERNAME_DESC,
    BY_CARNAME_ASC,
    BY_CARNAME_DESC,
    BY_CARCLASS_ASC,
    BY_CARCLASS_DESC
};

using UnlockData = std::unordered_set<std::string>;

///
/// @brief Used to display the best times information.
///
struct TimeEntry
{
    unsigned long int place = 0;    ///< True place, depending on time.
    RaceData rd;                    ///< Race data.
    bool highlighted = false;       ///< Highlight flag.

    TimeEntry(
        unsigned long int place,
        const RaceData &rd,
        bool highlighted
        ):
        place(place),
        rd(rd),
        highlighted(highlighted)
    {
    }
};

///
/// @brief Loads and saves the player's best times.
/// @details As can be seen from the name, this isn't supposed to a
///  final version. Also it doesn't concern itself as much with "score"
///  as it does with best times.
///
class HiScore1
{
public:

    HiScore1() = delete;

    ///
    /// @brief Constructs a high score object.
    /// @param [in] searchdir       Directory where to scan for score data (.PLAYER files).
    /// @param [in] playername      Name of the player such that the correct .PLAYER file is updated.
    /// @todo Don't use magic numbers for preemptive storage reservation.
    ///
    explicit HiScore1(const std::string &searchdir, const std::string &playername = "Player"):
        searchdir(searchdir),
        playername(playername)
    {
        currenttimes.reserve(16);
    }

    ///
    /// @brief Writes to the selected player's file.
    ///
    ~HiScore1()
    {
        writePlayerData(playername);
    }

    ///
    /// @brief Sets the `playername`.
    /// @param [in] pname           New player name to use.
    ///
    void setPlayerName(const std::string &pname)
    {
        playername = pname;
    }

    ///
    /// @brief Loads all times from .PLAYER files.
    ///
    void loadAllTimes()
    {
        if (PHYSFS_isInit() == 0)
            return;

        char **rc = PHYSFS_enumerateFiles(searchdir.c_str());

        for (char **fname = rc; *fname != nullptr; ++fname)
        {
            // remove the extension from the filename
            std::smatch mr; // Match Results
            std::regex pat(R"(^([\s\w]+)(\.player)$)"); // Pattern
            std::string fn(*fname); // Filename

            if (!std::regex_search(fn, mr, pat))
                continue;

            std::string pname = mr[1]; // Player Name
            PHYSFS_File *pfile = PHYSFS_openRead((searchdir + '/' + *fname).c_str()); // Player File
            std::string pdata(PHYSFS_fileLength(pfile), '\0'); // Player Data

            PHYSFS_read(pfile, &pdata.front(), sizeof (char), pdata.size());
            readPlayerData(pname, pdata);
            PHYSFS_close(pfile);
        }

        PHYSFS_freeList(rc);
    }

    ///
    /// @brief Adds new race data.
    /// @param [in] rd      Race data.
    ///
    void addNewTime(const RaceData &rd)
    {
        alltimes.insert({rd.mapname, rd});
    }

    ///
    /// @brief Sets how many saves to skip.
    /// @param sk       New value for skipped saves.
    ///
    void setSkipSaves(unsigned long int sk)
    {
        skipSaves = sk;
        sc = 0; // reset Skip Counter
    }

    ///
    /// @brief Saves the player's race data.
    ///
    void savePlayer() const
    {
        writePlayerData(playername);
    }

    ///
    /// @brief Saves the player's race data with possible skipping.
    /// @details The purpose of this function is to cut down on the expensive
    ///  file output operations by skipping calls to `writePlayerData()`.
    ///
    void skipSavePlayer() const
    {
        if (skipSaves <= -1) // save only by destructor
            return;

        if (sc++ == skipSaves)
        {
            writePlayerData(playername);
            sc = 0;
        }
    }

    ///
    /// @brief Adds new unlock data for the current player.
    /// @param [in] udata       Unlock data.
    ///
    void addNewUnlock(const std::string &udata)
    {
        addNewUnlock(playername, udata);
    }

    ///
    /// @brief Adds new unlock data for the player.
    /// @param [in] pname       Player name.
    /// @param [in] udata       Unlock data.
    ///
    void addNewUnlock(const std::string &pname, const std::string &udata)
    {
        allunlocks[pname].insert(udata);
    }

    ///
    /// @brief Retrieves unlock data for the current player.
    /// @returns Unlock data.
    ///
    UnlockData getUnlockData() const
    {
        return getUnlockData(playername);
    }

    ///
    /// @brief Retrieves unlock data for a player.
    /// @param [in] pname       Player name.
    /// @returns Unlock data.
    ///
    UnlockData getUnlockData(const std::string &pname) const
    {
        if (allunlocks.count(pname) == 0)
            return UnlockData {};

        return allunlocks.at(pname);
    }

    ///
    /// @brief Retrieves the best time for `mapname`, if available.
    /// @param [in] mapname         Map for which to get the best time.
    /// @returns The best time.
    /// @retval -1.0f               If no best time is available.
    /// @note Check for above like `(x < 0)` instead of `(x == -1)`.
    /// @todo Use `auto` parameters for lambda after C++17.
    ///
    float getBestTime(const std::string &mapname)
    {
        if (alltimes.count(mapname) == 0)
            return -1.0f;

        const auto range = alltimes.equal_range(mapname);

        const auto rdi = std::min_element(range.first, range.second,
            [](decltype (*range.first) a, decltype (*range.first) b) -> bool
            {
                return a.second.totaltime < b.second.totaltime;
            });

        return rdi->second.totaltime;
    }

    ///
    /// @brief Retrieves the best class time for `mapname`, if available.
    /// @param [in] mapname         Map for which to get the best class time.
    /// @param [in] carclass        Car class for which to retrieve the time.
    /// @returns The best class time.
    /// @retval -1.0f               If no best class time is available.
    /// @note Check for above like `(x < 0)` instead of `(x == -1)`.
    ///
    float getBestClassTime(const std::string &mapname, const std::string &carclass)
    {
        if (alltimes.count(mapname) == 0)
            return -1.0f;

        const auto range = alltimes.equal_range(mapname);

        bool found_a_time = false;
        float bct; // Best Class Time

        if (std::numeric_limits<float>::has_infinity) // "usually true"
            bct = std::numeric_limits<float>::infinity();
        else // maximum is good enough
            bct = std::numeric_limits<float>::max();

        for (auto i = range.first; i != range.second; ++i)
            if (i->second.carclass == carclass)
            {
                bct = std::min(bct, i->second.totaltime);
                found_a_time = true;
            }

        if (!found_a_time)
            return -1.0f;

        return bct;
    }

    ///
    /// @brief Retrieves best times list for `mapname`, sorted by `sortmethod`.
    /// @param [in] mapname         Map for which to get the times.
    /// @param sortmethod           How to sort the times.
    /// @see `HISCORE1_SORT` enum.
    /// @note For `mapname == ""` the current results list will be re-sorted.
    /// @returns Sorted list of results.
    ///
    const std::vector<TimeEntry> & getCurrentTimes(const std::string &mapname, HISCORE1_SORT sortmethod)
    {
        if (!mapname.empty())
        {
            const auto range = alltimes.equal_range(mapname);

            currenttimes.clear();

            for (auto i = range.first; i != range.second; ++i)
                currenttimes.push_back({0, i->second, false});

            sortAndUpdatePlaces();
        }

        sortCurrentTimes(sortmethod);
        return currenttimes;
    }

    ///
    /// @brief Sorts and retrieves current highlighted times.
    /// @param sortmethod           How to sort the times.
    /// @see `HISCORE1_SORT` enum.
    /// @returns Sorted list of highlighted results.
    ///
    const std::vector<TimeEntry> & getCurrentTimesHL(HISCORE1_SORT sortmethod)
    {
        sortCurrentTimes(sortmethod);
        return currenttimes;
    }

    ///
    /// @brief Inserts race data and retrieves the updated highlighted times.
    /// @remarks Sorting method is essentially `HISCORE1_SORT::BY_TOTALTIME_ASC`.
    /// @remarks Target map is deduced from `rd.mapname`.
    /// @param [in] rd              Race data to be inserted and highlighted.
    /// @returns Sorted list of highlighted results.
    ///
    const std::vector<TimeEntry> & insertAndGetCurrentTimesHL(const RaceData &rd)
    {
        // get old times before inserting newest one
        const auto range = alltimes.equal_range(rd.mapname);

        currenttimes.clear();

        for (auto i = range.first; i != range.second; ++i)
            currenttimes.push_back({0, i->second, false});

        currenttimes.push_back({0, rd, true}); // the newest, highlighted time
        sortAndUpdatePlaces();
        alltimes.insert({rd.mapname, rd});
        return currenttimes;
    }

#ifndef NDEBUG

    ///
    /// @brief Debug printing of current times.
    /// @param [in,out] os      Output stream to print to.
    ///
    void printCurrentTimes(std::ostream &os) const
    {
        for (const TimeEntry &te: currenttimes)
        {
            if (te.highlighted)
                os << std::setw(5) << "> " + std::to_string(te.place) << ' ';
            else
                os << std::setw(5) << te.place << ' ';

            os << std::setw(12) << te.rd.playername << ' ';
            os << std::setw(12) << te.rd.carname << ' ';
            os << std::setw(12) << te.rd.carclass << ' ';
            os << std::setw(6) << te.rd.maxspeed << " SU ";
            os << std::setw(6) << te.rd.totaltime;

            if (te.highlighted)
                os << " <\n";
            else
                os << '\n';
        }

        os << "***" << std::endl;
    }

    ///
    /// @brief Debug printing of current unlocks for all players.
    /// @param [in,out] os      Output stream to print to.
    ///
    void printCurrentUnlocks(std::ostream &os) const
    {
        for (const auto &p: allunlocks)
        {
            os << p.first << ":\n";

            for (const std::string &s: p.second)
                os << '\t' << s << '\n';
        }

        os << "***" << std::endl;
    }

#endif

private:

    ///
    /// @brief Sorts the current times list by time and updates places data.
    ///
    void sortAndUpdatePlaces()
    {
        sortCurrentTimes(HISCORE1_SORT::BY_TOTALTIME_ASC);

        unsigned long int p = 1; // Place

        for (TimeEntry &te: currenttimes)
            te.place = p++;
    }

    ///
    /// @brief Sorts the current times list.
    /// @remarks Helper function for internal use.
    /// @param sortmethod           How to sort the times.
    /// @see `HISCORE1_SORT` enum.
    ///
    void sortCurrentTimes(HISCORE1_SORT sortmethod)
    {
        std::function<bool (const TimeEntry &, const TimeEntry &)> cmpfunc; // Comparison Function

        switch (sortmethod)
        {
            // case HISCORE1_SORT::BY_TOTALTIME_ASC: // later, this is the default

            case HISCORE1_SORT::BY_TOTALTIME_DESC:

                cmpfunc = [](const TimeEntry &a, const TimeEntry &b) -> bool
                {
                    return a.rd.totaltime > b.rd.totaltime;
                };

                break;

            case HISCORE1_SORT::BY_MAXSPEED_ASC:

                cmpfunc = [](const TimeEntry &a, const TimeEntry &b) -> bool
                {
                    return a.rd.maxspeed > b.rd.maxspeed;
                };

                break;

            case HISCORE1_SORT::BY_MAXSPEED_DESC:

                cmpfunc = [](const TimeEntry &a, const TimeEntry &b) -> bool
                {
                    return a.rd.maxspeed < b.rd.maxspeed;
                };

                break;

            case HISCORE1_SORT::BY_PLAYERNAME_ASC:

                cmpfunc = [](const TimeEntry &a, const TimeEntry &b) -> bool
                {
                    if (a.rd.playername == b.rd.playername)
                        return a.rd.totaltime < b.rd.totaltime;

                    return a.rd.playername < b.rd.playername;
                };

                break;

            case HISCORE1_SORT::BY_PLAYERNAME_DESC:

                cmpfunc = [](const TimeEntry &a, const TimeEntry &b) -> bool
                {
                    if (a.rd.playername == b.rd.playername)
                        return a.rd.totaltime < b.rd.totaltime;

                    return a.rd.playername > b.rd.playername;
                };

                break;

            case HISCORE1_SORT::BY_CARNAME_ASC:

                cmpfunc = [](const TimeEntry &a, const TimeEntry &b) -> bool
                {
                    if (a.rd.carname == b.rd.carname)
                        return a.rd.totaltime < b.rd.totaltime;

                    return a.rd.carname < b.rd.carname;
                };

                break;

            case HISCORE1_SORT::BY_CARNAME_DESC:

                cmpfunc = [](const TimeEntry &a, const TimeEntry &b) -> bool
                {
                    if (a.rd.carname == b.rd.carname)
                        return a.rd.totaltime < b.rd.totaltime;

                    return a.rd.carname > b.rd.carname;
                };

                break;

            case HISCORE1_SORT::BY_CARCLASS_ASC:

                cmpfunc = [](const TimeEntry &a, const TimeEntry &b) -> bool
                {
                    if (a.rd.carclass == b.rd.carclass)
                        return a.rd.totaltime < b.rd.totaltime;

                    return a.rd.carclass < b.rd.carclass;
                };

                break;

            case HISCORE1_SORT::BY_CARCLASS_DESC:

                cmpfunc = [](const TimeEntry &a, const TimeEntry &b) -> bool
                {
                    if (a.rd.carclass == b.rd.carclass)
                        return a.rd.totaltime < b.rd.totaltime;

                    return a.rd.carclass > b.rd.carclass;
                };

                break;

            case HISCORE1_SORT::BY_TOTALTIME_ASC:
            default:

                cmpfunc = [](const TimeEntry &a, const TimeEntry &b) -> bool
                {
                    return a.rd.totaltime < b.rd.totaltime;
                };

                break;
        }

        std::sort(currenttimes.begin(), currenttimes.end(), cmpfunc);
    }

    ///
    /// @brief Encryption and decryption key, or rather pad.
    /// @warning Changing this invalidates all highscore files of previous versions!
    ///
    const std::vector<unsigned char> edkey {
        0x02, 0x43, 0x5E, 0xAC, 0x2E, 0x40, 0xD2, 0x7F, 0x84, 0xFB, 0xA0, 0x53, 0x52, 0x05, 0x4E, 0xEC,
        0x1A, 0xAB, 0x58, 0x8D, 0x2E, 0xFA, 0xC6, 0x2F, 0x65, 0x99, 0x69, 0x3D, 0xBC, 0x38, 0x0E, 0x64,
        0x45, 0x4B, 0xD9, 0x4B, 0xE5, 0x51, 0x73, 0xB3, 0x8A, 0x4E, 0x1B, 0xC1, 0x80, 0x11, 0x73, 0x16,
        0xE6, 0x66, 0x63, 0x09, 0x3A, 0x29, 0x90, 0x7F, 0xEC, 0xF6, 0x6B, 0xA5, 0x23, 0x2E, 0x77, 0xEC,
        0xDF, 0xA0, 0x92, 0x12, 0xB9, 0x7F, 0x3E, 0x63, 0x44, 0x9A, 0x53, 0x59, 0x97, 0xE0, 0x91, 0xE2,
        0x48, 0x20, 0xAA, 0x5C, 0x68, 0x4C, 0x09, 0x20, 0x63, 0xA6, 0x0A, 0xED, 0x80, 0x21, 0x12, 0xF0,
        0xE3, 0x4A, 0x74, 0xCA, 0x8C, 0xE0, 0x88, 0xDE, 0xC8, 0x47, 0xC8, 0xB2, 0x5B, 0x3C, 0x58, 0xFB,
        0x93, 0xC1, 0x1F, 0xFE, 0xEE, 0x16, 0x7D, 0xC7, 0x32, 0x00, 0x09, 0xE5, 0x32, 0x60, 0x5F, 0x31,
        0x98, 0x12, 0x30, 0x4D, 0x5A, 0xC8, 0x72, 0xF7, 0x83, 0xFE, 0x9B, 0xF1, 0x49, 0x6B, 0x83, 0x79,
        0xD4, 0xD1, 0x99, 0x1D, 0xB2, 0x1A, 0xC4, 0xFB, 0xB4, 0x6F, 0x8F, 0xE7, 0xE8, 0x0C, 0xB6, 0x14,
        0x84, 0x70, 0x37, 0xBE, 0x18, 0x84, 0xC9, 0x8B, 0xD9, 0x3D, 0xDD, 0x25, 0x1C, 0x17, 0x45, 0x20,
        0xED, 0x78, 0xC6, 0x40, 0xCA, 0x55, 0xF2, 0x2A, 0x4A, 0x28, 0x62, 0x3F, 0x94, 0xEB, 0xC9, 0x62,
        0x3F, 0xCF, 0x16, 0x9D, 0x6A, 0x53, 0x04, 0xEE, 0xFC, 0x2E, 0x10, 0xFE, 0xB6, 0xA7, 0x5B, 0x27,
        0x4C, 0x22, 0x15, 0xF9, 0x00, 0x73, 0x10, 0x3A, 0x29, 0x3B, 0x30, 0xCC, 0x41, 0x86, 0x15, 0x35,
        0xF1, 0x22, 0x22, 0x67, 0xC0, 0xEB, 0xA1, 0xD9, 0x9A, 0x12, 0x3B, 0x98, 0x70, 0x22, 0x3D, 0x6E,
        0x08, 0xF7, 0xF4, 0x98, 0xFE, 0x5A, 0xD3, 0x80, 0xC8, 0xC3, 0x78, 0x8F, 0xBB, 0xAD, 0x50, 0xF0,
        0xF3, 0x8A, 0xDB, 0x9B, 0xD1, 0xBD, 0xB3, 0x57, 0x67, 0xC4, 0x7B, 0xB2, 0xF1, 0x1E, 0x0B, 0xF7,
        0xF8, 0xC0, 0xEF, 0x31, 0x25, 0x3A, 0x4A, 0xE3, 0xC9, 0xDC, 0xAC, 0x52, 0x19, 0xC4, 0xC9, 0xBE,
        0x83, 0xC3, 0xDC, 0x53, 0xEC, 0xD7, 0xD1, 0x64, 0xF8, 0x39, 0x57, 0xBA, 0x84, 0x62, 0xF1, 0xEA,
        0x5E, 0x12, 0x9D, 0xF8, 0x59, 0x3D, 0xAB, 0x07, 0xBC, 0x62, 0x6F, 0x86, 0x4E, 0x41, 0x54, 0x23,
        0xB4, 0xFE, 0x3A, 0xB7, 0x1C, 0xFC, 0x86, 0x24, 0x69, 0xB8, 0x5E, 0xB7, 0x17, 0xA6, 0xA8, 0x0B,
        0xD8, 0x5C, 0x8B, 0x6E, 0x74, 0x70, 0xD9, 0x35, 0xBB, 0xEF, 0xAF, 0xBA, 0xD5, 0xCB, 0x6D, 0x21,
        0x38, 0x75, 0xC1, 0x77, 0x58, 0xC1, 0x76, 0xA6, 0x3D, 0xE7, 0xB7, 0x0A, 0x08, 0x55, 0x9D, 0xDA,
        0x2B, 0x12, 0xC1, 0xAE, 0xDE, 0x27, 0xB0, 0x5D, 0x9B, 0x49, 0xDD, 0x76, 0xAC, 0xD0, 0xAE, 0x55,
        0x61, 0x7C, 0x36, 0xE4, 0x2A, 0x0B, 0xC7, 0x7F, 0xA4, 0x8C, 0x86, 0xDE, 0x39, 0x79, 0x5C, 0xE6,
        0x5B, 0xE7, 0xFF, 0x80, 0x45, 0xD7, 0xD9, 0xDE, 0xF9, 0xC2, 0xAC, 0x50, 0x84, 0xA7, 0xD9, 0x13,
        0x95, 0xC9, 0xEB, 0x6B, 0x7D, 0x66, 0x1E, 0x88, 0xFE, 0xA4, 0xE4, 0xC9, 0x8F, 0x00, 0xF1, 0x9F,
        0x3F, 0x8C, 0x04, 0x5F, 0x30, 0xDF, 0x43, 0x7A, 0x73, 0x27, 0xAD, 0x1D, 0x90, 0x79, 0x36, 0x95,
        0x1F, 0xCE, 0x4D, 0xBA, 0xED, 0x28, 0x93, 0xD5, 0x08, 0xA4, 0x0B, 0x5A, 0xCA, 0x42, 0x9D, 0x84,
        0x66, 0x85, 0x8B, 0xCF, 0x25, 0xED, 0xB8, 0x91, 0x88, 0x04, 0x4F, 0x87, 0xE6, 0xBC, 0xA8, 0x6D,
        0xAE, 0xA4, 0x8F, 0x5E, 0x30, 0xB6, 0x39, 0x45, 0xDD, 0x78, 0x49, 0x08, 0xC5, 0x78, 0x72, 0x02,
        0x13, 0xB3, 0xA2, 0x90, 0x17, 0x1D, 0xA3, 0xC6, 0xD1, 0xD1, 0x77, 0x20, 0x0C, 0x54, 0x05, 0x15,
        0xB3, 0x76, 0x53, 0x33, 0x50, 0x9B, 0xF8, 0xDD, 0x28, 0x62, 0x27, 0x02, 0x97, 0xEF, 0xE7, 0x21,
        0x0A, 0x70, 0x5D, 0x84, 0x44, 0xAA, 0x38, 0x0E, 0xB4, 0xDE, 0xCA, 0xFA, 0x22, 0x98, 0x96, 0xF5,
        0x8F, 0x4B, 0xA5, 0xF9, 0xAF, 0xDE, 0x87, 0xCD, 0x70, 0x68, 0x2B, 0xCB, 0x28, 0xA1, 0x89, 0x2E,
        0x6D, 0xB3, 0x68, 0xA0, 0xB6, 0xD9, 0x64, 0xDA, 0xF9, 0xD9, 0xCB, 0xE7, 0x04, 0x33, 0xF2, 0xB8,
        0xCA, 0xDC, 0x61, 0xFC, 0x63, 0x7E, 0xDA, 0xD2, 0x27, 0x36, 0x44, 0xC1, 0x6D, 0xA0, 0xDB, 0xBD,
        0xB3, 0x0F, 0xD6, 0xF1, 0x0D, 0x18, 0xA6, 0x6F, 0x5B, 0xD7, 0x4F, 0xE5, 0xCA, 0xEE, 0xA5, 0xCE,
        0x5C, 0xB1, 0x52, 0x2F, 0xB5, 0x0F, 0xBF, 0xD3, 0x19, 0x5A, 0x65, 0x6E, 0x4B, 0xE5, 0xC8, 0x37,
        0x27, 0xF8, 0x7A, 0x4D, 0xA3, 0x3E, 0x33, 0x37, 0xDE, 0x16, 0x03, 0x1A, 0xC6, 0x2E, 0x87, 0x01,
        0xAE, 0x6B, 0xB7, 0x39, 0xBD, 0xE8, 0x17, 0x9B, 0x58, 0x4B, 0x01, 0x82, 0xD6, 0x09, 0x50, 0xBE,
        0xF3, 0x78, 0x2D, 0xB2, 0xB8, 0x8B, 0x17, 0x50, 0x02, 0x03, 0xFE, 0x1F, 0x45, 0x76, 0xF7, 0xD6,
        0x63, 0xCA, 0x85, 0x10, 0x3A, 0x61, 0x6D, 0xD2, 0x69, 0x96, 0x5E, 0x64, 0x09, 0xE4, 0x80, 0xC2,
        0x23, 0x63, 0x2E, 0x46, 0xF2, 0x3D, 0x4C, 0xE1, 0x11, 0xD5, 0x8F, 0x33, 0xBE, 0x10, 0x25, 0x8F,
        0x11, 0x7D, 0x90, 0xCC, 0x3A, 0xA0, 0x47, 0x09, 0xD7, 0xA4, 0x3B, 0x77, 0x96, 0x61, 0xFE, 0x8D,
        0xDB, 0x0A, 0x1F, 0x1B, 0xCC, 0x44, 0x32, 0x65, 0x2B, 0xB9, 0x7F, 0x3C, 0x75, 0x58, 0x52, 0x82,
        0x48, 0x50, 0xE5, 0xE7, 0x34, 0x53, 0xFD, 0x7A, 0x17, 0xF8, 0xE1, 0x91, 0x73, 0x65, 0x82, 0xAD,
        0xDB, 0x1F, 0xA3, 0xA5, 0x19, 0x90, 0x38, 0xDF, 0x0A, 0x0D, 0x96, 0x69, 0x0D, 0xB9, 0xA6, 0x88,
        0x3C, 0xC0, 0x02, 0xEB, 0x0A, 0xBF, 0x03, 0x09, 0x9D, 0x2F, 0x39, 0xBC, 0x73, 0x97, 0x65, 0xB3,
        0x79, 0x5B, 0x69, 0xE4, 0xAE, 0xF9, 0x6F, 0x32, 0xC8, 0x47, 0xBF, 0x14, 0x8F, 0x6E, 0x78, 0xDE
    };

    ///
    /// @brief Encrypts or decrypts the player data.
    /// @details The idea behind encrypting player data is not about stopping serious
    ///  cheaters from doing their thing. It's about discouraging otherwise honest
    ///  players from editing a very tempting text file.
    /// @param pdata        Encrypted/decrypted player data.
    /// @returns Decrypted/encrypted player data.
    ///
    std::string xorcrypt(std::string pdata) const
    {
        auto ki = edkey.cbegin(); // Key Iterator

        for (char &c: pdata)
        {
            if (ki == edkey.cend())
                ki = edkey.cbegin();

            c ^= *ki++;
        }

        return pdata;
    }

    ///
    /// @brief Reads the player name and race data into the `alltimes` collection.
    /// @param [in] pname       Player name.
    /// @param [in] pdata       Player data.
    ///
    bool readPlayerData(const std::string &pname, const std::string &pdata)
    {
        unsigned long int nu = 0; // Number of Unlocks
        std::string ts; // Temporary String
        RaceData rd(pname);

#define decrypt xorcrypt
        std::istringstream sspdata(decrypt(pdata));
#undef decrypt

        GETLINE_SKIP_EMPTY_LINES_B(sspdata, ts);
        nu = std::stoul(ts);

        while (nu-- != 0)
        {
            GETLINE_SKIP_EMPTY_LINES_B(sspdata, ts);
            allunlocks[pname].insert(ts);
        }

        while (sspdata >> rd)
            alltimes.insert({rd.mapname, rd});

        return static_cast<bool> (sspdata);
    }

    ///
    /// @brief Writes the player's race data to his .PLAYER file.
    /// @note If `pname` is an empty string, no data is saved.
    /// @todo Should return `bool` and check for stream errors.
    /// @param [in] pname       Player name.
    ///
    void writePlayerData(const std::string &pname) const
    {
        if (pname.empty())
            return;

        if (PHYSFS_isInit() == 0)
            return;

        std::string pfname = searchdir + '/' + pname + ".player"; // Player Filename
        std::ostringstream sspdata;

        // save unlock data
        if (allunlocks.count(pname) != 0)
        {
            sspdata << allunlocks.at(pname).size() << '\n';

            for (const std::string &s: allunlocks.at(pname))
                sspdata << s << '\n';
        }
        else
            sspdata << 0 << '\n';

        sspdata << '\n';

        // save race data
        for (const auto &p: alltimes)
            if (p.second.playername == pname)
                sspdata << p.second;

#define encrypt xorcrypt
        sspdata.str(encrypt(sspdata.str()));
#undef encrypt

        PHYSFS_File *pfile = PHYSFS_openWrite(pfname.c_str());

#ifndef NDEBUG
        if (pfile == nullptr)
        {
            std::clog << "pfname is \"" << pfname << "\"\n";
            std::clog << "PhysFS error: " << PHYSFS_getLastError() << std::endl;
            return;
        }
#endif

        PHYSFS_write(pfile, sspdata.str().data(), sizeof (char), sspdata.str().size());
        PHYSFS_close(pfile);
    }

    std::unordered_multimap<std::string, RaceData> alltimes;    ///< All times for all maps.
    std::unordered_map<std::string, UnlockData> allunlocks;     ///< All unlock data for all players.
    std::vector<TimeEntry> currenttimes;                        ///< Selected times, for current map.
    std::string searchdir;                                      ///< Directory where player profiles are.
    std::string playername;                                     ///< Name of the current player.
    long int skipSaves = 5;                                     ///< Number of saves to skip, -1 means "save all by dtor".
    mutable long int sc = 0;                                    ///< Skip counter.
};

#undef GETLINE_SKIP_EMPTY_LINES
#undef GETLINE_SKIP_EMPTY_LINES_B
