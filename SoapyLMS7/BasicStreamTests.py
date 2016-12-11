import time
import unittest
import SoapySDR
from SoapySDR import * #SOAPY_SDR_* constants
import numpy as np

SDR_ARGS = {'driver': 'lime'}

class TestBasicStreaming(unittest.TestCase):

    def setUp(self):
        self.sdr = SoapySDR.Device(SDR_ARGS)
        for ch in [0, 1]:
            self.sdr.setSampleRate(SOAPY_SDR_RX, ch, 10e6)
            self.sdr.setSampleRate(SOAPY_SDR_TX, ch, 10e6)
        self.rxStream = self.sdr.setupStream(SOAPY_SDR_RX, SOAPY_SDR_CF32, [0, 1])
        self.txStream = self.sdr.setupStream(SOAPY_SDR_TX, SOAPY_SDR_CF32, [0, 1])

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
        time.sleep(1.0)
        t1 = self.sdr.getHardwareTime()
        print('t1=%d ns'%t1)
        delta = float(t1-t0)/1e9
        print('delta=%f secs'%delta)
        self.assertGreater(delta, 0.9)
        self.assertLess(delta, 1.1)

        self.sdr.deactivateStream(self.txStream)
        self.sdr.deactivateStream(self.rxStream)

    def testLate(self):
        print('===== test txing a late packet =====')
        self.sdr.activateStream(self.txStream)
        self.sdr.activateStream(self.rxStream)
        tLate = self.sdr.getHardwareTime() + int(1e9)
        buff0 = np.zeros(1024, np.complex64)
        buff1 = np.zeros(1024, np.complex64)
        flags = SOAPY_SDR_HAS_TIME | SOAPY_SDR_END_BURST
        self.sdr.writeStream(self.txStream,
            [buff0, buff1], 1024,
            flags=flags,
            timeNs=tLate,
            timeoutUs=int(1e6))
        time.sleep(1.5)

        print('readStreamStatus for a late indicator...')
        r0 = self.sdr.readStreamStatus(self.txStream)
        self.assertEqual(r0.ret, SOAPY_SDR_TIME_ERROR)
        self.assertTrue(r0.flags & SOAPY_SDR_HAS_TIME)
        print((r0.timeNs-tLate)/1e9)

        print('readStreamStatus for a timeout...')
        r1 = self.sdr.readStreamStatus(self.txStream)
        self.assertEqual(r1.ret, SOAPY_SDR_TIMEOUT)

        self.sdr.deactivateStream(self.txStream)
        self.sdr.deactivateStream(self.rxStream)

if __name__ == '__main__':
    unittest.main()
