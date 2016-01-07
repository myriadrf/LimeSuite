import SoapySDR
from SoapySDR import * #SOAPY_SDR_* constants

import numpy as np

import matplotlib.pyplot as plt

if __name__ == '__main__':
    streamBoardSDR = SoapySDR.Device({"driver":"lime"})

    print("Set master clock rate")
    streamBoardSDR.setMasterClockRate(32e6)
    print streamBoardSDR.getMasterClockRate()/1e6, "MHz"

    print("Tune the LOs")
    streamBoardSDR.setFrequency(SOAPY_SDR_RX, 0, "RF", 1e9)
    streamBoardSDR.setFrequency(SOAPY_SDR_TX, 0, "RF", 1e9)

    print("Tune cordics")
    streamBoardSDR.setFrequency(SOAPY_SDR_RX, 0, "BB", 0.0)
    streamBoardSDR.setFrequency(SOAPY_SDR_RX, 1, "BB", 0.0)
    streamBoardSDR.setFrequency(SOAPY_SDR_TX, 0, "BB", 0.0)
    streamBoardSDR.setFrequency(SOAPY_SDR_TX, 1, "BB", 0.0)

    print("Pick sample rates")
    streamBoardSDR.setSampleRate(SOAPY_SDR_RX, 0, 32e6/8)
    streamBoardSDR.setSampleRate(SOAPY_SDR_RX, 1, 32e6/8)
    streamBoardSDR.setSampleRate(SOAPY_SDR_TX, 0, 32e6/8)
    streamBoardSDR.setSampleRate(SOAPY_SDR_TX, 1, 32e6/8)

    print("Create rx stream")
    rxStream = streamBoardSDR.setupStream(SOAPY_SDR_RX, SOAPY_SDR_CF32, [0, 1])
    streamBoardSDR.activateStream(rxStream)

    print("Test receive")
    sampsCh0 = np.array([0]*1024, np.complex64)
    sampsCh1 = np.array([0]*1024, np.complex64)
    sr = streamBoardSDR.readStream(rxStream, [sampsCh0, sampsCh1], 1024, 0)
    print sr

    plt.plot(np.real(sampsCh0))
    plt.ylabel('some numbers')
    plt.show()

    print("Cleanup rx stream")
    streamBoardSDR.deactivateStream(rxStream)
    streamBoardSDR.closeStream(rxStream)
