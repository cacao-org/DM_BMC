/*!
 * \file BMCExample.c
 *
 * The BMCExample command line application performs two tests and is intended as
 *  a simple verification that the library installed correctly. It will first
 *  piston the DM between two values for 5 cycles then poke each actuator.
 *  Depending on input options, BMCExample can advance automatically with a
 *  specified delay or advance on user input. The help message displayed when
 *  the program is run explains the options and their format.
 */

/*******************************************************************************
 * This example code is distributed under the following conditions.

Copyright (c) 2013-2017, Boston Micromachines Corporation
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted (subject to the limitations in the disclaimer
below) provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of the copyright holder nor the names of its contributors may be used
  to endorse or promote products derived from this software without specific
  prior written permission.

NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS
LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
DAMAGE.
*******************************************************************************/

#include "stdafx.h"

#include <BMCApi.h>

#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include <time.h>

#include <ImageStruct.h>   // cacao data structure definition
#include <ImageStreamIO.h> // function ImageStreamIO_read_sharedmem_image_toIMAGE()

#if defined(_WIN32)
#include <conio.h>
#define strnlen strnlen_s
#define getch _getch
#elif defined(__linux__)
#include <unistd.h>
#include <termios.h>
char getch(void);	// Defined below
#endif

#if defined(_MSC_VER) && (_MSC_VER < 1900)
#define snprintf _snprintf_s
#endif // _MSC_VER

#define DMVOLT_FILENAME "dmvolt"
#define DMPTT_FILENAME "dmptt"

typedef int bool_t;
#ifndef TRUE
#define TRUE	1
#endif
#ifndef FALSE
#define FALSE	0
#endif

/*==========================================================================*/


int main(int argc, char* argv[])
{
	DM hdm;
	BMCRC rv;
	IMAGE *SMdmvolt;




	/////// INIT SHARED MEM STUFF
	IMAGE *bmc_array; 	// pointer to array of images
	long naxis;				// number of axis
	uint8_t atype;			// data type
	uint32_t *imsize;  		// image size 
	int shared;        		// 1 if image in shared memory
	int NBkw; 				// number of keywords supported
	int CBsize;


    uint32_t *map_lut;
    char serial_number[BMC_SERIAL_NUMBER_LEN+1];
    char app_name[BMC_MAX_PATH] = "";
	int k, i=0;
	int select = 0;
	int err_count = 0;
	int pistons = 10;
	int delay_us = 500;
    char ch = '\0';
	bool_t continuous = FALSE;
	bool_t auto_exit = FALSE;
	bool_t ask_SN = TRUE;
	
	// Parse options
    snprintf(app_name, sizeof(app_name), "%s", argv[0]);
	
	argc--;
	



	// Add Static Serial Number
	// FIRST : strcpy(serial_number, "32AW005#010");
	strcpy(serial_number, "32AW038#027");

	// Open driver
    memset(&hdm, 0, sizeof(hdm));
	rv = BMCOpen(&hdm, serial_number);
	if(rv) {
		printf("Error %d opening the driver type %u.\n", rv, (unsigned int)hdm.Driver_Type);
		printf("%s\n\n", BMCErrorString(rv));
		return rv;
	}
	
    printf("Boston Micromachines C API Example.\n");
	printf("Opened Device %d with %d actuators.\n", hdm.DevId, hdm.ActCount);
	


    // Allocate and initialize control arrays.
    map_lut			= (uint32_t *)malloc(sizeof(uint32_t)*MAX_DM_SIZE);
	bmc_array 		= (IMAGE *)calloc(hdm.ActCount, sizeof(double));


	///////////////
	// Create shared memory
	///////////////
	naxis = 1;
	// image will be in shared memory
	shared = 1;
	// allocate space for 10 keywords
	NBkw = 10;
	CBsize = 0;

	atype = _DATATYPE_FLOAT;
	imsize = (uint32_t *) malloc(sizeof(uint32_t)*naxis);
	imsize[0] = hdm.ActCount;
	ImageStreamIO_createIm(bmc_array, DMVOLT_FILENAME, naxis, imsize, atype, shared, NBkw, CBsize);
	ImageStreamIO_createIm(bmc_array, DMPTT_FILENAME, naxis, imsize, atype, shared, NBkw, CBsize);
	
    rv = BMCClose(&hdm);
	if (rv)
		printf("Error %d closing the driver.\n", rv);

	return 0;
}


