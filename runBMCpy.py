from pyMilk.interfacing.isio_shmlib import SHM as shm
import time
import numpy as np


dmptt      = shm("dmptt", verbose=False)
dm_command = np.zeros(111)

# dm_command[16*3+1] = 6.e-4
# dm_command[16*3+2] = -6.e-4
# dm_command[1*3+2] = 

# for k in range(37):
    # dm_command = np.zeros(111)
    # dm_command[3*k] = 0
    # dm_command[k*3+2] = -6.e-4
    
    # dmptt.set_data(dm_command.astype(np.float32))
    # time.sleep(0.5)

# dm_command = np.zeros(111)
dmptt.set_data(dm_command.astype(np.float32))

# -----------------------------------------------------------------------------
# Close shared memory
# -----------------------------------------------------------------------------

dmptt.close()
