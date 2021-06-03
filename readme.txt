FRONTIER 1337 - v0.3.0 - www.atien.net

FRONTIER 1337 is a port for PSP from the Atari version of the game "FRONTIER - Elite II".
I used Tom Morton's sourcecode of his GLFrontier PC port. The whole merit goes to him.

For a better framerate result, set the level of shape detail to 'low' in the setup screen (start button),
and disable things like 'Space dust, clouds etc' and 'Background stars'...

You can find very useful informations on FRONTIER - Elite II at Frontierverse :
http://www.sharoma.com/frontierverse/index.htm


INSTALLATION
------------
unpack/copy FRONTIER_1337 folder to MS_ROOT/PSP/GAME

Download the MUSIC and SFX packs if you want, and place the folders in the FRONTIER_1337/data folder.
http://www.atien.net/?static2/frontier


CONTROLS
--------
Analog-stick - Move the cursor through the screen
Hold R trigger while moving the mouse cursor for a more slow and precise movment
SQUARE - Used like the left mouse clic to select things with the cursor
CROSS - Used like the right mouse clic + "p" key to get through the police arrest thing
Hold CROSS + Analog-stick - Roll the vessel
Hold CROSS + SQUARE - Fire laser beam
Hold CROSS + D-pad UP - Zoom-in with the camera in external view
Hold CROSS + D-pad DOWN - Zoom-out with the camera in external view
D-pad - Rotate the camera in external view, and make ravel the galactic chart or the stockmarket list
TRIANGLE - Rear propulsion to increase speed
CIRCLE - Front propulsion to decrease speed
L and R triggers - Choose the functions that where directly available on PC with F1 to F10 keys
L + R - Validate the choosen function
Start - Go to the option screen
Select - Switch between radar and missiles
R + D-pad UP - Force mis-jump
R + Select - Show/hide game engine FPS


SAVEFILES
---------
Don't try to change commander's name when you are saving your game.
If you want to change it, rename the file in the savs folder.

Savefiles are compatible with GLFrontier and the Amiga version.


SHINY THINGS TO DO
------------------
- Improve game speed!
- Implement OSK for changing the commander's name when saving your game.
- Add quick save/load shortcuts


COMPILATION
-----------
Use Minimalist PSPSDK to compile the game (for unknown reasons, the game only compiles with the 0.10.0 version).
http://sourceforge.net/projects/minpspw/files/SDK%20%2B%20devpak/pspsdk%200.10.0/
Your computer must have loads of RAM to compile the huge generated C file.


BENCHMARKS
----------
Length of the intro sequence from the first image to the apparition of the menu.
AMIGA - 03:25 <- ultimate goal
0.3.0 - 07:08
0.2.1 - 10:18


VERSIONS HISTORY
----------------
version 0.3.0
- 30% of speed improvement!
- Choice between fullscreen and original resolution at the begining.
- Select button is now also used to switch between radar and missiles.
- The game engine FPS can be show/hide by pressing right trigger plus select button.
- Now, you have to press CROSS button to pass the police arrest.
- Hold R trigger for moving the mouse cursor slowly (thanks JLF65 from DCEmu for the help).
- Hold R trigger and press D-pad UP to force mis-jump.
- Brand new PSP splashscreen and icon.

version 0.2.1
- The game is now compatible with PSP recent firmwares.

version 0.2
- The game is now running at 333MHz.
- The music has been enabled.
- All wav sounds and ogg music have been resampled to 44100Hz and are now working perfectly.
- All sub-folders (but savs) have been moved to data folder.
- The Fkeys highliter has been turned from red to black for not interfering with the intro and outro of the game.

version 0.1.2
- The game was compiled with fe2.part1.c optimized up to Os, thanks to Danzel.
  With the details set to 'low' in the option screen, the game is now playable when you are on a spaceport.

version 0.1.1
- The option screen will now show itself with the start button.
- The select button becomes a temporary keyboard hit to pass the police arrest bug.

version 0.1.0
- The game is almost playable when few objects are drawn on the screen (like in space).
- The sound is working but is a little crappy, no music yet.
- Save/load file is working.
- Don't try to rename the save file in the game as there is no OSK implemented yet and it will freeze the game.
  If you want to change your commander name, just rename the save file in FRONTIER/SAVS directory using your computer.

  
CREDITS
-------
David Braben for having done one of the best games ever back in 1993.
http://www.frontier.co.uk/

Tom Morton for his GLFrontier sourcecode and his advises.
http://www.tomatarium.pwp.blueyonder.co.uk/glfrontier.html

The guys at PS2DEV, Danzel and his NASA machine for compiling the huge generated C file.
http://forums.ps2dev.org/