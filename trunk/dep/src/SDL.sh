#!/bin/bash

cd SDL
./autogen.sh
./configure.dvj
make
sudo make install
cd ..

