//
// Copyright (C) 2004-2006 Jasmine Langridge, jas@jareiko.net
// Copyright (C) 2017 Emanuele Sorce, emanuele.sorce@hotmail.com
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

#include "app.h"
#include "codriver.h"
#include "config.h"
#include "control.h"
#include "ghost.h"
#include "hiscore1.h"
#include "option.h"
#include "rigidity.h"
#include "vmath.h"
#include <unordered_map>

// Forward declaration for TriggerGame to use
class MainApp;
class PParticleSystem;
class PSim;
class PTerrain;
class PVehicle;
class PVehicleType;
class PVehicleTypePart;

///
/// @brief This struct stores a checkpoint
/// @details basically just a coordinate
///
struct CheckPoint {
	vec3f pt;

	CheckPoint(const vec3f &_pt) : pt(_pt) { }
};

///
/// @brief Holds codriver checkpoint information.
///
struct CodriverCP
{
	// Where this checkpoint is on the map
    vec3f pt;
    // What the codriver should say.
    std::string notes;

    CodriverCP(const vec3f &pt, const std::string &notes):
        pt(pt),
        notes(notes)
    {
    }
};

///
/// @brief The status of the race ending
///
enum class Gamefinish {
	// race not finished yet
	not_finished,
	// race passed
	pass,
	// race failed
	fail
};

///
/// @brief Current state of the game
///
enum class Gamestate {
	// the few seconds before start
	countdown,
	// during racing
	racing,
	// race ended
	finished
};
///
/// @brief Camera view mode
///
enum class CameraMode{
	chase = 0,
	bumper,
	side,
	hood,
	periscope,
	// disabled
	//piggyback,
	count
};

///
/// @brief class containing information about the race is being played
///
class TriggerGame {

	friend class MainApp;

public:

    // Sets the codriver checkpoints to "ordered" (true) or "free" (false).
    bool cdcheckpt_ordered = false;

private:
	MainApp *app;

	// simulation context
	PSim *sim;

	// current state of the race
	Gamestate gamestate;

	int randomseed;

	// the vehicles
	std::vector<PVehicle *> vehicle;

	// User's vehicle
	PVehicle *uservehicle;

	// the terrain
	PTerrain *terrain;

	// Checkpoints
	std::vector<CheckPoint> checkpt;
	// Codriver checkpoints
	std::vector<CodriverCP> codrivercheckpt;

	int number_of_laps = 1;

	// Codriver voice and sign set
	PCodriverVoice cdvoice;
	PCodriverSigns cdsigns;

	// Rigidity map for foliage and road signs
	PRigidity rigidity;

public:
    const float offroadtime_penalty_multiplier = 2.5f;

    ///
    /// @brief Used for "real-time" counting of seconds, to scare the player.
    /// @details it read offroad time of the user vehicle
    ///
    float getOffroadTime() const;

private:

	// Time passed since the race started
	float coursetime;
	// time variable used for pre and post race (i.e. Countdown)
	float othertime;
	// checkpoint time
	float cptime;
	// the time needed to win
	float targettime;

	// level comment string
	std::string comment;

	vec3f start_pos;
	quatf start_ori;

    // used to reset vehicle at last passed checkpoint
    vec3f lastCkptPos;
    quatf lastCkptOri;

	// Structure that stores the current weather
	struct {
		struct {
			std::string texname;
			float scrollrate;
		} cloud;
		struct {
			vec3f color;
			float density;
			float density_sky;
		} fog;
		struct {
			float rain;
			float snowfall;
		} precip;
	} weather;

    ///
    /// @brief Water information.
    /// @todo Maybe remove defaults, used in `game.cpp`.
    ///
    struct
    {
        bool        enabled     = false;    ///< Enables the water?
        float       height      = 0.0f;     ///< The height of the water.
        std::string texname     = "";       ///< Custom water texture.
        bool        useralpha   = false;    ///< Use alpha provided by user?
        bool        fixedalpha  = false;    ///< Use the same alpha for all the water?
        float       alpha       = 1.0f;     ///< Default user alpha value.
    } water;

public:
	std::vector<PVehicleType *> vehiclechoices;

public:
	TriggerGame(MainApp *parent);
	~TriggerGame();

	void resetAtCheckpoint(PVehicle *veh);

	void renderCodriverSigns();

	bool loadVehicles();

