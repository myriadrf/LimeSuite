#!/usr/bin/env python
########################################################################
## Loopback sweep for corrections calibration data
########################################################################

import os
from optparse import OptionParser
import SoapySDR
from SoapySDR import * #*_SOAPY_SDR constants
import numpy as np
import scipy.signal
import cmath
import math
import time
import copy

##########################################
## Calibration constants
##########################################
CLOCK_RATE = 80e6
SAMPLE_RATE = 10e6
RX_ANTENNA = "LB2"
TX_ANTENNA = "BAND2"
LB_LNA_GAIN = 40.0
PGA_GAIN = 0.0
TIA_GAIN = 0.0
LNA_GAIN = 0.0
PAD_GAIN = -10.0
LB_PAD_GAIN = 0.0

TX_FREQ_DELTA = 0.7e6
TX_CORDIC_FREQ = [1.1e6, 1.5e6]

MAX_SEARCH_DEPTHS = range(2, 11)
NUM_STEPS_PER_ITER = 3

SAMPS_PER_CAPTURE = 1024*8
NUM_BINS_PER_FFT = 1024

##########################################
## Sample utilities
##########################################
def readSamps(limeSDR, rxStream, numSamps=SAMPS_PER_CAPTURE, sleep=False):
    """
    Read stream data from each channel
    @return a list of complex64 arrays
    """
    if sleep: time.sleep(0.05)
    rxSamples = [np.zeros(numSamps, np.complex64), np.zeros(numSamps, np.complex64)]
    limeSDR.activateStream(rxStream, SOAPY_SDR_END_BURST, 0, numSamps)
    b0 = rxSamples[0]
    b1 = rxSamples[1]
    while b0.size:
        sr = limeSDR.readStream(rxStream, [b0, b1], b0.size)
        if sr.ret < 0: raise Exception(str(sr))
        b0 = b0[sr.ret:]
        b1 = b1[sr.ret:]

    rxSamples[0] -= np.mean(rxSamples[0]) #remove dc from rx samples
    rxSamples[1] -= np.mean(rxSamples[1]) #remove dc from rx samples
    return rxSamples

def measureToneLevel(samps, freq, rate=SAMPLE_RATE):
    step = -2.0*math.pi*(freq)/rate
    shifted = samps * np.exp(1j*np.linspace(0, samps.size*step, samps.size))
    return 20*math.log10(max(1e-20, abs(np.mean(shifted))))

##########################################
## Plotting utilities
##########################################
def sampsToPowerFFT(rxSamples, fftSize=NUM_BINS_PER_FFT):
    """
    Get a power FFT for a complex samples array
    Averages multiple FFTs when rxSamples.size > fftSize.
    Window power and fft gain is not compensated for.
    @param rxSamples an array of samples (not list of arrays)
    """

    numFFTs = rxSamples.size/fftSize

    if numFFTs == 1:
        x = rxSamples[:fftSize]
        window = scipy.signal.hann(fftSize)
        windowPower = sum(window**2)/fftSize
        absBins = np.abs(np.fft.fft(x*window))
        absBins = np.concatenate((absBins[absBins.size/2:], absBins[:absBins.size/2])) #reorder
        absBins = np.maximum(absBins, 1e-20) #clip
        return 20*np.log10(absBins) - 20*math.log10(absBins.size) - 10*math.log10(windowPower)

    fftSet = list()
    for i in range(numFFTs):
        fft = sampsToPowerFFT(rxSamples=rxSamples[i*fftSize:(i+1)*fftSize], fftSize=fftSize)
        fftSet.append(np.exp(fft))

    return np.log(sum(fftSet)/len(fftSet))

