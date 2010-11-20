#!/bin/bash

cd oscpack
make	#or, on linux: make -f Makefile.linux
sudo make install
#ignore warning about ldconfig
cd ..


