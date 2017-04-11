#!/bin/sh

${CXX:-g++}         \
    -std=c++98      \
    -O2             \
    -Wall -Werror   \
    main.cc         \
    -lm -lstdc++    \
    -lcrypto        \
    -o oppai        \
                    \
&& strip -R .comment ./oppai
