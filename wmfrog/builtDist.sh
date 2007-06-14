#!/bin/sh

#clean
rm Src/*.o Src/wmfrog

#source tgz
tar czvf dist/wmfrog.tgz Src CHANGES COPYING HINTS INSTALL

#deb  TBD