	bool loadLevel(const std::string &filename);

	void chooseVehicle(PVehicleType *type);

	void tick(float delta);

	bool isFinished() const;

	bool isRacing() const;

	Gamefinish getFinishState();
};


#include "menu.h"


#define AS_LOAD_1           1
#define AS_LOAD_2           2
#define AS_LOAD_3           3
#define AS_LEVEL_SCREEN     10
#define AS_CHOOSE_VEHICLE   11
#define AS_IN_GAME          12
#define AS_END_SCREEN       13



struct TriggerLevel {
	std::string filename, name, description, comment, author, targettime, targettimeshort;

	float targettimefloat;

	PTexture *tex_minimap = nullptr;
	PTexture *tex_screenshot = nullptr;
};

///
/// @brief an Event with his levels
///
struct TriggerEvent {
	std::string filename, name, comment, author, totaltime;

	bool locked = false;
	UnlockData unlocks; ///< @see `HiScore1`

	// Note that levels are not linked to... they are
	// stored in the event because an event may have
	// "hidden" levels not otherwise available

	std::vector<TriggerLevel> levels;
};


class DirtParticleSystem : public PParticleSystem {
public:

	void tick(float delta)
	{
		PParticleSystem::tick(delta);

		for (unsigned int i=0; i<part.size(); i++)
		{
			PULLTOWARD(part[i].linvel, vec3f::zero(), delta * 25.0f);
		}
	}
};


struct RainDrop
{
	vec3f drop_pt, drop_vect;
	float life, prevlife;
};

struct SnowFlake
{
    vec3f drop_pt;
    vec3f drop_vect;
    float life;
    float prevlife;
};

///
/// @brief this class is the whole Trigger Rally game. Create a MainApp object is the only thing main() does
///
class MainApp : public PApp {
private:
	int appstate;

	// TODO: use `aspect` instead of these?
	// TODO: type should be GLdouble instead of double
	double hratio; ///< Horizontal ratio.
	double vratio; ///< Vertical ratio.

	UnlockData player_unlocks; ///< Unlocks for current player, see `HiScore1`.

public:
    ///
    /// @brief Checks if the given data was unlocked by the current player.
    /// @param [in] udata       Unlock data to be checked.
    /// @retval true            The player has unlocked the data.
    /// @retval false           The player has not unlocked the data.
    ///
    bool isUnlockedByPlayer(const std::string &udata) const
    {
        return player_unlocks.count(udata) != 0;
    }

    ///
    /// @brief Checks if the given vehicle is locked.
    /// @param [in] vefi        Filename of the vehicle.
    /// @retval true            Vehicle is marked as locked.
    /// @retval false           Vehicle is not marked as locked.
    ///
    bool isVehicleLocked(const std::string &vefi) const
    {
        XMLDocument xmlfile;
        XMLElement *rootelem = PUtil::loadRootElement(xmlfile, vefi, "vehicle");

        if (rootelem == nullptr)
        {
            PUtil::outLog() << "Couldn't read vehicle \"" << vefi << "\"" << std::endl;
            return false;
        }

        const char *val = rootelem->Attribute("locked");

        if (val != nullptr && std::string(val) == "yes")
            return true;

        return false;
    }

private:
	float splashtimeout;

	std::vector<TriggerLevel> levels;
	std::vector<TriggerEvent> events;

	std::string getVehicleUnlockEvent(const std::string &vehiclename) const;

	// for level screen
	Gui gui;
	// for option screen
	POption option;
    // for control screen
    PControl control;

public:
	LevelState lss;

	// for handling configuration
	PConfig cfg;

private:
	HISCORE1_SORT hs_sort_method = HISCORE1_SORT::BY_TOTALTIME_ASC;
	RaceData race_data;
	std::vector<TimeEntry> current_times;

	TriggerGame *game;

	PVehicleType *vt_tank;

	PTexture *tex_fontSourceCodeBold,
			*tex_fontSourceCodeOutlined,
			*tex_fontSourceCodeShadowed;

	PTexture *tex_detail,
			*tex_sky[1],
			*tex_water,
			*tex_waterdefault,
			*tex_dirt,
			*tex_snowflake,
			*tex_shadow,
			*tex_hud_revs,
			*tex_hud_revneedle,
			*tex_hud_life,
			*tex_hud_offroad,
			*tex_loading_screen,
			*tex_splash_screen,
			*tex_end_screen,
			*tex_race_no_screenshot,
			*tex_race_no_minimap,
			*tex_button_next,
			*tex_button_prev,
			*tex_damage_front_left,
			*tex_damage_front_right,
			*tex_damage_rear_left,
			*tex_damage_rear_right;