def plotSingleResult(rxInitial, rxFinal, txInitial, txFinal, outPath, freq, rate=SAMPLE_RATE):
    import matplotlib.pyplot as plt
    fig = plt.figure(figsize=(16, 9))
    fig.suptitle('Digital corrections @ %g MHz'%(freq/1e6), fontsize=12)

    for samps, idx, initial, isTx, ch in (
        (rxInitial, 1, True, False, 0),
        (rxInitial, 2, True, False, 1),
        (rxFinal, 3, False, False, 0),
        (rxFinal, 4, False, False, 1),
        (txInitial, 5, True, True, 0),
        (txInitial, 6, True, True, 1),
        (txFinal, 7, False, True, 0),
        (txFinal, 8, False, True, 1),
    ):
        otherCh = (ch + 1)%2
        chName = {0:'ChA', 1:'ChB'}[ch]
        otherChName = {0:'ChA', 1:'ChB'}[otherCh]
        title = "%s %s %s"%('Tx' if isTx else 'Rx', chName, 'Initial' if initial else 'Corrected')

        fftBins = sampsToPowerFFT(samps[ch])
        ax = fig.add_subplot(4, 2, idx)
        ax.plot(np.arange(-rate/2/1e6, rate/2/1e6, rate/fftBins.size/1e6)[:fftBins.size], fftBins, label=title)
        ax.grid(True)
        #ax.set_title(title, fontsize=8)
        #ax.set_xlabel('Freq (MHz)', fontsize=8)
        ax.set_ylabel('Power (dBfs)', fontsize=8)
        ax.set_ylim(top=0, bottom=-100)
        ax.locator_params(axis='y', nbins=6)
        legend = ax.legend(loc='upper left', fontsize=10)

        def annotate(label, freq, offset, color):
            x = int(fftBins.size*freq/rate) - fftBins.size/2
            power = max(fftBins[x-3:x+3])

            if initial: ax.annotate(label, fontsize=8,
                xy = (freq/1e6, power), xytext = (freq/1e6+offset[0], power+offset[1]),
                xycoords = 'data', textcoords = 'data',
                arrowprops = dict(arrowstyle = '->'))

            ax.scatter(freq/1e6, power, c=color, s=50)

        if isTx:
            annotate("Tx tone", TX_FREQ_DELTA + TX_CORDIC_FREQ[ch], (0.5, -10), 'green')
            annotate("Tx dc", TX_FREQ_DELTA, (-1, 10), 'yellow')
            annotate("Tx imbal", TX_FREQ_DELTA - TX_CORDIC_FREQ[ch], (-1, 10), 'red')
            annotate("%s\nleak"%otherChName, TX_FREQ_DELTA + TX_CORDIC_FREQ[otherCh], (0.5-ch, 10), 'blue')

        else:
            annotate("Tx tone", TX_FREQ_DELTA, (0.5, -10), 'green')
            annotate("Rx imbal", -TX_FREQ_DELTA, (-1, 10), 'red')

    print("Writing plot to %s"%outPath)
    fig.savefig(outPath)
    plt.close(fig)

##########################################
## Generate test points for IQ and DC
##########################################
def GenerateTestPoints(depth):

    scale = 1.0/(2**depth)
    phaseMax = (math.pi/2)*scale
    gainMax = 1.0*scale
    offMax = 1.0*scale

    dcPoints = list()
    for dcI in np.linspace(-offMax, +offMax, NUM_STEPS_PER_ITER):
        for dcQ in np.linspace(-offMax, +offMax, NUM_STEPS_PER_ITER):
            dcPoints.append(complex(dcI, dcQ))

    iqPoints = list()
    for phase in np.linspace(-phaseMax, phaseMax, NUM_STEPS_PER_ITER):
        for gain in np.linspace(-gainMax, gainMax, NUM_STEPS_PER_ITER):
            iqPoints.append(cmath.rect(2**gain, phase))

    return zip(dcPoints, iqPoints)

##########################################
## Calibrate sweep implementations
##########################################
def CalibrateSweepRx(limeSDR, rxStream):
    #sweep for best Rx IQ correction
    bestRxIqCorrs = [1.0]*2
    bestRxIqLevels = [1.0]*2
    for depth in MAX_SEARCH_DEPTHS:
        bestRxIqCorrsIter = copy.copy(bestRxIqCorrs)
        for dcPoint, iqPoint in GenerateTestPoints(depth):
            newIqCorrs = list()
            for ch in [0, 1]:
                newIqCorr = cmath.rect(
                    abs(iqPoint) * abs(bestRxIqCorrsIter[ch]),
                    cmath.phase(iqPoint) + cmath.phase(bestRxIqCorrsIter[ch]))
                limeSDR.setIQBalance(SOAPY_SDR_RX, ch, newIqCorr)
                newIqCorrs.append(newIqCorr)
            samps = readSamps(limeSDR, rxStream)
            for ch in [0, 1]:
                lvl = measureToneLevel(samps[ch], -TX_FREQ_DELTA)
                if lvl < bestRxIqLevels[ch]:
                    bestRxIqCorrs[ch] = newIqCorrs[ch]
                    bestRxIqLevels[ch] = lvl

    print("bestRxIqCorrs = %s"%bestRxIqCorrs)
    for ch in [0, 1]: limeSDR.setIQBalance(SOAPY_SDR_RX, ch, bestRxIqCorrs[ch])

