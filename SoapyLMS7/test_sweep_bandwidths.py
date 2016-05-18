########################################################################
## Test sweeping bandwidth settings
########################################################################

import SoapySDR
from SoapySDR import * #SOAPY_SDR_ constants
import numpy as np
from optparse import OptionParser
import time
import os
import math

def sweep_bw(
    args,
    chan,
    dir,
    start,
    stop,
    step,
):
    sdr = SoapySDR.Device(args)

    fails = list()
    bandwidth = start
    while True:
        print('#'*50)
        print('## Set channel %d %s bandwidth to %g MHz'%(chan, dir, bandwidth/1e6))
        print('#'*50)
        try: sdr.setBandwidth(SOAPY_SDR_RX if dir == "RX" else SOAPY_SDR_TX, chan, bandwidth)
        except Exception as ex:
            print(ex)
            fails.append((bandwidth, str(ex)))
        bandwidth += step
        if bandwidth > stop: break

    if fails:
        print('#'*50)
        print('## Failure summary')
        print('#'*50)
        for bw, err in fails: print(" * %g MHz - %s"%(bw/1e6, err))

    print("Done!")

def main():
    parser = OptionParser()
    parser.add_option("--args", type="string", dest="args", help="device factor arguments", default="")
    parser.add_option("--chan", type="int", dest="chan", help="which channel to sweep", default=0)
    parser.add_option("--dir", type="string", dest="dir", help="which direction RX or TX", default="RX")
    parser.add_option("--start", type="float", dest="start", help="bandwidth to start sweeping", default=1e6)
    parser.add_option("--stop", type="float", dest="stop", help="bandwidth to stop sweeping", default=60e6)
    parser.add_option("--step", type="float", dest="step", help="bandwidth sweep step", default=1e6)
    (options, args) = parser.parse_args()
    sweep_bw(
        args=options.args,
        chan=options.chan,
        dir=options.dir,
        start=options.start,
        stop=options.stop,
        step=options.step,
    )

if __name__ == '__main__': main()
