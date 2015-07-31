
// menu.cpp

// Copyright 2004-2006 Jasmine Langridge, jas@jareiko.net
// License: GPL version 2 (see included gpl.txt)

#include <sstream>
#include "main.h"

const int MAX_RACES_ON_SCREEN = 12;


void MainApp::levelScreenAction(int action, int index)
{
  appstate = AS_LEVEL_SCREEN;
  
  switch (action) {
  case AA_INIT:
    lss.state = AM_TOP;
    break;
  case AA_RESUME:
    // lss.state should be AM_TOP_EVT_PREP, continuing event
    break;
  case AA_GO_TOP:
    lss.state = AM_TOP;
    break;
  case AA_GO_EVT:
    if (lss.state == AM_TOP_EVT_PREP &&
      lss.currentlevel > 0 &&
      lss.currentlevel < (int)events[lss.currentevent].levels.size()) {
      lss.state = AM_TOP_EVT_ABANDON;
    } else {
      lss.state = AM_TOP_EVT;
    }
    break;
  case AA_PICK_EVT:
    lss.currentevent = index;
    lss.currentlevel = 0;
    lss.livesleft = 3;
    lss.leveltimes.clear();
    lss.totaltime = 0.0f;
    lss.state = AM_TOP_EVT_PREP;
    break;
  case AA_RESUME_EVT:
    lss.state = AM_TOP_EVT_PREP;
    break;
  case AA_RESTART_EVT:
    lss.currentlevel = 0;
    lss.livesleft = 3;
    lss.leveltimes.clear();
    lss.totaltime = 0.0f;
    lss.state = AM_TOP_EVT_PREP;
    break;
  case AA_GO_PRAC:
    lss.state = AM_TOP_PRAC;
    break;
  case AA_PICK_PRAC:
    lss.currentevent = index;
    lss.state = AM_TOP_PRAC_SEL;
    break;
  case AA_PICK_PRAC_LVL:
    lss.currentlevel = index;
    lss.state = AM_TOP_PRAC_SEL_PREP;
    break;
  case AA_GO_LVL:
    lss.state = AM_TOP_LVL;
    break;
  case AA_PICK_LVL:
    lss.currentlevel = index;
    lss.state = AM_TOP_LVL_PREP;
    break;
  case AA_GO_QUIT:
    lss.state = AM_TOP_QUIT;
    break;
  case AA_QUIT_CONFIRM:
    quitGame();
    break;
    
  case AA_START_EVT:
    startGame(events[lss.currentevent].levels[lss.currentlevel].filename);
    return;
  case AA_START_PRAC:
    startGame(events[lss.currentevent].levels[lss.currentlevel].filename);
    return;
  case AA_START_LVL:
    startGame(levels[lss.currentlevel].filename);
    return;
  
  default:
    PUtil::outLog() << "ERROR: invalid action code " << action << std::endl;
    requestExit();
    return;
  }
  
  gui.setSSRender(getSSRender());
  gui.setFont(tex_fontDsmShadowed);
  grabMouse(false);
  gui.clear();
  gui.addLabel(10.0f,570.0f, "Trigger Rally", PTEXT_HZA_LEFT | PTEXT_VTA_CENTER, 30.0f, LabelStyle::Weak);

  switch (lss.state) {
  case AM_TOP:
    gui.makeClickable(
      gui.addLabel(400.0f,350.0f, "events", PTEXT_HZA_CENTER | PTEXT_VTA_CENTER, 40.0f), AA_GO_EVT, 0);
    gui.makeClickable(
      gui.addLabel(400.0f,300.0f, "practice", PTEXT_HZA_CENTER | PTEXT_VTA_CENTER, 40.0f), AA_GO_PRAC, 0);
    gui.makeClickable(
      gui.addLabel(400.0f,250.0f, "single race", PTEXT_HZA_CENTER | PTEXT_VTA_CENTER, 40.0f), AA_GO_LVL, 0);
    gui.makeClickable(
      gui.addLabel(10.0f,30.0f, "quit", PTEXT_HZA_LEFT | PTEXT_VTA_CENTER, 40.0f), AA_GO_QUIT, 0);

    gui.addLabel(790.0f, 570.0f, "version " PACKAGE_VERSION, PTEXT_HZA_RIGHT | PTEXT_VTA_CENTER, 30.0f, LabelStyle::Weak);
    gui.addLabel(790.0f, 30.0f, "Build: " __DATE__ " at " __TIME__, PTEXT_HZA_RIGHT | PTEXT_VTA_CENTER, 20.0f, LabelStyle::Weak);
    break;
  case AM_TOP_EVT:
  {
    gui.makeClickable(
      gui.addLabel(10.0f,30.0f, "back", PTEXT_HZA_LEFT | PTEXT_VTA_CENTER, 40.0f), AA_GO_TOP, 0);
    gui.addLabel(100.0f,470.0f, "Choose Event:", PTEXT_HZA_LEFT | PTEXT_VTA_CENTER, 30.0f, LabelStyle::Strong);
    
      int firstraceindex = index;
      const int prevbutton = gui.addGraphic(20.0f, 275.0f, 50.0f, 50.0f, tex_button_prev, GraphicStyle::Button);
      const int nextbutton = gui.addGraphic(730.0f, 275.0f, 50.0f, 50.0f, tex_button_next, GraphicStyle::Button);

      if (firstraceindex > 0) { // FIXME: originally was AA_GO_PRAC?
        gui.makeClickable(prevbutton, AA_GO_EVT, firstraceindex - MAX_RACES_ON_SCREEN);
      }

      int racesonscreencount = events.size() - firstraceindex;

      if (racesonscreencount > MAX_RACES_ON_SCREEN) {
        racesonscreencount = MAX_RACES_ON_SCREEN;
        gui.makeClickable(nextbutton, AA_GO_EVT, firstraceindex + MAX_RACES_ON_SCREEN);
      }

      std::stringstream racecountmsg;
      racecountmsg << "events " << firstraceindex + 1 << '-' << firstraceindex + racesonscreencount << '/' << events.size();
      gui.addLabel(790.0f, 570.0f, racecountmsg.str(), PTEXT_HZA_RIGHT | PTEXT_VTA_CENTER, 20.0f, LabelStyle::Weak);
        gui.addLabel(700, 470, "races (timelimit)", PTEXT_HZA_RIGHT | PTEXT_VTA_CENTER, 20);

    for (int i = firstraceindex; i < firstraceindex + racesonscreencount; i++) {
      gui.makeClickable(
        gui.addLabel(100.0f,440.0f - (float)(i - firstraceindex) * 30.0f,
        events[i].name, PTEXT_HZA_LEFT | PTEXT_VTA_TOP, 25.0f, LabelStyle::List), AA_PICK_EVT, i);
        
      gui.addLabel(700.0f, 440.0f - (float)(i - firstraceindex) * 30.0f,
            PUtil::formatInt(events[i].levels.size()) + " (" + events[i].totaltime + ')',
            PTEXT_HZA_RIGHT | PTEXT_VTA_TOP, 25.0f, LabelStyle::List);
    }
    break;
  }
  case AM_TOP_EVT_PREP:
    gui.makeClickable(
      gui.addLabel(10.0f, 10.0f, "back", PTEXT_HZA_LEFT | PTEXT_VTA_BOTTOM, 40.0f),
      AA_GO_EVT, 0);
    gui.addLabel(790.0f, 570.0f, events[lss.currentevent].name, PTEXT_HZA_RIGHT | PTEXT_VTA_CENTER, 20.0f, LabelStyle::Weak);
    gui.addLabel(100.0f,470.0f, "Races:", PTEXT_HZA_LEFT | PTEXT_VTA_CENTER, 30.0f, LabelStyle::Strong);
    gui.addLabel(700, 470, "status/time", PTEXT_HZA_RIGHT | PTEXT_VTA_CENTER, 20);

    for (unsigned int i = 0; i < events[lss.currentevent].levels.size(); i++) {
      gui.addLabel(100.0f,440.0f - (float)i * 30.0f,
        events[lss.currentevent].levels[i].name, PTEXT_HZA_LEFT | PTEXT_VTA_TOP, 25.0f, LabelStyle::List);
      std::string infotext = "not yet raced";
      if (lss.currentlevel > (int)i)
        infotext = PUtil::formatTime(lss.leveltimes[i]);
      else if (lss.currentlevel == (int)i)
        infotext = "next";
      gui.addLabel(700.0f,440.0f - (float)i * 30.0f,
        infotext, PTEXT_HZA_RIGHT | PTEXT_VTA_TOP, 25.0f, LabelStyle::List);
    }
    gui.addLabel(700.0f,430.0f - (float)events[lss.currentevent].levels.size() * 30.0f,
      "Total: " + PUtil::formatTime(lss.totaltime), PTEXT_HZA_RIGHT | PTEXT_VTA_TOP, 25.0f);
    if (lss.livesleft <= 0) {
      gui.addLabel(400.0f,150.0f, "No tries remaining", PTEXT_HZA_CENTER | PTEXT_VTA_CENTER, 30.0f);
      gui.makeClickable(
        gui.addLabel(400.0f,100.0f, "Restart Event", PTEXT_HZA_CENTER | PTEXT_VTA_CENTER, 30.0f),
        AA_RESTART_EVT, 0);
    } else if (lss.currentlevel >= (int)events[lss.currentevent].levels.size()) {
      gui.addLabel(400.0f,150.0f, "CONGRATULATIONS!", PTEXT_HZA_CENTER | PTEXT_VTA_CENTER, 40.0f);
      gui.addLabel(400.0f,100.0f, "EVENT COMPLETED", PTEXT_HZA_CENTER | PTEXT_VTA_CENTER, 40.0f);
    } else {
      gui.addLabel(400.0f,10.0f, PUtil::formatInt(lss.livesleft) + " tries remaining",
        PTEXT_HZA_CENTER | PTEXT_VTA_BOTTOM, 20.0f, LabelStyle::Strong);

      for (int i = 0; i < lss.livesleft; i++) {
        gui.addGraphic(325.0f + i * 50.0f,30.0f, 50.0f,50.0f, tex_hud_life);
      }
      gui.makeDefault(
        gui.makeClickable(
          gui.addLabel(790.0f,10.0f, "race", PTEXT_HZA_RIGHT | PTEXT_VTA_BOTTOM, 40.0f),
          AA_START_EVT, 0));
    }
    break;
  case AM_TOP_EVT_ABANDON:
    gui.addLabel(400.0f,350.0f, "Really leave Event?", PTEXT_HZA_CENTER | PTEXT_VTA_TOP, 40.0f, LabelStyle::Strong);
    gui.makeClickable(
      gui.addLabel(300.0f,250.0f, "Yes", PTEXT_HZA_CENTER | PTEXT_VTA_TOP, 40.0f),
      AA_GO_EVT, 0);
    gui.makeClickable(
      gui.addLabel(500.0f,250.0f, "No", PTEXT_HZA_CENTER | PTEXT_VTA_TOP, 40.0f),
      AA_RESUME_EVT, 0);
    break;
  case AM_TOP_PRAC:
  {
    gui.makeClickable(
      gui.addLabel(10.0f, 10.0f, "back", PTEXT_HZA_LEFT | PTEXT_VTA_BOTTOM, 40.0f), AA_GO_TOP, 0);
    gui.addLabel(100.0f,470.0f, "Practice Event:", PTEXT_HZA_LEFT | PTEXT_VTA_CENTER, 30.0f, LabelStyle::Strong);
    
      int firstraceindex = index;
      const int prevbutton = gui.addGraphic(20.0f, 275.0f, 50.0f, 50.0f, tex_button_prev, GraphicStyle::Button);
      const int nextbutton = gui.addGraphic(730.0f, 275.0f, 50.0f, 50.0f, tex_button_next, GraphicStyle::Button);

      if (firstraceindex > 0) {
        gui.makeClickable(prevbutton, AA_GO_PRAC, firstraceindex - MAX_RACES_ON_SCREEN);
      }

      int racesonscreencount = events.size() - firstraceindex;

      if (racesonscreencount > MAX_RACES_ON_SCREEN) {
        racesonscreencount = MAX_RACES_ON_SCREEN;
        gui.makeClickable(nextbutton, AA_GO_PRAC, firstraceindex + MAX_RACES_ON_SCREEN);
      }

      std::stringstream racecountmsg;
      racecountmsg << "events " << firstraceindex + 1 << '-' << firstraceindex + racesonscreencount << '/' << events.size();
      gui.addLabel(790.0f, 570.0f, racecountmsg.str(), PTEXT_HZA_RIGHT | PTEXT_VTA_CENTER, 20.0f, LabelStyle::Weak);
      gui.addLabel(700, 470, "races (timelimit)", PTEXT_HZA_RIGHT | PTEXT_VTA_CENTER, 20);

    for (int i = firstraceindex; i < firstraceindex + racesonscreencount; i++) {
      gui.makeClickable(
        gui.addLabel(100.0f,440.0f - (float)(i - firstraceindex) * 30.0f,
        events[i].name, PTEXT_HZA_LEFT | PTEXT_VTA_TOP, 25.0f, LabelStyle::List), AA_PICK_PRAC, i);
        
      gui.addLabel(700.0f, 440.0f - (float)(i - firstraceindex) * 30.0f,
            PUtil::formatInt(events[i].levels.size()) + " (" + events[i].totaltime + ')',
            PTEXT_HZA_RIGHT | PTEXT_VTA_TOP, 25.0f, LabelStyle::List);
    }
    break;
  }
  case AM_TOP_PRAC_SEL:
  {
    gui.makeClickable(
      gui.addLabel(10.0f, 10.0f, "back", PTEXT_HZA_LEFT | PTEXT_VTA_BOTTOM, 40.0f),
      AA_GO_PRAC, 0);
    gui.addLabel(790.0f, 570.0f, events[lss.currentevent].name, PTEXT_HZA_RIGHT | PTEXT_VTA_CENTER, 20.0f, LabelStyle::Weak);
    gui.addLabel(100.0f,470.0f, "Choose Race:", PTEXT_HZA_LEFT | PTEXT_VTA_CENTER, 30.0f, LabelStyle::Strong);
    gui.addLabel(700, 470, "timelimit", PTEXT_HZA_RIGHT | PTEXT_VTA_CENTER, 20);

    for (unsigned int i = 0; i < events[lss.currentevent].levels.size(); i++) {
        gui.makeClickable(
          gui.addLabel(100.0f, 440.0f - (float)i * 30.0f,
          events[lss.currentevent].levels[i].name, PTEXT_HZA_LEFT | PTEXT_VTA_TOP, 25.0f, LabelStyle::List),
          AA_PICK_PRAC_LVL, i);

        gui.addLabel(700.0f, 440.0f - (float)i * 30.0f,
            events[lss.currentevent].levels[i].targettimeshort, PTEXT_HZA_RIGHT | PTEXT_VTA_TOP, 25.0f, LabelStyle::List);
    }
    break;
  }
  case AM_TOP_PRAC_SEL_PREP:
  {
      const int prevbutton = gui.addGraphic(20.0f, 275.0f, 50.0f, 50.0f, tex_button_prev, GraphicStyle::Button);
      const int nextbutton = gui.addGraphic(730.0f, 275.0f, 50.0f, 50.0f, tex_button_next, GraphicStyle::Button);

      int idxnext = lss.currentlevel + 1;
      int idxprev = lss.currentlevel - 1;

      CLAMP(idxnext, 0, static_cast<int> (events[lss.currentevent].levels.size() - 1));
      CLAMP(idxprev, 0, static_cast<int> (events[lss.currentevent].levels.size() - 1));

    if (lss.currentlevel < static_cast<int> (levels.size() - 1))
      gui.makeClickable(nextbutton, AA_PICK_PRAC_LVL, idxnext);

    if (lss.currentlevel > 0)
      gui.makeClickable(prevbutton, AA_PICK_PRAC_LVL, idxprev);

    gui.makeClickable(
      gui.addLabel(10.0f, 10.0f, "back", PTEXT_HZA_LEFT | PTEXT_VTA_BOTTOM, 40.0f),
      AA_PICK_PRAC, lss.currentevent);
    gui.addLabel(790.0f, 570.0f, events[lss.currentevent].name + " (" +
        PUtil::formatInt(lss.currentlevel + 1) + '/' + PUtil::formatInt(events[lss.currentevent].levels.size()) + ')',
        PTEXT_HZA_RIGHT | PTEXT_VTA_CENTER, 20.0f);
    gui.addLabel(100.0f,500.0f, events[lss.currentevent].levels[lss.currentlevel].name,
        PTEXT_HZA_LEFT | PTEXT_VTA_TOP, 35.0f, LabelStyle::Strong);
    gui.addLabel(100.0f,462.5f,
        std::string("by ") + events[lss.currentevent].levels[lss.currentlevel].author,
        PTEXT_HZA_LEFT | PTEXT_VTA_TOP, 20.0f, LabelStyle::Weak);
    gui.addLabel(700.0f, 462.5f, events[lss.currentevent].levels[lss.currentlevel].targettimeshort,
        PTEXT_HZA_RIGHT | PTEXT_VTA_TOP, 20.0f);

    if (events[lss.currentevent].levels[lss.currentlevel].tex_screenshot != nullptr)
        gui.addGraphic(100, 175, 250.0f * 4/3, 250, events[lss.currentevent].levels[lss.currentlevel].tex_screenshot);
    else
        gui.addGraphic(100, 175, 250.0f * 4/3, 250, tex_race_no_screenshot);

    if (events[lss.currentevent].levels[lss.currentlevel].tex_minimap != nullptr)
        gui.addGraphic(450, 175, 250, 250, events[lss.currentevent].levels[lss.currentlevel].tex_minimap);
    else
        gui.addGraphic(450, 175, 250, 250, tex_race_no_minimap);

    gui.addLabel(100.0f,150.0f, events[lss.currentevent].levels[lss.currentlevel].description,
        PTEXT_HZA_LEFT | PTEXT_VTA_TOP, 20.0f);
    gui.makeDefault(
      gui.makeClickable(
        gui.addLabel(790.0f,10.0f, "race", PTEXT_HZA_RIGHT | PTEXT_VTA_BOTTOM, 40.0f),
        AA_START_PRAC, 0));
    break;
  }
  case AM_TOP_LVL:
    {
      gui.makeClickable(
        gui.addLabel(10.0f, 10.0f, "back", PTEXT_HZA_LEFT | PTEXT_VTA_BOTTOM, 40.0f),
        AA_GO_TOP, 0);
      gui.addLabel(100.0f,470.0f, "Choose Race:", PTEXT_HZA_LEFT | PTEXT_VTA_CENTER, 30.0f, LabelStyle::Strong);

      int firstraceindex = index;
      const int prevbutton = gui.addGraphic(20.0f, 275.0f, 50.0f, 50.0f, tex_button_prev, GraphicStyle::Button);
      const int nextbutton = gui.addGraphic(730.0f, 275.0f, 50.0f, 50.0f, tex_button_next, GraphicStyle::Button);

      if (firstraceindex > 0) {
        gui.makeClickable(prevbutton, AA_GO_LVL, firstraceindex - MAX_RACES_ON_SCREEN);
      }

      int racesonscreencount = levels.size() - firstraceindex;

      if (racesonscreencount > MAX_RACES_ON_SCREEN) {
        racesonscreencount = MAX_RACES_ON_SCREEN;
        gui.makeClickable(nextbutton, AA_GO_LVL, firstraceindex + MAX_RACES_ON_SCREEN);
      }
      std::stringstream racecountmsg;
      racecountmsg << "single races " << firstraceindex + 1 << '-' << firstraceindex + racesonscreencount << '/' << levels.size();
      gui.addLabel(790.0f, 570.0f, racecountmsg.str(), PTEXT_HZA_RIGHT | PTEXT_VTA_CENTER, 20.0f, LabelStyle::Weak);

        gui.addLabel(700, 470, "timelimit", PTEXT_HZA_RIGHT | PTEXT_VTA_CENTER, 20);

      for (int i = firstraceindex; i < firstraceindex + racesonscreencount; i++) {
        gui.makeClickable(
          gui.addLabel(100.0f, 440.0f - (float)(i - firstraceindex) * 30.0f,
          levels[i].name, PTEXT_HZA_LEFT | PTEXT_VTA_TOP, 25.0f, LabelStyle::List),
          AA_PICK_LVL, i);
          
        gui.addLabel(700.0f, 440.0f - (float)(i - firstraceindex) * 30.0f,
            levels[i].targettimeshort, PTEXT_HZA_RIGHT | PTEXT_VTA_TOP, 25.0f, LabelStyle::List);
      }
    }
    break;
  case AM_TOP_LVL_PREP:
  {
      const int prevbutton = gui.addGraphic(20.0f, 275.0f, 50.0f, 50.0f, tex_button_prev, GraphicStyle::Button);
      const int nextbutton = gui.addGraphic(730.0f, 275.0f, 50.0f, 50.0f, tex_button_next, GraphicStyle::Button);
      
      int idxnext = lss.currentlevel + 1;
      int idxprev = lss.currentlevel - 1;

      CLAMP(idxnext, 0, static_cast<int> (levels.size() - 1));
      CLAMP(idxprev, 0, static_cast<int> (levels.size() - 1));
      
      if (lss.currentlevel < static_cast<int> (levels.size() - 1))
        gui.makeClickable(nextbutton, AA_PICK_LVL, idxnext);

      if (lss.currentlevel > 0)
        gui.makeClickable(prevbutton, AA_PICK_LVL, idxprev);
      
    gui.makeClickable(
      gui.addLabel(10.0f, 10.0f, "back", PTEXT_HZA_LEFT | PTEXT_VTA_BOTTOM, 40.0f),
      AA_GO_LVL, (lss.currentlevel / MAX_RACES_ON_SCREEN) * MAX_RACES_ON_SCREEN);

    std::stringstream racenummsg;

    racenummsg << "single race " << lss.currentlevel+1 << '/' << levels.size();
    gui.addLabel(790.0f,570.0f, racenummsg.str(), PTEXT_HZA_RIGHT | PTEXT_VTA_CENTER, 20.0f, LabelStyle::Weak);
    gui.addLabel(100.0f,500.0f, levels[lss.currentlevel].name, PTEXT_HZA_LEFT | PTEXT_VTA_TOP, 35.0f, LabelStyle::Strong);
    gui.addLabel(100.0f,462.5f,
        std::string("by ") + levels[lss.currentlevel].author,
        PTEXT_HZA_LEFT | PTEXT_VTA_TOP, 20.0f, LabelStyle::Weak);
    gui.addLabel(700.0f, 462.5f, levels[lss.currentlevel].targettimeshort, PTEXT_HZA_RIGHT | PTEXT_VTA_TOP, 20.0f);

    if (levels[lss.currentlevel].tex_screenshot != nullptr)
        gui.addGraphic(100, 175, 250.0f * 4/3, 250, levels[lss.currentlevel].tex_screenshot);
    else
        gui.addGraphic(100, 175, 250.0f * 4/3, 250, tex_race_no_screenshot);

    if (levels[lss.currentlevel].tex_minimap != nullptr)
        gui.addGraphic(450, 175, 250, 250, levels[lss.currentlevel].tex_minimap);
    else
        gui.addGraphic(450, 175, 250, 250, tex_race_no_minimap);

    gui.addLabel(100.0f,150.0f, levels[lss.currentlevel].description, PTEXT_HZA_LEFT | PTEXT_VTA_TOP, 20.0f);
    gui.makeDefault(
      gui.makeClickable(
        gui.addLabel(790.0f,10.0f, "race", PTEXT_HZA_RIGHT | PTEXT_VTA_BOTTOM, 40.0f),
        AA_START_LVL, 0));
  }
    break;
  case AM_TOP_QUIT:
    gui.addLabel(400.0f,350.0f, "Really quit?", PTEXT_HZA_CENTER | PTEXT_VTA_TOP, 40.0f, LabelStyle::Strong);
    gui.makeClickable(
      gui.addLabel(300.0f,250.0f, "Yes", PTEXT_HZA_CENTER | PTEXT_VTA_TOP, 40.0f),
      AA_QUIT_CONFIRM, 0);
    gui.makeClickable(
      gui.addLabel(500.0f,250.0f, "No", PTEXT_HZA_CENTER | PTEXT_VTA_TOP, 40.0f),
      AA_GO_TOP, 0);
    break;
  default:
    gui.addLabel(400.0f,300.0f, "Error in menu system, sorry", PTEXT_HZA_CENTER | PTEXT_VTA_TOP, 30.0f, LabelStyle::Strong);
    gui.makeClickable(
      gui.addLabel(400.0f,150.0f, "Go to top menu", PTEXT_HZA_CENTER | PTEXT_VTA_TOP, 30.0f),
      AA_GO_TOP, 0);
    break;
  }
  
  //gui.doLayout();
}

