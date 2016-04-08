########################################################################
## Loopback sweep for corrections calibration data
########################################################################

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
RX_ANTENNA = "LB1"
TX_ANTENNA = "BAND1"
LB_LNA_GAIN = 40.0
PGA_GAIN = 0.0
TIA_GAIN = 0.0
LNA_GAIN = 0.0
PAD_GAIN = -10.0
LB_PAD_GAIN = 0.0

TX_FREQ_DELTA = 0.7e6
TX_CORDIC_FREQ = 1.0e6

MAX_SEARCH_DEPTHS = range(2, 11)
NUM_STEPS_PER_ITER = 3

SAMPS_PER_CAPTURE = 1024*8
NUM_BINS_PER_FFT = 1024

##########################################
## Sample utilities
##########################################
def readSamps(limeSDR, rxStream, numSamps=SAMPS_PER_CAPTURE):
    """
    Read stream data from each channel
    @return a list of complex64 arrays
    """
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
    return abs(np.mean(shifted))

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
        if (absBins.size % 2) == 0: absBins = absBins[1:]
        absBins = np.maximum(absBins, 1e-20) #clip
        return 20*np.log10(absBins) - 20*math.log10(absBins.size) - 10*math.log10(windowPower)

    fftSet = list()
    for i in range(numFFTs):
        fft = sampsToPowerFFT(rxSamples=rxSamples[i*fftSize:(i+1)*fftSize], fftSize=fftSize)
        fftSet.append(np.exp(fft))

    return np.log(sum(fftSet)/len(fftSet))

def plotSingleResult(rxInitial, rxFinal, txInitial, txFinal, rate=SAMPLE_RATE):
    import matplotlib.pyplot as plt
    fig = plt.figure(figsize=(20, 10), dpi=80)

    for samps, idx, title, initial, isTx in (
        (rxInitial[0], 1, 'Rx ChA Initial', True, False),
        (rxInitial[1], 2, 'Rx ChB Initial', True, False),
        (rxFinal[0], 3, 'Rx ChA Corrected', False, False),
        (rxFinal[1], 4, 'Rx ChB Corrected', False, False),
        (txInitial[0], 5, 'Tx ChA Initial', True, True),
        (txInitial[1], 6, 'Tx ChB Initial', True, True),
        (txFinal[0], 7, 'Tx ChA Corrected', False, True),
        (txFinal[1], 8, 'Tx ChB Corrected', False, True),
    ):

        fftBins = sampsToPowerFFT(samps)
        ax = fig.add_subplot(4, 2, idx)
        ax.plot(np.arange(-rate/2/1e6, rate/2/1e6, rate/fftBins.size/1e6)[:fftBins.size], fftBins)
        ax.grid(True)
        ax.set_title(title, fontsize=8)
        #ax.set_xlabel('Freq (MHz)', fontsize=8)
        ax.set_ylabel('Power (dBfs)', fontsize=8)
        ax.set_ylim(top=-10, bottom=-90)

        def annotate(label, freq, offset, color):
            x = int(fftBins.size*freq/rate) - fftBins.size/2
            power = max(fftBins[x-3:x+3])

            if initial: ax.annotate(label,
                xy = (freq/1e6, power), xytext = (freq/1e6+offset[0], power+offset[1]),
                xycoords = 'data', textcoords = 'data',
                arrowprops = dict(arrowstyle = '->'))

            ax.scatter(freq/1e6, power, c=color, s=50)

        if isTx:
            annotate("Tx tone", TX_FREQ_DELTA + TX_CORDIC_FREQ, (0.5, -10), 'green')
            annotate("Tx dc", TX_FREQ_DELTA, (-1, 10), 'yellow')
            annotate("Tx imbal", TX_FREQ_DELTA - TX_CORDIC_FREQ, (-1, 10), 'red')

        else:
            annotate("Tx tone", TX_FREQ_DELTA, (0.5, -10), 'green')
            annotate("Rx imbal", -TX_FREQ_DELTA, (-1, 10), 'red')

    fig.savefig('/tmp/out.png')
    plt.close(fig)

##########################################
## Generate test points for IQ and DC
##########################################
def GenerateTestPoints(depth):

    scale = 1.0/(2**depth)
    phaseMax = (math.pi/2)*scale
    gainMax = 0.5*scale
    offMax = 1.0*scale

    dcPoints = list()
    for dcI in np.linspace(-offMax, +offMax, NUM_STEPS_PER_ITER):
        for dcQ in np.linspace(-offMax, +offMax, NUM_STEPS_PER_ITER):
            dcPoints.append(complex(dcI, dcQ))

    iqPoints = list()
    for phase in np.linspace(-phaseMax, phaseMax, NUM_STEPS_PER_ITER):
        for gain in np.linspace(-gainMax, gainMax, NUM_STEPS_PER_ITER):
            iqPoints.append(cmath.rect(1.0 + gain, phase))

    return zip(dcPoints, iqPoints)