def CalibrateSweepTx(limeSDR, rxStream):
    #sweep for best Tx IQ  and DC correction
    bestTxIqCorrs = [1.0]*2
    bestTxIqLevels = [1.0]*2
    bestTxDcCorrs = [0.0]*2
    bestTxDcLevels = [1.0]*2
    for depth in MAX_SEARCH_DEPTHS:
        bestTxIqCorrsIter = copy.copy(bestTxIqCorrs)
        bestTxDcCorrsIter = copy.copy(bestTxDcCorrs)
        for dcPoint, iqPoint in GenerateTestPoints(depth):
            newIqCorrs = list()
            newDcCorrs = list()
            for ch in [0, 1]:
                newIqCorr = cmath.rect(
                    abs(iqPoint) * abs(bestTxIqCorrsIter[ch]),
                    cmath.phase(iqPoint) + cmath.phase(bestTxIqCorrsIter[ch]))
                newDcCorr = complex(
                    max(min(bestTxDcCorrsIter[ch].real + dcPoint.real, 1.0), -1.0),
                    max(min(bestTxDcCorrsIter[ch].imag + dcPoint.imag, 1.0), -1.0))
                limeSDR.setIQBalance(SOAPY_SDR_TX, ch, newIqCorr)
                limeSDR.setDCOffset(SOAPY_SDR_TX, ch, newDcCorr)
                newIqCorrs.append(newIqCorr)
                newDcCorrs.append(newDcCorr)
            samps = readSamps(limeSDR, rxStream)
            for ch in [0, 1]:
                lvl = measureToneLevel(samps[ch], TX_FREQ_DELTA-TX_CORDIC_FREQ[ch])
                if lvl < bestTxIqLevels[ch]:
                    bestTxIqCorrs[ch] = newIqCorrs[ch]
                    bestTxIqLevels[ch] = lvl
                lvl = measureToneLevel(samps[ch], TX_FREQ_DELTA)
                if lvl < bestTxDcLevels[ch]:
                    bestTxDcCorrs[ch] = newDcCorrs[ch]
                    bestTxDcLevels[ch] = lvl

    print("bestTxIqCorrs = %s"%bestTxIqCorrs)
    print("bestTxDcCorrs = %s"%bestTxDcCorrs)
    for ch in [0, 1]:
        limeSDR.setIQBalance(SOAPY_SDR_TX, ch, bestTxIqCorrs[ch])
        limeSDR.setDCOffset(SOAPY_SDR_TX, ch, bestTxDcCorrs[ch])

##########################################
## Calibrate at a specified frequency
##########################################
def CalibrateAtFreq(limeSDR, rxStream, freq, dumpDir, validate):

    print('#'*40)
    print('## Calibrate @ %g MHz'%(freq/1e6))
    print('#'*40)

    #set the RF frequency on Rx and Tx
    limeSDR.setFrequency(SOAPY_SDR_RX, 0, "RF", freq, dict(CORRECTIONS="false"))
    limeSDR.setFrequency(SOAPY_SDR_TX, 0, "RF", freq + TX_FREQ_DELTA, dict(CORRECTIONS="false"))

    #clear correction for calibration
    for channel in [0, 1]:
        limeSDR.setFrequency(SOAPY_SDR_TX, channel, "BB", 0.0)
        limeSDR.setFrequency(SOAPY_SDR_RX, channel, "BB", 0.0)
        limeSDR.setDCOffset(SOAPY_SDR_TX, channel, 0.0)
        limeSDR.setIQBalance(SOAPY_SDR_TX, channel, 1.0)
        limeSDR.setIQBalance(SOAPY_SDR_RX, channel, 1.0)

    #tx tsp siggen constant
    limeSDR.writeSetting("TXTSP_CONST", str((1 << 14)))

    #adjust gain for best levels
    for ch in [0, 1]: limeSDR.setGain(SOAPY_SDR_RX, ch, "PGA", PGA_GAIN)
    samps = readSamps(limeSDR, rxStream)
    for ch in [0, 1]:
        lvldB = measureToneLevel(samps[ch], TX_FREQ_DELTA)
        deltadB = -5 - lvldB
        print('deltadB=%g, lvldB=%g'%(deltadB, lvldB))
        limeSDR.setGain(SOAPY_SDR_RX, ch, "PGA", min(19, PGA_GAIN + deltadB))

    #sweep for best Rx IQ correction
    rxInitial = readSamps(limeSDR, rxStream, sleep=True)
    if validate:
        limeSDR.writeSetting("APPLY_RX_CORRECTIONS", "true")
    else:
        CalibrateSweepRx(limeSDR, rxStream)
        limeSDR.writeSetting("STORE_RX_CORRECTIONS", "true")
    rxFinal = readSamps(limeSDR, rxStream, sleep=True)

    #sweep for best Tx IQ  and DC correction
    for ch in [0, 1]:
        limeSDR.setFrequency(SOAPY_SDR_TX, ch, "BB", TX_CORDIC_FREQ[ch])
    txInitial = readSamps(limeSDR, rxStream, sleep=True)
    if validate:
        limeSDR.writeSetting("APPLY_TX_CORRECTIONS", "true")
    else:
        CalibrateSweepTx(limeSDR, rxStream)
        limeSDR.writeSetting("STORE_TX_CORRECTIONS", "true")
    txFinal = readSamps(limeSDR, rxStream, sleep=True)

    if dumpDir is not None: plotSingleResult(
        rxInitial=rxInitial, rxFinal=rxFinal,
        txInitial=txInitial, txFinal=txFinal,
        freq=freq,
        outPath=os.path.join(dumpDir, 'results_%g_MHz.png'%(freq/1e6)))

