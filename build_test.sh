#!/bin/sh

dir=$(dirname $0)
if [ $(uname) = "Darwin" ]; then
    brew_prefix=$(brew --prefix)
    ${CXX:-clang++}                        \
        -I$brew_prefix/opt/openssl/include \
        -L$brew_prefix/opt/openssl/lib     \
        -std=c++98                         \
        -pedantic                          \
        -O2                                \
        $@                                 \
        -Wno-variadic-macros -Wall         \
        $dir/test.cc                       \
        -lm -lstdc++                       \
        -lcrypto -lcurl                    \
        -o oppai_test
else
    ${CXX:-g++}                    \
        -std=c++98                 \
        -pedantic                  \
        -O2                        \
        $@                         \
        -Wno-variadic-macros -Wall \
        $dir/test.cc               \
        -lm -lstdc++               \
        -lcrypto -lcurl            \
        -o oppai_test
    [ -f ./oppai_test ] && strip -R .comment ./oppai_test
fi
