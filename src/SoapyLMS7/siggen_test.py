########################################################################
## Measure round trip delay through RF loopback/leakage
########################################################################

import SoapySDR
from SoapySDR import * #SOAPY_SDR_ constants
import numpy as np
from optparse import OptionParser
import time
import os
import math

def siggen_app(
    args,
    rate,
    ampl=0.7,
    freq=None,
    txChan=0,
    rxChan=0,
    txGain=None,
    txAnt=None,
    clockRate=None,
    waveFreq=None,
):
    sdr = SoapySDR.Device(args)

    #set clock rate first
    if clockRate is not None: sdr.setMasterClockRate(clockRate)

    #set sample rate
    sdr.setSampleRate(SOAPY_SDR_RX, rxChan, rate)
    sdr.setSampleRate(SOAPY_SDR_TX, txChan, rate)
    sdr.setBandwidth(SOAPY_SDR_TX, txChan, 10e6)
    print("Actual Rx Rate %f Msps"%(sdr.getSampleRate(SOAPY_SDR_RX, rxChan)/1e6))
    print("Actual Tx Rate %f Msps"%(sdr.getSampleRate(SOAPY_SDR_TX, txChan)/1e6))

    #set antenna
    print("Set the antenna")
    if txAnt is not None: sdr.setAntenna(SOAPY_SDR_TX, txChan, txAnt)

    #set overall gain
    print("Set the gain")
    if txGain is not None: sdr.setGain(SOAPY_SDR_TX, txChan, txGain)

    #tune frontends
    print("Tune the frontend")
    if freq is not None: sdr.setFrequency(SOAPY_SDR_TX, txChan, freq)

    #create tx stream
    print("Create Tx stream")
    txStream = sdr.setupStream(SOAPY_SDR_TX, "CF32", [txChan])
    sdr.activateStream(txStream)

    #tx loop
    if waveFreq is None: waveFreq = rate/10
    phaseAcc = 0
    phaseInc = 2*math.pi*waveFreq/rate
    streamMTU = sdr.getStreamMTU(txStream)
    sampsCh0 = np.array([0]*streamMTU, np.complex64)
    while True:
        for i in range(streamMTU):
            sampsCh0[i] = ampl*math.e**(1j*phaseAcc)
            phaseAcc += phaseInc
        while phaseAcc > 2*math.pi:
            phaseAcc -= 2*math.pi
        while phaseAcc < -2*math.pi:
            phaseAcc += 2*math.pi

        sr = sdr.writeStream(txStream, [sampsCh0], sampsCh0.size)
        if sr.ret != sampsCh0.size:
            raise Exception("Expected writeStream() to consume all samples! %d"%sr.ret)

    #cleanup streams
    print("Cleanup stream")
    sdr.deactivateStream(txStream)
    sdr.closeStream(txStream)
    print("Done!")

def main():
    parser = OptionParser()
    parser.add_option("--args", type="string", dest="args", help="device factor arguments", default="")
    parser.add_option("--rate", type="float", dest="rate", help="Tx and Rx sample rate", default=1e6)
    parser.add_option("--ampl", type="float", dest="ampl", help="Tx digital amplitude rate", default=0.7)
    parser.add_option("--txAnt", type="string", dest="txAnt", help="Optional Tx antenna", default=None)
    parser.add_option("--txGain", type="float", dest="txGain", help="Optional Tx gain (dB)", default=None)
    parser.add_option("--txChan", type="int", dest="txChan", help="Transmitter channel (def=0)", default=0)
    parser.add_option("--freq", type="float", dest="freq", help="Optional Tx and Rx freq (Hz)", default=None)
    parser.add_option("--waveFreq", type="float", dest="waveFreq", help="Baseband waveform freq (Hz)", default=None)
    parser.add_option("--clockRate", type="float", dest="clockRate", help="Optional clock rate (Hz)", default=None)
    (options, args) = parser.parse_args()
    siggen_app(
        args=options.args,
        rate=options.rate,
        ampl=options.ampl,
        freq=options.freq,
        txAnt=options.txAnt,
        txGain=options.txGain,
        txChan=options.txChan,
        clockRate=options.clockRate,
        waveFreq=options.waveFreq,
    )

if __name__ == '__main__': main()
