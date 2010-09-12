#!/bin/bash

#Linux hint: Use ./configure.dvj.lin instead of ./configure.dvj

cd SDL_image-1.2.7
./configure.dvj
make
sudo make install
cd ..

