**** AFJ tbl Edit - version 1.10

This tbl-editor is more an "upgrade" to Darkstorms tbl editor. Most functions is still in the program
but more functions has been added like color codes and the like
Most functions has a Accelerator key bound to them so you can use those instead of using the menu all
the time. Also most functions are slef explaniory.
Any Comments and suggestions you might wanto share, do it at:
http://phrozenkeep.planetdiablo.gamespy.com/forum/viewtopic.php?t=15454

Enjoy
AFJ666

**** CREDITS ****
Kingpin
Paul Siramy
Joel
Evil Peer
pmpch
Kurziel
Forsaken
Myrhginoc
Sduibek
Necrolis
Nashi
Sir Xavius

**** Version 1.12 and 1.12u bug fixes and changes [23-02-2008]
-= Both  =-
Removed the right click menu (For now)

-= 1.12  =-
Fixed crash when saving.

-= 1.12u =-
Fixed Text Import and Export bug. (1.12u exports in Unicode txt file format)
Fixed color code bug


**** Version 1.11 and 1.11u bug fixes and changes [09-01-2008]
IMPORTANT:	Two versions are available. 1.11 and 1.11u. The 'u' version is a UNICODE supported version.
		However it hasn't been tested on any other system that my own, with Chinese fonts installed
		and it seems to work fine, both saving, loading and displaying.

Fixed the index numbers of Patchstring.tbl and Expansionstring.tbl
Added an option to use last used directory when loading and saving


**** Version 1.10 bug fixes and changes [06-10-2007]
Changed how options are handled. It is now saved in the registry, and added a menu for it too
Added a new window, so you can see the index number of a string in each .TBL file (the last icon on the bar)
Added a select all to the ListBox menu, and in the Edit menu as well
fixed the 'Enter' and 'ESC' bug. 'Enter' now does nothing, and 'ESC' simply closes the program in the right way
changed some keyboard shortcuts. Ctrl + A now selects all in the list box. Ctrl + N and 'Instert' adds a new string.
MFC dll's are no longer required

**** Version 1.00 bug fixes [13-01-2004]
fixed empty index entry when adding new indexes
Txt Import now doesn't import empty rows/cols
Export function to tabbed txt file added
added Hex value to index counter in the statusbar
(fixed a small bug, but let the tool keep the version number)

**** Version 0.52b bug fixes [21-09-2003]
Added Tool-Tip to Item Button
Fixed Commandline bug.
Fixed Tbl import bug

**** Version 0.51b bug fixes [08-09-2003]
Fixed the delete bug
Fixed the "space-bug" in .ini file

**** Version 0.50b bug fixes and features [05-09-2003]
First Beta release
Fixed multi-line items text-bug
commandline load of .tbl files
Keyboard Shortcuts added for most features
Copy/Cut/Paste for List. Currently only works when right-clicking
Multiline Deletion
Item-show text box (stil under development)
Default dir option included




**** Version 0.15a bug fixes and features [21-08-2003]
Fixed a memory bug, when saving a new tbl file
Added an .ini file with a few options + IniLoad.dll to load the ini-file
Tbl Import now functional - Import and overwrite existing only
extra feature when importing tabbed .txt files. It's now possible to import and overwrite existing entries.
changed colorcodes to match Peer Tbl editor (look at the end of this readme.txt


**** Version 0.14a bug fix - [17-08-2003]
CRC function was buggy. Changed it so it does the same as Enquettar script


**** Version 0.13a bug fixes - [15-08-2003]
fixed the anoying flicker when entering text or when changin selection in the list


**** Version 0.12a features - [15-08-2003]
Add/Edit/Delete keys (double click to edit)
Search functions
Color menu + color codes
Basic tabbed .txt file import
Fast load/save
Various visual features


**** Features not implemented yet
Backup of old file, when saving

**** Color codes
Folowing here is the color codes you can use:
(last changed 21-08-2003)

\red;
\green;
\dgreen;
\blue;
\white;
\yellow;
\orange;
\gold;
\tan;
\purple;
\grey;
\dgrey;
\black;

other codes
\n = newline. This can be used in the editor as well as in the imported .txt files.
