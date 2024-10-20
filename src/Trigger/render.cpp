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

#include "damage.h"
#include "main.h"
#include "vehicle.h"
#include <cmath>

void MainApp::resize()
{
    glClearColor(1.0,1.0,1.0,1.0);
    glEnable(GL_TEXTURE_2D);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE,GL_ZERO);

    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);
    glClearDepth(1.0);

    glEnable(GL_CULL_FACE);

    glEnable(GL_FOG);
    glFogi(GL_FOG_MODE, GL_EXP);

    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);

    const GLfloat ambcol[] = {0.1f, 0.1f, 0.1f, 0.0f};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambcol);

    float white[] = { 1.0,1.0,1.0,1.0 };
    //float black[] = { 0.0,0.0,0.0,1.0 };
    glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,white);

    float spec[] = { 0.3f, 0.5f, 0.5f, 1.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spec);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 6.0f);

    float litcol[] = { 0.6,0.6,0.6,0.0 };
    glLightfv(GL_LIGHT0,GL_DIFFUSE,litcol);
    glLightfv(GL_LIGHT0,GL_SPECULAR,litcol);

    glEnable(GL_NORMALIZE);
}

void drawBlades(float radius, float ang, float trace)
{
    float invtrace = 1.0 / trace;
    glPushMatrix();
    glScalef(radius, radius, 1.0);
    for (float ba=0; ba<PI*2.0-0.01; ba+=PI/2.0)
    {
        glBegin(GL_TRIANGLE_FAN);
        glColor4f(0.1,0.1,0.1,0.24 * invtrace);
        glVertex2f(0.0,0.0);
        glColor4f(0.1,0.1,0.1,0.06 * invtrace);
        int num = (int)(trace / 0.1);
        if (num < 2) num = 2;
        float mult = trace / (float)(num-1);
        float angadd = ba + ang;
        for (int i=0; i<num; ++i)
        {
            float a = (float)i * mult + angadd;
            glVertex2f(cos(a),sin(a));
        }
        glEnd();
    }
    glPopMatrix();
}

void MainApp::renderWater()
{
    tex_water->bind();
    {
        float tgens[] = { 0.5,0,0,0 };
        float tgent[] = { 0,0.5,0,0 };
        glTexGeni(GL_S,GL_TEXTURE_GEN_MODE,GL_OBJECT_LINEAR);
        glTexGenfv(GL_S,GL_OBJECT_PLANE,tgens);
        glTexGeni(GL_T,GL_TEXTURE_GEN_MODE,GL_OBJECT_LINEAR);
        glTexGenfv(GL_T,GL_OBJECT_PLANE,tgent);
    }
    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);
    glPushMatrix();
    glScalef(20.0,20.0,1.0);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    {
        int minx = (int)(campos.x / 20.0)-20,
            maxx = minx + 40,
            miny = (int)(campos.y / 20.0)-20,
            maxy = miny + 40;
        for (int y=miny; y<maxy; ++y)
        {
            glBegin(GL_TRIANGLE_STRIP);
            for (int x=minx; x<=maxx; ++x)
            {
                float maxalpha = 0.5f;

                if (game->water.useralpha)
                    maxalpha = game->water.alpha;

                if (game->water.fixedalpha)
                {
                    glColor4f(1.0f, 1.0f, 1.0f, maxalpha);
                    glVertex3f(x, y+1, game->water.height);
                    glVertex3f(x, y, game->water.height);
                }
                else
                {
                    float ht,alpha;
                    ht = game->terrain->getHeight((x)*20.0,(y+1)*20.0);
                    alpha = 1.0 - exp(ht - game->water.height);
                    CLAMP(alpha, 0.0f, maxalpha);
                    glColor4f(1.0,1.0,1.0,alpha);
                    glVertex3f(x, y+1, game->water.height);
                    ht = game->terrain->getHeight((x)*20.0,(y)*20.0);
                    alpha = 1.0 - exp(ht - game->water.height);
                    CLAMP(alpha, 0.0f, maxalpha);
                    glColor4f(1.0,1.0,1.0,alpha);
                    glVertex3f(x, y, game->water.height);
                }
            }
            glEnd();
        }
    }
    glPopMatrix();
    glBlendFunc(GL_ONE,GL_ZERO);

    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);
}

void MainApp::renderSky(const mat44f &cammat)
{
    glFogf(GL_FOG_DENSITY, game->weather.fog.density_sky);
    glDepthRange(0.999,1.0);
    glDisable(GL_CULL_FACE);
    glPushMatrix(); // 1
    glLoadIdentity();
    glMultMatrixf(cammat);
    tex_sky[0]->bind();
#define CLRANGE     10
#define CLFACTOR    0.02//0.014
    glMatrixMode(GL_TEXTURE);
    glPushMatrix();
    glTranslatef(cloudscroll,0.0,0.0);
    glRotatef(30.0,0.0,0.0,1.0);
    glScalef(0.4,0.4,1.0);
    for (int y=-CLRANGE; y<CLRANGE; y++)
    {
        glBegin(GL_TRIANGLE_STRIP);
        for (int x=-CLRANGE; x<CLRANGE+1; x++)
        {
            glTexCoord2i(x,y);
            glVertex3f(x,y,0.3-(x*x+y*y)*CLFACTOR);
            glTexCoord2i(x,y+1);
            glVertex3f(x,y+1,0.3-(x*x+(y+1)*(y+1))*CLFACTOR);
        }
        glEnd();
    }
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix(); // 1
    glEnable(GL_CULL_FACE);
    glDepthRange(0.0,0.999);
    glFogf(GL_FOG_DENSITY, game->weather.fog.density);
}

void MainApp::render(float eyetranslation)
{
    switch (appstate)
    {
        case AS_LOAD_1:
            renderStateLoading(eyetranslation);
            break;

        case AS_LOAD_2:
        case AS_LOAD_3:
            break;

        case AS_LEVEL_SCREEN:
            renderStateLevel(eyetranslation);
            break;

        case AS_CHOOSE_VEHICLE:
            renderStateChoose(eyetranslation);
            break;

        case AS_IN_GAME:
            renderStateGame(eyetranslation);
            break;

        case AS_END_SCREEN:
            renderStateEnd(eyetranslation);
            break;
    }

    glFinish();
}

void MainApp::renderStateLoading(float eyetranslation)
{
    UNREFERENCED_PARAMETER(eyetranslation);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);

    tex_splash_screen->bind();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_FOG);
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

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

    tex_loading_screen->bind();

    GLfloat logovratio = static_cast<float> (getWidth()) / getHeight();
    GLfloat logohratio = static_cast<float> (getHeight()) / getWidth();

    // FIXME: nasty, nasty code
    if (logovratio > 1.0f)
        logohratio = 1.0f;
    else
    if (logohratio > 1.0f)
        logovratio = 1.0f;

#define LOGO_VRATIO     (logovratio/3.5)
#define LOGO_HRATIO     (logohratio/3.5)
    glBegin(GL_QUADS);
      glTexCoord2f(1.0f, 1.0f); glVertex2f( LOGO_HRATIO,  LOGO_VRATIO);
      glTexCoord2f(0.0f, 1.0f); glVertex2f(-LOGO_HRATIO,  LOGO_VRATIO);
      glTexCoord2f(0.0f, 0.0f); glVertex2f(-LOGO_HRATIO, -LOGO_VRATIO);
      glTexCoord2f(1.0f, 0.0f); glVertex2f( LOGO_HRATIO, -LOGO_VRATIO);
    glEnd();
#undef LOGO_VRATIO
#undef LOGO_HRATIO

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_FOG);
    glEnable(GL_LIGHTING);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

const char *creditstext[] =
{
    "Trigger Rally " PACKAGE_VERSION,
    "",
    "Copyright (C) 2004-2006",
    "Jasmine Langridge and Richard Langridge",
    "Posit Interactive",
    "",
    "Copyright (C) 2006-2016",
    "Various Contributors",
    "(see DATA_AUTHORS.txt)",
    "",
    "",
    "",
    "Coding",
    "Jasmine Langridge",
    "",
    "Art & SFX",
    "Richard Langridge",
    "",
    "",
    "",
    "Contributors",
    "",
    "Build system",
    "Matze Braune",
    "",
    "Stereo support",
    "Chuck Sites",
    "",
    "Mac OS X porting",
    "Tim Douglas",
    "",
    "Fixes",
    "LavaPunk",
    "Bernhard Kaindl",
    "Liviu Andronic",
    "Ishmael Turner",
    "Iwan 'qubodup' Gabovitch",
    "Farrer",
    "Andrei Bondor",
    "Nikolay Orlyuk",
	"Emanuele Sorce",
    "",
    "New levels",
    "Tim Wintle",
    "David Pagnier",
    "Jared Buckner",
    "Andreas Rosdal",
    "Ivan",
    "Viktor Radnai",
    "Pierre-Alexis",
    "Bruno 'Fuddl' Kleinert",
    "Agnius Vasiliauskas",
    "Matthias Keysermann",
    "Marcio Bremm",
    "Onsemeliot",
    "",
    "Graphics",
    "Alex",
    "Roberto Diez Gonzalez",
    "",
    "",
    "",
    "",
    "",
    "Thanks to Jonathan C. Hatfull",
    "",
    "",
    "",
    "",
    "And thanks to Simon Brown too",
    "",
    "",
    "",
    "",
    "",
    "",
    "Thanks for playing Trigger"
};

