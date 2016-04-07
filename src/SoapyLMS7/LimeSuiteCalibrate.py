########################################################################
## Loopback sweep for corrections calibration data
########################################################################

from optparse import OptionParser
import SoapySDR
from SoapySDR import * #*_SOAPY_SDR constants
import numpy as np
import scipy.signal
import math
import time

##########################################
## Calibration constants
##########################################
CLOCK_RATE = 80e6
SAMPLE_RATE = 10e6
RX_ANTENNA = "LB1"
TX_ANTENNA = "BAND1"
LB_LNA_GAIN = 35.0
PGA_GAIN = 0.0
TIA_GAIN = 0.0
LNA_GAIN = 0.0
PAD_GAIN = -10.0
LB_PAD_GAIN = 0.0
TX_FREQ_DELTA = 0.7e6

##########################################
## FFT utilities
##########################################
def readSamps(limeSDR, rxStream, numSamps=1024*8):
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

def sampsToPowerFFT(rxSamples, fftSize=1024):
    """
    Get a power FFT for a complex samples array
    Averages multiple FFTs when rxSamples.size > fftSize.
    Window power and fft gain is not compensated for.
    @param rxSamples an array of samples (not list of arrays)
    """

    numFFTs = rxSamples.size/fftSize

    if numFFTs == 1:
        x = rxSamples[:fftSize]
        absBins = np.abs(np.fft.fft(x))
        absBins = np.concatenate((absBins[absBins.size/2:], absBins[:absBins.size/2])) #reorder
        if (absBins.size % 2) == 0: absBins = absBins[1:]
        absBins = np.maximum(absBins, 1e-20) #clip
        return 20*np.log10(absBins) - 20*math.log10(absBins.size)

    fftSet = list()
    for i in range(numFFTs):
        fft = sampsToPowerFFT(rxSamples=rxSamples[i*fftSize:(i+1)*fftSize], fftSize=fftSize)
        fftSet.append(np.exp(fft))

    return np.log(sum(fftSet)/len(fftSet))

def plotFFT(samps0, samps1, fftBins0, fftBins1, rate=SAMPLE_RATE):
    import matplotlib.pyplot as plt
    fig = plt.figure(figsize=(20, 8), dpi=80)

    fftBins = fftBins0
    ax = fig.add_subplot(2, 2, 1)
    ax.plot(np.arange(-rate/2/1e6, rate/2/1e6, rate/fftBins.size/1e6)[:fftBins.size], fftBins)
    ax.grid(True)
    ax.set_title('Channel A', fontsize=8)
    ax.set_xlabel('Freq (MHz)', fontsize=8)
    ax.set_ylabel('Power (dBx)', fontsize=8)

    fftBins = fftBins1
    ax = fig.add_subplot(2, 2, 2)
    ax.plot(np.arange(-rate/2/1e6, rate/2/1e6, rate/fftBins.size/1e6)[:fftBins.size], fftBins)
    ax.grid(True)
    ax.set_title('Channel B', fontsize=8)
    ax.set_xlabel('Freq (MHz)', fontsize=8)
    ax.set_ylabel('Power (dBx)', fontsize=8)

    samps = samps0[:100]
    ax = fig.add_subplot(2, 2, 3)
    ax.grid(True)
    ax.set_title('Channel A', fontsize=8)
    ax.set_xlabel('Time (us)', fontsize=8)
    ax.set_ylabel('Amplitude (units)', fontsize=8)
    timeIndex = np.arange(0, samps.size/rate*1e6, 1/rate*1e6)[:samps.size]
    ax.plot(timeIndex, np.real(samps), 'b', timeIndex, np.imag(samps), 'g')

    samps = samps1[:100]
    ax = fig.add_subplot(2, 2, 4)
    ax.grid(True)
    ax.set_title('Channel B', fontsize=8)
    ax.set_xlabel('Time (us)', fontsize=8)
    ax.set_ylabel('Amplitude (units)', fontsize=8)
    timeIndex = np.arange(0, samps.size/rate*1e6, 1/rate*1e6)[:samps.size]
    ax.plot(timeIndex, np.real(samps), 'b', timeIndex, np.imag(samps), 'g')

    fig.savefig('/tmp/out.png')
    plt.close(fig)

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

    #the sweep
    limeSDR.setFrequency(SOAPY_SDR_RX, 0, "RF", freq_start)
    limeSDR.setFrequency(SOAPY_SDR_TX, 0, "RF", freq_start + TX_FREQ_DELTA)

    for channel in [0, 1]:
        limeSDR.setFrequency(SOAPY_SDR_TX, channel, "BB", 0.0)
        limeSDR.setFrequency(SOAPY_SDR_RX, channel, "BB", 0.0)
        limeSDR.setDCOffset(SOAPY_SDR_TX, channel, 0.0)
        limeSDR.setIQBalance(SOAPY_SDR_TX, channel, 1.0)
        limeSDR.setIQBalance(SOAPY_SDR_RX, channel, 1.0)

    time.sleep(1)
    samps = readSamps(limeSDR, rxStream)
    fft0 = sampsToPowerFFT(samps[0])
    fft1 = sampsToPowerFFT(samps[1])
    plotFFT(samps[0], samps[1], fft0, fft1)

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
