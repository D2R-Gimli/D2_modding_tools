<#
.SYNOPSIS
    This script scans the current folder for all .dt1 files and generates
    individual batch files to extract each .dt1 file with a selected palette.

.DESCRIPTION
    1. Finds all .dt1 files in the current directory.
    2. Lists the filenames found.
    3. Prompts the user to select a palette (1-5).
    4. For each .dt1 file, creates a new batch file named:
        <originalfilename>_extract.bat
       containing the command:
        dt1extr <originalfile.dt1> -pal d2pal\actX.dat
       where X corresponds to the selected palette number.
#>

# Step 1: Get current directory and all .dt1 files
$CurrentDir = Get-Location
$DT1Files = Get-ChildItem -Path $CurrentDir -Filter *.dt1

# Step 2: Check if any .dt1 files exist
if ($DT1Files.Count -eq 0) {
    Write-Host "No .dt1 files found in the current folder." -ForegroundColor Red
    exit
}

# Step 3: Display found .dt1 files
Write-Host "Found the following .dt1 files:`n" -ForegroundColor Cyan
$DT1Files | ForEach-Object { Write-Host $_.Name }

# Step 4: Ask user which palette to use
do {
    $Palette = Read-Host "`nSelect a palette number (1-5)"
} while ($Palette -notmatch "^[1-5]$")  # ensures only 1-5 is accepted

# Step 5: Generate a batch file for each .dt1 file
foreach ($File in $DT1Files) {
    # Extract base filename without extension
    $BaseName = [System.IO.Path]::GetFileNameWithoutExtension($File.Name)
    
    # Construct batch filename
    $BatchFileName = "$BaseName`_extract.bat"
    
    # Construct batch content
    $BatchContent = "@echo off`r`ndt1extr $($File.Name) -pal d2pal\act$Palette.dat`r`n"
    
    # Write content to batch file
    Set-Content -Path $BatchFileName -Value $BatchContent -Encoding ASCII
    
    Write-Host "Created batch file: $BatchFileName" -ForegroundColor Green
}

Write-Host "`nAll batch files created successfully!" -ForegroundColor Cyan
