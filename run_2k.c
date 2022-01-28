/*****************************************************************************

  bmc_ltest.c			Boston Micromachines Corp. library test program
            Modified to integrate with cacao

  Nov 1, 2014			CIW/SJT original entry, � Copyright 2014, Boston Micromachines Corporation. All rights reserved.
  2015-2018			  Modified for cacao support
  2022            Shrink down to minimal SCExAO functionality


  To build:
  ./compile-exec.sh (sudo prompt will show up, that's for suid permission)

  Required files:
  bmc_ltest.c (this file)
  bmc_mdlib.h
  ../../lib/libbmcmd.a
  ImageStreamIO shared library

  To run loop, execute :
  ./bmc_ltest -K      # initialize board for 2k DM
  ./bmc_ltest -D 0    # set driver spin delay count to 0
  ./bmc_ltest -L      # real time control loop

  Follow the compile-exec.sh for include path

*****************************************************************************/

/*==========================================================================*/
//#include <stdio.h>
/*
#include <stdarg.h>
#include <math.h>
#include <semaphore.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <ctype.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <sys/file.h>
#include <errno.h>
#include <sys/mman.h>
*/

#define _GNU_SOURCE
#include <sched.h>
#include <unistd.h>

#include "funcs_2k.h"

#define DMVOLT_FILENAME "dmvolt"

static int status_exit = 0; // program exit code
static int end = 0; // termination flag for funcs

struct timespec tnow;
double tnowdouble;
double tlastupdatedouble;

double dt_update;              // time since last update
double dt_update_lim = 3600.0; // if no command is received during this time, set DM to zero V [sec]

IMAGE *SMdmvolt;


// Termination function for SIGINT callback
static void endme(int dummy)
{
  end = 1;
}


static char *sArgv0 = NULL; // name of executable

static void show_help(void)
{
  SHOW(("%s of %s at %s\n", sArgv0, __DATE__, __TIME__));
  SHOW(("   arguments:\n"));
  SHOW(("   -D count         set driver spin delay count\n"));
  SHOW(("   -K               initialize board for 2k DM\n"));
  SHOW(("   -L               real time control loop\n"));
  SHOW(("   -A               Run -K, -D 0, -L\n"));
}

static int dm2k_realtimeloop()
{
  long ActIndex[50][50];
  IMAGE_METADATA *map;
  char *mapv; // pointed cast in bytes
  long DMStartRow[50] = {18, 15, 13, 11, 10, 8, 7, 6, 5, 5, 4, 3, 3, 2, 2, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 2, 2, 3, 3, 4, 5, 5, 6, 7, 8, 10, 11, 13, 15, 18};
  long DMEndRow[50] = {31, 34, 36, 38, 39, 41, 42, 43, 44, 44, 45, 46, 46, 47, 47, 48, 48, 48, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 48, 48, 48, 47, 47, 46, 46, 45, 44, 44, 43, 42, 41, 39, 38, 36, 34, 31};
  struct stat file_stat;
  long cntloop;
  int sx, sy, s1;
  int SMdmvoltfd;
  long framecnt;
  long cnt0;
  int Index = 0;
  short s;

  long ii;
  tE err;

  unsigned short int temp[2500];

  int RT_priority = 90; // any number from 0-99
  struct sched_param schedpar;
  int r;

  // register interrupt signal to terminate the main loop
  signal(SIGINT, endme);

  printf("Starting loop\n");
  fflush(stdout);

  // build up mapping
  for (sx = 0; sx < 50; sx++)
    for (sy = 0; sy < 50; sy++)
      ActIndex[sx][sy] = -1;
  s = 1;
  for (sy = 0; sy < 50; sy++)
  {
    // printf("RANGE %3ld:  %ld - %ld\n", sy, DMStartRow[sy], DMEndRow[sy]);
    for (sx = 0; sx < 50; sx++)
    {
      if ((sx >= DMStartRow[sy]) && (sx <= DMEndRow[sy]))
      {
        ActIndex[sx][sy] = s;
        printf("[%4ld %4ld -> %4ld]", (long)sx, (long)sy, (long)ActIndex[sx][sy]);
        s++;
        if (s % 8 == 7) {
          printf("\n");
        }
      }
    }
  }
  printf("\n");

  SMdmvolt = (IMAGE *)malloc(sizeof(IMAGE));

  // CONNECT TO VOLT MAP SHARED MEMORY
  ImageStreamIO_read_sharedmem_image_toIMAGE(DMVOLT_FILENAME, &SMdmvolt[0]);

  framecnt = -1;

  // MAIN LOOP
  printf("ENTERING LOOP\n");
  fflush(stdout);
  cntloop = 0;
  while ((end == 0) && (SMdmvolt[0].md[0].status < 100)) //&&(cntloop<10000))
  {
    cnt0 = SMdmvolt[0].md[0].cnt0;
    if ((cnt0 > framecnt) && (SMdmvolt[0].md[0].write == 0)) // new frame is here... apply on DM
    {
      // LOG DM FILE
      s = 0;
      s1 = 0;
      memcpy(temp, SMdmvolt[0].array.UI16, 2500 * sizeof(unsigned short int));
      for (sx = 0; sx < 50; sx++)
        for (sy = 0; sy < 50; sy++)
        {
          Index = ActIndex[sx][sy];
          if (Index >= 0)
            sActValue[Index] = (short)temp[s1];
          s1++;
        }

      if (kBMCEnoErr != (err = BMCburstHVA(sBMC, 1024, sActValue)))
      {
        ERR(("decode_args:BMCburstHVA %p gave %s\n",
             sBMC, BMCgetErrStr(err)));
      }

      framecnt = cnt0;
      SMdmvolt[0].md[0].cnt1++;
      clock_gettime(CLOCK_REALTIME, &tnow);
      // tnowdouble = 1.0*tnow.tv_sec + 1.0e-9*tnow.tv_nsec;
      //       dt_update = tnowdouble - tlastupdatedouble;

      tlastupdatedouble = 1.0 * tnow.tv_sec + 1.0e-9 * tnow.tv_nsec;
      cntloop++;
    }
    else // do not apply anything... wait 10us
    {
      usleep(10); // 10 us
      clock_gettime(CLOCK_REALTIME, &tnow);
      tnowdouble = 1.0 * tnow.tv_sec + 1.0e-9 * tnow.tv_nsec;
      dt_update = tnowdouble - tlastupdatedouble;
      if (dt_update > dt_update_lim)
      {
        // set DM to zero
        printf("DM going to sleep ... \n");
        SMdmvolt[0].md[0].write = 1;
        for (ii = 0; ii < 2500; ii++)
          SMdmvolt[0].array.UI16[ii] = 0;
        SMdmvolt[0].md[0].write = 0;
        SMdmvolt[0].md[0].cnt0++;
      }
    }
  }

  printf("EXITING MAIN LOOP\n");
  printf("[%10ld] SENT TO DM : %ld / %ld  (%6.2f%%)\n", cntloop, SMdmvolt[0].md[0].cnt1, SMdmvolt[0].md[0].cnt0, 100.0 * SMdmvolt[0].md[0].cnt1 / SMdmvolt[0].md[0].cnt0);
  fflush(stdout);

  return 0;
}

