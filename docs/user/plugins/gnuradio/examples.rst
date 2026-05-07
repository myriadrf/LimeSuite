Examples
########

The following example flowgraphs can be found in the :code:`gr-limesdr/examples` directory.

Note that each flowgraph has an options block, which is always used in GNU Radio flowgraphs. From the GNU Radio Wiki: 

*In order to make a hierarchical block, the parameters in the options block must be set properly. The id of the options block sets the module name and must be unique among the entire library of blocks (built-in and custom). The title parameter sets the display name for the block. The generate options must be set to "Hier Block". The category parameter sets the category for the new block. This category can be an existing category in the block selection window or a new category.*

FM Transmitter
**************

.. figure:: /images/FM_Transmitter_Example_GnuRadio.png
   :align: center

Open the :code:`FM_transmitter.grc` example and you should see the flowgraph shown above. This is comprised of:

* Wav file source is used to create a data source from an audio wave file. In order for this example to work you should select the :code:`gr-limesdr/examples/piano.wav` file. Audio rate should be 48 kHz. Wav file source could be changed to Audio source in order to transmit real time recording device data.
* NBFM Transmit block stands for narrowband transmit. This block performs as FM modulator.
* Rational Resampler block is used to the change sample rate of the NBFM output in order to match the LimeSuite Sink (TX) input sample rate. NBFM output sample rate (480 kS) is multiplied by 25 and divided by 6 (480 000*25/6 =2 000 000). The result is 2MS.
* LimeSuite Sink (TX) is used to consume (transmit) samples at 2MS.
* WX GUI FFT Sink is used to analyze FFT of samples produced for LimeSuite Sink (TX) to consume.

FM Receiver
***********

.. figure:: /images/FM_Receiver_Example_GnuRadio.png
   :align: center

Open the :code:`FM_receiver.grc` example and you should see the flowgraph shown above. This is comprised of:

* LimeSuite Source (RX) is used to produce (receive) samples at 2MS rate.
* The Low Pass Filter filters out unnecessary frequencies. Frequencies above 500kHz with a transition width of 100kHz are filtered. The filter can be modified real time by using WX GUI Slider blocks, in order to modify Cutoff Frequency and Transition Width.
* Rational Resampler block is used to change sample rate of Low Pass Filter output in order to meet WBFM Receive input sample rate. Low Pass Filter output sample rate (2MS) is multiplied by 48 and divided by 200 (2 000 000*48/200 =480 000). The result is 480kS.
* WBFM Receive stands for wideband FM receive. It acts as an FM demodulator.
* Multiply const is used to adjust volume (amplitude) by using WX GUI Slider with Volume slider.
* Audio Sink is used to produce real time sound through playback device.

FM Receiver with RDS
********************

The :code:`rds_rx.grc` example is an advanced FM receiver based on the `gr-rds`_ package, which must be installed before use.

To understand how this flowgraph works, see:

* http://jmfriedt.free.fr/lm_rds_eng.pdf
* https://github.com/bastibl/gr-rds/tree/maint-3.10/docs (generate Doxygen docs)

GFSK Loopback
*************

.. figure:: /images/GFSK_loopback_example.png
   :align: center

Open the :code:`GFSK.grc` example and you should see blocks as shown. 

Here you can see both GFSK modulation and GFSK demodulation parts, which demonstrate realtime data processing using a single LimeSDR board. Ensure that you have antennas connected to your board, or you may use cabling and an attenuator instead.

Select your data source in block File Source, which may contain text to send. Packet Encoder is used to add logical structure to raw bytes; the block takes a number of bytes equal to Payload Length and groups them with a header and CNC,  which when left blank uses default values. 

The GFSK Mod block performs GFSK modulation and following which samples are transmitted using LimeSuite Sink (TX). 

The Quadrature Demod block is used to convert complex samples to I and Q floating point sample stream, following which these can be analysed in QT GUI Time Sink.

Data send is received using LimeSuite Source (RX) and we can analyse this via Quadrature Demod and QT GUI Time Sink. 

GFSK Demod is used to extract packets, which are decoded using Packet Decoder, and then saved to a file via File Sink. Finally, we can kill the flow graph and analyse transmitted text data and received text data in order to see the results. 

Signal Generator
****************

.. figure:: /images/Signal_generator_example.png
   :align: center

Open the :code:`signal_generator.grc` example and you should see blocks as shown. 

Using Signal Source block we can generate different waveforms (constant, sine, cosine, square, triangle and saw tooth) with different frequencies and sampling rates. The Signal Source block is connected directly to the LimeSuite Sink (TX) block which transmits generated data.

In order to see our waveform we use QT GUI Sink to view waterfall, FFT, time and constellation plots. 

In order to configure the device while the flowgraph is executing there are baseband, TX gain, NCO frequency, NCO direction, analog and digital filters controls implemented using QT. This way we have powerful Signal Generator. 

Signal Analyser
***************

.. figure:: /images/Signal_analyser_example.png
   :align: center

Open the :code:`signal_analyzer.grc` example and you should see blocks as shown. 

In this example we use LimeSuite Source (RX) to receive samples and view them using QT GUI Sink. In QT GUI Sink we can see waterfall, constellation, time and FFT plots. 

By using QT controls we can adjust RX gain, baseband, NCO, analog and digital filters. 

.. note::
   To use the Signal Generator and Signal Analyser examples together we can execute them via a single flowgraph, which will allow us to observe transmitted and received samples. When doing this we should create a loopback condition using either antennas, or attenuators and cables. 

DVB-T Transmitter
*****************

.. figure:: /images/DVB-T_transmitter_example.png
   :align: center

Open the :code:`DVB_T_2k_64QAM_TX.grc` example and you should see blocks as shown. 

A `test transport stream`_ should also be downloaded and placed in the :code:`gr-limesdr/examples/` directory.

To run the example, you may connect your LimeSDR to an RTLSDR dongle (while using it in DVB mode) or any other DVB receiver. Simply connect the output marked TX-1 (LimeSDR USB) or TX (LimeSDR Mini) to your DVB-T dongle's RF input using a coaxial cable (or by connecting separate antennas to both devices).

Before executing the flowgraph, you need to double-click on the file source block and select the :code:`adv8dvbt23qam64.ts` test transport stream file. You may then use `VLC Media Player`_ with a USB DVB-T dongle to display the digital TV stream by running it with the following command:

.. code-block:: bash

   vlc dvb://frequency=700000

This will configure VLC to receive a DVB-T stream transmitted with a carrier frequency of 700 MHz.

.. note::
   A fairly capable CPU is required for realtime encoding and transmission of a DVB-T stream. Lack of processing power for stream encoding and transmission usualy results in the received video stream stuttering, random glitches or losing audio (typically accompanied by VLC's 'TS Discontinuity' errors in the terminal).

Various tools, additional information and links to variant bitrate test streams for the DVB-T flow graphs can be found here: 

* https://github.com/drmpeg/dtv-utils 

.. _gr-rds: https://github.com/bastibl/gr-rds
.. _test transport stream: http://www.w6rz.net/adv8dvbt23qam64.ts
.. _VLC Media Player: https://www.videolan.org/

