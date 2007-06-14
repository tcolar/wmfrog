#!/bin/sh
rm Src/*.o Src/wmfrog
tar czvf dist/wmfrog.tgz --exclude='*.svn' Src CHANGES COPYING HINTS INSTALL
cd Src
make clean
make
cd ..
cp Src/wmfrog dist
cp Src/weather.pl dist
