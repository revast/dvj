#!/bin/bash

#Linux hint: You may need to replace jpeg-6b/libool with ln -s /usr/bin/libtool...

cd jpeg-6b
./configure.dvj
make
sudo mkdir -p /usr/local/man/man1
sudo make install
cd ..

