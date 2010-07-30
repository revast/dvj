#!/bin/bash

cd fftw-3.2.1
./configure.int.dvj
make
sudo make install
./configure.float.dvj
make
sudo make install
cd ..

