@echo off
REM ===============================================================
REM Batch Script: Create DT1 Extract Batches
REM ===============================================================
REM Description:
REM   1. Finds all .dt1 files in the current folder.
REM   2. Lists them for the user.
REM   3. Asks the user to select a palette (1-5).
REM   4. Creates a new batch file for each .dt1 file:
REM        <filename>_extract.bat
REM      containing:
REM        dt1extr <filename>.dt1 -pal d2pal\actX.dat
REM      where X is the user-selected palette number.
REM ===============================================================

REM Step 1: Check if there are any .dt1 files
SETLOCAL ENABLEDELAYEDEXPANSION
SET dt1found=0
FOR %%F IN (*.dt1) DO (
    SET dt1found=1
    GOTO :FoundFiles
)
IF %dt1found%==0 (
    ECHO No .dt1 files found in this folder.
    PAUSE
    EXIT /B
)

:FoundFiles
REM Step 2: List all .dt1 files
ECHO Found the following .dt1 files:
FOR %%F IN (*.dt1) DO (
    ECHO %%F
)

REM Step 3: Ask the user for palette number
:AskPalette
SET /P palette=Choose a palette number (1-5): 
REM Validate input
IF "%palette%"=="1" GOTO PaletteSelected
IF "%palette%"=="2" GOTO PaletteSelected
IF "%palette%"=="3" GOTO PaletteSelected
IF "%palette%"=="4" GOTO PaletteSelected
IF "%palette%"=="5" GOTO PaletteSelected
ECHO Invalid choice. Please enter a number between 1 and 5.
GOTO AskPalette

:PaletteSelected
REM Step 4: Create a batch file for each .dt1 file
FOR %%F IN (*.dt1) DO (
    REM Get filename without extension
    SET filename=%%~nF
    REM Construct batch filename
    SET batchname=!filename!_extract.bat
    REM Write content to batch file
    (
        ECHO @echo off
        ECHO dt1extr %%F -pal d2pal\act%palette%.dat
    ) > "!batchname!"
    ECHO Created batch file: !batchname!
)

ECHO.
ECHO All batch files created successfully!
PAUSE
