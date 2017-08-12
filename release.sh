#!/bin/sh

dir=$(dirname $0)

git pull origin master

echo -e "\nCompiling and Stripping"
. "$dir"/build.sh -static || exit 1

echo -e "\nPackaging"
folder="oppai-$(./oppai -version)-"
folder="${folder}$(uname | tr '[:upper:]' '[:lower:]')-$(uname -m)"
mkdir -p "$folder"
mv ./oppai $folder/oppai
cp ./LICENSE $folder/LICENSE

rm "$folder".tar.xz
tar -cvJf "$folder".tar.xz \
    "$folder"/oppai \
    "$folder"/LICENSE

echo -e "\nResult:"
tar tf "$folder".tar.xz

readelf --dynamic "$folder"/oppai
ldd "$folder"/oppai

