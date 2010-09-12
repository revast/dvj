#!/bin/bash

#Linux hint: Use ./configure.dvj.lin instead of ./configure.dvj...

cd SDL
./autogen.sh
./configure.dvj
make
sudo make install
cd ..

