#!/bin/bash

rm -rf automake-1.14.1
tar -xf automake-1.14.1.tar
cd automake-1.14.1
./configure
make
sudo make install
cd ..

