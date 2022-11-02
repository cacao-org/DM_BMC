#!/bin/bash

if [ -z "$MILK_ROOT" ]; then
    echo "MILK_ROOT env variable undefined! Aborting."
    exit 1
fi
ISIO_ROOT=${MILK_ROOT}/src/ImageStreamIO

if [ -z "$MILK_INSTALLDIR" ]; then
    echo "MILK_INSTALLDIR env variable undefined! Aborting."
    exit 1
fi

BMC_DIR="/opt/Boston"

IFLAGS="-I${BMC_DIR}/include/ -I${ISIO_ROOT}"
LFLAGS="-L${BMC_DIR}/lib/ -L${MILK_INSTALLDIR}/lib/"
lFLAGS="-lBMC -lBMC_PCIeAPI -lBMC_USBAPI -lImageStreamIO -lBMC_OpenLoop -lpthread"
OTHFLAGS="-Wl,-rpath=${MILK_INSTALLDIR}/lib:${BMC_DIR}/lib/"

gcc createBMCshm.c -o createBMCshm ${IFLAGS} ${LFLAGS} ${lFLAGS} ${OTHFLAGS}