@echo off

mkdir ..\build
pushd ..\build
cl 	-D_CRT_SECURE_NO_WARNINGS=1 ^
	-DNOMINMAX=1 ^
	-nologo -MTd -Gm- -GR- -EHa- -W4 -WX ^
	-wd4201 ^
	-wd4100 ^
	..\main.cc
popd
