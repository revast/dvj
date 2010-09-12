#!/bin/bash

#Linux hint: Use ./configure instead of ./configure.dvj...

cd SDL
./autogen.sh
./configure.dvj
make
sudo make install
cd ..

