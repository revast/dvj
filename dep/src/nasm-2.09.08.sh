#!/bin/bash

cd nasm-2.09.08
./autogen.sh
./configure
make
sudo make install
cd ..

