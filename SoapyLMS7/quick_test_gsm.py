import SoapySDR
from SoapySDR import * #SOAPY_SDR_* constants

import numpy as np

import time

if __name__ == '__main__':
    streamBoardSDR = SoapySDR.Device({"driver":"lime"})

    print("Pick sample rates")
    streamBoardSDR.setSampleRate(SOAPY_SDR_RX, 0, 13e6/48)
    streamBoardSDR.setSampleRate(SOAPY_SDR_TX, 0, 13e6/12)

    print 'Results...'
    print 'Rx', streamBoardSDR.getSampleRate(SOAPY_SDR_RX, 0)/1e6, "MHz"
    print 'Tx', streamBoardSDR.getSampleRate(SOAPY_SDR_TX, 0)/1e6, "MHz"
    print 'CGEN', streamBoardSDR.getMasterClockRate()/1e6, "MHz"
