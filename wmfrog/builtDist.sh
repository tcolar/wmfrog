#!/bin/sh
read -p "Version:" version
cd Src
make clean
make
cd ..
cp Src/wmfrog dist
cp Src/weather.pl dist

make -C Src clean
tar czvf dist/wmfrog-${version}.tgz --exclude='*.svn' Src CHANGES COPYING HINTS INSTALL
