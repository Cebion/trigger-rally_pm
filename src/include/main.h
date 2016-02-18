
// main.h

// Copyright 2004-2006 Jasmine Langridge, jas@jareiko.net
// License: GPL version 2 (see included gpl.txt)

#pragma once

#include <pengine.h>

#include <psim.h>

#include <unordered_map>


// Forward declaration for TriggerGame to use
class MainApp;




struct CheckPoint {
  vec3f pt;
  
  CheckPoint(const vec3f &_pt) : pt(_pt) { }
};

///
/// @brief Holds codriver checkpoint information.
///
struct CodriverCP
{
    vec3f pt;           ///< Where this checkpoint is on the map.
    std::string notes;  ///< What the codriver should say.

    CodriverCP(const vec3f &pt, const std::string &notes):
        pt(pt),
        notes(notes)
    {
    }
};

/*
struct AIDriver {
  int vehic;
  
  AIDriver(int v) : vehic(v) { }
};
*/


#define GS_COUNTDOWN        0
#define GS_RACING           1
#define GS_FINISHED         2

#define GF_NOT_FINISHED     0
#define GF_PASS             1
#define GF_FAIL             2


class TriggerGame {
  friend class MainApp;

public:

    /// Sets the codriver checkpoints to "ordered" (true) or "free" (false).
    bool cdcheckpt_ordered = false;

private:
  MainApp *app;
  
  PSim *sim;
  
  int randomseed;
  
  std::vector<PVehicle *> vehicle;
  
  //std::vector<AIDriver> aid;
  
  PTerrain *terrain;
  
  std::vector<CheckPoint> checkpt;
  std::vector<CodriverCP> codrivercheckpt;
  
  PCodriverVoice cdvoice;
  PCodriverSigns cdsigns;

  float offroadtime_begin   = 0.0f;
  float offroadtime_end     = 0.0f;
  float offroadtime_total   = 0.0f;

public:

    const float offroadtime_penalty_multiplier = 2.5f;

    ///
    /// @brief Used for "real-time" counting of seconds, to scare the player.
    ///
    float getOffroadTime() const
    {
        return offroadtime_total + (coursetime - offroadtime_begin);
    }

private:

  int gamestate;
  
  float coursetime;
  float othertime; // used pre and post race
  float cptime; // checkpoint time
  float targettime; // the time needed to win
  
  std::string comment; // level comment string
  
  vec3f start_pos;
  quatf start_ori;
  
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
  
  void renderCodriverSigns()
  {
      cdsigns.render(coursetime);
  }
  
  bool loadVehicles();
  
  bool loadLevel(const std::string &filename);
  
  void chooseVehicle(PVehicleType *type);
  
  void tick(float delta);
  
  bool isFinished() { return (gamestate == GS_FINISHED) && (othertime <= 0.0f); }
  int getFinishState() {
    if (gamestate != GS_FINISHED) return GF_NOT_FINISHED;
    if (coursetime + offroadtime_total * offroadtime_penalty_multiplier <= targettime) return GF_PASS;
    else return GF_FAIL;
  }
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
  void tick(float delta) {
    
    PParticleSystem::tick(delta);
    
    for (unsigned int i=0; i<part.size(); i++) {
      PULLTOWARD(part[i].linvel, vec3f::zero(), delta * 25.0f);
    }
  }
};


struct RainDrop {
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

struct UserControl {
  enum {
    TypeUnassigned,
    TypeKey,
    TypeJoyButton,
    TypeJoyAxis
  } type;
  union {
    struct {
      SDLKey sym;
    } key;
    struct {
      int button;
    } joybutton;
    struct {
      int axis;
      float sign;
      float deadzone;
      float maxrange;
    } joyaxis; // more like half-axis, really
  };
  
  float value; // from 0.0 to 1.0 depending on activation level
};

class MainApp : public PApp {
public:
  enum Speedunit {
    mph,
    kph
  };

  enum Speedstyle {
    analogue,
    hybrid
  };

  enum SnowFlakeType
  {
      point,
      square,
      textured
  };

  // TODO: these shouldn't be static+public, but the simplicity is worth it for now
  static GLfloat    cfg_anisotropy;     ///< Anisotropic filter quality.
  static bool       cfg_foliage;        ///< Foliage on/off flag.
  static bool       cfg_weather;        ///< Weather on/off flag.

private:
  
  int appstate;
  
  // TODO: use `aspect` instead of these?
  // TODO: type should be GLdouble instead of double
  double hratio; ///< Horizontal ratio.
  double vratio; ///< Vertical ratio.
  
  UnlockData player_unlocks; ///< Unlocks for current player, see `HiScore1`.
  
  // Config settings
  
  std::string cfg_playername;
  
  int cfg_video_cx, cfg_video_cy;
  bool cfg_video_fullscreen;
  
  float cfg_drivingassist;
  bool cfg_enable_sound;
  bool cfg_enable_codriversigns;
  
