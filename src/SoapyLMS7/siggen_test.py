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
    rxRate=None,
    ampl=0.7,
    freq=None,
    txChan=0,
    rxChan=0,
    txGain=None,
    txAnt=None,
    clockRate=None,
    waveFreq=None,
    txTSP=False,
):
    if waveFreq is None: waveFreq = rate/10

    if rxRate is None: rxRate = rate

    sdr = SoapySDR.Device(args)
    #set clock rate first
    if clockRate is not None: sdr.setMasterClockRate(clockRate)
    #set sample rate
    sdr.setSampleRate(SOAPY_SDR_RX, rxChan, rxRate)
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

    if txTSP:
        sdr.setFrequency(SOAPY_SDR_TX, txChan, "BB", waveFreq)
        sdr.writeSetting("ACTIVE_CHANNEL", {0:"A",1:"B"}[txChan])
        sdr.writeSetting("ENABLE_TXTSP_CONST", "true");

    #tx loop
    #create tx stream
    print("Create Tx stream")
    txStream = sdr.setupStream(SOAPY_SDR_TX, "CF32", [txChan])
    print("Activate Tx Stream")
    sdr.activateStream(txStream)
    phaseAcc = 0
    phaseInc = 2*math.pi*waveFreq/rate
    streamMTU = sdr.getStreamMTU(txStream)
    sampsCh0 = np.array([ampl]*streamMTU, np.complex64)
    
    timeLastPrint = time.time()
    totalSamps = 0
    while True:
        phaseAccNext = phaseAcc + streamMTU*phaseInc
        sampsCh0 = ampl*np.exp(1j*np.linspace(phaseAcc, phaseAccNext, streamMTU)).astype(np.complex64)
        phaseAcc = phaseAccNext
        while phaseAcc > math.pi*2: phaseAcc -= math.pi*2

        sr = sdr.writeStream(txStream, [sampsCh0], sampsCh0.size)
        if sr.ret != sampsCh0.size:
            raise Exception("Expected writeStream() to consume all samples! %d"%sr.ret)
        totalSamps += sr.ret

        if time.time() > timeLastPrint + 5.0:
            print("Python siggen rate: %f Msps"%(totalSamps/(time.time()-timeLastPrint)/1e6))
            totalSamps = 0
            timeLastPrint = time.time()

    #cleanup streams
    print("Cleanup stream")
    sdr.deactivateStream(txStream)
    sdr.closeStream(txStream)
    print("Done!")

def main():
    parser = OptionParser()
    parser.add_option("--args", type="string", dest="args", help="device factor arguments", default="")
    parser.add_option("--rate", type="float", dest="rate", help="Tx sample rate", default=1e6)
    parser.add_option("--rxRate", type="float", dest="rxRate", help="Rx sample rate for testing", default=None)
    parser.add_option("--ampl", type="float", dest="ampl", help="Tx digital amplitude rate", default=0.7)
    parser.add_option("--txAnt", type="string", dest="txAnt", help="Optional Tx antenna", default=None)
    parser.add_option("--txGain", type="float", dest="txGain", help="Optional Tx gain (dB)", default=None)
    parser.add_option("--txChan", type="int", dest="txChan", help="Transmitter channel (def=0)", default=0)
    parser.add_option("--freq", type="float", dest="freq", help="Optional Tx and Rx freq (Hz)", default=None)
    parser.add_option("--waveFreq", type="float", dest="waveFreq", help="Baseband waveform freq (Hz)", default=None)
    parser.add_option("--clockRate", type="float", dest="clockRate", help="Optional clock rate (Hz)", default=None)
    parser.add_option("--txTSP", action="store_true", dest="txTSP", help="Use internal TX siggen w/ CORDIC", default=False)
    (options, args) = parser.parse_args()
    siggen_app(
        args=options.args,
        rate=options.rate,
        rxRate=options.rxRate,
        ampl=options.ampl,
        freq=options.freq,
        txAnt=options.txAnt,
        txGain=options.txGain,
        txChan=options.txChan,
        clockRate=options.clockRate,
        waveFreq=options.waveFreq,
        txTSP=options.txTSP,
    )

if __name__ == '__main__': main()
