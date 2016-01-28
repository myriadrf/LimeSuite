import SoapySDR
from SoapySDR import * #SOAPY_SDR_* constants

import numpy as np

import matplotlib.pyplot as plt

import time

if __name__ == '__main__':
    streamBoardSDR = SoapySDR.Device({"driver":"lime"})

    print("Set master clock rate")
    streamBoardSDR.setMasterClockRate(32e6)
    print streamBoardSDR.getMasterClockRate()/1e6, "MHz"

    print("Tune the LOs")
    streamBoardSDR.setFrequency(SOAPY_SDR_RX, 0, "RF", 1e9)
    streamBoardSDR.setFrequency(SOAPY_SDR_TX, 0, "RF", 1e9)

    print("Tune cordics")
    streamBoardSDR.setFrequency(SOAPY_SDR_RX, 0, "BB", 50e3)
    streamBoardSDR.setFrequency(SOAPY_SDR_RX, 1, "BB", 50e3)
    streamBoardSDR.setFrequency(SOAPY_SDR_TX, 0, "BB", 0.0)
    streamBoardSDR.setFrequency(SOAPY_SDR_TX, 1, "BB", 0.0)

    print("Pick sample rates")
    streamBoardSDR.setSampleRate(SOAPY_SDR_RX, 0, 32e6/8)
    streamBoardSDR.setSampleRate(SOAPY_SDR_RX, 1, 32e6/8)
    streamBoardSDR.setSampleRate(SOAPY_SDR_TX, 0, 32e6/8)
    streamBoardSDR.setSampleRate(SOAPY_SDR_TX, 1, 32e6/8)

    print("Set RX gains")
    streamBoardSDR.setGain(SOAPY_SDR_RX, 0, "LNA", 10.0)
    streamBoardSDR.setGain(SOAPY_SDR_RX, 0, "TIA", 12.0)
    streamBoardSDR.setGain(SOAPY_SDR_RX, 0, "PGA", -3.0)
    streamBoardSDR.setGain(SOAPY_SDR_RX, 1, "LNA", 10.0)
    streamBoardSDR.setGain(SOAPY_SDR_RX, 1, "TIA", 12.0)
    streamBoardSDR.setGain(SOAPY_SDR_RX, 1, "PGA", -3.0)

    streamBoardSDR.setDCOffsetMode(SOAPY_SDR_RX, 0, False)
    streamBoardSDR.setDCOffsetMode(SOAPY_SDR_RX, 1, False)

    print("set RX bandwidth")
    streamBoardSDR.setAntenna(SOAPY_SDR_RX, 0, "LNAL")
    streamBoardSDR.setAntenna(SOAPY_SDR_RX, 1, "LNAL")
    streamBoardSDR.setBandwidth(SOAPY_SDR_RX, 0, 10e6)
    streamBoardSDR.setBandwidth(SOAPY_SDR_RX, 1, 10e6)

    print("Enable Rx TSG")
    streamBoardSDR.writeSetting("ACTIVE_CHANNEL", "A");
    streamBoardSDR.writeSetting("ENABLE_RXTSP_CONST", "true");
    streamBoardSDR.writeSetting("ACTIVE_CHANNEL", "B");
    streamBoardSDR.writeSetting("ENABLE_RXTSP_CONST", "true");

    print("Create rx stream")
    rxStream = streamBoardSDR.setupStream(SOAPY_SDR_RX, SOAPY_SDR_CF32, [0, 1])
    sampsCh0 = np.array([0]*1024, np.complex64)
    sampsCh1 = np.array([0]*1024, np.complex64)

    print("Create tx stream")
    txStream = streamBoardSDR.setupStream(SOAPY_SDR_TX, SOAPY_SDR_CF32, [0, 1])
    streamBoardSDR.activateStream(txStream)

    #print(">>> test tx late burst")
    #timeStream = streamBoardSDR.getHardwareTime() + int(1e8) #100ms in the future
    #time.sleep(1.0) #time will be late
    #sr = streamBoardSDR.writeStream(txStream, [sampsCh0, sampsCh1], sampsCh0.size, SOAPY_SDR_END_BURST | SOAPY_SDR_HAS_TIME, timeStream, timeoutUs=int(1e6))
    #print streamBoardSDR.readStreamStatus(txStream, 

    print(">>> test burst aquisition")
    streamBoardSDR.activateStream(rxStream, SOAPY_SDR_END_BURST, 0, 100)
    sr = streamBoardSDR.readStream(rxStream, [sampsCh0, sampsCh1], sampsCh0.size, 0)
    print sr
    assert sr.ret == 100
    assert sr.flags & SOAPY_SDR_END_BURST
    sr = streamBoardSDR.readStream(rxStream, [sampsCh0, sampsCh1], sampsCh0.size, 0)
    assert sr.ret == SOAPY_SDR_TIMEOUT

    print(">>> test larger burst aquisition")
    totalSamps = 1024*100
    streamBoardSDR.activateStream(rxStream, SOAPY_SDR_END_BURST, 0, totalSamps)
    while totalSamps > 0:
        sr = streamBoardSDR.readStream(rxStream, [sampsCh0, sampsCh1], sampsCh0.size, 0)
        assert sr.ret > 0
        totalSamps -= sr.ret
        assert (totalSamps == 0) or not (sr.flags & SOAPY_SDR_END_BURST)
    assert totalSamps == 0
    sr = streamBoardSDR.readStream(rxStream, [sampsCh0, sampsCh1], sampsCh0.size, 0)
    assert sr.ret == SOAPY_SDR_TIMEOUT

    print(">>> test timed burst aquisition")
    totalSamps = 1024*100
    timeStream = streamBoardSDR.getHardwareTime() + int(1e8) #100ms in the future
    print("timeStream = %d ns"%timeStream)
    streamBoardSDR.activateStream(rxStream, SOAPY_SDR_END_BURST | SOAPY_SDR_HAS_TIME, timeStream, totalSamps)
    timeStreamActual = None
    while totalSamps > 0:
        sr = streamBoardSDR.readStream(rxStream, [sampsCh0, sampsCh1], sampsCh0.size, 0, timeoutUs=int(1e6))
        assert sr.ret > 0
        if timeStreamActual is None: timeStreamActual = sr.timeNs
        totalSamps -= sr.ret
    print("timeStreamActual = %d ns"%timeStreamActual)
    assert totalSamps == 0
    sr = streamBoardSDR.readStream(rxStream, [sampsCh0, sampsCh1], sampsCh0.size, 0)
    assert sr.ret == SOAPY_SDR_TIMEOUT

    print("Test receive")
    streamBoardSDR.activateStream(rxStream)

    t0 = streamBoardSDR.getHardwareTime()
    totalSamps = 0
    for i in range(1000):
        sr = streamBoardSDR.readStream(rxStream, [sampsCh0, sampsCh1], sampsCh0.size, 0)
        if i == 0: print 'sr0', sr
        if i == 1: print 'sr1', sr
        if sr.ret < 1:
            print sr
            break
        totalSamps += sr.ret

    print sr
    t1 = streamBoardSDR.getHardwareTime()
    print "totalSamps", totalSamps
    print "t0", t0
    print "t1", t1

    plt.plot(np.real(sampsCh0[:32]), label="ChA:I")
    plt.plot(np.imag(sampsCh0[:32]), label="ChA:Q")
    plt.plot(np.real(sampsCh1[:32]) + .1, label="ChB:I")
    plt.plot(np.imag(sampsCh1[:32]) + .1, label="ChB:Q")
    plt.legend()
    plt.ylabel('some numbers')
    plt.show()

    print("Cleanup rx stream")
    streamBoardSDR.deactivateStream(rxStream)
    streamBoardSDR.deactivateStream(txStream)
    streamBoardSDR.closeStream(rxStream)
    streamBoardSDR.closeStream(txStream)
