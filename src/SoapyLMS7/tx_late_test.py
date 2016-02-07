import SoapySDR
from SoapySDR import * #SOAPY_SDR_* constants

import numpy as np

import time

if __name__ == '__main__':
    streamBoardSDR = SoapySDR.Device({"driver":"lime"})

    print("Set master clock rate")
    streamBoardSDR.setMasterClockRate(32e6)
    print streamBoardSDR.getMasterClockRate()/1e6, "MHz"

    print("Pick sample rates")
    rate = 32e6/8
    streamBoardSDR.setSampleRate(SOAPY_SDR_RX, 0, rate)
    streamBoardSDR.setSampleRate(SOAPY_SDR_RX, 1, rate)
    streamBoardSDR.setSampleRate(SOAPY_SDR_TX, 0, rate)
    streamBoardSDR.setSampleRate(SOAPY_SDR_TX, 1, rate)

    print("Create tx stream")
    txStream = streamBoardSDR.setupStream(SOAPY_SDR_TX, SOAPY_SDR_CF32, [0, 1])
    streamBoardSDR.activateStream(txStream)
    streamMTU = streamBoardSDR.getStreamMTU(txStream)
    sampsCh0 = np.array([0]*streamMTU, np.complex64)
    sampsCh1 = np.array([0]*streamMTU, np.complex64)

    #flush the async report queue
    while streamBoardSDR.readStreamStatus(txStream).ret != SOAPY_SDR_TIMEOUT: pass

    t0 = streamBoardSDR.getHardwareTime()
    time.sleep(1.0)
    t1 = streamBoardSDR.getHardwareTime()
    print("t0 = %s"%t0)
    print("t1 = %s"%t1)
    print("delta %s secs"%((t1-t0)/1e9))

    print(">>> test tx late burst")
    timeStream = streamBoardSDR.getHardwareTime() + int(1e8) #100ms in the future
    print("timeStream = %s"%timeStream)
    time.sleep(1.0) #time will be late
    for  i in range(16):
        sr = streamBoardSDR.writeStream(txStream, [sampsCh0, sampsCh1], sampsCh0.size, SOAPY_SDR_HAS_TIME, timeStream, timeoutUs=int(1e6))
        timeStream += long(1e9/rate)*sampsCh0.size
    print streamBoardSDR.readStreamStatus(txStream, timeoutUs=int(1e6))

    print(">>> test tx late burst")
    timeStream = streamBoardSDR.getHardwareTime() + int(1e8) #100ms in the future
    print("timeStream = %s"%timeStream)
    time.sleep(1.0) #time will be late
    for  i in range(16):
        sr = streamBoardSDR.writeStream(txStream, [sampsCh0, sampsCh1], sampsCh0.size, SOAPY_SDR_HAS_TIME, timeStream, timeoutUs=int(1e6))
        timeStream += long(1e9/rate)*sampsCh0.size
    print streamBoardSDR.readStreamStatus(txStream, timeoutUs=int(1e6))

    print("Cleanup tx stream")
    streamBoardSDR.deactivateStream(txStream)
    streamBoardSDR.closeStream(txStream)
