DT1 Tools, by Paul SIRAMY, FREEWARE  BETA v0.97c       29 december 2002
===================================

This a package of simple programs for the managment of the DT1 files (the
graphism of floors and walls of Diablo II). They don't use the GUI of Windows,
they are for the command-line. The sources are provided, so if a talented
programmer stop by here... Keep in mind that these progs aren't as powerfull
as they could be, and their size is also too big for what they do, but at
least they works.

There are 4 progs.

DT1debug.exe  : a simple viewer, with some debug info
DT1extr.exe   : extract the datas from a dt1 into pcx and ini
DT1make.exe   : make a dt1 from the previous pcx and ini
DT1info.exe   : a "dumper", produce two .txt for excel

Let's take an exemple.

Say you want to modify some floors / walls in Tristram (the dt1 file
from d2data.mpq : data\global\tiles\act1\tristram\town.dt1 is already
in this directory).

Launch the "1-DT1debug Tristram.bat".
You can see all the graphism there are in the dt1. The interface's keys are
indicated on the screen. Look around for a while, then quit.

Launch the "2-DT1extr Tristram.bat".
It will extract the graphism and data you have seen in 3 files :
* town-fs.pcx
* town-wu.pcx
* town.ini

They are named after the dt1 name. For the pcx, behind the name stand a code.
-fs stand for "Floor Static"
-fa stand for "Floor Animated" (like the lava in act 4)
-wu stand for "Walls Up"
-wd stand for "Walls Down" (some cliff in expansion's siege)

The ini is a sort of a dump of the other data that the dt1 have, but that we
can't draw on a bitmap. Like tile's sound, floor flags, orientation...

By editing the pcx and / or the ini you can make another dt1. Be carefull
with the ini, just change the "tile sound", or the "floor flags", if needed,
but you better have to leave the ini in its original state (except for test
purpose, or if you really know what you're doing).

Now, edit some pcx : put some tags on the walls, draw flowers on floors ...
Do what you want, and save it.

Now, launch "3-DT1make Tristram.bat". It make a dt1 from the ini and pcx.
It is always named after the ini name, but with an "NEW_" in front, to
avoid an eventual overwrite of an older dt1.

----> Now you can use the dt1 in a mod. Put it in the good directory, and
      don't forget to rename it (no "NEW_" in the name)

As a last debug info, you can launch "4-dt1info tristram.bat", but you'll
barely need it. It was useful when I was working on the format of the dt1
but you can easily forget it.


## ADDED BY Gimli#4667 ## How to make changes (2025-04-28):

1. Extract pcx from dt1
2. Load pcx with Photoshop
3. Image > Mode > Color Tables > Save > #directory#
4. Image > Mode > RGB (This way I am able to use layers, which is greatly needed)
5. Edit Image
6. Image > Mode > Indexed Color > Pallete > Custom > Load > #directory of saved .act file#
7. Save as bmp (optional - only if .pcx file export does not work somehow)
8. Use Photoshop/XnView/ACDSee to save as/convert to .pcx
9. Pack pcx and infos into dt1
10. check the dt1 file if all is fine