#define NUMCREDITSTRINGS (sizeof(creditstext) / sizeof(char*))

void MainApp::renderStateEnd(float eyetranslation)
{
    eyetranslation = eyetranslation;

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);

    tex_end_screen->bind();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_FOG);
    glDisable(GL_LIGHTING);
    glBlendFunc(GL_ONE, GL_ZERO);

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

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

    tex_fontSourceCodeOutlined->bind();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0 - hratio, hratio, 0 - vratio, vratio, 0 - 1.0, 1.0);
    //glOrtho(-1, 1, -1, 1, -1, 1);
    //glOrtho(800, 0, 600, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glPushMatrix();

    float scroll = splashtimeout;
    const float maxscroll = (float)(NUMCREDITSTRINGS - 1) * 2.0f;
    RANGEADJUST(scroll, 0.0f, 0.9f, -10.0f, maxscroll);
    CLAMP_UPPER(scroll, maxscroll);

    glScalef(0.1f, 0.1f, 1.0f);

    glTranslatef(0.0f, scroll, 0.0f);

    for (int i = 0; i < (int)NUMCREDITSTRINGS; i++)
    {
        float level = fabsf(scroll + (float)i * -2.0f);
        RANGEADJUST(level, 0.0f, 9.0f, 3.0f, 0.0f);

        if (level > 0.0f)
        {
            CLAMP_UPPER(level, 1.0f);

            glPushMatrix();
            glTranslatef(0.0f, (float)i * -2.0f, 0.0f);

            float enlarge = 1.0f;

#if 1
            if (splashtimeout > 0.9f)
            {
                float amt = (splashtimeout - 0.9f) * 10.0f;
                float amt2 = amt * amt;

                enlarge += amt2 / ((1.0001f - amt) * (1.0001f - amt));
                level -= amt2;
            }
#endif

            glScalef(enlarge, enlarge, 0.0f);
            glColor4f(1.0f, 1.0f, 1.0f, level);

            getSSRender().drawText(creditstext[i], PTEXT_HZA_CENTER | PTEXT_VTA_CENTER);
            glPopMatrix();
        }
    }

    glPopMatrix();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_FOG);
    glEnable(GL_LIGHTING);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