void MainApp::finishRace(int gamestate, float coursetime)
{
  switch (lss.state) {
  case AM_TOP_EVT_PREP:
    switch (gamestate) {
    case GF_PASS:
      lss.leveltimes.resize(events[lss.currentevent].levels.size(), 0.0f);
      lss.leveltimes[lss.currentlevel] += coursetime;
      lss.totaltime += coursetime;
      lss.currentlevel++;
      break;
    case GF_FAIL:
      lss.totaltime += coursetime;
      lss.livesleft--;
      break;
    default:
      break;
    }
    levelScreenAction(AA_RESUME, 0);
    break;
  case AM_TOP_PRAC_SEL_PREP:
    levelScreenAction(AA_PICK_PRAC_LVL, lss.currentlevel);
    break;
  case AM_TOP_LVL_PREP:
    // Calculate the index of first level in the page by truncating the current level index to the nearest 10
    //levelScreenAction(AA_GO_LVL, (lss.currentlevel / MAX_RACES_ON_SCREEN) * MAX_RACES_ON_SCREEN );
    levelScreenAction(AA_PICK_LVL, lss.currentlevel);
    break;
  default:
    PUtil::outLog() << "Race finished in invalid state " << lss.state << std::endl;
    break;
  }
}

void MainApp::tickStateLevel(float delta)
{
  gui.tick(delta);
}

