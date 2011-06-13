#!/bin/bash

#Linux hints:
#Use ./configure instead of ./configure.dvj...
#If you need autoconf:
#sudo apt-get install autoconf2.13 

cd SDL
./autogen.sh
./configure
make
sudo make install
cd ..

