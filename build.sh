#!/bin/sh

${CXX:-g++}         \
    -std=c++0x      \
    -O2             \
    -Wall -Werror   \
    main.cc         \
    -lm -lstdc++    \
    -o oppai        \
                    \
&& strip -R .comment ./oppai