// TODO: fix this code
void MainApp::cursorMoveEvent(int posx, int posy)
{
  if (appstate != AS_LEVEL_SCREEN) return;
  
  const GLdouble margin = (800.0 - 600.0 * cfg_video_cx / cfg_video_cy) / 2.0;
  
  gui.setCursorPos(
    (float)posx / (float)getWidth() * (600.0 * cfg_video_cx / cfg_video_cy) + margin,
    (1.0f - (float)posy / (float)getHeight()) * 600.0f);
}

void MainApp::mouseButtonEvent(const SDL_MouseButtonEvent &mbe)
{
  if (mbe.type != SDL_MOUSEBUTTONDOWN) return;
  
  switch (appstate) {
  case AS_LEVEL_SCREEN:
    break;
  case AS_LOAD_3:
    levelScreenAction(AA_INIT, 0);
    break;
  default:
    return;
  }
  
  // TODO: fix this code
  
  const GLdouble margin = (800.0 - 600.0 * cfg_video_cx / cfg_video_cy) / 2.0;

  int action, index;
  
  if (!gui.getClickAction(action, index)) return;
  
  levelScreenAction(action, index);

  gui.setCursorPos(
    (float)mbe.x / (float)getWidth() * (600.0 * cfg_video_cx / cfg_video_cy) + margin,
    (1.0f - (float)mbe.y / (float)getHeight()) * 600.0f);
}

