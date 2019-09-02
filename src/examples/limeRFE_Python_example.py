import ctypes
from ctypes import *
import sys
import os
import time

import limeRFE_H as LH

if os.name == 'nt':
    libLimeSuite = ctypes.WinDLL ("c:\\path\\to\\LimeSuite.dll")
else:
    libLimeSuite = ctypes.cdll.LoadLibrary('./libLimeSuite.so')

baudrate = 9600

# Open port
if os.name == 'nt':
    fd = libLimeSuite.RFE_Open(c_char_p("COM17"), c_int(baudrate));
else:
    fd = libLimeSuite.RFE_Open(c_char_p("/dev/ttyUSB0"), c_int(baudrate));    

if (fd == -1):
    print(stderr, "Error initializing serial port\n")
    exit(1)

cinfo = (c_ubyte*4)()
print "Port opened; fd =", fd

#Configure LimeRFE to use channel HAM 2m channel in receive mode.
#Transmit output is routed to TX/RX output. Notch is off. Attenuation is 0.
libLimeSuite.RFE_Configure(c_int(0), c_int(fd), c_int(LH.CID_HAM_0145), c_int(LH.CID_HAM_0145), c_int(LH.PORT_1), c_int(LH.PORT_1), c_int(LH.MODE_RX), c_int(LH.NOTCH_VALUE_OFF), c_int(0))
if(result != 0):
    print "RFE_Configure Error Code:", result
    print "Check \'LimeRFE error codes' in LimeRFE.h"
    libLimeSuite.RFE_Close(c_int(fd))
    exit(1)

time.sleep(1.0)

# Change mode to transmit
libLimeSuite.RFE_Mode(c_int(0), c_int(fd), c_int(LH.MODE_TX))

time.sleep(1.0)

# Reset LimeRFE
libLimeSuite.RFE_Reset(c_int(0), c_int(fd))

# Close port
libLimeSuite.RFE_Close(c_int(fd))