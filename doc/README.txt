

///////////////////////////
// Trigger v0.5.x README //
///////////////////////////


Thanks for downloading Trigger! I hope you enjoy
the game. For updates, please visit:

  http://www.positro.net/trigger/


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
< >     Rotate camera when in 3rd person cam
F12     Save a screenshot


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

Trigger is released under the GPL (see gpl.txt in
source package, or http://www.gnu.org/licenses/gpl.txt ).

NO WARRANTY WHATSOEVER.


Note that this differs from previous versions, in which
the accompanying data files were released under the
Creative Commons ShareAlike 1.0 license.


///////////////
// Compiling //
///////////////


Source dependencies:
	Jam
		(http://www.perforce.com/jam/jam.html)
	PhysFS
		(http://icculus.org/physfs/)
	SDL
		(http://www.libsdl.org/)
	SDL_image
		(http://www.libsdl.org/projects/SDL_image/)
	
	OpenAL on linux platform
		(http://www.openal.org/)
	
	FMOD on windows platform
		(http://www.fmod.org/)
		(i386-mingw32msvc is the only tested build)

To build trigger:

./configure
jam

You'll probably want to check ./configure --help for
available options, although the defaults are usually
fine. Those interested in packaging trigger will want
to specify --datadir=...

Thanks to Matze Braun for this excellent new build system.


/////////////////////
// Troubleshooting //
/////////////////////


If something goes wrong, the first thing to check is
your settings in data/trigger.conf.

Failing that, check back to the game website to see
if the bug has been discovered (I may have posted a
workaround.)

If it still doesn't work, please send me an email at:

jas@jareiko.net

and I'll get back to you as soon as I can. I can't
promise to be able to fix the bug however :|


/////////////////////
// Version history //
/////////////////////


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


Code                Jasmine Langridge <jas@jareiko.net>
Art & sfx           Richard Langridge <r_langridge@wincoll.ac.uk>

Build system
and Jamrules        Matze Braun <matze@braunis.de>
Stereo support      Chuck Sites <chuck@cvip.uofl.edu>
Mac OS X porting    Tim Douglas
                    Coleman Nitroy
Fixes & distro      LavaPunk <lavapunk@gmail.com>
support             Bernhard Kaindl
                    Stefan Potyra
New levels          Tim Wintle
                    David Pagnier
                    Jared Buckner
                    Andreas Rosdal
                    Ivan
                    Viktor Radnai
                    Pierre-Alexis
Splash screens      Alex <rep65@tlen.pl>
                    Roberto D�z Gonz�ez

Mirroring           Washu (irc.afternet.org #gamedev)
                    daaw.org
                    sh.nu

Many thanks to all contributors!

I haven't been keeping strict records of who has
contributed what. If you've been left out, or if
you'd like your email to be listed, please email
me. (jas@jareiko.net)

