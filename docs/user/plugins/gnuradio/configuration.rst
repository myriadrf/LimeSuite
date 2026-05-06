Configuration
#############

Lime Suite Sink (TX)
********************

.. figure:: /images/LimeSuite_Sink_selection_in_LimeSuite.png
   :align: center

Select LimeSuite Sink (TX) from the LimeSuite category as shown.

.. figure:: /images/LimeSuite_Sink_block.png
   :align: center

The LimeSuite Sink (TX) block appears in GNU Radio workspace.

.. figure:: /images/LimeSuite_Sink_settings.png
   :align: center

To configure the block double-click on it to enable editing settings.

The settings are as follows:

* **ID**: The ID of the current block in GNU Radio environment. This is usually left unchanged.
* **Device serial**: Select which device to use if there are multiple devices. A list of devices can be obtained by running following command in your Linux terminal window:

.. code-block:: bash

   LimeUtil --find

* **Channel**: Use this setting to set single-input and single-output (SISO) or multiple-input and multiple-output (MIMO) channel configuration. This setting enables/disables channels, configures the correct number of streams and block inputs. 

   * In SISO mode you can select which channel to control (A or B).
   * Not all boards support MIMO mode. For example LimeSDR Mini only supports SISO mode, while LimeSDR USB and LimeSDR PCIe support both SISO and MIMO modes.

* **File**: This can be used to select an .ini file which was saved by using LimeSuiteGUI. When this feature is used, the same .ini file must be loaded in both the LimeSuite Sink (TX) and LimeSuite Source (RX) blocks for a device.

  * *The Channel selection should match between LimeSuite Sink (TX) and LimeSuite Source (RX) blocks for the same device.*

* **RF frequency (callback function value)**: Set the RF frequency for TX (both channels).
* **Sample rate**: Select sample rate for TX.

  * LimeSDR USB and LimeSDR PCIe sample rate must be no more than 61.44 MS/s.
  * LimeSDR Mini sample rate must be no more than 30.72 MS/s.
  * LimeNET Micro sample rate must be no more than 10 MS/s.
  * *The sample rate should match in the LimeSuite Sink (TX) and LimeSuite Source (RX) blocks for the same device.*

* **Oversample**: This enables TX signal sampling at a higher rate than would be required to just preserve the desired signal frequencies.

  * Possible values (multipliers): Default (i.e. 0), 1, 2, 4, 8, 16, 32.
  * Default value uses highest possible value based on other settings. 
  * Oversample is connected to the sample rate and there are limitations (see below). 
  * With LimeSDR Mini and LimeNET Micro the same oversampling must be used for TX and RX.

.. list-table:: TX oversampling LimeSDR Mini
    :header-rows: 1

    * - Sampling rate range (x)
      - Valid oversampling values
    * - (15.36 < x ≤ 30.72) MS/s
      - 1, 2, 4 
    * - (7.68 < x ≤ 15.36) MS/s
      - 1, 2, 4, 8
    * - (3.84 < x ≤ 7.86) MS/s
      - 1, 2, 4, 8, 16
    * - X ≤ 3.84 MS/s
      - 1, 2, 4, 8, 16, 32

.. list-table:: TX oversampling LimeSDR USB and LimeSDR PCIe 
    :header-rows: 1

    * - Sampling rate range (x)
      - Valid oversampling values
    * - (30.72 < x ≤ 61.44) MS/s
      - 1, 2, 4, 8
    * - (15.36 < x ≤ 30.72) MS/s
      - 1, 2, 4, 8, 16
    * - X ≤ 15.36 MS/s
      - 1, 2, 4, 8, 16, 32

.. list-table:: TX oversampling LimeNET Micro
    :header-rows: 1

    * - Sampling rate range (x)
      - Valid oversampling values
    * - (7.68 < x ≤ 10) MS/s
      - 1, 2, 4, 8
    * - (3.84 < x ≤ 7.86) MS/s
      - 1, 2, 4, 8, 16
    * - X ≤ 3.84 MS/s
      - 1, 2, 4, 8, 16, 32

