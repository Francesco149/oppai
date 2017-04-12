#!/bin/sh

${CXX:-g++}         \
    -std=c++98      \
    -pedantic       \
    -O2             \
    $@              \
    -Wno-variadic-macros \
    -Wall -Werror   \
    main.cc         \
    -lm -lstdc++    \
    -lcrypto        \
    -DOPPAI_LIB=1   \
    -o test         \
                    \
&& strip -R .comment ./test