//
// TODO: use ActionLeft and ActionRight instead of hardcoded right/left arrow
//
void MainApp::handleLevelScreenKey(const SDL_KeyboardEvent &ke)
{
  switch (ke.keysym.sym) {
  case SDLK_ESCAPE:
    switch(lss.state) {
    case AM_TOP:
      levelScreenAction(AA_GO_QUIT, 0);
      break;
    case AM_TOP_EVT_PREP:
    case AM_TOP_EVT_ABANDON:
      levelScreenAction(AA_GO_EVT, 0);
      break;
    case AM_TOP_PRAC_SEL:
      levelScreenAction(AA_GO_PRAC, 0);
      break;
    case AM_TOP_PRAC_SEL_PREP:
      levelScreenAction(AA_PICK_PRAC, lss.currentevent);
      break;
    case AM_TOP_LVL_PREP:
      levelScreenAction(AA_GO_LVL, (lss.currentlevel / MAX_RACES_ON_SCREEN) * MAX_RACES_ON_SCREEN);
      break;
    case AM_TOP_QUIT:
      quitGame();
      break;
    default:
      levelScreenAction(AA_GO_TOP, 0);
      break;
    }
    break;
  case SDLK_RETURN:
  case SDLK_KP_ENTER: {
      int data1, data2;
      
      if (gui.getDefaultAction(data1, data2))
        levelScreenAction(data1, data2);
    } break;

    case SDLK_LEFT:
    {
        switch (lss.state)
        {
            case AM_TOP_LVL_PREP:
            {
                lss.currentlevel -= 1;
                CLAMP(lss.currentlevel, 0, static_cast<int> (levels.size() - 1));
                levelScreenAction(AA_PICK_LVL, lss.currentlevel);
                break;
            }

            case AM_TOP_LVL:
            {
                lss.currentlevel = (lss.currentlevel / MAX_RACES_ON_SCREEN - 1) * MAX_RACES_ON_SCREEN;
                CLAMP(lss.currentlevel, 0, static_cast<int> (levels.size() - 1));
                levelScreenAction(AA_GO_LVL, lss.currentlevel);
                break;
            }

            case AM_TOP_EVT:
            {
                lss.currentevent = (lss.currentevent / MAX_RACES_ON_SCREEN - 1) * MAX_RACES_ON_SCREEN;
                CLAMP(lss.currentevent, 0, static_cast<int> (events.size() - 1));
                levelScreenAction(AA_GO_EVT, lss.currentevent);
                break;
            }

            case AM_TOP_PRAC:
            {
                lss.currentevent = (lss.currentevent / MAX_RACES_ON_SCREEN - 1) * MAX_RACES_ON_SCREEN;
                CLAMP(lss.currentevent, 0, static_cast<int> (events.size() - 1));
                levelScreenAction(AA_GO_PRAC, lss.currentevent);
                break;
            }

            case AM_TOP_PRAC_SEL_PREP:
            {
                lss.currentlevel -= 1;
                CLAMP(lss.currentlevel, 0, static_cast<int> (events[lss.currentlevel].levels.size() - 1));
                levelScreenAction(AA_PICK_PRAC_LVL, lss.currentlevel);
                break;
            }
        }

        break;
    }

    case SDLK_RIGHT:
    {
        switch (lss.state)
        {
            case AM_TOP_LVL_PREP:
            {
                lss.currentlevel += 1;
                CLAMP(lss.currentlevel, 0, static_cast<int> (levels.size() - 1));
                levelScreenAction(AA_PICK_LVL, lss.currentlevel);
                break;
            }

            case AM_TOP_LVL:
            {
                if (levels.size() - lss.currentlevel < MAX_RACES_ON_SCREEN)
                    break;

                lss.currentlevel = (lss.currentlevel / MAX_RACES_ON_SCREEN + 1) * MAX_RACES_ON_SCREEN;
                CLAMP(lss.currentlevel, 0, static_cast<int> (levels.size() - 1));
                levelScreenAction(AA_GO_LVL, lss.currentlevel);
                break;
            }

            case AM_TOP_EVT:
            {
                if (events.size() - lss.currentevent < MAX_RACES_ON_SCREEN)
                    break;

                lss.currentevent = (lss.currentevent / MAX_RACES_ON_SCREEN + 1) * MAX_RACES_ON_SCREEN;
                CLAMP(lss.currentevent, 0, static_cast<int> (events.size() - 1));
                levelScreenAction(AA_GO_EVT, lss.currentevent);
                break;
            }

            case AM_TOP_PRAC:
            {
                if (events.size() - lss.currentevent < MAX_RACES_ON_SCREEN)
                    break;

                lss.currentevent = (lss.currentevent / MAX_RACES_ON_SCREEN + 1) * MAX_RACES_ON_SCREEN;
                CLAMP(lss.currentevent, 0, static_cast<int> (events.size() - 1));
                levelScreenAction(AA_GO_PRAC, lss.currentevent);
                break;
            }

            case AM_TOP_PRAC_SEL_PREP:
            {
                lss.currentlevel += 1;
                CLAMP(lss.currentlevel, 0, static_cast<int> (events[lss.currentevent].levels.size() - 1));
                levelScreenAction(AA_PICK_PRAC_LVL, lss.currentlevel);
                break;
            }
        }

        break;
    }

  default:
    break;
  }
}


