from pyMilk.interfacing.isio_shmlib import SHM as shm
import time
import numpy as np

dmptt = shm(
    "dmptt",
    verbose=False)  # initialize the shm containing piston tip and tilt values
dm_command = np.zeros(
    111, dtype=np.float64)  # command sent to MEMS must be a double

# NOTES : tip/tilt values in radians. [min;max] = [-1e-3 ; 1e-3]
#         piston values in nanometers

# dm_command[3*3+2] = -1.e-3
# dm_command[3*3+1] = -1.e-3
# dm_command[3*33+2] = -1.e-3
# dm_command[3*33+1] = -1.e-3

# dm_command[3*21] = -2000
# dm_command[3*21+2] = -2.e-3
# dm_command[3*21+1] = -2.e-3
# dmptt.set_data(dm_command.astype(np.float64))

for k in range(37):
    # dm_command  = np.zeros(111, dtype=np.float64)
    dm_command[k * 3] = -2000
    dm_command[k * 3 + 2] = 2.e-3
    dm_command[k * 3 + 1] = 2.e-3

# for i in np.arange(-3,3):
#     for j in np.arange(-3,3):
#         dm_command[29*3] = -2000
#         dm_command[29*3+2] = i*1e-3
#         dm_command[29*3+1] = j*1e-3
#         dmptt.set_data(dm_command.astype(np.float64))
#         time.sleep(0.5)
#         print(i,j)

dmptt.set_data(dm_command.astype(np.float64))

#     time.sleep(1.)

# time.sleep(5.)
# dm_command = np.zeros(111)
# dmptt.set_data(dm_command.astype(np.float64))
# -----------------------------------------------------------------------------
# Close shared memory
# -----------------------------------------------------------------------------

dmptt.close()
