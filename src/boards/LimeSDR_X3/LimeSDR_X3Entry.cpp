#ifdef __unix__
#include <unistd.h>
#endif

#include "LitePCIe.h"
#include "LimeSDR_X3.h"

#include <fstream>
#include <map>
#include <unistd.h>
#include <fcntl.h>

using namespace lime;

void __loadLimeSDR_X3(void) //TODO fixme replace with LoadLibrary/dlopen
{
    static LimeSDR_X3Entry limesdr_X3Support; // self register on initialization
}

LimeSDR_X3Entry::LimeSDR_X3Entry() : DeviceRegistryEntry("LimeSDR_X3")
{
}

LimeSDR_X3Entry::~LimeSDR_X3Entry()
{
}

std::vector<DeviceHandle> LimeSDR_X3Entry::enumerate(const DeviceHandle &hint)
{
    std::vector<DeviceHandle> handles;
    DeviceHandle handle;
    handle.media = "PCIe";

    if (!hint.media.empty() && hint.media != handle.media)
        return handles;

    const std::string searchDevName("LimeX3");
    const std::vector<std::string> boardNames = {
        GetDeviceName(LMS_DEV_LIMESDR_X3)
        , "LimeSDR-X3"
        , searchDevName
    };
    if (!hint.name.empty())
    {
        bool foundMatch = false;
        for (const std::string& name : boardNames)
        {
            if (name.find(hint.name) != std::string::npos)
            {
                foundMatch = true;
                break;
            }
        }
        if (!foundMatch)
            return handles;
    }

    const std::string pattern(searchDevName + "[0-9]*_control");
    const std::vector<std::string> devices = LitePCIe::GetDevicesWithPattern(pattern);

    for(const auto& devPath : devices)
    {
        size_t pos = devPath.find(searchDevName);
        if(pos == std::string::npos)
            continue;

        if (!hint.addr.empty() && devPath.find(hint.addr) == std::string::npos)
            continue;

        std::string dev_nr(&devPath[pos+searchDevName.length()], &devPath[devPath.find("_")]);
        handle.name = boardNames[0];// + (dev_nr == "0" ? "" : " (" + dev_nr + ")");

        handle.addr = devPath.substr(0, devPath.find("_"));
        handles.push_back(handle);
    }
    return handles;
}

class PCIE_CSR_Pipe : public ISerialPort
{
public:
    explicit PCIE_CSR_Pipe(LitePCIe& port) : port(port) {};
    virtual int Write(const uint8_t* data, size_t length, int timeout_ms) override
    {
        return port.WriteControl(data, length, timeout_ms);
    }
    virtual int Read(uint8_t* data, size_t length, int timeout_ms) override
    {
        return port.ReadControl(data, length, timeout_ms);
    }
protected:
    LitePCIe& port;
};

class LMS64C_LMS7002M_Over_PCIe : public lime::IComms
{
public:
    LMS64C_LMS7002M_Over_PCIe(LitePCIe* dataPort) : pipe(*dataPort) {}
    virtual void SPI(const uint32_t *MOSI, uint32_t *MISO, uint32_t count) override
    {
        LMS64CProtocol::LMS7002M_SPI(pipe, 0, MOSI, MISO, count);
        return;
    }
    virtual void SPI(uint32_t spiBusAddress, const uint32_t *MOSI, uint32_t *MISO, uint32_t count) override
    {
        LMS64CProtocol::LMS7002M_SPI(pipe, spiBusAddress, MOSI, MISO, count);
        return;
    }
    virtual int ResetDevice(int chipSelect) override
    {
        return LMS64CProtocol::DeviceReset(pipe, chipSelect);
    };
private:
    PCIE_CSR_Pipe pipe;
};

class LMS64C_FPGA_Over_PCIe : public lime::IComms
{
public:
    LMS64C_FPGA_Over_PCIe(LitePCIe* dataPort) : pipe(*dataPort) {}
    void SPI(const uint32_t *MOSI, uint32_t *MISO, uint32_t count) override
    {
        SPI(0, MOSI, MISO, count);
    }
    void SPI(uint32_t spiBusAddress, const uint32_t *MOSI, uint32_t *MISO, uint32_t count) override
    {
        LMS64CProtocol::FPGA_SPI(pipe, MOSI, MISO, count);
    }

    virtual int CustomParameterWrite(const int32_t *ids, const double *values, const size_t count, const std::string& units) override
    {
        return LMS64CProtocol::CustomParameterWrite(pipe, ids, values, count, units);
    };
    virtual int CustomParameterRead(const int32_t *ids, double *values, const size_t count, std::string* units) override
    {
        return LMS64CProtocol::CustomParameterRead(pipe, ids, values, count, units);
    }
    virtual int ProgramWrite(const char* data, size_t length, int prog_mode, int target, ProgressCallback callback = nullptr) override
    {
        return LMS64CProtocol::ProgramWrite(pipe, data, length, prog_mode, (LMS64CProtocol::ProgramWriteTarget)target, callback);
    }
private:
    PCIE_CSR_Pipe pipe;
};

SDRDevice* LimeSDR_X3Entry::make(const DeviceHandle &handle)
{
    // Data transmission layer
    LitePCIe* control = new LitePCIe();
    std::vector<LitePCIe*> trxStreams(3);

    // protocol layer
    IComms* route_lms7002m = new LMS64C_LMS7002M_Over_PCIe(control);
    IComms* route_fpga = new LMS64C_FPGA_Over_PCIe(control);

    try {
        std::string controlFile(handle.addr + "_control");
        control->Open(controlFile.c_str(), O_RDWR);

        std::string streamFile("");
        for (int i=0; i<3; ++i)
        {
            char portName[128];
            sprintf(portName, "%s_trx%i", handle.addr.c_str(), i);
            trxStreams[i] = new LitePCIe();
            trxStreams[i]->SetPathName(portName);
        }
        return new LimeSDR_X3(route_lms7002m, route_fpga, std::move(trxStreams));
    }
    catch ( std::runtime_error &e )
    {
        delete control;
        char reason[256];
        sprintf(reason, "Unable to connect to device using handle(%s): %s", handle.Serialize().c_str(), e.what());
        throw std::runtime_error(reason);
    }
}
