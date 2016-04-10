================================
Trigger Rally ReadMe for Windows
================================

1. Default game keys
2. Game configuration
3. Installing plugins
4. Getting help
5. Miscellaneous notes

--------------------
1. Default game keys
--------------------

    KEY             ACTION
    ----------------------
    Up Arrow        accelerate
    Down Arrow      brake/reverse
    Left Arrow      steer left
    Right Arrow     steer right
    Space           handbrake
    R               recover car
    Q               recover car at last checkpoint
    P               pause race
    C               change camera view
    K               toggle checkpoints
    M               toggle map
    N               toggle on-screen display
    F12             take screenshot
    ,               rotate camera view left
    .               rotate camera view right
    ----------------------

These key bindings can be changed in the game configuration...

---------------------
2. Game configuration
---------------------

When Trigger Rally is run for the first time, it creates a ".trigger-rally" folder
in your home directory, where it copies its default configuration and player profiles.
This is also where it saves screenshots and your own player profile.

    (Windows XP)
    C:\Documents and Settings\UserName\.trigger-rally\

    (Windows Vista and later)
    C:\Users\UserName\.trigger-rally\

In this folder you will find a text file named "trigger-rally-VERSION.config" which
can be edited with Notepad in order to change the game settings.

---------------------
3. Installing plugins
---------------------

The Trigger Rally plugins, consisting mostly of maps, can be downloaded from:

    https://sourceforge.net/projects/trigger-rally/files/plugins/

They can be installed by simply copying them to the game's "plugins" folder:

    (example location)
    C:\Program Files\Trigger Rally\data\plugins\map-monza.zip

---------------
4. Getting help
---------------

If you have questions on configuring the game, or if you experience bugs,
please join the Discussion forums at:

    https://sourceforge.net/p/trigger-rally/discussion/

If you experience crashes you may be asked to provide a run log.
You can create a run log by starting the RunLog batch file:

    trigger-rally.RUNLOG.cmd

which is located in the "bin" folder of Trigger Rally.

It will run the game and save a text file named "trigger-rally.log"
in your Trigger Rally game configuration folder (see 2. above).

----------------------
5. Miscellaneous notes
----------------------

In the Windows Setup release of Trigger Rally, the contents of the "data" folder
are distributed in a .ZIP file named "data-VERSION.zip". Apparently the game files
will load faster if they are stored in an uncompressed .ZIP archive.
