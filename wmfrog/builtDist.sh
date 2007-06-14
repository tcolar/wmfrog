#!/bin/sh
read -p "Version:" version
rm Src/*.o Src/wmfrog
tar czvf dist/wmfrog-${version}.tgz --exclude='*.svn' Src CHANGES COPYING HINTS INSTALL
cd Src
make clean
make
cd ..
cp Src/wmfrog dist
cp Src/weather.pl dist
