#!/bin/bash

export CFLAGS=-arch i386
cd jpeg-6b
./configure.dvj
make
sudo make install
cd ..

