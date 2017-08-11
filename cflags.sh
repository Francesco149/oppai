#!/bin/sh

cflags="-std=c++98 -pedantic"
cflags="$cflags -O3"
cflags="$cflags -Wno-variadic-macros -Wall"
cflags="$cflags -ffunction-sections -fdata-sections"
cflags="$cflags -g0 -fno-unwind-tables -s"
cflags="$cflags -fno-asynchronous-unwind-tables"

ldflags="-lm -lstdc++"

cflags="$cflags $CXXFLAGS"
ldflags="$ldflags $LDFLAGS"

cxx=$CXX

if [ $(uname) = "Darwin" ]; then
    cxx=${cxx:-clang++}
else
    cxx=${cxx:-g++}
fi

(
    uname -a
    echo $cxx
    echo $cflags
    echo $ldflags
    $cxx --version
) \
> flags.log

export cflags="$cflags"
export ldflags="$ldflags"
export cxx="$cxx"