* **TCXO:DAC value**: This setting modifies the VCTXCO DAC parameter value used to calibrate the reference clock. To enable this parameter *"Allow TCXO DAC control"* in the *"Advanced"* tab must be set to *"Yes"*. Keep in mind that this parameter returns to its default value only after device power off. 

  * LimeSDR Mini default value is 180, range is [0,255].
  * LimeSDR USB default value is 125, range is [0,255].
  * LimeSDR PCIe default value is 134, range is [0,255].
  * LimeNET Micro default value is 30714, range is [0,65535].

* **Length tag name**: Here you can set the name of the incoming stream tag. This tag is used to set how many samples are to be sent and it should be used in conjunction with *“tx_time”* tag, in order to set the exact time at which samples should be sent. 

  * Position of the length tag name configures where the burst starts and its value tells how many samples the burst has. Length tag name value is an :code:`int64` and *“tx_time”* tag is :code:`pmt::cons`` pair which consist of two numbers: the number of full seconds(int64) and number of fractional seconds(double). More about stream tags can be read here: `GNU Radio Stream Tags documentation`_.

* **NCO frequency (callback function value)**: Here you can adjust the LMS7002M numerically controlled oscillator. 0 disables the NCO.
* **Calibration bandw.:** This setting is used to set the TX calibration bandwidth of each channel. 

  * This value should be equal to your signal bandwidth. 
  * Calibration bandwidth range must be 2.5MHz–120MHz.
  * 0 disables calibration, but this is not recommended as it may lead to suboptimal performance.

* **PA path (callback function value)**: Select the active power amplifier path of each channel. 

  * Possible selections: Auto(Default), BAND1, BAND2.
  * This setting also controls LimeSDR Mini and LimeNET Micro board path switch (BAND1/BAND2).
  * When Auto(Default) is selected, the RF path is automatically selected for LimeSDR Mini and LimeNET Micro boards depending on the configured RF frequency. Auto(Default) sets BAND1 for LimeSDR USB and LimeSDR PCIe.

* **Analog filter bandw (callback function value)**: Enter the TX analog filter bandwidth for each channel. 0 disables the analog filter.

  * TX analog filter bandwidth range must be 5MHz–130MHz.

* **Digital filter bandw. (callback function value)**:Enter TX digital filter bandwidth for each channel. 0 means that digital filter is turned OFF.

    * TX digital filter bandwidth should not be higher than sampling rate.

* **Gain (callback function value)**: Controls TX channel gain.

    * Gain range must be 0dB–73dB.

Lime Suite Source (RX)
**********************

.. figure:: /images/LimeSuite_Source_selection_in_LimeSuite.png
   :align: center

Pick LimeSuite Source (RX) from LimeSuite category as shown.

.. figure:: /images/LimeSuite_Source_block.png
   :align: center

The LimeSuite Source (RX) block appears in the GNU Radio workspace.

.. figure:: /images/LimeSuite_Source_settings.png
   :align: center

Double-click on the block to configure the settings.

The available settings (in General, CH_A and CH_B tabs) are:

* **ID**: ID of current block in GNU Radio environment. This is usually left unchanged.
* **Device number**: Select which device to use from a list of devices to use multiple devices. This list can be obtained by running following command in your Linux terminal window:

.. code-block:: bash

   LimeUtil --find

* **Channel**: Use this to configure single-input and single-output (SISO) or multiple-input and multiple-output (MIMO) mode. This setting enables/disables channels, sets correct number of streams and controls number of block inputs.

   * In SISO mode you can select which channel to control (A or B).
   * Not all boards support MIMO mode. For example LimeSDR Mini only supports SISO mode, while LimeSDR USB and LimeSDR PCIe support both SISO and MIMO modes.

* **File**: This can be used to select an .ini file which was saved by using LimeSuiteGUI. When this feature is used, the same .ini file must be loaded in both the LimeSuite Sink (TX) and LimeSuite Source (RX) blocks for a device.

  * *The Channel selection should match between LimeSuite Sink (TX) and LimeSuite Source (RX) blocks for the same device.*

* **RF frequency (callback function value)**: Set the RF frequency for TX (both channels).
* **Sample rate**: Select sample rate for TX.

  * LimeSDR USB and LimeSDR PCIe sample rate must be no more than 61.44 MS/s.
  * LimeSDR Mini sample rate must be no more than 30.72 MS/s.
  * LimeNET Micro sample rate must be no more than 10 MS/s.
  * *The sample rate should match in the LimeSuite Sink (TX) and LimeSuite Source (RX) blocks for the same device.*

* **Oversample**: This enables TX signal sampling at a higher rate than would be required to just preserve the desired signal frequencies.

  * Possible values (multipliers): Default (i.e. 0), 1, 2, 4, 8, 16, 32.
  * Default value uses highest possible value based on other settings. 
  * Oversample is connected to the sample rate and there are limitations (see below). 
  * With LimeSDR Mini and LimeNET Micro the same oversampling must be used for TX and RX.

.. list-table:: RX oversampling LimeSDR Mini
    :header-rows: 1

    * - Sampling rate range (x)
      - Valid oversampling values
    * - (15.36 < x ≤ 30.72) MS/s
      - 1, 2, 4 
    * - (7.68 < x ≤ 15.36) MS/s
      - 1, 2, 4, 8
    * - (3.84 < x ≤ 7.86) MS/s
      - 1, 2, 4, 8, 16
    * - X ≤ 3.84 MS/s
      - 1, 2, 4, 8, 16, 32

.. list-table:: RX oversampling LimeSDR USB and LimeSDR PCIe 
    :header-rows: 1

    * - Sampling rate range (x)
      - Valid oversampling values
    * - (30.72 < x ≤ 61.44) MS/s
      - 1, 2
    * - (15.36 < x ≤ 30.72) MS/s
      - 1, 2, 4
    * - (7.68 < x ≤ 15.36) MS/s
      - 1, 2, 4, 8
    * - (3.84 < x ≤ 7.86) MS/s
      - 1, 2, 4, 8, 16
    * - X ≤ 3.84 MS/s
      - 1, 2, 4, 8, 16, 32

.. list-table:: RX oversampling LimeNET Micro
    :header-rows: 1

    * - Sampling rate range (x)
      - Valid oversampling values
    * - (7.68 < x ≤ 10) MS/s
      - 1, 2, 4, 8
    * - (3.84 < x ≤ 7.86) MS/s
      - 1, 2, 4, 8, 16
    * - X ≤ 3.84 MS/s
      - 1, 2, 4, 8, 16, 32

* **TCXO:DAC value**: This setting modifies the VCTXCO DAC parameter value used to calibrate the reference clock. To enable this parameter *"Allow TCXO DAC control"* in the *"Advanced"* tab must be set to *"Yes"*. Keep in mind that this parameter returns to its default value only after device power off. 

  * LimeSDR Mini default value is 180, range is [0,255].
  * LimeSDR USB default value is 125, range is [0,255].
  * LimeSDR PCIe default value is 134, range is [0,255].
  * LimeNET Micro default value is 30714, range is [0,65535].

* **NCO frequency (callback function value)**: Here you can adjust the LMS7002M numerically controlled oscillator. 0 disables the NCO.
* **Calibration bandw.:** This setting is used to set the TX calibration bandwidth of each channel. 

  * This value should be equal to your signal bandwidth. 
  * Calibration bandwidth range must be 2.5MHz–120MHz.
  * 0 disables calibration, but this is not recommended as it may lead to suboptimal performance.

* LNA path (callback function value): Select active low-noise amplifier path of each channel.

  * Auto(Default) for all boards.

    * LNA path is automatically selected for LimeSDR Mini and LimeNET Micro boards based on the RF frequency
    * Sets LNAH for LimeSDR-USB and LimeSDR-PCIe.

  * LimeSDR USB and LimeSDR PCIe possible selections: LNAH, LNAL, LNAW.
  * LimeSDR Mini possible selections: LNAH, LNAW.
  * LimeSDR Micro possible selections: LNAH, LNAL.
  * This setting also controls LimeSDR Mini and LimeNET Micro board path switch.

* **Analog filter bandw. (callback function value)**: Enter the RX analog filter bandwidth for each channel. 

  * RX analog filter bandwidth range must be 1.5–130MHz.
  * 0 disables the analog filter.

* **Digital filter bandw. (callback function value)**: Enterthe  RX digital filter bandwidth for each channel.

  * The RX digital filter bandwidth should not be higher than sampling rate.
  * 0 disables the digital filter..

* **Gain (callback function value)**: Controls RX channel gain.

  * The gain range must be 0–73dB.    

LimeSuite Source (RX) also transmits “rx_time” tag which tells when exactly samples have been received. It is pmt::cons pair which consist of two numbers: number of full seconds(int64) and number of fractional seconds(double). This tag is transmitted whenever the stream is started or interrupted. More about stream tags can be read here: `GNU Radio Stream Tags documentation`_.

LimeRFE Control
****************

.. figure:: /images/RFE_Control.png
   :align: center

Select the LimeRFE Control block from LimeSuite category as shown.

.. figure:: /images/RFE_block.png
   :align: center

The LimeRFE Control block appears in the GNU Radio workspace.

.. figure:: /images/RFE_Settings.png
   :align: center

Double-click on the block to configure the settings.

The following settings are available:

* **ID**: ID of the current block in the GNU Radio environment. This is usually left unchanged.
* **Communication**: Select how LimeRFE device will be connected, available options are:

  * **Direct USB**: LimeRFE is configured directly through USB COM port.
  * **SDR**: LimeRFE is configured through an attached LimeSDR GPIO port.

* **USB COM PORT**: This option appears when "Direct USB" communication is selected. Here you need to enter the LimeRFE device. On Linux you can find the COM port by using dmesg command as shown.

.. figure:: /images/RFE_dmesg_example.png
   :align: center

.. note::
   In the settings you need to enter "/dev/ttyUSB0" instead of just "ttyUSB0"

On Windows you have to find the COM port via the Device Manager as shown below.

.. figure:: /images/RFE_device_manager_example.png
   :align: center

* **SDR DEVICE SERIAL**: This option appears when "SDR" communication is selected. Here you can provide the serial number of the device which the LimeRFE is connected to. A list of devices can be obtained by running following command in a terminal window:

.. code-block:: bash

   LimeUtil --find

.. note::
   This option can be left blank and first device in the list will be used.  
   
.. warning::
   The LimeRFE must be connected to the GPIO port of the specified LimeSDR for this to work! 

* **Configuration File**: If set, the LimeRFE device will be configured using an .ini file.

.. warning::
   The .ini file must be generated using LimeSuite->Modules->LimeRFE->save. A general LimeSuite .ini file (as used to configure LMS7002M etc.) will not work!

* **Enable Fan (callback function value)**: Enable or disable a fan connected to the LimeRFE.
* **Mode (callback function value)**: With this option the LimeRFE device mode is set.

  * Possible selections: RX(0), TX(1), RX+TX(2), NONE(3)

* **RX Channel**: Select RX channel to be configured. 

  * When cellular bands are selected the same channel is set for TX.

* **RX Port**: Select the hardware port to be used for receive.
* **RX Attenuation (callback function value)**: Specifies the attenuation in the RX path. Attenuation [dB] = 2 * attenuation.
* **AM FM NOTCH FILTER (callback function value)**: Enables or disables AM FM notch filter
* **TX Channel**: Select TX channel to be configured. 

  * This option is hidden if cellular bands are selected for RX.

* **TX Port**: Select the hardware port to be used for transmit.

Documentation
*************

.. figure:: /images/Documentation_Tab_GnuRadio.png
   :align: center

Documentation can be accessed by double-clicking on a block to open its settings, then select the Documentation tab as shown. This applies to all GNU Radio blocks and is not limited to gr-limesdr.

.. _GNU Radio Stream Tags documentation: https://wiki.gnuradio.org/index.php/Stream_Tags