	std::unordered_map<std::string, PTexture *> tex_codriversigns;
	std::unordered_map<std::string, PAudioSample *> aud_codriverwords;

	DirtParticleSystem *psys_dirt;

	// Tones
	PAudioSample *aud_engine,
				*aud_wind,
				*aud_shiftup,
				*aud_shiftdown,
				*aud_gravel,
				*aud_crash1;

	// Audio instances
	PAudioInstance *audinst_engine, *audinst_wind, *audinst_gravel;

	std::vector<PAudioInstance *> audinst;

	float cloudscroll;

	vec3f campos, campos_prev;
	quatf camori;

	vec3f camvel;

	float nextcpangle;

	float cprotate;

	// what view mode the camera is
	CameraMode cameraview;
	float camera_angle;
	float camera_user_angle;

	// If with the the vehicle should be rendered (depends on cameraview)
	bool renderowncar;

	bool showmap;

	bool pauserace;

	bool showui;

	bool showcheckpoint;

	float crashnoise_timeout;

	std::vector<RainDrop> rain;
	std::vector<SnowFlake> snowfall;

	int loadscreencount;

	float choose_spin;

	int choose_type;

	// Time and count to calculate frames per second
	float fpstime;
	unsigned int fpscount;
	float fps;
	void tickCalculateFps(float delta);

	// Record and display of ghost vehicles
	PGhost ghost;

	void loadCodriversigns();
	void loadCodrivername();
    void reloadAll();

protected:
	void renderWater();
	void renderSky(const mat44f &cammat);

	bool startGame(const std::string &filename);
	void toggleSounds(bool to);
	void initAudio();
	void endGame(Gamefinish state);

	void quitGame()
	{
		endGame(Gamefinish::not_finished);
		splashtimeout = 0.0f;
		appstate = AS_END_SCREEN;
	}

	void levelScreenAction(int action, int index);
	void handleLevelScreenKey(const SDL_KeyboardEvent &ke);
	void finishRace(Gamefinish state, float coursetime);

public:
	MainApp(const std::string &title, const std::string &name) :
            PApp(title, name),
            option(gui, cfg),
            control(gui, cfg),
            cfg(this),
            ghost(0.1f)
	{
	}
	//MainApp::~MainApp(); // should not have destructor, use unload

	float getCodriverVolume() const;

	void config();
	void load();
	void unload();

	void copyDefaultPlayers() const;
	bool loadAll();
	bool loadLevelsAndEvents();
	bool loadLevel(TriggerLevel &tl);

	void calcScreenRatios();

	void tick(float delta);

	void resize();
	void render(float eyetranslation);

	void renderStateLoading(float eyetranslation);
	void renderStateEnd(float eyetranslation);
	void tickStateLevel(float delta);
	void renderStateLevel(float eyetranslation);
	void tickStateChoose(float delta);
	void renderStateChoose(float eyetranslation);
	void tickStateGame(float delta);
	void renderStateGame(float eyetranslation);

	void renderDamageIndicator(
	    const PTexture *texture, float posx, float posy, float scalex, float scaley, float damage);
	void renderDamageIndicatorGroup();
	void renderVehiclePart(const PVehicleType &type, const PVehiclePart &part,
	    const PVehicleTypePart &typepart, float alpha);

	void keyEvent(const SDL_KeyboardEvent &ke);
	void mouseMoveEvent(int dx, int dy);
	void cursorMoveEvent(int posx, int posy);
	void mouseButtonEvent(const SDL_MouseButtonEvent &mbe);
	void joyButtonEvent(int which, int button, bool down);
	bool joyAxisEvent(int which, int axis, float value, bool down);

	float getCtrlActionBackValue();
	int getVehicleCurrentGear();

    std::unordered_map<std::string, PAudioSample *> getCodriverWords() const
    {
        return aud_codriverwords;
    }

    std::unordered_map<std::string, PTexture *> getCodriverSigns() const
    {
        return tex_codriversigns;
    }

    PCodriverUserConfig getCodriverUserConfig() const;
};
