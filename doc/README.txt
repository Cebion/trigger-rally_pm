

///////////////////////////
// Trigger v0.6.x README //
///////////////////////////


Thanks for downloading Trigger! I hope you enjoy
the game. For updates, please visit:

  http://trigger-rally.sourceforge.net/


Before playing, building, or copying, please read
the license later in this file. If you don't agree
to it, you should delete this software.


Look at ~/.trigger/trigger.config in your favourite text editor
for configuration options.


///////////////////
// Configuration //
///////////////////


Defaults:

 key     action
Up      Accelerate
Down    Foot brake / Reverse
Left    Steer left
Right   Steer right
Space   Handbrake
C       Change camera view
M       Toggle map
N       Toggle user interface
K       Toggle checkpoints
R       Recover car
Q       Recover car at previous checkpoint
P       Pause race
< >     Rotate camera when in 3rd person cam
F12     Save a screenshot to ~/.trigger/


Joystick support can be enabled in the
config file. I'd be interested in any
reports on the performance of the
joystick support.


Stereo support is available, for both quad buffer
hardware and anaglyph glasses. See README-stereo.txt
Kudos to Chuck Sites for this patch.


/////////////
// License //
/////////////


Trigger Copyright (C) 2004-2006 Jasmine Langridge and Richard Langridge.