// render the car selection menu
void MainApp::renderStateChoose(float eyetranslation)
{
    PVehicleType *vtype = game->vehiclechoices[choose_type];

    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

glMatrixMode(GL_PROJECTION);

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

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    float fnear = 0.1f, fov = 0.6f;
    float aspect = (float)getWidth() / (float)getHeight();
    stereoFrustum(-fnear*aspect*fov,fnear*aspect*fov,-fnear*fov,fnear*fov,fnear,100000.0f,
                  0.8f, eyetranslation);
    glMatrixMode(GL_MODELVIEW);


    glPushMatrix(); // 0

//    glTranslatef(-eyetranslation, 0.5f, -5.0f);
    glTranslatef(-eyetranslation, 0.9f, -5.0f);
    glRotatef(28.0f, 1.0f, 0.0f, 0.0f);

    glDisable(GL_FOG);
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);

    vec4f lpos = vec4f(0.0f, 1.0f, 0.0f, 0.0f);
    glLightfv(GL_LIGHT0, GL_POSITION, lpos);

    //float tmp = 1.0f;
    //float tmp = sinf(choose_spin * 2.0f) * 0.5f;
    float tmp = cosf(choose_spin * 2.0f) * 0.5f;
    tmp += choose_spin;
    glRotatef(90.0f, -1.0f, 0.0f, 0.0f);
    glRotatef(DEGREES(tmp), 0.0f, 0.0f, 1.0f);

    // render vehicle
    for (unsigned int i=0; i<vtype->part.size(); ++i)
    {
		glPushMatrix(); // 1

		vec3f vpos = vtype->part[i].render_ref_local.getPosition();
		glTranslatef(vpos.x, vpos.y, vpos.z);

		mat44f vorim = vtype->part[i].render_ref_local.getInverseOrientationMatrix();
		glMultMatrixf(vorim);
		if (vtype->part[i].model)
		{
			glPushMatrix(); // 2

			float scale = vtype->part[i].scale;
			glScalef(scale,scale,scale);
			drawModel(*vtype->part[i].model, 1.0f);

			glPopMatrix(); // 2
		}

		// render wheels
		if (vtype->wheelmodel)
		{
			for (unsigned int j=0; j<vtype->part[i].wheel.size(); j++)
			{

				glPushMatrix(); // 2

				vec3f &wpos = vtype->part[i].wheel[j].pt;
				glTranslatef(wpos.x, wpos.y, wpos.z);

				float scale = vtype->wheelscale * vtype->part[i].wheel[j].radius;
				glScalef(scale,scale,scale);

				drawModel(*vtype->wheelmodel, 1.0f);

				glPopMatrix(); // 2
			}
		}

		glPopMatrix(); // 1
	}

    glPopMatrix(); // 0

    glDisable(GL_LIGHTING);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);

    // use the same colors as the menu
    const GuiWidgetColors gwc = gui.getColors();

    tex_fontSourceCodeShadowed->bind();

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);

    glPushMatrix(); // 0

    const GLdouble margin = (800.0 - 600.0 * cx / cy) / 2.0;

    glOrtho(margin, 600.0 * cx / cy + margin, 0.0, 600.0, -1.0, 1.0);

    glPushMatrix(); // 1
    glTranslatef(10.0f, 570.0f, 0.0f);
    glScalef(30.0f, 30.0f, 1.0f);
    glColor4f(gwc.weak.x, gwc.weak.y, gwc.weak.z, gwc.weak.w);
    getSSRender().drawText("Trigger Rally", PTEXT_HZA_LEFT | PTEXT_VTA_CENTER);
    glPopMatrix(); // 1

    glPushMatrix(); // 1
    glTranslatef(790.0f, 570.0f, 0.0f);
    glScalef(20.0f, 20.0f, 1.0f);
    glColor4f(gwc.weak.x, gwc.weak.y, gwc.weak.z, gwc.weak.w);
    getSSRender().drawText(
        "car selection " + std::to_string(choose_type + 1) + '/' + std::to_string(game->vehiclechoices.size()),
        PTEXT_HZA_RIGHT | PTEXT_VTA_CENTER);
    glPopMatrix(); // 1

    glPushMatrix(); // 1
    glTranslatef(100.0f, 230.0f, 0.0f);
    glScalef(30.0f, 30.0f, 1.0f);
    glColor4f(gwc.header.x, gwc.header.y, gwc.header.z, gwc.header.w);
    getSSRender().drawText(vtype->proper_name.substr(0, 9), PTEXT_HZA_LEFT | PTEXT_VTA_CENTER);
    glPopMatrix(); // 1

    glPushMatrix(); // 1
    glTranslatef(100.0f, 200.0f, 0.0f);
    glScalef(20.0f, 20.0f, 1.0f);
    glColor4f(gwc.strong.x, gwc.strong.y, gwc.strong.z, gwc.strong.w);
    getSSRender().drawText(vtype->proper_class.substr(0, 8), PTEXT_HZA_LEFT | PTEXT_VTA_CENTER);
    glPopMatrix(); // 1

    glPushMatrix(); // 1
    glTranslatef(500.0f, 230.0f, 0.0f);
    glScalef(20.0f, 20.0f, 1.0f);
    glColor4f(gwc.weak.x, gwc.weak.y, gwc.weak.z, gwc.weak.w);
    getSSRender().drawText("Weight (Kg)", PTEXT_HZA_RIGHT | PTEXT_VTA_CENTER);
    glPopMatrix(); // 1

    glPushMatrix(); // 1
    glTranslatef(500.0f, 190.0f, 0.0f);
    glScalef(20.0f, 20.0f, 1.0f);
    glColor4f(gwc.weak.x, gwc.weak.y, gwc.weak.z, gwc.weak.w);
    getSSRender().drawText("Engine (BHP)", PTEXT_HZA_RIGHT | PTEXT_VTA_CENTER);
    glPopMatrix(); // 1

    glPushMatrix(); // 1
    glTranslatef(500.0f, 150.0f, 0.0f);
    glScalef(20.0f, 20.0f, 1.0f);
    glColor4f(gwc.weak.x, gwc.weak.y, gwc.weak.z, gwc.weak.w);
    getSSRender().drawText("Wheel drive", PTEXT_HZA_RIGHT | PTEXT_VTA_CENTER);
    glPopMatrix(); // 1

    glPushMatrix(); // 1
    glTranslatef(500.0f, 110.0f, 0.0f);
    glScalef(20.0f, 20.0f, 1.0f);
    glColor4f(gwc.weak.x, gwc.weak.y, gwc.weak.z, gwc.weak.w);
    getSSRender().drawText("Roadholding", PTEXT_HZA_RIGHT | PTEXT_VTA_CENTER);
    glPopMatrix(); // 1

    glPushMatrix(); // 1
    glTranslatef(520.0f, 230.0f, 0.0f);
    glScalef(30.0f, 30.0f, 1.0f);
    glColor4f(gwc.strong.x, gwc.strong.y, gwc.strong.z, gwc.strong.w);
    getSSRender().drawText(std::to_string(static_cast<int>(vtype->mass)), PTEXT_HZA_LEFT | PTEXT_VTA_CENTER);
    glPopMatrix(); // 1

    glPushMatrix(); // 1
    glTranslatef(520.0f, 190.0f, 0.0f);
    glScalef(30.0f, 30.0f, 1.0f);
    glColor4f(gwc.strong.x, gwc.strong.y, gwc.strong.z, gwc.strong.w);
    getSSRender().drawText(vtype->pstat_enginepower, PTEXT_HZA_LEFT | PTEXT_VTA_CENTER);
    glPopMatrix(); // 1

    glPushMatrix(); // 1
    glTranslatef(520.0f, 150.0f, 0.0f);
    glScalef(30.0f, 30.0f, 1.0f);
    glColor4f(gwc.strong.x, gwc.strong.y, gwc.strong.z, gwc.strong.w);
    getSSRender().drawText(vtype->pstat_wheeldrive, PTEXT_HZA_LEFT | PTEXT_VTA_CENTER);
    glPopMatrix(); // 1

    glPushMatrix(); // 1
    glTranslatef(520.0f, 110.0f, 0.0f);
    glScalef(30.0f, 30.0f, 1.0f);
    glColor4f(gwc.strong.x, gwc.strong.y, gwc.strong.z, gwc.strong.w);
    getSSRender().drawText(vtype->pstat_roadholding, PTEXT_HZA_LEFT | PTEXT_VTA_CENTER);
    glPopMatrix(); // 1

    std::string racename;

    if (lss.state == AM_TOP_EVT_PREP || lss.state == AM_TOP_PRAC_SEL_PREP)
        racename = events[lss.currentevent].name + ": " + events[lss.currentevent].levels[lss.currentlevel].name;
    else
    if (lss.state == AM_TOP_LVL_PREP)
        racename = levels[lss.currentlevel].name;

    glPushMatrix(); // 1
    glTranslatef(400.0f, 30.0f, 0.0f);
    glScalef(20.0f, 20.0f, 1.0f);
    glColor4f(gwc.weak.x, gwc.weak.y, gwc.weak.z, gwc.weak.w);
    getSSRender().drawText(racename, PTEXT_HZA_CENTER | PTEXT_VTA_CENTER);
    glPopMatrix(); // 1

    if (vtype->getLocked()) {
      const std::string unlockevent = getVehicleUnlockEvent(vtype->getName());

      glPushMatrix(); // 1
      glTranslatef(400.0f, 425.0f, 0.0f);
      glScalef(40.0f, 40.0f, 1.0f);
      glColor4f(gwc.marked.x, gwc.marked.y, gwc.marked.z, gwc.marked.w);
      getSSRender().drawText("Locked", PTEXT_HZA_CENTER | PTEXT_VTA_CENTER);
      glPopMatrix(); // 1

      if (unlockevent != "") {
        glPushMatrix(); // 1
        glTranslatef(400.0f, 375.0f, 0.0f);
        glScalef(20.0f, 20.0f, 20.0f);
        glColor4f(gwc.marked.x, gwc.marked.y, gwc.marked.z, gwc.marked.w);
        getSSRender().drawText("Complete event " + unlockevent + " to unlock", PTEXT_HZA_CENTER | PTEXT_VTA_CENTER);
        glPopMatrix(); // 1
      }
    }

    glPopMatrix(); // 0

    glBlendFunc(GL_ONE, GL_ZERO);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_FOG);
    glEnable(GL_LIGHTING);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void MainApp::renderStateGame(float eyetranslation)
{
    PVehicle *vehic = game->vehicle[0];

    glClear(GL_DEPTH_BUFFER_BIT);
    //glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    float fnear = 0.1f, fov = 0.6f;
    float aspect = (float)getWidth() / (float)getHeight();
    stereoFrustum(-fnear*aspect*fov,fnear*aspect*fov,-fnear*fov,fnear*fov,fnear,100000.0f,
                  0.8f, eyetranslation);
    glMatrixMode(GL_MODELVIEW);

    glColor3f(1.0,1.0,1.0);

    vec4f fogcolor(game->weather.fog.color, 1.0f);
    glFogfv(GL_FOG_COLOR, fogcolor);

    glDepthRange(0.0,0.999);

    glPushMatrix(); // 0

    mat44f cammat = camori.getMatrix();
    mat44f cammat_inv = cammat.transpose();

    //glTranslatef(0.0,0.0,-40.0);
    glTranslatef(-eyetranslation, 0.0f, 0.0f);

    glMultMatrixf(cammat);

    glTranslatef(-campos.x, -campos.y, -campos.z);

    float lpos[] = { 0.2, 0.5, 1.0, 0.0 };
    glLightfv(GL_LIGHT0, GL_POSITION, lpos);

    glColor3ub(255,255,255);

    glDisable(GL_LIGHTING);

    glActiveTextureARB(GL_TEXTURE1_ARB);
    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_COMBINE);
    glTexEnvi(GL_TEXTURE_ENV,GL_COMBINE_RGB,GL_ADD_SIGNED);
    glTexEnvi(GL_TEXTURE_ENV,GL_COMBINE_ALPHA,GL_MODULATE);
    tex_detail->bind();
    glTexGeni(GL_S,GL_TEXTURE_GEN_MODE,GL_OBJECT_LINEAR);
    glTexGeni(GL_T,GL_TEXTURE_GEN_MODE,GL_OBJECT_LINEAR);
    float tgens[] = { 0.05, 0.0, 0.0, 0.0 };
    float tgent[] = { 0.0, 0.05, 0.0, 0.0 };
    glTexGenfv(GL_S,GL_OBJECT_PLANE,tgens);
    glTexGenfv(GL_T,GL_OBJECT_PLANE,tgent);
    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);
    glActiveTextureARB(GL_TEXTURE0_ARB);

    // draw terrain
    game->terrain->render(campos, cammat_inv);

    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);

    glActiveTextureARB(GL_TEXTURE1_ARB);
    glDisable(GL_TEXTURE_2D);
    glActiveTextureARB(GL_TEXTURE0_ARB);

    if (renderowncar)
    {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        tex_shadow->bind();

        glColor4f(1.0f, 1.0f, 1.0f, 0.7f);

        vec3f vpos = game->vehicle[0]->body->getPosition();
        vec3f forw = makevec3f(game->vehicle[0]->body->getOrientationMatrix().row[0]);
        float forwangle = atan2(forw.y, forw.x);
        game->terrain->drawSplat(vpos.x, vpos.y, 1.4f, forwangle + PI*0.5f);

        glBlendFunc(GL_ONE, GL_ZERO);
    }

    renderSky(cammat);

    glEnable(GL_LIGHTING);

    for (unsigned int v=0; v<game->vehicle.size(); ++v)
    {
        if (!renderowncar && v == 0) continue;

        PVehicle *vehic = game->vehicle[v];
        for (unsigned int i=0; i<vehic->part.size(); ++i)
        {
            renderVehiclePart(*vehic->type, vehic->part[i], vehic->type->part[i], 1.0f);
        }
    }

    glDisable(GL_LIGHTING);

    PGhost::GhostData ghostdata;
    std::string vehiclename = "";

    if (cfg.getEnableGhost() && ghost.getReplayData(ghostdata, vehiclename))
    {
        PVehiclePart vehiclepart;

        vehiclepart.ref_world.setPosition(ghostdata.pos);
        vehiclepart.ref_world.setOrientation(ghostdata.ori);
        vehiclepart.ref_world.updateMatrices();

        for (unsigned int i = 0; i < ghostdata.wheel.size(); ++i)
        {
            PVehicleWheel wheel;

            wheel.ref_world.setPosition(ghostdata.wheel[i].pos);
            wheel.ref_world.setOrientation(ghostdata.wheel[i].ori);
            wheel.ref_world.updateMatrices();
            vehiclepart.wheel.push_back(wheel);
        }

        // Theoretically there can be multiple vehicles with multiple parts.
        // However, the assumption is, that the model of the first part is relevant.
        for (unsigned int i = 0; i < game->vehiclechoices.size(); ++i) {
          if (game->vehiclechoices[i]->getName() == vehiclename) {
            renderVehiclePart(*game->vehiclechoices[i], vehiclepart,
                game->vehiclechoices[i]->part[0], 0.5f);
            break;
          }
        }
    }

    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);

    glDisable(GL_TEXTURE_2D);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

