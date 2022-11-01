#include <BMCApi.h>

#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <signal.h>

#include <time.h>

#include <ImageStruct.h>   // cacao data structure definition
#include <ImageStreamIO.h> // function ImageStreamIO_read_sharedmem_image_toIMAGE()

#include <unistd.h>

#define DMVOLT_FILENAME "dmvolt"
#define DMPTT_FILENAME "dmptt"

typedef int bool_t;
#define TRUE 1
#define FALSE 0

struct timespec tnow;
double tnowdouble;
double tlastupdatedouble;

double dt_update;              // time since last update
double dt_update_lim = 360000000000000.0; // if no command is received during this time, set DM to zero V [sec]
/*==========================================================================*/

static int loop_abort = 0; // termination flag
// termination function for SIGINT callback
static void endme(int dummy)
{
    loop_abort = 1;
}

void Sleep_us(double us)
{
    usleep((unsigned int)us);
}

void ShowHelp(void)
{
    printf("Boston Micromachines DM tester.\n");
    printf("    DM-SDK Version %s\n", BMCVersionString());
    printf("Test 1: Piston mirror between two values.\n");
    printf("Test 2: Poke each actuator to half amplitude.\n");
    printf("Options are:\n");
    printf("    -s xxxxxxx  Device serial number. If this\n");
    printf("                option is not used, you will be\n");
    printf("                prompted to enter the SN.\n");
    printf("    -c x        Run test continuously with x\n");
    printf("                millisecond delay between commands.\n");
    printf("    -a          Exit when done, don't wait for input.\n\n");
    printf("    -h          Display this message and exit.\n\n");
}

/*!
 * \brief poke_pcie Poke actuators one by one using BMCSetSingle()
 * \param hdm DM handle
 * \param continuous If true, don't wait for user input.
 * \param delay Delay between actuators in continuous mode.
 * \return Error code
 */

int main(int argc, char *argv[])
{
    DM hdm;
    BMCRC rv;
    double *test_array1;
    double *test_array2;
    double *bmc_array;
    double *bmc_voltarray;
    IMAGE *SMdmvolt;
    IMAGE *SMdmptt;

    /////// INIT SHARED MEM STUFF
    long naxis;       // number of axis
    uint8_t atype;    // data type
    uint32_t *imsize; // image size
    int shared;       // 1 if image in shared memory
    int NBkw;         // number of keywords supported
    int CBsize;

    uint32_t *map_lut;
    char serial_number[BMC_SERIAL_NUMBER_LEN + 1];
    char app_name[BMC_MAX_PATH] = "";
    int k, i = 0;
    long ii;
    int select = 0;
    int err_count = 0;
    int pistons = 10;
    int delay_us = 500;
    char ch = '\0';
    bool_t continuous = FALSE;
    bool_t auto_exit = FALSE;
    bool_t ask_SN = TRUE;
    long framecnt;
    long cnt0;
    long cntloop;
    int activate_MEMS;

    // Parse options
    snprintf(app_name, sizeof(app_name), "%s", argv[0]);

    // register interrupt signal to terminate the main loop
    signal(SIGINT, endme);

    argc--;

    ShowHelp();

    // Add Static Serial Number
    strcpy(serial_number, "32AW005#010");
    // strcpy(serial_number, "32AW038#027");

    // Open driver
    memset(&hdm, 0, sizeof(hdm));
    rv = BMCOpen(&hdm, serial_number);
    unsigned int numSegments = hdm.ActCount / 3;
    if (rv)
    {
        printf("Error %d opening the driver type %u.\n", rv, (unsigned int)hdm.Driver_Type);
        printf("%s\n\n", BMCErrorString(rv));
    }
    rv = BMCLoadCalibrationFile(&hdm, "/opt/Boston Micromachines/Calibration/LUT_32AW005#010.mat");
    printf("Boston Micromachines C API Example.\n");
    printf("Opened Device %d with %d actuators.\n", hdm.DevId, hdm.ActCount);

    // Allocate and initialize control arrays.
    map_lut = (uint32_t *)malloc(sizeof(uint32_t) * MAX_DM_SIZE);
    bmc_array = (double *)calloc(hdm.ActCount, sizeof(double));
    bmc_voltarray = (double *)calloc(hdm.ActCount, sizeof(double));


    for (k = 0; k < (int)hdm.ActCount; k++)
    {
        map_lut[k] = 0;
    }

    // Open default actuator map from disk
    rv = BMCLoadMap(&hdm, NULL, map_lut);

    double temp[hdm.ActCount];

    SMdmvolt = (IMAGE *)malloc(sizeof(IMAGE));
    SMdmptt = (IMAGE *)malloc(sizeof(IMAGE));

    ImageStreamIO_read_sharedmem_image_toIMAGE(DMVOLT_FILENAME, SMdmvolt);
    ImageStreamIO_read_sharedmem_image_toIMAGE(DMPTT_FILENAME, SMdmptt);

    memset(SMdmvolt->array.D, 0, hdm.ActCount * sizeof(double));
    memset(SMdmptt->array.D, 0, hdm.ActCount * sizeof(double));

    int semID_dmvolt = ImageStreamIO_getsemwaitindex(SMdmvolt, -1);
    int semID_dmptt = ImageStreamIO_getsemwaitindex(SMdmptt, -1);

    // MAIN LOOP
    printf("ENTERING LOOP\n");

    while (!loop_abort && (SMdmvolt[0].md[0].status < 100) && (SMdmptt[0].md[0].status < 100))
    {
        if (!sem_trywait(SMdmvolt->semptr[semID_dmvolt]))
        {
            // Post an actuator command !
            rv = BMCSetArray(&hdm, SMdmvolt->array.D, map_lut);
            clock_gettime(CLOCK_REALTIME, &tnow);
            tlastupdatedouble = 1.0 * tnow.tv_sec + 1.0e-9 * tnow.tv_nsec;
        }
        else if (!sem_trywait(SMdmptt->semptr[semID_dmvolt]))
        {
            // Post a PTT command !
            for (k = 0; k < numSegments; k++)
            {
                rv = BMCSetSegment(&hdm, k, 
                                   SMdmptt->array.D[3 * k],
                                   SMdmptt->array.D[3 * k + 1],
                                   SMdmptt->array.D[3 * k + 2],
                                   TRUE, k == numSegments - 1);
            }
            clock_gettime(CLOCK_REALTIME, &tnow);
            tlastupdatedouble = 1.0 * tnow.tv_sec + 1.0e-9 * tnow.tv_nsec;
        }
        else
        {
            // Do not post a command
            usleep(10); // 10 us
            clock_gettime(CLOCK_REALTIME, &tnow);
            tnowdouble = 1.0 * tnow.tv_sec + 1.0e-9 * tnow.tv_nsec;
            dt_update = tnowdouble - tlastupdatedouble;
            if (dt_update > dt_update_lim)
            {
                // set DM to zero
                printf("DM going to sleep ... \n");
                // Send zero volts??
                BMCClearArray(&hdm);
                tlastupdatedouble = 1.0 * tnow.tv_sec + 1.0e-9 * tnow.tv_nsec;
            }
        }
    }

    printf("EXITING MAIN LOOP\n");

    // Clear array
    rv = BMCClearArray(&hdm);
    if (rv)
        printf("Error %d clearing voltages.\n", rv);

    rv = BMCClose(&hdm);

    return 0;
}
