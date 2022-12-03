**** AFJ Sheet Edit - version 0.61

I know. It's been a long time since i last released anything to this program, but personal stuff got in the way
Actually only a few issues has been fixed and the program should not need any mfc dll's anymore.

Enjoy


**** Version 0.61 Features [11-10-2004]
You are now able copy/paste into a single row
Hide/Delete now also works on columns/rows even if the first Row/Column is fixed
Insert Column/Row now also works if you only have one cell selected
The program is now staticaly linked with MFC 7.0, no MFC DLL's needed anymore


**** Version 0.60b Features [07-08-2004]
Fixed Menu bug (Fill/Increment Fill)
Fixed Rollover when using Trunc/Round
Fixed Copy/Cut/Paste, wont work for selected text in a cell
Selecting cells with CTRL and then hit Delete doesn't crash the program anymore
Added automatic "0"ing of 'eol' or '*eol' rows
Copying a Cell and selects random cells to paste into doesn't crash the program anymore
Changed so FindDialog only can be opened once
Search box no regains focus after end sarch message box
Number in Math functions now have focus by default
Number in Cell Operations Dialog now have focus by default
The time to mass clear cells, has now been greatly increased
Added Usage of customizable fonts
Added Timed backup
Undo now recognizes numbers and normal text when undoing
The Shell extension has been changed. You should now be able to open closed files again

**** Version 0.55b Features [29-04-2004]
Added 'One instance Only' option
Added Sheet Tab Control and an option to disable it
Fixed a small bug regarding Drag'n'Drop

**** Version 0.54b Features [04-03-2004]
Improved Load/Save Code
Added a Shell Extension
Enabled Drag & Drop
'Round', 'Trunc' and 'Apply Function' menu added
Multiple Backup Files
Added "weak" Undo support

**** Version 0.53b bug fixes and Features [16-02-2004]
Sorting Function added
BUG: Fixed memory Bug
BUG: Fixed decimal number bug
BUG: Fixed Search function

