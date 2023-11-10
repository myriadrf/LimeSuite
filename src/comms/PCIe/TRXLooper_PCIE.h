#ifndef TRXLooper_PCIE_H
#define TRXLooper_PCIE_H

#include "dataTypes.h"
#include <vector>

#include "TRXLooper.h"
#include "MemoryPool.h"
#include "limesuite/SDRDevice.h"

namespace lime {

class LitePCIe;

class TRXLooper_PCIE : public lime::TRXLooper
{
  public:
    TRXLooper_PCIE(
        std::shared_ptr<LitePCIe> rxPort, std::shared_ptr<LitePCIe> txPort, FPGA* f, LMS7002M* chip, uint8_t moduleIndex);
    virtual ~TRXLooper_PCIE();
    virtual void Setup(const SDRDevice::StreamConfig& config);
    virtual void Start();

    static int UploadTxWaveform(FPGA* fpga,
        std::shared_ptr<LitePCIe> port,
        const SDRDevice::StreamConfig& config,
        uint8_t moduleIndex,
        const void** samples,
        uint32_t count);

    typedef SamplesPacket<2> SamplesPacketType;

    struct TransferArgs {
        std::shared_ptr<LitePCIe> port;
        std::vector<uint8_t*> buffers;
        int32_t bufferSize;
        int16_t packetSize;
        uint8_t packetsToBatch;
        int32_t samplesInPacket;
        int64_t cnt;
        int64_t sw;
        int64_t hw;
    };

  protected:
    virtual int RxSetup() override;
    virtual void ReceivePacketsLoop() override;
    virtual void RxTeardown() override;

    virtual int TxSetup() override;
    virtual void TransmitPacketsLoop() override;
    virtual void TxTeardown() override;

    TransferArgs mRxArgs;
    TransferArgs mTxArgs;
};

} // namespace lime

#endif // TRXLooper_PCIE_H
