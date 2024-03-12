API wrapper notes
=================

This is the table for all the known/expected changes to the legacy API.

..
  TODO: Correct this whenever code changes

.. list-table:: Table 1: LMS API functionality changes
   :header-rows: 1

   * - LMS API function
     - Changes
   * - ``LMS_GetDeviceList()``
     - No functional difference.
   * - ``LMS_Open()``
     - No functional difference.
   * - ``LMS_Close()``
     - No functional difference.
   * - ``LMS_Init()``
     - No functional difference.
   * - ``LMS_GetNumChannels()``
     - No functional difference.
   * - ``LMS_EnableChannel()``
     - No functional difference.
   * - ``LMS_SetSampleRate()``
     - No functional difference.
   * - ``LMS_GetSampleRate()``
     - ``rf_Hz`` will always be the same as ``host_Hz`` (i.e. :math:`2^{ratio}` times smaller compared to the old API).
   * - ``LMS_GetSampleRateRange()``
     - No functional difference.
   * - ``LMS_SetLOFrequency()``
     - No functional difference once the bugs are fixed.
   * - ``LMS_GetLOFrequency()``
     - The NCO offset is currently not applied.
   * - ``LMS_GetLOFrequencyRange()``
     - No functional difference.
   * - ``LMS_GetAntennaList()``
     - No functional difference.
   * - ``LMS_SetAntenna()``
     - No functional difference.
   * - ``LMS_GetAntenna()``
     - No functional difference.
   * - ``LMS_GetAntennaBW()``
     - No functional difference.
   * - ``LMS_SetNormalizedGain()``
     - No functional difference.
   * - ``LMS_SetGaindB()``
     - No functional difference.
   * - ``LMS_GetNormalizedGain()``
     - No functional difference.
   * - ``LMS_GetGaindB()``
     - No functional difference.
   * - ``LMS_SetLPFBW()``
     - No functional difference.
   * - ``LMS_GetLPFBW()``
     - No functional difference.
   * - ``LMS_GetLPFBWRange()``
     - No functional difference.
   * - ``LMS_SetLPF()``
     - Currently broken, just sets the last saved LPF value for the direction channel combo.
   * - ``LMS_SetGFIRLPF()``
     - No functional difference.
   * - ``LMS_Calibrate()``
     - No functional difference.
   * - ``LMS_LoadConfig()``
     - No functional difference.
   * - ``LMS_SaveConfig()``
     - No functional difference.
   * - ``LMS_SetTestSignal()``
     - No functional difference.
   * - ``LMS_GetTestSignal()``
     - No functional difference.
   * - ``LMS_GetChipTemperature()``
     - No functional difference.
   * - ``LMS_SetSampleRateDir()``
     - Sets the sample rate for the whole device anyway, regardless of the direction used.
   * - ``LMS_SetNCOFrequency()``
     - No functional difference.
   * - ``LMS_GetNCOFrequency()``
     - Negative values can be returned as well, use ``std::fabs()`` to fix that.
   * - ``LMS_SetNCOPhase()``
     - No functional difference.
   * - ``LMS_GetNCOPhase()``
     - No functional difference.
   * - ``LMS_SetNCOIndex()``
     - No functional difference.
   * - ``LMS_GetNCOIndex()``
     - No functional difference.
   * - ``LMS_SetGFIRCoeff()``
     - No functional difference.
   * - ``LMS_GetGFIRCoeff()``
     - No functional difference.
   * - ``LMS_SetGFIR()``
     - No functional difference.
   * - ``LMS_EnableCache()``
     - No functional difference.
   * - ``LMS_Reset()``
     - No functional difference.
   * - ``LMS_ReadLMSReg()``
     - No functional difference.
   * - ``LMS_WriteLMSReg()``
     - No functional difference.
   * - ``LMS_ReadParam()``
     - No functional difference.
   * - ``LMS_WriteParam()``
     - No functional difference.
   * - ``LMS_ReadFPGAReg()``
     - No functional difference.
   * - ``LMS_WriteFPGAReg()``
     - No functional difference.
   * - ``LMS_ReadCustomBoardParam()``
     - No functional difference.
   * - ``LMS_WriteCustomBoardParam()``
     - No functional difference.
   * - ``LMS_GetClockFreq()``
     - No functional difference.
   * - ``LMS_SetClockFreq()``
     - No functional difference.
   * - ``LMS_VCTCXOWrite()``
     - No functional difference.
   * - ``LMS_VCTCXORead()``
     - No functional difference.
   * - ``LMS_Synchronize()``
     - No functional difference.
   * - ``LMS_GPIORead()``
     - No functional difference.
   * - ``LMS_GPIOWrite()``
     - No functional difference.
   * - ``LMS_GPIODirRead()``
     - No functional difference.
   * - ``LMS_GPIODirWrite()``
     - No functional difference.
   * - ``LMS_SetupStream()``
     - Only one call is now needed to set up all the streams on all the channels on a given device now,
       multiple calls are still fine, albeit wasteful.
   * - ``LMS_DestroyStream()``
     - Does not stop the stream anymore, a call to ``LMS_StopStream()`` before hand is necessary.
   * - ``LMS_StartStream()``
     - Starts all the set up streams at the same time, multiple calls are allowed for legacy reasons only.
   * - ``LMS_StopStream()``
     - Stops all the set up streams at the same time, multiple calls are allowed for legacy reasons only.
   * - ``LMS_RecvStream()``
     - + SISO mode - no functional difference.
       + MIMO mode - on the first call data for both channels is received and cached,
         on the second call (to the other channel) it returns the previously cached samples.
   * - ``LMS_GetStreamStatus()``
     - No functional difference.
   * - ``LMS_SendStream()``
     - + SISO mode - no functional difference.
       + MIMO mode - on the first call data is cached and return value is returned as if all the samples were sent,
         on the second call (to the other channel) the samples are actually sent to the device and the correct sent sample count is being returned.
   * - ``LMS_UploadWFM()``
     - No functional difference.
   * - ``LMS_EnableTxWFM()``
     - No functional difference.
   * - ``LMS_GetProgramModes()``
     - No functional difference.
   * - ``LMS_Program()``
     - No functional difference.
   * - ``LMS_GetDeviceInfo()``
     - No functional difference.
   * - ``LMS_GetLibraryVersion()``
     - No functional difference.
   * - ``LMS_GetLastErrorMessage()``
     - No functional difference.
   * - ``LMS_RegisterLogHandler()``
     - No functional difference.
