@echo off

mkdir ..\build
pushd ..\build
del oppai.exe
del oppai.obj
cl  -D_CRT_SECURE_NO_WARNINGS=1 ^
    -DNOMINMAX=1 ^
    -O2 ^
    -nologo -MT -Gm- -GR- -EHsc -W4 -WX ^
    -wd4201 ^
    -wd4100 ^
    -wd4458 ^
    -F8000000 ^
    ..\main.cc ^
    -Feoppai.exe ^
    Advapi32.lib
popd