##########################################
## Main calibration sweep
##########################################
def LimeSuiteCalibrate(
    args,
    freqStart,
    freqStop,
    freqStep,
    dumpDir,
    validate,
):
    if freqStart is None: raise Exception("No start frequency specified")
    if freqStop is None: freqStop = freqStart

    #open device
    print('#'*40)
    print('## Open device with "%s"'%(args))
    print('#'*40)
    limeSDR = SoapySDR.Device(args)
    print(str(limeSDR))
    info = limeSDR.getHardwareInfo()
    for k in info.keys(): print("%s:%s"%(k,info[k]))

    #FIXME work around so set sample rate programs the rates
    limeSDR.setFrequency(SOAPY_SDR_RX, 0, "RF", 1e9, dict(CORRECTIONS="false"))
    limeSDR.setFrequency(SOAPY_SDR_TX, 0, "RF", 1e9, dict(CORRECTIONS="false"))

    #initialize parameters
    print('#'*40)
    print('## Initialize "%s"'%(str(limeSDR)))
    print('#'*40)
    limeSDR.setMasterClockRate(CLOCK_RATE)
    for channel in [0, 1]:
        limeSDR.setSampleRate(SOAPY_SDR_TX, channel, SAMPLE_RATE)
        limeSDR.setSampleRate(SOAPY_SDR_RX, channel, SAMPLE_RATE)
        limeSDR.setAntenna(SOAPY_SDR_RX, channel, RX_ANTENNA)
        limeSDR.setAntenna(SOAPY_SDR_TX, channel, TX_ANTENNA)
        limeSDR.setGain(SOAPY_SDR_TX, channel, "PAD", PAD_GAIN)
        limeSDR.setGain(SOAPY_SDR_TX, channel, "LB_PAD", LB_PAD_GAIN)
        limeSDR.setGain(SOAPY_SDR_RX, channel, "PGA", PGA_GAIN)
        limeSDR.setGain(SOAPY_SDR_RX, channel, "TIA", TIA_GAIN)
        limeSDR.setGain(SOAPY_SDR_RX, channel, "LNA", LNA_GAIN)
        limeSDR.setGain(SOAPY_SDR_RX, channel, "LB_LNA", LB_LNA_GAIN)
        limeSDR.setDCOffsetMode(SOAPY_SDR_RX, channel, False)

    #open the rx stream
    rxStream = limeSDR.setupStream(SOAPY_SDR_RX, SOAPY_SDR_CF32, [0, 1])

    #sweep for each frequency
    for freq in np.arange(freqStart, freqStop+freqStep, freqStep):
        t0 = time.time()
        try: CalibrateAtFreq(limeSDR=limeSDR, rxStream=rxStream, freq=freq, dumpDir=dumpDir, validate=validate)
        except Exception as ex: print("Failed at %g MHz, skipping...\n    %s"%(freq/1e6, str(ex)))
        if not validate: print("Cal took %s seconds"%(time.time()-t0))

    #close the rx stream
    limeSDR.closeStream(rxStream)

    #close the device
    limeSDR = None
    print("Done")

def main():
    parser = OptionParser()
    parser.add_option("--args", type="string", dest="args", help="Device construction arguments [%default]", default='driver=lime')
    parser.add_option("--freqStart", type="float", dest="freqStart", help="Start frequency sweep in Hz")
    parser.add_option("--freqStop", type="float", dest="freqStop", help="Stop frequency sweep in Hz")
    parser.add_option("--freqStep", type="float", dest="freqStep", help="Frequency sweep step in Hz [%default]", default=500e3)
    parser.add_option("--dumpDir", type="string", dest="dumpDir", help="Directory to dump debug data and plots")
    parser.add_option("--validate", action="store_true", dest="validate", help="Validate cached corrections data", default=False)
    (options, args) = parser.parse_args()

    LimeSuiteCalibrate(
        args = options.args,
        freqStart = options.freqStart,
        freqStop = options.freqStop,
        freqStep = options.freqStep,
        dumpDir = options.dumpDir,
        validate = options.validate,
    )

if __name__ == '__main__': main()
