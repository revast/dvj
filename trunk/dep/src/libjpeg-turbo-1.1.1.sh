#!/bin/bash

cd libjpeg-turbo-1.1.1
./autogen.sh
./configure.dvj
make
sudo make install
cd ..

