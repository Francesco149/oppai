#!/bin/sh

dir=$(dirname $0)

source "$dir"/cflags.sh
ldflags="-lcrypto -lcurl $ldflags"

if [ $(uname) = "Darwin" ]
then
    brew_prefix=$(brew --prefix)
    cflags="$cflags -I$brew_prefix/opt/openssl/include"
    cflags="$cflags -L$brew_prefix/opt/openssl/lib"
fi

$cxx $cflags "$@" "$dir"/test.cc $ldflags -o oppai_test

