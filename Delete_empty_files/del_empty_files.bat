@ECHO OFF
SET topLevel=%CD%
FOR /D /R %%D IN (*) DO (
  CD %%D 
  CALL :innerLoop
)
CD %topLevel%
FOR /F "usebackq delims=" %%D IN (`"DIR /AD/B/S | SORT /R"`) DO RD "%%D"
GOTO :break

:innerLoop
  FOR /F "delims=" %%F IN ('DIR/B/A-D/OS') DO IF %%~zF EQU 0 (DEL "%%F") ELSE (GOTO :break)

:break