void MainApp::renderStateLevel(float eyetranslation)
{
  eyetranslation = eyetranslation;
  
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();

  const GLdouble margin = (800.0 - 600.0 * cfg_video_cx / cfg_video_cy) / 2.0;

  glOrtho(margin, 600.0 * cfg_video_cx / cfg_video_cy + margin, 0.0, 600.0, -1.0, 1.0);

  glPushMatrix();
  glLoadIdentity();
  glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
  
  glMatrixMode(GL_MODELVIEW);
  
  // draw background image
  
  glBlendFunc(GL_ONE, GL_ZERO);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_FOG);
  glDisable(GL_LIGHTING);

  tex_splash_screen->bind();
  
  //glColor4f(0.0f, 0.0f, 0.2f, 1.0f); // make image dark blue
  glColor4f(1.0f, 1.0f, 1.0f, 1.0f); // use image's normal colors
  //glColor4f(0.5f, 0.5f, 0.5f, 1.0f); // make image darker
  
    glBegin(GL_QUADS);
    // the background image is square and cut out a piece based on aspect ratio
    // -------- if aspect ratio is larger than 4:3
    // if aspect ratio is larger than 1:1
    if ((float)getWidth()/(float)getHeight() > 1.0f)
    {

      // lower and upper offset based on aspect ratio
      float off_l = (1 - ((float)getHeight() / (float)getWidth())) / 2.f;
      float off_u = 1 - off_l;
      glTexCoord2f(1.0f,off_u); glVertex2f(1.0f, 1.0f);
      glTexCoord2f(0.0f,off_u); glVertex2f(-1.0f, 1.0f);
      glTexCoord2f(0.0f,off_l); glVertex2f(-1.0f, -1.0f);
      glTexCoord2f(1.0f,off_l); glVertex2f(1.0f, -1.0f);
    }
    // other cases (including 4:3, in which case off_l and off_u are = 1)
    else
    {

      float off_l = (1 - ((float)getWidth() / (float)getHeight())) / 2.f;
      float off_u = 1 - off_l;
      glTexCoord2f(off_u,1.0f); glVertex2f(1.0f, 1.0f);
      glTexCoord2f(off_l,1.0f); glVertex2f(-1.0f, 1.0f);
      glTexCoord2f(off_l,0.0f); glVertex2f(-1.0f, -1.0f);
      glTexCoord2f(off_u,0.0f); glVertex2f(1.0f, -1.0f);
    }
    glEnd();


  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  glMatrixMode(GL_MODELVIEW);
  // draw GUI

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  
  glColor4f(1.0f, 1.0f, 1.0f, 0.2f);
  
  tex_fontDsmOutlined->bind();
  
  glPushMatrix(); // 0
  
  gui.render();
  
  glPopMatrix(); // 0
  
  glBlendFunc(GL_ONE, GL_ZERO);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_FOG);
  glEnable(GL_LIGHTING);
  
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
}

