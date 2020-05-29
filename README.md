# cacao hardware interface to BMC 2k DM


To build:

	gcc -o runBMCcacao runBMCcacao.c -L../DMoptical/lib/ -L/usr/local/milk/lib -I../DMoptical/src/inc/ -I/usr/local/milk/include/ImageStreamIO -lrt -lbmcmd -lm -lpthread -lImageStreamIO -Wl,-rpath=/usr/local/milk/lib

Required files:
- runBMCcacao.c
- bmc_mdlib.h
- libbmcmd.a
- ImageStreamIO shared library

To run loop, execute :

	./runBMCcacao -K      # initialize board for 2k DM
	./runBMCcacao -D 0    # set driver spin delay count to 0
	./runBMCcacao -L      # real time control loop

If sudo permission required, make sure sudo inherits LD_LIBRARY_PATH to find libimagestreamio:

	sudo -E env LD_LIBRARY_PATH=${LD_LIBRARY_PATH} ./runBMCcacao -K 