#define RAINDROP_WIDTH          0.015
    const vec4f raindrop_col(0.5,0.5,0.5,0.4);

    vec3f offsetdrops = campos - campos_prev;

    for (unsigned int i = 0; i < rain.size(); i++)
    {
        vec3f tempv;
        const float prevlife = rain[i].prevlife;
        vec3f pt1 = rain[i].drop_pt + rain[i].drop_vect * prevlife + offsetdrops;
        vec3f pt2 = rain[i].drop_pt + rain[i].drop_vect * rain[i].life;
        vec3f zag = campos - rain[i].drop_pt;
        zag = zag.cross(rain[i].drop_vect);
        zag *= RAINDROP_WIDTH / zag.length();
        glBegin(GL_TRIANGLE_STRIP);
        glColor4f(raindrop_col[0],raindrop_col[1],raindrop_col[2],0.0);
        tempv = pt1 - zag;
        glVertex3fv(tempv);
        tempv = pt2 - zag;
        glVertex3fv(tempv);

        glColor4fv(raindrop_col);
        glVertex3fv(pt1);
        glVertex3fv(pt2);

        glColor4f(raindrop_col[0],raindrop_col[1],raindrop_col[2],0.0);
        tempv = pt1 + zag;
        glVertex3fv(tempv);
        tempv = pt2 + zag;
        glVertex3fv(tempv);
        glEnd();
    }

#define SNOWFLAKE_POINT_SIZE        3.0f
#define SNOWFLAKE_BOX_SIZE          0.175f

