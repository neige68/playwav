@rem <install.bat> -*- coding: cp932-dos -*-
@echo off
rem
rem Project playwav
rem Copyright (C) 2023 neige68
rem
rem Note: install
rem
rem Windows:   XP and lator
rem

setlocal

replace build\release\playwav.exe %LOCALBIN%\ /a
replace build\release\playwav.exe %LOCALBIN%\ /u

@rem end of <install.bat>
