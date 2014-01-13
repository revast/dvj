#!/bin/bash

cd autoconf-2.69
make clean
cd ..

cd automake-1.14.1
make clean
cd ..

cd libtool-2.4
make clean
cd ..

cd jpeg-6b
make clean
cd ..

cd libpng-1.2.46
make clean
cd ..

cd SDL
make clean
cd ..

cd SDL_image-1.2.7
make clean
cd ..

cd SDL_net-1.2.7
make clean
cd ..

cd fftw-3.2.1
make clean
cd ..

cd libsamplerate-0.1.8
make clean
cd ..

cd yasm-1.0.1
make clean
cd ..

cd ffmpeg
make clean
cd ..

cd jack-audio-connection-kit-0.118.0
sudo rm /usr/local/lib/libjack*
sudo rm -rf /usr/local/lib/jack
make clean
cd ..

cd nasm-2.09.08
make clean
cd ..

cd libjpeg-turbo-1.1.1
make clean
cd ..

rm clean