static void decode_args(int argc, char **argv)
/*
**	Parse the input arguments.
*/
{
  char *str;
  int a1, a2, a3, a4, a5, i;
  tU32 x;
  tU32 data32[kBIGDATA];
  tU16 data16[kBIGDATA];
  tD ad, add;

  argv += 1;
  argc -= 1; /* skip program name */

  if (argc == 0) {
    show_help();
    return;
  }

  while (argc-- > 0)
  {
    str = *argv++;
    if (str[0] != '-')
    {
      ERR(("Do not know arg '%s'\n", str));
      show_help();
      exit(1);
    }

    switch (str[1])
    {
    case 'D':
      (void)sscanf(*argv++, "%d", &a1);
      argc -= 1;
      make_open();
      set_spin_delay(a1);
      break;

    /* ADDED BY OLIVIER GUYON 2015-07-14: 2k DM support */
    case 'K':
      make_open();
      init_hva();
      break;

    case 'L':
      make_open();
      printf("2k DM real time control\n");
      dm2k_realtimeloop();
      break;

    case 'A':
      SHOW(("Running -K, -D 0, -L: init+start main loop\n"));
      make_open();
      init_hva();
      set_spin_delay(0);
      dm2k_realtimeloop();
      break;
    
    default:
      show_help();
      break;
    }
  }
  return;
}

/*
** Manage UID to promote to cset and RT priority
*/
static void set_rt_priority()
{

    uid_t ruid; // Real UID (= user launching process at startup)
    uid_t euid; // Effective UID (= owner of executable at startup)
    uid_t suid; // Saved UID (= owner of executable at startup)

    int RT_priority = 93; //any number from 0-99
    struct sched_param schedpar;
    int ret;

    getresuid(&ruid, &euid, &suid);

    //This sets it to the privileges of the normal user
    ret = seteuid(ruid);
    if (ret != 0)
    {
        SHOW(("setuid error\n"));
    }

    schedpar.sched_priority = RT_priority;

    if (ret != 0)
    {
        SHOW(("setuid error\n"));
    }
    ret = seteuid(euid); //This goes up to maximum privileges
    sched_setscheduler(0, SCHED_FIFO,
                       &schedpar); //other option is SCHED_RR, might be faster
    ret = seteuid(ruid);           //Go back to normal privileges
    if (ret != 0)
    {
        SHOW(("setuid error\n"));
    }
}



/*
**	Main
*/
int main(int argc, char **argv)
{
  SHOW(("%d", size_2k));

  set_rt_priority();

  sArgv0 = *argv;

  sBMC = NULL;

  decode_args(argc, argv);

  if (NULL != sBMC)
  {
    (void)BMCclose(sBMC);
  }

  return (status_exit);
}
/*==========================================================================*/
