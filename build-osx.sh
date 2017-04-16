#!/bin/sh

${CXX:-clang++}     \
    -I$(brew --prefix)/opt/openssl/include \
    -L$(brew --prefix)/opt/openssl/lib \
    -std=c++98      \
    -pedantic       \
    -O2             \
    $@              \
    -Wno-variadic-macros \
    -Wall -Werror   \
    main.cc         \
    -lm -lstdc++    \
    -lcrypto        \
    -o oppai
