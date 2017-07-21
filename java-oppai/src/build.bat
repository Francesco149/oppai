@echo off

del oppai.dll
del build.obj
del oppai.exp
del oppai.lib

set CXXFLAGS=%CXXFLAGS% /I"C:\Program Files\Java\jdk1.8.0_131\include"
set CXXFLAGS=%CXXFLAGS% /I"C:\Program Files\Java\jdk1.8.0_131\include\win32"
cl /D_CRT_SECURE_NO_WARNINGS=1 ^
	/DNOMINMAX=1 /DOPPAI_LIB=1 ^
	/O2 ^
	/nologo /MD /LD /GR /EHsc /W4 /WX ^
	/wd4201 ^
	/wd4100 ^
	/wd4458 ^
	/wd4800 ^
	/F8000000 ^
	%CXXFLAGS%  ^
	/Fe"oppai.dll" ^
	Build.cpp ^
	Advapi32.lib

	