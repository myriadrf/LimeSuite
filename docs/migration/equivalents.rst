LMS API and SDRDevice API function equivalents
==============================================

This section of the documentation provides the porting guide to change from the old LMS API to the new SDRDevice API.

..
  TODO: Correct this whenever code changes

.. list-table:: Table 1: LMS API and SDRDevice API function equivalency table.
   :header-rows: 1

   * - LMS API (old)
     - SDRDevice API (new)
   * - ``LMS_GetDeviceList()``
     - ``lime::DeviceRegistry::enumerate()``
   * - ``LMS_Open()``
     - ``lime::DeviceRegistry::makeDevice()``
   * - ``LMS_Close()``
     - ``lime::DeviceRegistry::freeDevice()``
   * - ``LMS_Init()``
     - ``lime::SDRDevice::Init()``
   * - ``LMS_GetNumChannels()``
     - ``lime::SDRDevice::RFSOCDescriptor::channelCount`` obtained from ``lime::SDRDevice::Descriptor::rfSOC`` obtained from ``lime::SDRDevice::GetDescriptor()``
   * - ``LMS_EnableChannel()``
     - + ``lime::SDRDevice::EnableChannel()``
       + ``lime::SDRDevice::ChannelConfig::Direction::enabled`` located at ``lime::SDRDevice::SDRConfig::channel``, which is later passed to ``lime::SDRDevice::Configure()``
   * - ``LMS_SetSampleRate()``
     - + ``lime::SDRDevice::SetSampleRate()``
       + ``lime::SDRDevice::ChannelConfig::Direction::sampleRate`` located at ``lime::SDRDevice::SDRConfig::channel``, which is later passed to ``lime::SDRDevice::Configure()``
   * - ``LMS_GetSampleRate()``
     - ``lime::SDRDevice::GetSampleRate()``
   * - ``LMS_GetSampleRateRange()``
     - ``lime::SDRDevice::RFSOCDescriptor::samplingRateRange`` obtained from ``lime::SDRDevice::Descriptor::rfSOC`` obtained from ``lime::SDRDevice::GetDescriptor()``
   * - ``LMS_SetLOFrequency()``
     - + ``lime::SDRDevice::SetFrequency()``
       + ``lime::SDRDevice::ChannelConfig::Direction::centerFrequency`` located at ``lime::SDRDevice::SDRConfig::channel``, which is later passed to ``lime::SDRDevice::Configure()``
   * - ``LMS_GetLOFrequency()``
     - ``lime::SDRDevice::GetFrequency()``
   * - ``LMS_GetLOFrequencyRange()``
     - ``lime::SDRDevice::RFSOCDescriptor::frequencyRange`` obtained from ``lime::SDRDevice::Descriptor::rfSOC`` obtained from ``lime::SDRDevice::GetDescriptor()``
   * - ``LMS_GetAntennaList()``
     - ``lime::SDRDevice::RFSOCDescriptor::pathNames`` obtained from ``lime::SDRDevice::Descriptor::rfSOC`` obtained from ``lime::SDRDevice::GetDescriptor()``
   * - ``LMS_SetAntenna()``
     - + ``lime::SDRDevice::SetAntenna()``
       + ``lime::SDRDevice::ChannelConfig::Direction::path`` located at ``lime::SDRDevice::SDRConfig::channel``, which is later passed to ``lime::SDRDevice::Configure()``
   * - ``LMS_GetAntenna()``
     - ``lime::SDRDevice::GetAntenna()``
   * - ``LMS_GetAntennaBW()``
     - ``lime::SDRDevice::RFSOCDescriptor::antennaRange`` obtained from ``lime::SDRDevice::Descriptor::rfSOC`` obtained from ``lime::SDRDevice::GetDescriptor()``
   * - ``LMS_SetNormalizedGain()``
     - Equivalent does not exist, look at ``LMS_SetGaindB()``.
   * - ``LMS_SetGaindB()``
     - + ``lime::SDRDevice::SetGain()``
       + ``lime::SDRDevice::ChannelConfig::Direction::gain`` located at ``lime::SDRDevice::SDRConfig::channel``, which is later passed to ``lime::SDRDevice::Configure()``
   * - ``LMS_GetNormalizedGain()``
     - Equivalent does not exist, look at ``LMS_GetGaindB()``.
   * - ``LMS_GetGaindB()``
     - ``lime::SDRDevice::GetGain()``
   * - ``LMS_SetLPFBW()``
     - + ``lime::SDRDevice::SetLowPassFilter()``
       + ``lime::SDRDevice::ChannelConfig::Direction::lpf`` located at ``lime::SDRDevice::SDRConfig::channel``, which is later passed to ``lime::SDRDevice::Configure()``
   * - ``LMS_GetLPFBW()``
     - ``lime::SDRDevice::GetLowPassFilter()``
   * - ``LMS_GetLPFBWRange()``
     - ``lime::SDRDevice::RFSOCDescriptor::lowPassFilterRange`` obtained from ``lime::SDRDevice::Descriptor::rfSOC`` obtained from ``lime::SDRDevice::GetDescriptor()``
   * - ``LMS_SetLPF()``
     - ``lime::SDRDevice::ChannelConfig::Direction::lpf`` located at ``lime::SDRDevice::SDRConfig::channel``, which is later passed to ``lime::SDRDevice::Configure()``
   * - ``LMS_SetGFIRLPF()``
     - ``lime::SDRDevice::ChannelConfig::Direction::gfir`` located at ``lime::SDRDevice::SDRConfig::channel``, which is later passed to ``lime::SDRDevice::Configure()``
   * - ``LMS_Calibrate()``
     - ``lime::SDRDevice::Calibrate()``
   * - ``LMS_LoadConfig()``
     - ``lime::SDRDevice::LoadConfig()``
   * - ``LMS_SaveConfig()``
     - ``lime::SDRDevice::SaveConfig()``
   * - ``LMS_SetTestSignal()``
     - + ``lime::SDRDevice::SetTestSignal()``
       + ``lime::SDRDevice::ChannelConfig::Direction::testSignal`` located at ``lime::SDRDevice::SDRConfig::channel``, which is later passed to ``lime::SDRDevice::Configure()``
   * - ``LMS_GetTestSignal()``
     - ``lime::SDRDevice::GetTestSignal()``
   * - ``LMS_GetChipTemperature()``
     - ``lime::SDRDevice::GetTemperature()``
   * - ``LMS_SetSampleRateDir()``
     - Loop for every channel and set ``lime::SDRDevice::ChannelConfig::Direction::sampleRate``
       located at ``lime::SDRDevice::SDRConfig::channel``, which is later passed to ``lime::SDRDevice::Configure()``
   * - ``LMS_SetNCOFrequency()``
     - + ``lime::SDRDevice::SetNCOFrequency()``
       + ``lime::SDRDevice::ChannelConfig::Direction::NCOoffset`` located at ``lime::SDRDevice::SDRConfig::channel``, which is later passed to ``lime::SDRDevice::Configure()``

         Important change if switching to the variable version: it is an offset from the center frequency, not the frequency of the NCO itself.
   * - ``LMS_GetNCOFrequency()``
     - ``lime::SDRDevice::GetNCOFrequency()``
   * - ``LMS_SetNCOPhase()``
     - No direct equivalent.
   * - ``LMS_GetNCOPhase()``
     - No direct equivalent.
   * - ``LMS_SetNCOIndex()``
     - No direct equivalent.
   * - ``LMS_GetNCOIndex()``
     - No direct equivalent.
   * - ``LMS_SetGFIRCoeff()``
     - ``lime::SDRDevice::SetGFIRCoefficients()``
   * - ``LMS_GetGFIRCoeff()``
     - ``lime::SDRDevice::GetGFIRCoefficients()``
   * - ``LMS_SetGFIR()``
     - ``lime::SDRDevice::SetGFIR()``
   * - ``LMS_EnableCache()``
     - ``lime::SDRDevice::EnableCache()``
   * - ``LMS_Reset()``
     - ``lime::SDRDevice::Reset()``
   * - ``LMS_ReadLMSReg()``
     - ``lime::SDRDevice::ReadRegister()``
   * - ``LMS_WriteLMSReg()``
     - ``lime::SDRDevice::WriteRegister()``
   * - ``LMS_ReadParam()``
     - ``lime::SDRDevice::GetParameter()``
   * - ``LMS_WriteParam()``
     - ``lime::SDRDevice::SetParameter()``
   * - ``LMS_ReadFPGAReg()``
     - ``lime::SDRDevice::ReadRegister()``
   * - ``LMS_WriteFPGAReg()``
     - ``lime::SDRDevice::WriteRegister()``
   * - ``LMS_ReadCustomBoardParam()``
     - ``lime::SDRDevice::CustomParameterRead()``
   * - ``LMS_WriteCustomBoardParam()``
     - ``lime::SDRDevice::CustomParameterWrite()``
   * - ``LMS_GetClockFreq()``
     - ``lime::SDRDevice::GetClockFreq()``
   * - ``LMS_SetClockFreq()``
     - ``lime::SDRDevice::SetClockFreq()``
   * - ``LMS_VCTCXOWrite()``
     - ``lime::SDRDevice::MemoryWrite()``
   * - ``LMS_VCTCXORead()``
     - ``lime::SDRDevice::MemoryRead()``
   * - ``LMS_Synchronize()``
     - ``lime::SDRDevice::Synchronize()``
   * - ``LMS_GPIORead()``
     - ``lime::SDRDevice::GPIORead()``
   * - ``LMS_GPIOWrite()``
     - ``lime::SDRDevice::GPIOWrite()``
   * - ``LMS_GPIODirRead()``
     - ``lime::SDRDevice::GPIODirRead()``
   * - ``LMS_GPIODirWrite()``
     - ``lime::SDRDevice::GPIODirWrite()``
   * - ``LMS_SetupStream()``
     - ``lime::SDRDevice::StreamSetup()``
   * - ``LMS_DestroyStream()``
     - Not needed anymore.
   * - ``LMS_StartStream()``
     - ``lime::SDRDevice::StreamStart()``
   * - ``LMS_StopStream()``
     - ``lime::SDRDevice::StreamStop()``
   * - ``LMS_RecvStream()``
     - ``lime::SDRDevice::StreamRx()``
   * - ``LMS_GetStreamStatus()``
     - ``lime::SDRDevice::StreamStatus()``
   * - ``LMS_SendStream()``
     - ``lime::SDRDevice::StreamTx()``
   * - ``LMS_UploadWFM()``
     - ``lime::SDRDevice::UploadTxWaveform()``
   * - ``LMS_EnableTxWFM()``
     - No direct equivalent.
   * - ``LMS_GetProgramModes()``
     - ``lime::SDRDevice::Descriptor::memoryDevices`` obtained from ``lime::SDRDevice::GetDescriptor()``
   * - ``LMS_Program()``
     - ``lime::SDRDevice::UploadMemory()``
   * - ``LMS_GetDeviceInfo()``
     - ``lime::SDRDevice::GetDescriptor()``
   * - ``LMS_GetLibraryVersion()``
     - ``lime::GetLibraryVersion()``
   * - ``LMS_GetLastErrorMessage()``
     - ``lime::GetLastErrorMessage()``
   * - ``LMS_RegisterLogHandler()``
     - ``lime::registerLogHandler()``
