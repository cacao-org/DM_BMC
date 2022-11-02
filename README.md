# cacao hardware interface to BMC 2k DM


##[DEPRECATED 2022/11/01]

Merged into [scexao-org/hardwaresecrets](https://github.com/scexao-org/hardwaresecrets) under (respectively) `interfaces/bmc2k` and `interfaces/bmc111`.

See you there!

## Previous content.

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
	
Or all in one: 
    ./runBMCcacao -A      (equiv -K, -D 0, -L)

If sudo permission required, make sure sudo inherits LD_LIBRARY_PATH to find libimagestreamio:

	sudo -E env LD_LIBRARY_PATH=${LD_LIBRARY_PATH} ./runBMCcacao -K 

