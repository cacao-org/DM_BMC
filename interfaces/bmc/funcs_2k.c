#include "funcs_2k.h"

errno_t make_open()
{
  tE err;

  if (NULL != sBMC)
  {
    return EXIT_FAILURE;
  }
  if (kBMCEnoErr != (err = BMCopen(sNdx, &sBMC)))
  {
    ERR(("make_open: %d gave %s\n", sNdx, BMCgetErrStr(err)));
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

errno_t init_hva()
{
    tE err;

    int mode = 7;

    if (kBMCEnoErr != (err = BMCsetUpHVA(sBMC, mode)))
    {
        ERR(("decode_args:BMCsetUpHVA %p/%d gave %s\n",
             sBMC, mode, BMCgetErrStr(err)));
    }
    else
    {
        printf("BMCsetUpHVA: to %d OK\n", mode);
    }

    if (kBMCEnoErr != (err = BMCwriteHVALUT(sBMC, size_2k, lut_2k, amap_2k, minlut_2k)))
    {
        ERR(("decode_args:BMCwriteHVALUT gave %s\n", BMCgetErrStr(err)));
        return EXIT_FAILURE;
    }
        printf("BMCwriteHVALUT: OK\n");
    return EXIT_SUCCESS;
}

errno_t set_spin_delay(int delay)
{
    tE err;

    if (kBMCEnoErr != (err = BMCsetSpinDelay(sBMC, delay)))
    {
        ERR(("decode_args:BMCsetSpinDelay %p/%d gave %s\n",
             sBMC, delay, BMCgetErrStr(err)));
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}