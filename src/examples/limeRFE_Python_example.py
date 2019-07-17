import ctypes
from ctypes import *
import sys
import os
import time

import limeRFE_H as LH

if os.name == 'nt':
    libLimeSuite = ctypes.WinDLL ("d:\\Lime\\Projects\\Misc\\LimeRFE\\Software\\LimeSuite\\LimeSuite-master\\build\\bin\\Release\\LimeSuite.dll")
else:
    libLimeSuite = ctypes.cdll.LoadLibrary('./libLimeSuite.so')

baudrate = 9600

# Open port
if os.name == 'nt':
    fd = libLimeSuite.LIMERFE_Open(c_char_p("COM24"), c_int(baudrate));
else:
    fd = libLimeSuite.LIMERFE_Open(c_char_p("/dev/ttyUSB0"), c_int(baudrate));    

if (fd == -1):
    print(stderr, "Error initializing serial port\n")
    exit(1)

cinfo = (c_ubyte*4)()
print "Port opened; fd =", fd

# Configure LimeRFE to use channel HAM 2m channel in receive mode.
# Transmit output is routed to TX output. Notch is off. Attenuation is 0.
libLimeSuite.LIMERFE_Configure(c_int(LH.LIMERFE_USB), c_int(0), c_int(0), c_int(fd), c_int(LH.CID_HAM_0030), c_int(LH.TX2TXRX_INDEX_TX), c_int(LH.NOTCH_VALUE_OFF), c_int(0))

time.sleep(1.0)

# Change mode to transmit
libLimeSuite.LIMERFE_Mode(c_int(LH.LIMERFE_USB), c_int(0), c_int(0), c_int(fd), c_int(LH.RFE_MODE_TX))

time.sleep(1.0)

# Reset LimeRFE
libLimeSuite.LIMERFE_Reset(c_int(fd))

# Close port
libLimeSuite.LIMERFE_Close(c_int(fd))