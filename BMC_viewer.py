import numpy as np 
from pyMilk.interfacing.isio_shmlib import SHM as shm
### plot modules
import matplotlib.pyplot as plt

plt.ion()


# Create shm
try:
    dm_map=shm('dmmap')
except:
    dm_map      = shm('dmmap',((512,512), np.float32), location = -1, shared=1)

# Get Piston Tip Tilt commands
dmptt       = shm("dmptt", verbose=False)
ptt         = dmptt.get_data()

# Get DM modes
modes = np.load('/home/alala/src/DM_BMC/BMC_modes.npy')
map_dm = np.zeros([512,512],dtype=np.float32)

while True:
    # Create map image
    ptt         = dmptt.get_data()
    for k in range(111):
        map_dm += np.dot(modes[k],ptt[k])

    dm_map.set_data(map_dm)
    map_dm *=0.



# plt.figure()
# plt.imshow(map_dm)