// NOTE: must be greater than 1.0f
#define SNOWFLAKE_MAXLIFE           4.5f

    GLfloat ops; // Original Point Size, for to be restored

    if (cfg.getSnowflaketype() == PConfig::SnowFlakeType::point)
    {
        glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
        glGetFloatv(GL_POINT_SIZE, &ops);
        glPointSize(SNOWFLAKE_POINT_SIZE);
    }
    else
    if (cfg.getSnowflaketype() == PConfig::SnowFlakeType::textured)
    {
        glEnable(GL_TEXTURE_2D);
        glBlendFunc(GL_SRC_COLOR, GL_ONE);
        tex_snowflake->bind();
    }

    for (const SnowFlake &sf: snowfall)
    {
        const vec3f pt = sf.drop_pt + sf.drop_vect * sf.life;
        GLfloat alpha;

        if (sf.life > SNOWFLAKE_MAXLIFE)
        {
            alpha = 0.0f;
        }
        else
        if (sf.life > 1.0f)
        {
#define ML      SNOWFLAKE_MAXLIFE
            // this equation ensures that snowflaks fade in
            alpha = (sf.life - ML) / (1 - ML);
#undef ML
        }
        else
            alpha = 1.0f;

        if (cfg.getSnowflaketype() == PConfig::SnowFlakeType::point)
        {
            glBegin(GL_POINTS);
            glColor4f(1.0f, 1.0f, 1.0f, alpha);
            glVertex3fv(pt);
            glEnd();
        }
        else
        {
#define SBS     SNOWFLAKE_BOX_SIZE
            vec3f zag = campos - sf.drop_pt;

            zag = zag.cross(sf.drop_vect);
            zag.normalize();
            zag *= SBS;

            if (cfg.getSnowflaketype() == PConfig::SnowFlakeType::square)
            {
                glBegin(GL_TRIANGLE_STRIP);
                glColor4f(1.0f, 1.0f, 1.0f, alpha);
                glVertex3f(pt.x,            pt.y,           pt.z                );
                glVertex3f(pt.x,            pt.y,           pt.z + zag.z + SBS  );
                glVertex3f(pt.x + zag.x,    pt.y + zag.y,   pt.z                );
                glVertex3f(pt.x + zag.x,    pt.y + zag.y,   pt.z + zag.z + SBS  );
                glEnd();
            }
            else // cfg_snowflaketype == SnowFlakeType::textured
            {
                glBegin(GL_TRIANGLE_STRIP);
                glColor4f(1.0f, 1.0f, 1.0f, alpha);
                glTexCoord2f(1.0f, 1.0f);
                glVertex3f(pt.x,            pt.y,           pt.z                );
                glTexCoord2f(0.0f, 1.0f);
                glVertex3f(pt.x,            pt.y,           pt.z + zag.z + SBS  );
                glTexCoord2f(1.0f, 0.0f);
                glVertex3f(pt.x + zag.x,    pt.y + zag.y,   pt.z                );
                glTexCoord2f(0.0f, 0.0f);
                glVertex3f(pt.x + zag.x,    pt.y + zag.y,   pt.z + zag.z + SBS  );
                glEnd();
            }
#undef SBS
        }
    }

    if (cfg.getSnowflaketype() == PConfig::SnowFlakeType::point)
        glPointSize(ops); // restore original point size

    // disable textures
    if (cfg.getSnowflaketype() == PConfig::SnowFlakeType::textured)
    {
        glDisable(GL_TEXTURE_2D);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    const vec4f checkpoint_col[3] =
    {
        vec4f(1.0f, 0.0f, 0.0f, 0.8f),  // 0 = next checkpoint
        vec4f(0.7f, 0.7f, 0.1f, 0.6f),  // 1 = checkpoint after next
        vec4f(0.2f, 0.8f, 0.2f, 0.4f)  // 2 = all other checkpoints
    };

    if (showcheckpoint)
    {
        for (unsigned int i=0; i<game->checkpt.size(); i++)
        {
            vec4f colr = checkpoint_col[2];

            if ((int)i == vehic->nextcp)
                colr = checkpoint_col[0];
            else if ((int)i == (vehic->nextcp + 1) % (int)game->checkpt.size())
                colr = checkpoint_col[1];

            glPushMatrix(); // 1
            glTranslatef(game->checkpt[i].pt.x, game->checkpt[i].pt.y, game->checkpt[i].pt.z);
            glScalef(25.0f, 25.0f, 1.0f);

#if 0 // Checkpoint style one
            glBegin(GL_TRIANGLE_STRIP);

            for (float a = 0.0f; a < 0.99f; a += 0.05f)
            {
                glColor4f(colr[0], colr[1], colr[2], colr[3] * a);
                float ang = cprotate + a * 6.0f;
                float ht = sinf(ang * 1.7f) * 7.0f + 8.0f;
                glVertex3f(cosf(ang), sinf(ang), ht - 1.0f);
                glVertex3f(cosf(ang), sinf(ang), ht + 1.0f);
            }

            for (float a = 1.0f; a < 2.01f; a += 0.05f)
            {
                glColor4f(colr[0], colr[1], colr[2], colr[3] * (2.0f - a));
                float ang = cprotate + a * 6.0f;
                float ht = sinf(ang * 1.7f) * 7.0f + 8.0f;
                glVertex3f(cosf(ang), sinf(ang), ht - 1.0f);
                glVertex3f(cosf(ang), sinf(ang), ht + 1.0f);
            }

            glEnd();
#else // Regular checkpoint style
            glBegin(GL_TRIANGLE_STRIP);
            float ht = sinf(cprotate * 6.0f) * 7.0f + 8.0f;
            glColor4f(colr[0], colr[1], colr[2], 0.0f);
            glVertex3f(1.0f, 0.0f, ht - 1.0f);
            glColor4f(colr[0], colr[1], colr[2], colr[3]);
            glVertex3f(1.0f, 0.0f, ht + 0.0f);
            for (float a = PI/10.0f; a < PI*2.0f-0.01f; a += PI/10.0f)
            {
                glColor4f(colr[0], colr[1], colr[2], 0.0f);
                glVertex3f(cosf(a), sinf(a), ht - 1.0f);
                glColor4f(colr[0], colr[1], colr[2], colr[3]);
                glVertex3f(cosf(a), sinf(a), ht + 0.0f);
            }
            glColor4f(colr[0], colr[1], colr[2], 0.0f);
            glVertex3f(1.0f, 0.0f, ht - 1.0f);
            glColor4f(colr[0], colr[1], colr[2], colr[3]);
            glVertex3f(1.0f, 0.0f, ht + 0.0f);
            glEnd();

            glBegin(GL_TRIANGLE_STRIP);
            glColor4f(colr[0], colr[1], colr[2], colr[3]);
            glVertex3f(1.0f, 0.0f, ht - 0.0f);
            glColor4f(colr[0], colr[1], colr[2], 0.0f);
            glVertex3f(1.0f, 0.0f, ht + 1.0f);
            for (float a = PI/10.0f; a < PI*2.0f-0.01f; a += PI/10.0f)
            {
                glColor4f(colr[0], colr[1], colr[2], colr[3]);
                glVertex3f(cosf(a), sinf(a), ht - 0.0f);
                glColor4f(colr[0], colr[1], colr[2], 0.0f);
                glVertex3f(cosf(a), sinf(a), ht + 1.0f);
            }
            glColor4f(colr[0], colr[1], colr[2], colr[3]);
            glVertex3f(1.0f, 0.0f, ht - 0.0f);
            glColor4f(colr[0], colr[1], colr[2], 0.0f);
            glVertex3f(1.0f, 0.0f, ht + 1.0f);
            glEnd();
#endif
            glPopMatrix(); // 1
        }

// codriver checkpoints rendering
#ifdef INDEVEL

    // codriver checkpoints for debugging purposes
    const vec4f cdcheckpoint_col[3] =
    {
        {0.0f, 0.0f, 1.0f, 0.8f},       // 0 = next checkpoint
        {0.3f, 0.3f, 1.0f, 0.6f},       // 1 = checkpoint after next
        {0.6f, 0.6f, 1.0f, 0.4f}        // 2 = all other checkpoints
    };

        for (unsigned int i=0; i<game->codrivercheckpt.size(); i++)
        {
            vec4f colr = cdcheckpoint_col[2];

            if (game->cdcheckpt_ordered)
            {
                if ((int)i == vehic->nextcdcp)
                    colr = cdcheckpoint_col[0];
                else if ((int)i == (vehic->nextcdcp + 1) % (int)game->codrivercheckpt.size())
                    colr = cdcheckpoint_col[1];
            }
            else
                colr = cdcheckpoint_col[1];

            glPushMatrix(); // 1
            glTranslatef(game->codrivercheckpt[i].pt.x, game->codrivercheckpt[i].pt.y, game->codrivercheckpt[i].pt.z);
            glScalef(15.0f, 15.0f, 1.0f);

            glBegin(GL_TRIANGLE_STRIP);
            float ht = sinf(cprotate * 6.0f) * 7.0f + 8.0f;
            glColor4f(colr[0], colr[1], colr[2], 0.0f);
            glVertex3f(1.0f, 0.0f, ht - 1.0f);
            glColor4f(colr[0], colr[1], colr[2], colr[3]);
            glVertex3f(1.0f, 0.0f, ht + 0.0f);
            for (float a = PI/10.0f; a < PI*2.0f-0.01f; a += PI/10.0f)
            {
                glColor4f(colr[0], colr[1], colr[2], 0.0f);
                glVertex3f(cosf(a), sinf(a), ht - 1.0f);
                glColor4f(colr[0], colr[1], colr[2], colr[3]);
                glVertex3f(cosf(a), sinf(a), ht + 0.0f);
            }
            glColor4f(colr[0], colr[1], colr[2], 0.0f);
            glVertex3f(1.0f, 0.0f, ht - 1.0f);
            glColor4f(colr[0], colr[1], colr[2], colr[3]);
            glVertex3f(1.0f, 0.0f, ht + 0.0f);
            glEnd();

            glBegin(GL_TRIANGLE_STRIP);
            glColor4f(colr[0], colr[1], colr[2], colr[3]);
            glVertex3f(1.0f, 0.0f, ht - 0.0f);
            glColor4f(colr[0], colr[1], colr[2], 0.0f);
            glVertex3f(1.0f, 0.0f, ht + 1.0f);
            for (float a = PI/10.0f; a < PI*2.0f-0.01f; a += PI/10.0f)
            {
                glColor4f(colr[0], colr[1], colr[2], colr[3]);
                glVertex3f(cosf(a), sinf(a), ht - 0.0f);
                glColor4f(colr[0], colr[1], colr[2], 0.0f);
                glVertex3f(cosf(a), sinf(a), ht + 1.0f);
            }
            glColor4f(colr[0], colr[1], colr[2], colr[3]);
            glVertex3f(1.0f, 0.0f, ht - 0.0f);
            glColor4f(colr[0], colr[1], colr[2], 0.0f);
            glVertex3f(1.0f, 0.0f, ht + 1.0f);
            glEnd();
            glPopMatrix(); // 1
        }
#endif
    }

    glEnable(GL_TEXTURE_2D);

    if (game->water.enabled)
        renderWater();

    if (psys_dirt != nullptr) // cfg_dirteffect == false
        getSSRender().render(psys_dirt);

    glDepthMask(GL_TRUE);
    glBlendFunc(GL_ONE,GL_ZERO);
    glEnable(GL_LIGHTING);
    glEnable(GL_CULL_FACE);
    glEnable(GL_FOG);

    glDisable(GL_LIGHTING);

    glPopMatrix(); // 0

    glDisable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix(); // 0
    glLoadIdentity();

    glOrtho(0 - hratio, hratio, 0 - vratio, vratio, 0 - 1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);

    glPushMatrix(); // 1

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (showui)
    {
        game->renderCodriverSigns();

        glPushMatrix(); // 2
        // position of rpm dial and needle
        //glTranslatef( hratio * (1.f - (5.75f/50.f)) - 0.3f, -vratio * (40.f/50.f) + 0.22f, 0.0f);
        glTranslatef( hratio * (1.f - (2.5f/50.f)) - 0.3f, -vratio * (43.5f/50.f) + 0.22f, 0.0f);
        glScalef(0.30f, 0.30f, 1.0f);

        tex_hud_revs->bind();
        glColor3f(1.0f, 1.0f, 1.0f);
        glBegin(GL_QUADS);
        glTexCoord2f(1.0f,1.0f);
        glVertex2f(1.0f,1.0f);
        glTexCoord2f(0.0f,1.0f);
        glVertex2f(-1.0f,1.0f);
        glTexCoord2f(0.0f,0.0f);
        glVertex2f(-1.0f,-1.0f);
        glTexCoord2f(1.0f,0.0f);
        glVertex2f(1.0f,-1.0f);
        glEnd();

        // draw the needle of the RPM dial
        glRotatef(225.0f - vehic->getEngineRPM() * 15.0f / 1000.0f, 0.0f, 0.0f, 1.0f);
        tex_hud_revneedle->bind();
        glColor3f(1.0f, 1.0f, 1.0f);
        glPushMatrix(); // 3
        glTranslatef(0.62f, 0.0f, 0.0f);
        glScalef(0.16f, 0.16f, 0.16f);
        glBegin(GL_QUADS);
        glTexCoord2f(1.0f,1.0f);
        glVertex2f(1.0f,1.0f);
        glTexCoord2f(0.0f,1.0f);
        glVertex2f(-1.0f,1.0f);
        glTexCoord2f(0.0f,0.0f);
        glVertex2f(-1.0f,-1.0f);
        glTexCoord2f(1.0f,0.0f);
        glVertex2f(1.0f,-1.0f);
        glEnd();
        glPopMatrix(); // 3
        glDisable(GL_TEXTURE_2D);
        glPopMatrix(); // 2
    }

    // checkpoint pointing arrow thing
#if 0
    glPushMatrix(); // 2

    glTranslatef(0.0f, 0.8f, 0.0f);

    glScalef(0.2f, 0.2f, 0.2f);

    glRotatef(-30.0f, 1.0f, 0.0f, 0.0f);
    glRotatef(DEGREES(nextcpangle), 0.0f, -1.0f, 0.0f);

    glBegin(GL_TRIANGLES);
    glColor4f(0.8f, 0.4f, 0.4f, 0.6f);
    glVertex3f(0.0f, 0.0f, -2.0f);
    glColor4f(0.8f, 0.8f, 0.8f, 0.6f);
    glVertex3f(1.0f, 0.0f, 1.0f);
    glVertex3f(-1.0f, 0.0f, 1.0f);
    glEnd();
    glBegin(GL_TRIANGLE_STRIP);
    glColor4f(0.8f, 0.4f, 0.4f, 0.6f);
    glVertex3f(0.0f, 0.0f, -2.0f);
    glColor4f(1.0f, 0.5f, 0.5f, 0.6f);
    glVertex3f(0.0f, 0.2f, -2.0f);
    glColor4f(0.8f, 0.8f, 0.8f, 0.6f);
    glVertex3f(1.0f, 0.0f, 1.0f);
    glColor4f(1.0f, 1.0f, 1.0f, 0.6f);
    glVertex3f(1.0f, 0.2f, 1.0f);
    glColor4f(0.8f, 0.8f, 0.8f, 0.6f);
    glVertex3f(-1.0f, 0.0f, 1.0f);
    glColor4f(1.0f, 1.0f, 1.0f, 0.6f);
    glVertex3f(-1.0f, 0.2f, 1.0f);
    glColor4f(0.8f, 0.4f, 0.4f, 0.6f);
    glVertex3f(0.0f, 0.0f, -2.0f);
    glColor4f(1.0f, 0.5f, 0.5f, 0.6f);
    glVertex3f(0.0f, 0.2f, -2.0f);
    glEnd();

    glPopMatrix(); // 2
#endif

    if (showmap)
    {
        // position and size of map
        //glViewport(getWidth() * (5.75f/100.f), getHeight() * (6.15f/100.f), getHeight()/3.5f, getHeight()/3.5f);
        glViewport(getWidth() * (2.5f/100.f), getHeight() * (2.5f/100.f), getHeight()/3.5f, getHeight()/3.5f);

        glPushMatrix(); // 2
        glScalef(hratio, vratio, 1.0f);

        if (game->terrain->getHUDMapTexture())
        {
            glEnable(GL_TEXTURE_2D);
            game->terrain->getHUDMapTexture()->bind();
        }

        glMatrixMode(GL_TEXTURE);
        glPushMatrix(); // 3
        float scalefac = 1.0f / game->terrain->getMapSize();
        glScalef(scalefac, scalefac, 1.0f);
        glTranslatef(campos.x, campos.y, 0.0f);
        glRotatef(DEGREES(camera_angle), 0.0f, 0.0f, 1.0f);
        glScalef(1.0f / 0.003f, 1.0f / 0.003f, 1.0f);

        glBegin(GL_QUADS);
        glColor4f(1.0f, 1.0f, 1.0f, 0.7f);
        glTexCoord2f(1.0f, 1.0f);
        glVertex2f(1.0f, 1.0f);
        glTexCoord2f(-1.0f, 1.0f);
        glVertex2f(-1.0f, 1.0f);
        glTexCoord2f(-1.0f, -1.0f);
        glVertex2f(-1.0f, -1.0f);
        glTexCoord2f(1.0f, -1.0f);
        glVertex2f(1.0f, -1.0f);
        glEnd();

        glPopMatrix(); // 3
        glMatrixMode(GL_MODELVIEW);

        glDisable(GL_TEXTURE_2D);

        glPushMatrix(); // 3
        glScalef(0.003f, 0.003f, 1.0f);
        glRotatef(DEGREES(-camera_angle), 0.0f, 0.0f, 1.0f);
        glTranslatef(-campos.x, -campos.y, 0.0f);
        for (unsigned int i=0; i<game->checkpt.size(); i++)
        {
            glPushMatrix(); // 4
            vec3f vpos = game->checkpt[i].pt;
            glTranslatef(vpos.x, vpos.y, 0.0f);
            glRotatef(DEGREES(camera_angle), 0.0f, 0.0f, 1.0f);
            glScalef(30.0f, 30.0f, 1.0f);
            vec4f colr = checkpoint_col[2];
            if ((int)i == vehic->nextcp)
            {
                float sc = 1.5f + sinf(cprotate * 10.0f) * 0.5f;
                glScalef(sc, sc, 1.0f);
                colr = checkpoint_col[0];
            }
            else if ((int)i == (vehic->nextcp + 1) % (int)game->checkpt.size())
            {
                colr = checkpoint_col[1];
            }
            glBegin(GL_TRIANGLE_FAN);
            glColor4fv(colr);
            glVertex2f(0.0f, 0.0f);
            glColor4f(colr[0], colr[1], colr[2], 0.0f);
            //glColor4f(1.0f, 1.0f, 1.0f, 0.0f);
            glVertex2f(1.0f, 0.0f);
            glVertex2f(0.0f, 1.0f);
            glVertex2f(-1.0f, 0.0f);
            glVertex2f(0.0f, -1.0f);
            glVertex2f(1.0f, 0.0f);
            glEnd();
            glPopMatrix(); // 4
        }
        for (unsigned int i=0; i<game->vehicle.size(); i++)
        {
            glPushMatrix(); // 4
            vec3f vpos = game->vehicle[i]->body->getPosition();
            glTranslatef(vpos.x, vpos.y, 0.0f);
            glRotatef(DEGREES(camera_angle), 0.0f, 0.0f, 1.0f);
            glScalef(30.0f, 30.0f, 1.0f);
            glBegin(GL_TRIANGLE_FAN);
            glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
            glVertex2f(0.0f, 0.0f);
            glColor4f(1.0f, 1.0f, 1.0f, 0.0f);
            glVertex2f(1.0f, 0.0f);
            glVertex2f(0.0f, 1.0f);
            glVertex2f(-1.0f, 0.0f);
            glVertex2f(0.0f, -1.0f);
            glVertex2f(1.0f, 0.0f);
            glEnd();
            glPopMatrix(); // 4
        }
        glPopMatrix(); // 3

        glPopMatrix(); // 2

        glViewport(0, 0, getWidth(), getHeight());
    }

    glEnable(GL_TEXTURE_2D);

    if (showui)
    {
        // Work-around for the "TIME" label once penalty time is displayed
        float time_offset = 0.0f;
        /*
        tex_hud_gear->bind();
        glPushMatrix(); // 2

        glTranslatef(1.0f, 0.35f, 0.0f);
        glScalef(0.2f, 0.2f, 1.0f);
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        glBegin(GL_QUADS);
        glTexCoord2f(1.0f,1.0f); glVertex2f(1.0f,1.0f);
        glTexCoord2f(0.0f,1.0f); glVertex2f(-1.0f,1.0f);
        glTexCoord2f(0.0f,0.0f); glVertex2f(-1.0f,-1.0f);
        glTexCoord2f(1.0f,0.0f); glVertex2f(1.0f,-1.0f);
        glEnd();

        glPopMatrix(); // 2
        */

        tex_fontSourceCodeOutlined->bind();

        // time counter
        glPushMatrix(); // 2

        // time position (other time strings inherit this position)
        // -hratio is left border, 0 is center, +hratio is right
        // hratio * (1/50) gives 1% of the entire width
        // +vratio is top border, 0 is middle, -vratio is bottom
        glTranslatef(-hratio + hratio * (2.5f/50.f), vratio - vratio * (5.5f/50.f), 0.0f);

        // time label
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        glScalef(0.125f, 0.125f, 1.0f);
        if (game->gamestate == Gamestate::finished)
        {
            getSSRender().drawText(
                PUtil::formatTime(game->coursetime),
                PTEXT_HZA_LEFT | PTEXT_VTA_TOP);
        }
        else if (game->coursetime < game->cptime + 1.50f)
        {
            getSSRender().drawText(
                PUtil::formatTime(game->cptime),
                PTEXT_HZA_LEFT | PTEXT_VTA_TOP);
        }
        else if (game->coursetime < game->cptime + 3.50f)
        {
            float a = (((game->cptime + 3.50f) - game->coursetime) / 2);
            glColor4f(1.0f, 1.0f, 1.0f, a);
            getSSRender().drawText(
                PUtil::formatTime(game->cptime),
                PTEXT_HZA_LEFT | PTEXT_VTA_TOP);
        }
        else
        {
            getSSRender().drawText(
                PUtil::formatTime(game->coursetime),
                PTEXT_HZA_LEFT | PTEXT_VTA_TOP);
        }

        // show target time
        glColor4f(0.5f, 1.0f, 0.5f, 1.0f);
        glTranslatef(0.0f, -0.8f, 0.0f);
        getSSRender().drawText(PUtil::formatTime(game->targettime), PTEXT_HZA_LEFT | PTEXT_VTA_TOP);

        {
            // show the time penalty if there is any
            const float timepen = game->uservehicle->offroadtime_total * game->offroadtime_penalty_multiplier;

            if (timepen >= 0.1f)
            {
                glColor4f(1.0f, 1.0f, 0.5f, 1.0f);
                glTranslatef(0.0f, -0.8f, 0.0f);
                getSSRender().drawText(PUtil::formatTime(timepen) + '+', PTEXT_HZA_LEFT | PTEXT_VTA_TOP);
                time_offset = 0.8;
            }
        }

        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        glTranslatef(0.0f, 1.32f + time_offset, 0.0f);
        glScalef(0.65f, 0.65f, 1.0f);
        getSSRender().drawText("TIME", PTEXT_HZA_LEFT | PTEXT_VTA_TOP);

        glPopMatrix(); // 2

        // show Next/Total checkpoints
        {
            // checkpoint counter
            glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
            glPushMatrix(); // 2
            const std::string totalcp = std::to_string(game->checkpt.size());
            const std::string nextcp = std::to_string(vehic->nextcp);

            // checkpoint position
            glTranslatef(hratio - hratio * (2.5f/50.f), vratio - vratio * (5.5f/50.f), 0.0f);
            glScalef(0.125f, 0.125f, 1.0f);

              if (game->getFinishState() != Gamefinish::not_finished)
                  getSSRender().drawText(totalcp + '/' + totalcp, PTEXT_HZA_RIGHT | PTEXT_VTA_TOP);
              else
                  getSSRender().drawText(nextcp + '/' + totalcp, PTEXT_HZA_RIGHT | PTEXT_VTA_TOP);

            // checkpoint label
            glTranslatef(0.0f, 0.52f, 0.0f);
            glScalef(0.65f, 0.65f, 1.0f);
            getSSRender().drawText("CKPT", PTEXT_HZA_RIGHT | PTEXT_VTA_TOP);

            glPopMatrix(); // 2
        }

        // show Current/Total laps
        if (game->number_of_laps > 1)
        {
            const std::string currentlap = std::to_string(vehic->currentlap);
            const std::string number_of_laps = std::to_string(game->number_of_laps);

            glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
            glPushMatrix(); // 2
            glTranslatef(hratio - hratio * (2.5f/50.f), vratio - vratio * (5.5f/50.f) - 0.20f, 0.0f);
            glScalef(0.125f, 0.125f, 1.0f);

            if (game->getFinishState() != Gamefinish::not_finished)
                getSSRender().drawText(number_of_laps + '/' + number_of_laps, PTEXT_HZA_RIGHT | PTEXT_VTA_TOP);
            else
                getSSRender().drawText(currentlap + '/' + number_of_laps, PTEXT_HZA_RIGHT | PTEXT_VTA_TOP);

            glTranslatef(0.0f, 0.52f, 0.0f);
            glScalef(0.65f, 0.65f, 1.0f);
            getSSRender().drawText("LAP", PTEXT_HZA_RIGHT | PTEXT_VTA_TOP);

            glPopMatrix(); // 2
        }

        if (cfg.getEnableFps())
        {
            std::stringstream stream;

            glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
            glPushMatrix(); // 2
            glTranslatef(0.0f, vratio - vratio * (5.5f/50.f), 0.0f);
            glScalef(0.125f, 0.125f, 1.0f);
            stream << std::fixed << std::setprecision(1) << fps;
            getSSRender().drawText(stream.str(), PTEXT_HZA_CENTER | PTEXT_VTA_TOP);

            glTranslatef(0.0f, 0.52f, 0.0f);
            glScalef(0.65f, 0.65f, 1.0f);
            getSSRender().drawText("FPS", PTEXT_HZA_CENTER | PTEXT_VTA_TOP);
            glPopMatrix(); // 2
        }

#ifdef INDEVEL
        // show codriver checkpoint text (the pace notes)
        if (!game->codrivercheckpt.empty() && vehic->nextcdcp != 0)
        {
            glColor3f(1.0f, 1.0f, 0.0f);
            glPushMatrix(); // 2
            glTranslatef(0.0f, 0.3f, 0.0f);
            glScalef(0.1f, 0.1f, 1.0f);
            getSSRender().drawText(game->codrivercheckpt[vehic->nextcdcp - 1].notes, PTEXT_HZA_CENTER | PTEXT_VTA_CENTER);
            glPopMatrix(); // 2
        }
#endif

        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

        tex_fontSourceCodeBold->bind();

        // show current gear and speed
        {
            // gear number
            const int gear = vehic->getCurrentGear();
            const std::string buff = (gear >= 0) ? PUtil::formatInt(gear + 1, 1) : "R";

            glPushMatrix(); // 2
            // position of gear & speed number & label
            //glTranslatef( hratio * (1.f - (5.75f/50.f)) - 0.3f, -vratio * (40.f/50.f) + 0.21f, 0.0f);
            glTranslatef( hratio * (1.f - (2.5f/50.f)) - 0.3f, -vratio * (43.5f/50.f) + 0.21f, 0.0f);
            glScalef(0.20f, 0.20f, 1.0f);
            getSSRender().drawText(buff, PTEXT_HZA_CENTER | PTEXT_VTA_CENTER);

            // speed number
            const int speed = std::fabs(vehic->getWheelSpeed()) * cfg.getHudSpeedoMpsSpeedMult();
            std::string speedstr = std::to_string(speed);

            //glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
            glTranslatef(1.1f, -0.625f, 0.0f);
            glScalef(0.5f, 0.5f, 1.0f);
            getSSRender().drawText(speedstr, PTEXT_HZA_RIGHT | PTEXT_VTA_CENTER);

            // speed label
            glTranslatef(0.0f, -0.82f, 0.0f);
            glScalef(0.5f, 0.5f, 1.0f);

            if (cfg.getSpeedUnit() == PConfig::Speedunit::mph)
                getSSRender().drawText("MPH", PTEXT_HZA_RIGHT | PTEXT_VTA_CENTER);
            else
                getSSRender().drawText("km/h", PTEXT_HZA_RIGHT | PTEXT_VTA_CENTER);

            glPopMatrix(); // 2
        }

        renderDamageIndicatorGroup();

#ifndef NDEBUG
        // draw revs for debugging
        glPushMatrix(); // 2
        glTranslatef(1.17f, 0.52f, 0.0f);
        glScalef(0.2f, 0.2f, 1.0f);
        getSSRender().drawText(std::to_string(vehic->getEngineRPM()), PTEXT_HZA_RIGHT | PTEXT_VTA_TOP);
        glPopMatrix(); // 2
#endif

#ifndef NDEBUG
        // draw real time penalty for debugging
        glPushMatrix(); // 2
        glScalef(0.1f, 0.1f, 1.0f);
        glTranslatef(0.0f, -4.0f, 0.0f);
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        tex_fontSourceCodeOutlined->bind();
        getSSRender().drawText(std::string("true time penalty: ") +
            std::to_string(game->getOffroadTime() * game->offroadtime_penalty_multiplier),
            PTEXT_HZA_CENTER | PTEXT_VTA_TOP);
        glPopMatrix(); // 2
#endif
    }

    tex_fontSourceCodeShadowed->bind();

    // draw "off road" warning sign and text
    if (game->isRacing())
    {
        //const vec3f bodypos = vehic->part[0].ref_world.getPosition();
        const vec3f bodypos = vehic->body->getPosition();

        if (!game->terrain->getRmapOnRoad(bodypos))
        {
            glPushMatrix(); // 2
            glLoadIdentity();
            glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
            glScalef(0.25f, 0.25f, 1.0f);
            tex_hud_offroad->bind();
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
            glPopMatrix(); // 2
            glPushMatrix(); // 2
            glScalef(0.1f, 0.1f, 1.0f);
            glTranslatef(0.0f, -2.5f, 0.0f);
            glColor4f(1.0f, 1.0f, 0.0f, 1.0f);
            tex_fontSourceCodeOutlined->bind();
            getSSRender().drawText(
                std::to_string(static_cast<int> (game->getOffroadTime() * game->offroadtime_penalty_multiplier)) +
                " seconds",
                PTEXT_HZA_CENTER | PTEXT_VTA_TOP);
            glPopMatrix(); // 2
        }
    }

    // draw terrain info for debugging
    #ifdef INDEVEL
    {
        const vec3f wheelpos = vehic->part[0].wheel[0].ref_world.getPosition(); // wheel 0
        const TerrainType tt = game->terrain->getRoadSurface(wheelpos);
        const rgbcolor c = PUtil::getTerrainColor(tt);
        const std::string s = PUtil::getTerrainInfo(tt);

        glPushMatrix(); // 2
        glTranslatef(0.0f, 0.5f, 0.0f);
        glScalef(0.1f, 0.1f, 1.0f);

        if (tt != TerrainType::Unknown)
        {
            const GLfloat endx = s.length() * 8.0f / 12.0f + 0.1f;

            glPushMatrix(); // 3
            glDisable(GL_TEXTURE_2D);
            glTranslatef(-0.5f * s.length() * 8.0f / 12.0f, 0.0f, 0.0f);
            glTranslatef(0.0f, -1.0f, 0.0f);
            glBegin(GL_TRIANGLE_STRIP);
                glColor3f(c.r / 255.0f, c.g / 255.0f, c.b / 255.0f);
                glVertex2f(-0.2f,   0.0f);
                glVertex2f(endx,    0.0f);
                glVertex2f(-0.2f,   1.1f);
                glVertex2f(endx,    1.1f);
            glEnd();
            glEnable(GL_TEXTURE_2D);
            glPopMatrix(); // 3
        }

        glColor3f(1.0f, 1.0f, 1.0f);
        getSSRender().drawText(s, PTEXT_HZA_CENTER | PTEXT_VTA_TOP);
        glPopMatrix(); // 2
    }
    #endif

    // draw if we're on road for debugging
    //#ifdef INDEVEL
    #if 0
    {
        const vec3f wheelpos = vehic->part[0].wheel[0].ref_world.getPosition(); // wheel 0
        std::string s;
        rgbcolor c;

        if (game->terrain->getRmapOnRoad(wheelpos))
        {
            c = rgbcolor(0xFF, 0xFF, 0xFF);
            s = "on the road";
        }
        else
        {
            c = rgbcolor(0x00, 0x00, 0x00);
            s = "off-road";
        }

        const GLfloat endx = s.length() * 8.0f / 12.0f + 0.1f;

        glPushMatrix(); // 2
        glTranslatef(0.0f, 0.25f, 0.0f);
        glScalef(0.1f, 0.1f, 1.0f);
        glPushMatrix(); // 3
        glDisable(GL_TEXTURE_2D);
        glTranslatef(-0.5f * s.length() * 8.0f / 12.0f, 0.0f, 0.0f);
        glTranslatef(0.0f, -1.0f, 0.0f);
        glBegin(GL_TRIANGLE_STRIP);
            glColor3f(c.r / 255.0f, c.g / 255.0f, c.b / 255.0f);
            glVertex2f(-0.2f,   0.0f);
            glVertex2f(endx,    0.0f);
            glVertex2f(-0.2f,   1.1f);
            glVertex2f(endx,    1.1f);
        glEnd();
        glEnable(GL_TEXTURE_2D);
        glPopMatrix(); // 3
        glColor3f(1.0f, 1.0f, 1.0f);
        getSSRender().drawText(s, PTEXT_HZA_CENTER | PTEXT_VTA_TOP);
        glPopMatrix(); // 2
    }
    #endif
    {
        tex_fontSourceCodeOutlined->bind();

        glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
        glPushMatrix(); // 2
        glTranslatef(0.0f, 0.2f, 0.0f);
        glScalef(0.6f, 0.6f, 1.0f);
        if (game->gamestate == Gamestate::countdown)
        {
            float sizer = fmodf(game->othertime, 1.0f) + 0.5f;
            glScalef(sizer, sizer, 1.0f);
            getSSRender().drawText(
                PUtil::formatInt(((int)game->othertime + 1), 1),
                PTEXT_HZA_CENTER | PTEXT_VTA_CENTER);
        }
        else if (game->gamestate == Gamestate::finished)
        {
            if (game->getFinishState() == Gamefinish::pass)
            {
                glColor4f(0.5f, 1.0f, 0.5f, 1.0f);
                getSSRender().drawText("WIN", PTEXT_HZA_CENTER | PTEXT_VTA_CENTER);
            }
            else
            {
                glScalef(0.5f, 0.5f, 1.0f);
                glColor4f(0.5f, 0.0f, 0.0f, 1.0f);
                getSSRender().drawText("TIME EXCEEDED", PTEXT_HZA_CENTER | PTEXT_VTA_CENTER);
            }
        }
        else if (game->coursetime < 1.0f)
        {
            glColor4f(0.5f, 1.0f, 0.5f, 1.0f);
            getSSRender().drawText("GO!", PTEXT_HZA_CENTER | PTEXT_VTA_CENTER);
        }
        else if (game->coursetime < 2.0f)
        {
            float a = 1.0f - (game->coursetime - 1.0f);
            glColor4f(0.5f, 1.0f, 0.5f, a);
            getSSRender().drawText("GO!", PTEXT_HZA_CENTER | PTEXT_VTA_CENTER);
        }
        glPopMatrix(); // 2

        if (game->gamestate == Gamestate::countdown)
        {
            glPushMatrix(); // 2
            glTranslatef(0.0f, 0.6f, 0.0f);
            glScalef(0.08f, 0.08f, 1.0f);
            if (game->othertime < 1.0f)
            {
                glColor4f(1.0f, 1.0f, 1.0f, game->othertime);
                getSSRender().drawText(game->comment, PTEXT_HZA_CENTER | PTEXT_VTA_CENTER);
            }
            else
            {
                glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
                getSSRender().drawText(game->comment, PTEXT_HZA_CENTER | PTEXT_VTA_CENTER);
            }
            glPopMatrix(); // 2
        }

        if (pauserace)
        {
            glPushMatrix(); // 2
            glColor4f(0.25f, 0.25f, 1.0f, 1.0f);
            glScalef(0.25f, 0.25f, 1.0f);
            getSSRender().drawText("PAUSED", PTEXT_HZA_CENTER | PTEXT_VTA_CENTER);
            glPopMatrix(); // 2
        }
    }

    glPopMatrix(); // 1

    glMatrixMode(GL_PROJECTION);
    glPopMatrix(); // 0
    glMatrixMode(GL_MODELVIEW);

    glBlendFunc(GL_ONE, GL_ZERO);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

void MainApp::renderDamageIndicator(
        const PTexture *texture, float posx, float posy, float scalex, float scaley, float damage)
{
    float red = 0.0f;
    float green = 0.0f;
    float blue = 0.0f;

    if (damage > 1.0f)
        damage = 1.0f;

    if (damage < 0.0f)
    {
        red = 1.0;
        green = 1.0;
        blue = 1.0;
    }
    else if (damage < 0.5f)
    {
        red = 2.0f * damage;
        green = 1.0;
    }
    else
    {
        red = 1.0;
        green = 2.0 * (1.0 - damage);
    }

    glPushMatrix(); // 2

    glTranslatef(posx, posy, 0.0f);
    glScalef(scalex, scaley, 1.0f);
    texture->bind();
    glColor4f(red, green, blue, 0.5f);

    glBegin(GL_QUADS);
    glTexCoord2f(1.0f,1.0f);
    glVertex2f(1.0f,1.0f);
    glTexCoord2f(0.0f,1.0f);
    glVertex2f(-1.0f,1.0f);
    glTexCoord2f(0.0f,0.0f);
    glVertex2f(-1.0f,-1.0f);
    glTexCoord2f(1.0f,0.0f);
    glVertex2f(1.0f,-1.0f);
    glEnd();

    glPopMatrix(); // 2
}

void MainApp::renderDamageIndicatorGroup()
{
    // Theoretically there can be multiple vehicles with multiple parts.
    // However, the assumption is, that the damage of the first part of the first vehicle is relevant.
    PVehiclePart &part = game->vehicle[0]->part[0];

    renderDamageIndicator(
        tex_damage_front_left, hratio * 45.0f/50.0f - 0.075f, -vratio * 32.5f/50.0f + 0.032f, 0.025f, 0.032f,
        part.damage.getDamage(PDamage::DamageSide::DamageFrontLeft));
    renderDamageIndicator(
        tex_damage_front_right, hratio * 45.0f/50.0f - 0.025f, -vratio * 32.5f/50.0f + 0.032f, 0.025f, 0.032f,
        part.damage.getDamage(PDamage::DamageSide::DamageFrontRight));
    renderDamageIndicator(
        tex_damage_rear_left, hratio * 45.0f/50.0f - 0.075f, -vratio * 32.5f/50.0f - 0.032f, 0.025f, 0.032f,
        part.damage.getDamage(PDamage::DamageSide::DamageRearLeft));
    renderDamageIndicator(
        tex_damage_rear_right, hratio * 45.0f/50.0f - 0.025f, -vratio * 32.5f/50.0f - 0.032f, 0.025f, 0.032f,
        part.damage.getDamage(PDamage::DamageSide::DamageRearRight));
}

void MainApp::renderVehiclePart(const PVehicleType &type, const PVehiclePart &part,
    const PVehicleTypePart &typepart, float alpha)
{
    if (typepart.model)
    {
        glPushMatrix();

        vec3f vpos = part.ref_world.getPosition();
        glTranslatef(vpos.x, vpos.y, vpos.z);

        mat44f vorim = part.ref_world.getInverseOrientationMatrix();
        glMultMatrixf(vorim);

        float scale = typepart.scale;
        glScalef(scale,scale,scale);

        drawModel(*typepart.model, alpha);

        glPopMatrix();
    }

    if (type.wheelmodel)
    {
        for (unsigned int i=0; i<typepart.wheel.size(); ++i)
        {
            glPushMatrix();

            vec3f wpos = part.wheel[i].ref_world.getPosition();
            glTranslatef(wpos.x,wpos.y,wpos.z);

            mat44f worim = part.wheel[i].ref_world.getInverseOrientationMatrix();
            glMultMatrixf(worim);

            float scale = type.wheelscale * typepart.wheel[i].radius;
            glScalef(scale,scale,scale);

            drawModel(*type.wheelmodel, alpha);

            glPopMatrix();
        }
    }
}