##########################################
## Calibrate at a specified frequency
##########################################
def CalibrateAtFreq(limeSDR, rxStream, freq):

    #set the RF frequency on Rx and Tx
    limeSDR.setFrequency(SOAPY_SDR_RX, 0, "RF", freq)
    limeSDR.setFrequency(SOAPY_SDR_TX, 0, "RF", freq + TX_FREQ_DELTA)

    #clear correction for calibration
    for channel in [0, 1]:
        limeSDR.setFrequency(SOAPY_SDR_TX, channel, "BB", 0.0)
        limeSDR.setFrequency(SOAPY_SDR_RX, channel, "BB", 0.0)
        limeSDR.setDCOffset(SOAPY_SDR_TX, channel, 0.0)
        limeSDR.setIQBalance(SOAPY_SDR_TX, channel, 1.0)
        limeSDR.setIQBalance(SOAPY_SDR_RX, channel, 1.0)

    #adjust gain for best levels
    for ch in [0, 1]: limeSDR.setGain(SOAPY_SDR_RX, ch, "PGA", PGA_GAIN)
    samps = readSamps(limeSDR, rxStream)
    for ch in [0, 1]:
        lvl = measureToneLevel(samps[ch], TX_FREQ_DELTA)
        deltadB = -10 - 20*math.log10(lvl)
        print 'deltadB', deltadB, lvl, 20*math.log10(lvl)
        limeSDR.setGain(SOAPY_SDR_RX, ch, "PGA", min(19, PGA_GAIN + deltadB))

    #sweep for best Rx IQ correction
    rxInitial = readSamps(limeSDR, rxStream)
    bestRxIqCorrs = [1.0]*2
    bestRxIqLevels = [1.0]*2
    for depth in MAX_SEARCH_DEPTHS:
        bestRxIqCorrsIter = copy.copy(bestRxIqCorrs)
        for dcPoint, iqPoint in GenerateTestPoints(depth):
            newIqCorrs = list()
            for ch in [0, 1]:
                newIqCorr = cmath.rect(
                    abs(iqPoint) + abs(bestRxIqCorrsIter[ch]) - 1.0,
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
    rxFinal = readSamps(limeSDR, rxStream)

    #sweep for best Tx IQ  and DC correction
    for ch in [0, 1]: limeSDR.setFrequency(SOAPY_SDR_TX, ch, "BB", TX_CORDIC_FREQ)
    txInitial = readSamps(limeSDR, rxStream)
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
                    abs(iqPoint) + abs(bestTxIqCorrsIter[ch]) - 1.0,
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
                lvl = measureToneLevel(samps[ch], TX_FREQ_DELTA-TX_CORDIC_FREQ)
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
    txFinal = readSamps(limeSDR, rxStream)

    plotSingleResult(rxInitial, rxFinal, txInitial, txFinal)

##########################################
## Main calibration sweep
##########################################
def LimeSuiteCalibrate(
    args,
    freq_start,
):
    #open device
    limeSDR = SoapySDR.Device(args)
    print(str(limeSDR))

    #FIXME work around so set sample rate programs the rates
    limeSDR.setFrequency(SOAPY_SDR_RX, 0, "RF", 1e9)
    limeSDR.setFrequency(SOAPY_SDR_TX, 0, "RF", 1e9)

    #initialize parameters
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

    #tx tsp siggen constant
    limeSDR.writeSetting("ACTIVE_CHANNEL", "A")
    limeSDR.writeSetting("ENABLE_TXTSP_CONST", "true")
    limeSDR.writeSetting("ACTIVE_CHANNEL", "B")
    limeSDR.writeSetting("ENABLE_TXTSP_CONST", "true")

    #open the rx stream
    rxStream = limeSDR.setupStream(SOAPY_SDR_RX, SOAPY_SDR_CF32, [0, 1])

    t0 = time.time()
    CalibrateAtFreq(limeSDR, rxStream, freq_start)
    print("Cal took %s seconds"%(time.time()-t0))

    #close the rx stream
    limeSDR.closeStream(rxStream)

    #close the device
    limeSDR = None

def main():
    parser = OptionParser()
    parser.add_option("--args", type="string", dest="args", help="Device construction arguments", default='')
    parser.add_option("--freq-start", type="float", dest="freq_start", help="Start frequency sweep in Hz")
    (options, args) = parser.parse_args()

    LimeSuiteCalibrate(
        args = options.args,
        freq_start = options.freq_start,
    )

if __name__ == '__main__': main()