**** Version 0.52b bug fixes and Features [09-12-2003]
BUG: Fixed the apperance of "box"`s in some rows
BUG: Fixed the Fix button for coloumn got checked even thou it isn't fixed
BUG: Fixed an Cell selection bug, when you selected a cell inside a selected area the ficus wouldn't change
Moved "Cell Fixing" to View menu and added Accelerators

**** Version 0.51b bug fixes and Features [01-12-2003]
Added more Keyboard Shortcuts
New 'Math' menu
5 More Options
BUG: Math in cells using '=' would only work until you get an '#Expression Invalid'
BUG: On some machines the Program wouldn't run, dues to some hidden option. Default state of that
	option has been changed to the opposite.
	(HKEY_CURRENT_USER\\Sorftware\\Afj\\SheetProg\\One Instance)

**** Version 0.50b bug fixes and Features [29-11-2003]
First Beta Release
New Search/Replace Dialog
New Toolbar
Options is moved to Windows Registry
Entered numbers doesn't get converted like 1.2e+4 anymore (Works for expressions now also)
Toolbars is now not Dockable anymore (eg. you cannot move them)
You can load Multiple Files at the same time in the File Open Dialog





**** Version 0.05a bug fixes and features [13-11-2003]
ATTENTION: Change the quit Dialog to the opposite!!!
copy/past to right-click menu
Name title not appearing when loading with double_click
Fixed Fill function
Shortcuts to Search buttons
Multi Add/Insert Rows/Cols
option to auto fix row/column when loading
Copy/Paste no longer splits text at ","
Non-Alpha characters behave like normal characters
Entered numbers doesn't get converted like 1.2e+4 anymore (Only works for entered values not in expressions)


**** Version 0.04a bug fixes and features [22-09-2003]
Added Clone Row menu command
Added more math commands
Fixed Search Routines


**** Version 0.03a bug fixes and features [13-09-2003]
Change default behaviour of Enter and Arrow keys
Included a Fill function like in Excel
Added mathfunctions support for cells
It is possible to create custom functions based on Hardcoded ones
Basic math in cells


**** Version 0.02a bug fixes and features [31-08-2003]
Fixed instance bug
Fixed multible row/column delete bug. 
Fixed center text on insert column bug
Cell with value is now right aligned
Changed menus
Inplemented Search Routine
Autosize on loading (optional)
Hiding columns/rows
First column/row can be fixed
Cell Swap


**** Version 0.01a features [29-08-2003]
Tabbed .txt file edit
More .txt files open at one time in same program
Add/Remove/Insert and Copy/Cut/Paste on rows/columns
Backup of edited files (optional)


**** Features not implemented yet
Print
Moving Cells

**** Operators
+	add and unary positive sign
-	subtract and unary negative sign
*	multiply
/	divide
^	square
%	modulus

**** Functions - 1 parameter only
Bare in mind that trigonometric functions take Radians as their input

cos		Cosine
acos		Arccosine
cosh		Hyperbolic Cosine
sin		Sine
asin		Arcsine
sinh		Hyperbolic Sine
tan		Tangent
atan		Arctangent
tanh		Hyperbolic Tangent
log		Logarithm based on 10
ln		Natuaral Logarithm
exp		Exponential
abs		Absolute
floor		Nearest Lowest
ceil		Nearest Highest
trunc		Remove fragtion eg. 2.234 = 2
round		Round up or down to integer eg 2.234 = 2 and 2.56 = 3
frag		Remove first part eg 2.234 = 0.234
sqrt		Square Root
rad		convert Degree to Radians
deg		Convert Radians to Degree
grad		Convert Degree to Grad
grad2deg	Convert Grad to Degree
and		Logical And
or		Logical Or
xor		Logical Xor
not		Logical Not


**** Functions - 2 or more parameters
atan2( y : x )	arctangent operation on y/x
pow( x : n )	does the same a ^ does
root( x : n )	take the n'th root of x
cell( y : x )	get value from cell


**** SFL and UFL instructions
If you want comments in the files use '#'

Basic function declraration:

	function_name[<parameter_count>] = <expression>

functions_name: 	the name you want the functions to be named
parameter_count: 	number of parameters in the expression. Must be at least 1 and no higher than 9
			Each parameter is name like arg<num> in the expression
expression: 		normal expression

Example:

	sum[2] = log(arg1) + log(arg2)

you would then call the function in a cell like this:

	=sum(3:4)	this would be the same as: =log(3)+log(4)

The '=' is to call the function parser from a cell. The ':' is the splitter for parameters
Each file have to end with at least an empty or commented line

**** 'Apply Function' menu
You can apply all the basic math and SFL UFL commands to all slected cells, using this menu.
Three Variables are supported currently:

$c = cell content (eg. take the value that the cell contains)
$x = col number
$y = row number

fx:
convert the cell values to their sinus equlivant and add 3 you do:

	sin($c)+3

if you want the cell just above instead you do:

	sin(cell($y-1:$x))+3

and a simple inc from above cell:

	cell($y-1:$x)+1

**** Credits

Paul Siramy
Joel
InhalantNut
4ever~|link
Myhrginoc
Drackin
Darque
Adhin
Drac0
Grave
SVR
mouse
Sloovy
DeClineD
luoriming
Sduibek
matze
BoomWav

Testers:
Onyx
Kingpin


*************************************************************
Prior Releases:

0.60b

I Large update this time. I have added a few new things as well as fixed (i hope) i big amount of bugs

Cut/Copy/Paste now works like it does in MS Excel 2003, this mean you cannot select multiple single cells, that aren't
next to each other and copy/cut them anymore.
Also the program now put a '0' in the last column if the column title is either 'eol' or '*eol'.
The grid now also supports font changes. You can change the font used in the Edit->Options dialog box, here you can also
se a new option: "Timed Backup". Enabling this optin will make a save og the current work into <path><filename>.tim. This
file will be deleted when you close the file.
The last thing added this time is a 'Close' function in the 'File' menu
Enjoy

0.551b

A small bugfix

Apparantly the "Fill Cells" function didn't work. Bug removed

0.55b
Just a small update.

Added the One Instance only option so people who doesn't like only one instance the program running can disable that.

Added Sheet Tab control, to keep track of what sheets you have open. Can be disabled from the options menu.

0.54b
Just a So, this time i have use almost a week on making the new version.  have beeing improving load and save speed, so it wont
take forever to load the animdata.tx created by Paul's animdata_edit tool. Also you can open i more than one time now.

A Shell Extension is also added this time. I have placed it in data\Extension along with two .bat files. The file
names should be easy enough to understand. Just remember you have to reboot to remove the Extension from memory, thou
it is not needed to reboot to disable it.

More fun: Drag and Drop is implemented. Now you can drag the files into you progam to open them. Also it is now possible to open more than one file at the command line.

Menu Functions: in the Math menu 3 new options is now added. Round and Trunc is two of them. they are working in the same way as the 'round' and 'trunc' commands does.
The third menu is the "Apply Function", now this is a strong menu function for applying whatever you need onto the cells. Look in the "Math Func" section below, to see what you can do.

Options: one small option. Backup Files is now numbered like this <orgfilename>-<number>.bak. lowest number is the most recent file. In the option dialog you can set how many backup files you want.

Last but not Least: Undo, yes that's right, I Finaly enabled it.
HOWEVER!!!! This only works if you edit the cells or select some cells and press 'Delete'. I have only enabled 10 levels
of this undo so if you make a change 11th times the 1st undo will be removed automaticly.
Expect more news on this in future versions, suggestions are welcome.

0.53b:
This is just a smal update to a few bugs that have been found. Thou a small search function is added
under the context menu.

0.52b:
Only a little over a week has gone this time before a new update.
This time it's mainly BUG killing time, so there isn't much to say, other than the "Cell Fxing" menu
have been moved from "edit" to "view" and has added Accelerators to them:

Alt + 1	=	Fix Collumn
Alt + 2 =	Fix Row

Another accelerator that I didn't mention before, mainly because I didn't know it was enabled:

Ctrl + Tab	Switch Txt Window

0.51b:
Okay it have been a few days, and i have already made some new updates. Someone stuffed some ideas into my head
and i just have to add them to the editor. There has been added a few things to options area:

Hide Top Row: hides the row with the letters (eg. A, B, C, etc.)
Don't Fix Column: Doesn't fix the 1st column when selecting "Fix Fist Column and Row"
Cell Width/Height & Font Size: The text says it all ;)

A new Menu is also added: Math
This menu makes it possible do the simplest math in multiple cell's a time

Added a few more Keyboard Shortcuts:
Ctrl + N 	= New
Ctrl + O	= Open
Ctrl + S 	= Save
Ctrl+ Shift + s = Save As...

0.50b:
So now I have decided to move to beta stage with the Sheet Editor. This forces me ofcourse to replace the
Search routine with a Search/Replace routine and i have got it to work just fine. The dialog now doesn't
close when you click "Find Next" it stays open but moves the marker to the text. "Replace" replaces the text
where the marker is at. The Repeat Seach Button functions like it did before.
A new toolbar has also entered the scene. It contains a Progress bar that show the file load. It also includes
an edit field where you can edit the text also. Every time you select a new Cell the text will also be placed
in this field. The green "V" sign is used when you enter text in the edit field. All marked cells will be
replaced with the text in the edit field.
Of other changes:
There's no need for the IniLoad.dll and SheetCfg.ini files anymore as i have moved all options to the Windows
Registry. In the Menu "Edit" you find an "Options" choise wich shows you an Option Dialog. Simply change the
options in here.

0.05a:
I have been lazy in tha last two months. Not a single update. But anyway here is the 0.05a version.
Mainly bug fixes.

0.04a:
Minor update. Clone row have been introduced this time. Also a fix for the search routines
should be in order now. There has also been space for a few new commands. New commands are marked
with a {NEW}

0.03a:
This new release have a few changes and most of all, a function parser.
each time you enter '=' as the first character in a cell, it will invoke the parser
basic +-/*^% is included as well as some basic math functions, like 'cos' and 'sin'.
Look at the bottom for a complete list of functions. What more is, there is also included
a SFL (Standard Function Library) and UFL (User Function Library). Both librarys is text
based files called sfl.dat and ufl.dat contained in the .\data dir. The SFL lib you should
not change as it is possible to change for each release. The UFL lib you can change as you
wish, since I have no intention to include UFL's in each release. If ufl.dat isn't in the
.\data dir, you can create it yourself