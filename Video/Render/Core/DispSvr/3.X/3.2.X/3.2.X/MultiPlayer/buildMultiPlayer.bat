@REM You can build Release or Debug version with this .bat. As default, it's set as Release version unless you set BUILDCONFIG=Debug
set BUILDTARGET=BuildMultiPlayer
@REM You can change SSROOTDIR into the special Macroaccording to your P4 real root working DIR, such as: D:\Usr\sean\p4
set SSROOTDIR=..\..\..\..
nmake /nologo /e /f makeMultiPlayer %BUILDTARGET%
pause