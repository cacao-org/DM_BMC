#!/bin/bash

IFLAGS="-I/opt/Boston/include/ -I/usr/local/milk/include/ImageStreamIO"
LFLAGS="-L/opt/Boston/lib/ -L/usr/local/milk/lib/"
lFLAGS="-lBMC -lBMC_PCIeAPI -lBMC_USBAPI -lImageStreamIO -lBMC_OpenLoop -lpthread"
OTHFLAGS="-Wl,-rpath=/usr/local/milk/lib:/opt/Boston/lib/"

GCCLINE="gcc createBMCshm.c -o createBMCshm ${IFLAGS} ${LFLAGS} ${lFLAGS} ${OTHFLAGS}"

$GCCLINE

sudo chown root:root ./runBMChex111
sudo chmod u+s ./runBMChex111
