@echo off
for %%f in (NEW_*.dt1) do (
    set "filename=%%f"
    setlocal enabledelayedexpansion
    ren "%%f" "!filename:~4!"
    endlocal
)
exit