  /// Basic volume control.
  float cfg_volume_engine       = 0.33f;    ///< Engine.
  float cfg_volume_sfx          = 1.00f;    ///< Sound effects (wind, gear change, crash, skid, etc.)
  float cfg_volume_codriver     = 1.00f;    ///< Codriver voice.
  
  /// Search paths for the data directory, as read from the configuration.
  std::list<std::string> cfg_datadirs;
  
  /// Name of the codriver whose words to load.
  /// Must be a valid directory in /data/sounds/codriver/.
  std::string cfg_codrivername;

  /// Name of the codriver icon set to load.
  /// Must be a valid directory in /data/textures/CodriverSigns/.
  std::string cfg_codriversigns;

  Speedunit cfg_speed_unit;
  Speedstyle cfg_speed_style;
  float hud_speedo_start_deg;
  float hud_speedo_mps_deg_mult;
  float hud_speedo_mps_speed_mult;

    SnowFlakeType cfg_snowflaketype = SnowFlakeType::point;
    
    bool cfg_dirteffect = true;

  enum Action {
    ActionForward,
    ActionBack,
    ActionLeft,
    ActionRight,
    ActionHandbrake,
    ActionRecover,
    ActionCamMode,
    ActionCamLeft,
    ActionCamRight,
    ActionShowMap,
    ActionShowUi,
    ActionShowCheckpoint,
    ActionCount
  };
  struct {
    std::string action_name[ActionCount];
    UserControl map[ActionCount];
  } ctrl;
  
  //
  
  float splashtimeout;
  
  //
  
  std::vector<TriggerLevel> levels;
  std::vector<TriggerEvent> events;
  
  // for level screen
  Gui gui;
public:
  LevelState lss;
private:
  //
  
  HISCORE1_SORT hs_sort_method = HISCORE1_SORT::BY_TOTALTIME_ASC;
  HiScore1 best_times; // FIXME: this belongs in PApp as a protected member
  RaceData race_data;
  std::vector<RaceData> current_times;
  std::vector<RaceDataHL> current_times_hl;

  TriggerGame *game;
  
  PVehicleType *vt_tank;
  
  PTexture *tex_fontDsmNormal,
           *tex_fontDsmOutlined,
           *tex_fontDsmShadowed;
  
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
           *tex_button_prev;
  
  std::unordered_map<std::string, PTexture *> tex_codriversigns;
  std::unordered_map<std::string, PAudioSample *> aud_codriverwords;

  DirtParticleSystem *psys_dirt;
  
  PAudioSample *aud_engine,
               *aud_wind,
               *aud_gearchange,
               *aud_gravel,
               *aud_crash1;
  
  PAudioInstance *audinst_engine, *audinst_wind, *audinst_gravel;
  std::vector<PAudioInstance *> audinst;
  
  float cloudscroll;
  
  vec3f campos, campos_prev;
  quatf camori;
  
  vec3f camvel;
  
  float nextcpangle;
  
  float cprotate;
  
  int cameraview;
  float camera_angle;
  float camera_user_angle;
  
  bool renderowncar; // this is determined from cameraview
  
  bool showmap;

  bool showui;
  
  bool showcheckpoint;

  float crashnoise_timeout;
  
  std::vector<RainDrop> rain;
  std::vector<SnowFlake> snowfall;

  //
  
  int loadscreencount;
  
  float choose_spin;
  
  int choose_type;
  
protected:
  void renderWater();
  void renderSky(const mat44f &cammat);
  
  bool startGame(const std::string &filename);
  void startGame2();
  void endGame(int gamestate);
  
  void quitGame() {
    endGame(GF_NOT_FINISHED);
    splashtimeout = 0.0f;
    appstate = AS_END_SCREEN;
  }
  
  void levelScreenAction(int action, int index);
  void handleLevelScreenKey(const SDL_KeyboardEvent &ke);
  void finishRace(int gamestate, float coursetime);
  
public:
  MainApp(const std::string &title, const std::string &name):
    PApp(title, name),
    best_times("/players")
  {
  }
  //MainApp::~MainApp(); // should not have destructor, use unload
  
  float getCodriverVolume() const
  {
      return cfg_volume_codriver;
  }
  
  void config();
  void load();
  void unload();
  
  void copyDefaultPlayers() const;
  void loadConfig();
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
  
  void keyEvent(const SDL_KeyboardEvent &ke);
  void mouseMoveEvent(int dx, int dy);
  void cursorMoveEvent(int posx, int posy);
  void mouseButtonEvent(const SDL_MouseButtonEvent &mbe);
  void joyButtonEvent(int which, int button, bool down);

    std::unordered_map<std::string, PAudioSample *> getCodriverWords() const
    {
        return aud_codriverwords;
    }
    
    std::unordered_map<std::string, PTexture *> getCodriverSigns() const
    {
        return tex_codriversigns;
    }
};

