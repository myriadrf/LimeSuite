import time
import unittest
import SoapySDR
from SoapySDR import * #SOAPY_SDR_* constants
import numpy as np

SDR_ARGS = {'driver': 'lime'}

class TestBasicStreaming(unittest.TestCase):

    def setUp(self):
        self.sdr = SoapySDR.Device(SDR_ARGS)
        # We need to detect the number of channels as the LimeSDR has two
        # and the LimeSDR-Mini has one
        chans = range(self.sdr.getNumChannels(0))

        for ch in chans:
            self.sdr.setSampleRate(SOAPY_SDR_RX, ch, 10e6)
            self.sdr.setSampleRate(SOAPY_SDR_TX, ch, 10e6)
        self.rxStream = self.sdr.setupStream(SOAPY_SDR_RX, SOAPY_SDR_CF32, chans)
        self.txStream = self.sdr.setupStream(SOAPY_SDR_TX, SOAPY_SDR_CF32, chans)

    def tearDown(self):
        self.sdr.closeStream(self.rxStream)
        self.sdr.closeStream(self.txStream)
        self.sdr = None

    def testTime(self):
        print('===== test the timestamps =====')
        self.sdr.activateStream(self.txStream)
        self.sdr.activateStream(self.rxStream)
        t0 = self.sdr.getHardwareTime()
        print('t0=%d ns'%t0)
        time.sleep(0.1)
        t1 = self.sdr.getHardwareTime()
        print('t1=%d ns'%t1)
        delta = float(t1-t0)/1e8
        print('delta=%f secs'%delta)
        self.assertGreater(delta, 0.9)
        self.assertLess(delta, 1.1)

        self.sdr.deactivateStream(self.txStream)
        self.sdr.deactivateStream(self.rxStream)

    def testRxContinuous(self):
        print('===== receive a continuous stream =====')
        self.sdr.activateStream(self.rxStream)

        buff0 = np.zeros(1024, np.complex64)
        buff1 = np.zeros(1024, np.complex64)

        print('readStream continuously...')
        doneLoopTime = time.time() + 0.1
        while time.time() < doneLoopTime:
            sr = self.sdr.readStream(self.rxStream, [buff0, buff1], 1024)
            self.assertGreater(sr.ret, 0)

        #deactivate
        self.sdr.deactivateStream(self.rxStream)

        print('readStream for a timeout...')
        sr = self.sdr.readStream(self.rxStream, [buff0, buff1], 1024)
        print(sr)
        self.assertEqual(sr.ret, SOAPY_SDR_TIMEOUT)

    def testRxBurstNow(self):
        print('===== receive a burst asap =====')
        numElemsRequest = 10000
        self.sdr.activateStream(self.rxStream, SOAPY_SDR_END_BURST, 0, numElemsRequest)

        buff0 = np.zeros(1024, np.complex64)
        buff1 = np.zeros(1024, np.complex64)

        print('readStream for a burst...')
        while numElemsRequest > 0:
            sr = self.sdr.readStream(self.rxStream, [buff0, buff1], 1024)
            print(sr)
            self.assertGreater(sr.ret, 0)
            numElemsRequest -= sr.ret

            #only end burst marked on the last
            if numElemsRequest: self.assertFalse(sr.flags & SOAPY_SDR_END_BURST)
            else: self.assertTrue(sr.flags & SOAPY_SDR_END_BURST)

        self.assertEqual(numElemsRequest, 0)

        print('readStream for a timeout...')
        sr = self.sdr.readStream(self.rxStream, [buff0, buff1], 1024)
        print(sr)
        self.assertEqual(sr.ret, SOAPY_SDR_TIMEOUT)

        self.sdr.deactivateStream(self.rxStream)

    def testRxBurstAtTime(self):
        print('===== receive a timed burst =====')
        numElemsRequest = 10000
        tBurst = self.sdr.getHardwareTime() + int(1e8)
        self.sdr.activateStream(self.rxStream, SOAPY_SDR_END_BURST | SOAPY_SDR_HAS_TIME, tBurst, numElemsRequest)

        buff0 = np.zeros(1024, np.complex64)
        buff1 = np.zeros(1024, np.complex64)

        print('readStream for a timed burst...')
        tFirst = -1
        while numElemsRequest > 0:
            sr = self.sdr.readStream(self.rxStream, [buff0, buff1], 1024, timeoutUs=int(1e6))
            print(sr)
            self.assertGreater(sr.ret, 0)
            numElemsRequest -= sr.ret
            if tFirst == -1:
                tFirst = sr.timeNs
                self.assertTrue(sr.flags & SOAPY_SDR_HAS_TIME)

            #only end burst marked on the last
            if numElemsRequest: self.assertFalse(sr.flags & SOAPY_SDR_END_BURST)
            else: self.assertTrue(sr.flags & SOAPY_SDR_END_BURST)

        #check that the first timestamp was correct
        #there is some play in the nanosecond conversion,
        #but it should be the same 10 MHz clock tick
        self.assertEqual(tFirst/100, tBurst/100)

        self.assertEqual(numElemsRequest, 0)

        print('readStream for a timeout...')
        sr = self.sdr.readStream(self.rxStream, [buff0, buff1], 1024)
        print(sr)
        self.assertEqual(sr.ret, SOAPY_SDR_TIMEOUT)

        self.sdr.deactivateStream(self.rxStream)

    def testRxLateBurst(self):
        print('===== receive a late burst =====')
        numElemsRequest = 10000
        tBurst = self.sdr.getHardwareTime() + int(1e8)
        self.sdr.activateStream(self.rxStream, SOAPY_SDR_END_BURST | SOAPY_SDR_HAS_TIME, tBurst, numElemsRequest)

        buff0 = np.zeros(1024, np.complex64)
        buff1 = np.zeros(1024, np.complex64)

        print('readStream for a late burst...')
        time.sleep(1.0) #make sure the readStream is late
        sr = self.sdr.readStream(self.rxStream, [buff0, buff1], 1024, timeoutUs=int(1e6))
        self.assertEqual(sr.ret, SOAPY_SDR_TIME_ERROR)

        print('readStream for a timeout...')
        sr = self.sdr.readStream(self.rxStream, [buff0, buff1], 1024)
        print(sr)
        self.assertEqual(sr.ret, SOAPY_SDR_TIMEOUT)

        self.sdr.deactivateStream(self.rxStream)

    def testTxLate(self):
        print('===== test txing a late packet =====')
        self.sdr.activateStream(self.txStream)
        self.sdr.activateStream(self.rxStream)
        t0 = self.sdr.getHardwareTime()
        tLate = t0 + int(1e8)
        buff0 = np.zeros(1024, np.complex64)
        buff1 = np.zeros(1024, np.complex64)
        flags = SOAPY_SDR_HAS_TIME | SOAPY_SDR_END_BURST
        time.sleep(1.0) #make sure the writeStream is late
        self.sdr.writeStream(self.txStream,
            [buff0, buff1], 1024,
            flags=flags,
            timeNs=tLate,
            timeoutUs=int(1e6))

        #small wait for late packet to get reported
        time.sleep(0.1)

        print('readStreamStatus for a late indicator...')
        r0 = self.sdr.readStreamStatus(self.txStream)
        self.assertEqual(r0.ret, SOAPY_SDR_TIME_ERROR)
        self.assertTrue(r0.flags & SOAPY_SDR_HAS_TIME)

        #TODO check the delta when we have an understanding of the time reported
        delta = float(r0.timeNs-t0)/1e9
        print('delta=%f secs'%delta)
        #self.assertGreater(delta, 1.4)
        #self.assertLess(delta, 1.6)

        print('readStreamStatus for a timeout...')
        r1 = self.sdr.readStreamStatus(self.txStream)
        self.assertNotEqual(r1.ret, SOAPY_SDR_TIME_ERROR)

        self.sdr.deactivateStream(self.txStream)
        self.sdr.deactivateStream(self.rxStream)

if __name__ == '__main__':
    unittest.main()
