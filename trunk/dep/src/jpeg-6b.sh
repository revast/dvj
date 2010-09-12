#!/bin/bash

#LINUX Notes: You may need to replace jpeg-6b/libool with ln -s /usr/bin/libtool...

export CFLAGS=-arch i386
cd jpeg-6b
./configure.dvj
make
sudo make install
cd ..

