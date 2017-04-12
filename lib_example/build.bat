@echo off

del test.exe
del test.obj
cl  -D_CRT_SECURE_NO_WARNINGS=1 ^
    -DNOMINMAX=1 ^
    -DOPPAI_LIB=1 ^
    -O2 ^
    -nologo -MT -Gm- -GR- -EHsc -W4 -WX ^
    -wd4201 ^
    -wd4100 ^
    -F8000000 ^
    main.cc ^
    -Fetest.exe ^
    Advapi32.lib
