#!/bin/sh

#clean
rm Src/*.o Src/wmfrog

#source tgz in dist
tar czvf dist/wmfrog.tgz Src CHANGES COPYING HINTS INSTALL

#build and copy to dist
cd Src
make
cp wmfrog dist
cp weather.pl dist

#deb  TBD