Trigger is released under the GPL version 2 (see COPYING,
or http://www.gnu.org/licenses/gpl-2.0.html ).

Some of the data files have different authors and licenses.
Please see DATA_AUTHORS.txt for this information.


//////////////////////
// Content Creation //
//////////////////////

A. Car models
-------------

Trigger ships car models in both .ase and .obj formats. The historic 
.ase models are used by default. You can easily switch to the FOSS 
.obj textures by renaming `{car}-foss.vehicle` files to `{car}.vehicle`.
For example, (optionally) backup `focus.vehicle`, then rename 
`focus-foss.vehicle` to `focus.vehicle`.

You can add completely new cars in the Wavefront .obj format with the
following restrictions:
 
  - Use only one material per .obj.
  - All faces must be triangles.
  - The material is ignored, only the texture defined by it gets loaded.
  - All meshes, besides the wheels, need to be on a single object.
  - Wheels are in their own files.
 
When exporting from Blender, following settings work:

  - "Apply Modifiers"
  - "Include Normals"
  - "Include Edges"
  - "Write Materials"
  - "Triangulate Faces"
  - "Objects as OBJ Objects"
 
To test the model, replace the file name in an existing .vehicle file inside

data/vehicles/VEHICLENAME/

 
B. Levels/maps/tracks
---------------------
 
Use any tool that saves .png or .jpg images, for example GIMP and Inkscape.

You will need to create a heightmap, color map and need to assign coordinates.
You can create optional foliage and hud-map image files.

View existing .level files in data/maps or data/plugins/ to learn.

Best use .png for heightmaps, as .jpg can cause artefacts, which will change
the level geometry.

C. Car textures
---------------

We used Inkscape to create the .svg files for the car textures.

You might have to get the following freely licensed fonts from sites like http://www.google.com/fonts/ to be able to render the currently used .svg files to .png correctly, if you don't want to download the packages that contain them:
  - TeX Gyre Heros
  - Roboto
  - Bowlby One
  - URW Gothic L


/////////////////////
// Troubleshooting //
/////////////////////


If something goes wrong, the you should check
your settings in ~/.trigger/trigger.config. You can
delete that file to reset everything to default. (It
will be recreated next time you run Trigger.)

Failing that, please visit Sourceforge to ask for help
on the forums or file a bug on the tracker:

http://sourceforge.net/projects/trigger-rally/support


/////////////////////
// Version history //
/////////////////////

30/01/2016 - Trigger 0.6.3
  - Improve menu and in-race OSD
  - Add codriver for 75% of maps
  - Tweak terrain physics
  - Add many new maps
  - Change vehicle skins and presets
  - Made various minor code improvements

05/05/2015 - Trigger 0.6.2
  - Fix several Windows bugs and compilation issues
  - Fix compilation warnings
  - Windows binaries

25/10/2014 - Trigger 0.6.1
  - Add support for .obj textures
  - Include new .obj textures (disabled by default)
  - Add 6 new events and 23 new single races courtesy of Onsemeliot
  - Add several new textures
  - Replace most media content (textures, audio, fonts, etc.) with 
    FOSS equivalents
  - New FOSS Trigger Rally icon
  - Many other changes (check SVN logs from about r48)

08/10/2011 - Trigger 0.6.0
  - New (and old) contributed tracks and events
  - New Practice Mode
  - Paging on the Single Races screen to show all the available tracks
  - Option to show speedometer in KPH or MPH
  - Option to show digital speed on the speed dial ('hybrid' style)
  - Fading track comment and GO at race start
  - Freezing course time when passing through a checkpoint
  - Tweak menu colours for more contrast

4/07/2010 - Trigger 0.5.3
  - Removed splash screen delay
  - Updated contact email address

20/10/2006 - Trigger 0.5.2.1
  - gcc 4 fixes
  - PhysFS/OpenAL interaction fix
  - PhysFS/SDL interaction fix
    Thanks to Stefan Potyra for the patches!

11/01/2005 - Trigger 0.5.1a,b,c
  - Fixed joystick deadzone and added a maxrange (unfortunately there's
    still no user-friendly joystick calibration)
  - Most of the engine migrated to RAII design
  - Other minor stuff

12/12/2004 - Trigger 0.5.1
  - Switch to ARB multitex from core GL to support older cards
  - Added some code to take screenshots

05/10/2004 - Trigger 0.5.0 (was 0.4.5)
  - Chuck Sites has contributed a stereo patch, so now we have
    quadbuffer and anaglyph stereo!

03/10/2004 - Trigger 0.4.4.1
  - PhysFS linked statically with linux binary

01/10/2004 - Trigger 0.4.4
  - Windows build back online (fixed FMOD/PhysFS integration)
  - Another license change, because apparently CCSA isn't considered
    Free by Debian. Now the whole thing is GPL. See "License."

01/10/2004 - Trigger 0.4.4-pre2
  - Menu fixes: forgot to show times/lives left in pre1

20/09/2004 - Trigger 0.4.4-pre1
  - using PhysFS http://icculus.org/physfs/
  - added auto ~/.trigger creation for config and extensions
  - new menu system, with auto searching for tracks and events
  - rule change: total time incremented even if you fail an attempt
  - new control config system, hopefully better joystick support
  - per-level weather settings
  - camera rotate: < and >
  - fixed: boost::format was crashing libstdc++, removed
  - friction model changed to better simulate dirt

09/09/2004 - Trigger 0.4.3
  - config SDL GL settings
  - config keyboard controls
  - config sound enable/disable
  - experimental joystick support (see data/trigger.config)
  - vehicle crunch sound effects
  - fixed: controls not responding when joystick connected

06/09/2004 - Trigger 0.4.2
  - Text configuration file
  - More physics tweaks, and simple driving assist config setting
  - extgl replaced with GLEW
  - License altered, now 100% Free Software

05/09/2004 - Trigger 0.4.1
  - Physics tweaks and driving assist

04/09/2004 - Trigger 0.4 (First public release)
  - Lots of coolness


/////////////
// Credits //
/////////////


Code                Jasmine Langridge <ja-reiko@users.sourceforge.net>

Art & sfx           Richard Langridge

Build system
and Jamrules        Matze Braun <matze@braunis.de>

Stereo support      Chuck Sites <chuck@cvip.uofl.edu>

Mac OS X porting    Tim Douglas
                    Coleman Nitroy

Fixes & distro      LavaPunk <lavapunk@gmail.com>
support             Bernhard Kaindl
                    Stefan Potyra
                    Liviu Andronic
                    Ishmael Turner
                    Andrei Bondor

New levels          Tim Wintle
                    David Pagnier
                    Jared Buckner
                    Andreas Rosdal
                    Ivan
                    Viktor Radnai
                    Pierre-Alexis
                    Bruno "Fuddl" Kleinert
                    Agnius Vasiliauskas
                    Matthias Keysermann
                    Márcio Bremm
                    Onsemeliot

Splash screens      Alex <rep65@tlen.pl>
                    Roberto D�z Gonz�ez

Mirroring           Washu (irc.afternet.org #gamedev)
                    daaw.org
                    sh.nu

obj model support
and conversion      Farrer <farpro@users.sourceforge.net>

Freely licensed
art replacements    Iwan Gabovitch <qubodup@gmail.com>

Many thanks to all contributors!

I haven't been keeping strict records of who has
contributed what. If you've been left out, or if
you'd like your email to be listed, please email
me. (ja-reiko@users.sourceforge.net)