/// @see GuiWidgetColors
#define LIST_OF_GUIWIDGETCOLORS_FIELDS  \
    X(normal)                           \
    X(click)                            \
    X(hover)                            \
    X(listnormal)                       \
    X(listclick)                        \
    X(listhover)                        \
    X(weak)                             \
    X(strong)                           \
    X(bnormal)                          \
    X(bclick)                           \
    X(bhover)

///
/// @brief Loads the widget colors from the specified filename.
/// @todo Should check `sscanf()` calls for success.
/// @param [in] filename    Name of the XML file to be read.
/// @returns Whether or not the operation was successful.
/// @retval true            The colors were read successfully.
/// @retval false           Some (or none) colors could not be read.
///
bool Gui::loadColors(const std::string &filename)
{
    if (PUtil::isDebugLevel(DEBUGLEVEL_TEST))
        PUtil::outLog() << "Loading GUI colors from \"" << filename << "\"\n";

    TiXmlDocument xmlfile(filename.c_str());
    TiXmlElement *rootelem = PUtil::loadRootElement(xmlfile, "menucolors");

    if (rootelem == nullptr)
        return false;

    bool r = true;
    const char *val;

#define X(ColorField) \
    val = rootelem->Attribute(#ColorField); \
    if (val != nullptr) \
        sscanf(val, "%f, %f, %f, %f", &colors.ColorField.x, &colors.ColorField.y, &colors.ColorField.z, &colors.ColorField.w); \
    else \
        r = false;

    LIST_OF_GUIWIDGETCOLORS_FIELDS

#undef X
    return r;
}

void Gui::tick(float delta)
{
  float decay = delta * 3.0f;
  
  for (unsigned int i = 0; i < widget.size(); i++)
  {
    widget[i].glow -= decay;
    CLAMP_LOWER(widget[i].glow, 0.0f);
  }

  if (highlight != -1) {
    widget[highlight].glow = 1.0f;
  }
  
  defflash = fmodf(defflash + delta * 50.0f, PI*2.0f);
}

void Gui::setCursorPos(float x, float y)
{
  highlight = -1;
  
  for (unsigned int i = 0; i < widget.size(); i++) {
    
    if (!widget[i].clickable) continue;
    
    if (x >= widget[i].pos.x &&
      y >= widget[i].pos.y &&
      x < widget[i].pos.x + widget[i].dims_min.x &&
      y < widget[i].pos.y + widget[i].dims_min.y)
      highlight = i;
  }
}

bool Gui::getClickAction(int &data1, int &data2)
{
  if (highlight == -1) return false;
  
  data1 = widget[highlight].d1;
  data2 = widget[highlight].d2;
  
  return true;
}

bool Gui::getDefaultAction(int &data1, int &data2)
{
  if (defwidget == -1) return false;
  
  data1 = widget[defwidget].d1;
  data2 = widget[defwidget].d2;
  
  return true;
}

void Gui::render()
{
  for (unsigned int i = 0; i < widget.size(); i++) {
    
    switch(widget[i].type) {
    case GWT_LABEL: {
      vec4f colc;
      if (widget[i].clickable) {
        colc = INTERP(widget[i].colclick, widget[i].colhover, widget[i].glow);
      } else {
        colc = widget[i].colnormal;
      }
      
      if ((int)i == defwidget)
        colc += vec4f(0.1f, -0.1f, -0.1f, 0.0f) * sinf(defflash);
      
      glPushMatrix();
      
      vec2f ctr = widget[i].pos;
      glTranslatef(ctr.x, ctr.y, 0.0f);
      
      glScalef(widget[i].fontsize, widget[i].fontsize, 1.0f);
      
      fonttex->bind();
      
      glColor4fv(colc);
      ssRender->drawText(widget[i].text, PTEXT_HZA_LEFT | PTEXT_VTA_BOTTOM);
      glPopMatrix();
      } break;
      
    case GWT_GRAPHIC: {
      vec4f colc = vec4f(1.0f, 1.0f, 1.0f, 1.0f);

      if (widget[i].clickable) {
        colc = INTERP(widget[i].colclick, widget[i].colhover, widget[i].glow);
      } else {
        colc = widget[i].colnormal;
      }

      vec2f min = widget[i].pos;
      vec2f max = widget[i].pos + widget[i].dims_min;
      
      widget[i].tex->bind();
      
      glColor4fv(colc);
      
      glBegin(GL_QUADS);
      glTexCoord2f(0.0f, 0.0f); glVertex2f(min.x, min.y);
      glTexCoord2f(1.0f, 0.0f); glVertex2f(max.x, min.y);
      glTexCoord2f(1.0f, 1.0f); glVertex2f(max.x, max.y);
      glTexCoord2f(0.0f, 1.0f); glVertex2f(min.x, max.y);
      glEnd();
      } break;
    }
  }
}

// Widget tree stuff wasn't working properly, so I removed it for
// now. If I need ultra-snazzy menus, I may finish this code

#if 0

void Gui::doLayout()
{
  // Calculate sizes
  for (unsigned int i = 0; i < widget.size(); i++) {
    if (widget[i].parent == GWPARENT_NONE) {
      measureWidgetTree(i);
      placeWidgetTree(i);
    }
  }
}

void Gui::measureWidgetTree(int w)
{
  widget[w].childcount = 0;
  widget[w].fillercount = 0;
  
  switch (widget[w].type) {
  default:
    
    widget[w].dims_measure = widget[w].dims_min;
    
    break;
    
  case GWT_CONTAINER: {
    
    vec2f measure = vec2f(0.0f, 0.0f);
    
    for (unsigned int i = 0; i < widget.size(); i++) {
      if (widget[i].parent == w) {
        measureWidgetTree(i);
        
        widget[w].childcount++;
        
        if (widget[i].type == GWT_FILLER)
          widget[w].fillercount++;
        
        if (widget[w].vert) {
          CLAMP_LOWER(measure.x, widget[i].dims_measure.x);
          measure.y += widget[i].dims_measure.y;
        } else {
          measure.x += widget[i].dims_measure.x;
          CLAMP_LOWER(measure.y, widget[i].dims_measure.y);
        }
      }
    }
    
    widget[w].dims_measure = measure;
    
    } break;
  }
}

void Gui::placeWidgetTree(int w)
{
  if (widget[w].childcount <= 0) return;
  
  float extraspace = widget[w].vert ?
    - widget[w].dims_measure.x :
    - widget[w].dims_measure.y;
  if (widget[w].parent == GWPARENT_NONE) {
    extraspace += widget[w].vert ?
      widget[w].dims_min.x :
      widget[w].dims_min.y;
  }
  
  CLAMP_LOWER(extraspace, 0.0f);
  
  //CLAMP_LOWER(widget[w].dims_measure.x, widget[w].dims_min.x);
  //CLAMP_LOWER(widget[w].dims_measure.y, widget[w].dims_min.y);
  
  float
    addtofillers = 0.0f,
    addtochildren = 0.0f;
  /*
  if (widget[w].fillercount > 0)
    addtofillers = extraspace / (float)widget[w].fillercount;
  else
    addtochildren = extraspace / (float)widget[w].childcount;*/
    
  if (widget[w].vert) {
    float distrib = widget[w].pos.y;
    
    for (unsigned int i = 0; i < widget.size(); i++) {
      if (widget[i].parent == w) {
        
        widget[i].pos.x = widget[w].pos.x;
        widget[i].pos.y = distrib;
        
        widget[i].dims_measure.x = widget[w].dims_measure.x;
        
        switch (widget[i].type) {
        case GWT_FILLER:
          widget[i].dims_measure.y += addtofillers;
          break;
        case GWT_CONTAINER:
          widget[i].dims_measure.y += addtochildren;
          placeWidgetTree(i);
          break;
        default:
          widget[i].dims_measure.y += addtochildren;
          break;
        }
        
        distrib += widget[i].dims_measure.y;
      }
    }
  } else {
    float distrib = widget[w].pos.x;
    
    for (unsigned int i = 0; i < widget.size(); i++) {
      if (widget[i].parent == w) {
        
        widget[i].pos.x = distrib;
        widget[i].pos.y = widget[w].pos.y;
        
        widget[i].dims_measure.y = widget[w].dims_measure.y;
        
        switch (widget[i].type) {
        case GWT_FILLER:
          widget[i].dims_measure.x += addtofillers;
          break;
        case GWT_CONTAINER:
          widget[i].dims_measure.x += addtochildren;
          placeWidgetTree(i);
          break;
        default:
          widget[i].dims_measure.x += addtochildren;
          break;
        }
        
        distrib += widget[i].dims_measure.x;
      }
    }
  }
}

void Gui::render()
{
  // Render trees of all root containers
  
  for (unsigned int i = 0; i < widget.size(); i++) {
    if (widget[i].parent == GWPARENT_NONE)
      renderWidgetTree(i);
  }
}

void Gui::renderWidgetTree(int w)
{
  vec2f min, max;
  
  switch (widget[w].type) {
  case GWT_CONTAINER:
    glColor4f(1.0f,0.0f,0.0f,0.2f);
    break;
  case GWT_FILLER:
    glColor4f(0.0f,1.0f,0.0f,0.2f);
    break;
  case GWT_LABEL:
    glColor4f(0.0f,0.0f,1.0f,0.2f);
    break;
  }
  
  min = widget[w].pos;
  max = widget[w].pos + widget[w].dims_measure;
  
  glDisable(GL_TEXTURE_2D);
  glBegin(GL_QUADS);
  glVertex2f(min.x, min.y);
  glVertex2f(max.x, min.y);
  glVertex2f(max.x, max.y);
  glVertex2f(min.x, max.y);
  glEnd();
  glEnable(GL_TEXTURE_2D);
  
  // Render this widget
  switch (widget[w].type) {
  default:
    break;
    
  case GWT_LABEL: {
    glPushMatrix();
    vec2f ctr = widget[w].pos + widget[w].dims_measure * 0.5f;
    glTranslatef(ctr.x, ctr.y, 0.0f);
    glScalef(widget[w].fontsize, widget[w].fontsize, 1.0f);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    ssRender->drawText(widget[w].text, PTEXT_HZA_CENTER | PTEXT_VTA_CENTER);
    glPopMatrix();
    } break;
  }
  
  // Render children
  switch (widget[w].type) {
  case GWT_CONTAINER:
    for (unsigned int i = 0; i < widget.size(); i++) {
      if (widget[i].parent == w)
        renderWidgetTree(i);
    }
    break;
  }
}

#endif

int Gui::getFreeWidget()
{
  for (unsigned int i = 0; i < widget.size(); i++) {
    if (widget[i].type == GWT_FREE)
      return i;
  }
  widget.push_back(GuiWidget(GWT_FREE));
  return widget.size() - 1;
}

#if 0
int Gui::addRootContainer(float x, float y, float width, float height, bool vert)
{
  int w = getFreeWidget();
  widget[w].type = GWT_CONTAINER;
  widget[w].vert = vert;
  widget[w].parent = GWPARENT_NONE;
  widget[w].dims_min = vec2f(width, height);
  widget[w].pos = vec2f(x, y);
  
  return w;
}

int Gui::addFiller(int parent, float minwidth, float minheight)
{
  int w = getFreeWidget();
  widget[w].type = GWT_FILLER;
  widget[w].parent = parent;
  widget[w].dims_min = vec2f(minwidth, minheight);
  
  return w;
}

int Gui::addContainer(float x, float y, float width, float height, bool vert)
{
  int w = getFreeWidget();
  widget[w].type = GWT_CONTAINER;
  widget[w].vert = vert;
  widget[w].parent = parent;
  widget[w].dims_min = vec2f(minwidth, minheight);
  
  return w;
}
#endif

int Gui::addLabel(float x, float y, const std::string &text, uint32 flags, float fontsize, LabelStyle ls)
{
  int w = getFreeWidget();
  widget[w].type = GWT_LABEL;
  widget[w].text = text;
  widget[w].fontsize = fontsize;
  widget[w].dims_min = ssRender->getTextDims(text) * fontsize;
  widget[w].pos = vec2f(x, y);
  
  if (ls == LabelStyle::Regular)
  {
      widget[w].colnormal   = colors.normal;
      widget[w].colclick    = colors.click;
      widget[w].colhover    = colors.hover;
  }
  else
  if (ls == LabelStyle::Weak)
  {
      widget[w].colnormal   = colors.weak;
      widget[w].colclick    = colors.click;
      widget[w].colhover    = colors.hover;
  }
  else
  if (ls == LabelStyle::Strong)
  {
      widget[w].colnormal   = colors.strong;
      widget[w].colclick    = colors.click;
      widget[w].colhover    = colors.hover;
  }
  else
  if (ls == LabelStyle::List)
  {
      widget[w].colnormal   = colors.listnormal;
      widget[w].colclick    = colors.listclick;
      widget[w].colhover    = colors.listhover;
  }

  if (flags & PTEXT_HZA_CENTER)
    widget[w].pos.x -= widget[w].dims_min.x * 0.5f;
  else if (flags & PTEXT_HZA_RIGHT)
    widget[w].pos.x -= widget[w].dims_min.x;
  
  if (flags & PTEXT_VTA_CENTER)
    widget[w].pos.y -= widget[w].dims_min.y * 0.5f;
  else if (flags & PTEXT_VTA_TOP)
    widget[w].pos.y -= widget[w].dims_min.y;
  
  return w;
}

int Gui::addGraphic(float x, float y, float width, float height, PTexture *tex, GraphicStyle gs)
{
  int w = getFreeWidget();
  widget[w].type = GWT_GRAPHIC;
  widget[w].dims_min = vec2f(width, height);
  widget[w].pos = vec2f(x, y);
  widget[w].tex = tex;

    if (gs == GraphicStyle::Button)
    {
        widget[w].colnormal = colors.bnormal;
        widget[w].colclick  = colors.bclick;
        widget[w].colhover  = colors.bhover;
    }
    else
    if (gs == GraphicStyle::Image)
    {
        widget[w].colnormal = {1.00f, 1.00f, 1.00f, 1.00f};
        widget[w].colclick  = {1.00f, 1.00f, 1.00f, 1.00f};
        widget[w].colhover  = {1.00f, 1.00f, 1.00f, 1.00f};
    }

  return w;
}

