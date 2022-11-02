#!/bin/bash

sudo rm run_2k_main

gcc -o run_2k_main run_2k.c funcs_2k.c -L../DMoptical/lib/ -L/usr/local/milk/lib -I../DMoptical/src/inc/ -I/usr/local/milk/include/ImageStreamIO -lrt -lbmcmd -lm -lpthread -lImageStreamIO -Wl,-rpath=/usr/local/milk/lib

sudo chown root:root ./run_2k_main
sudo chmod u+s ./run_2k_main
