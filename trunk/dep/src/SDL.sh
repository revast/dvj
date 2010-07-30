#!/bin/bash

cd SDL
./autogen.sh
./configure
make
sudo make install
cd ..

