#ifdef __unix__
#include <unistd.h>
#endif

#include "LitePCIe.h"
#include "LimeSDR_XTRX.h"

#include <fstream>
#include <map>
#include <unistd.h>
#include <fcntl.h>
#include "LMSBoards.h"

using namespace lime;

void __loadLimeSDR_XTRX(void) //TODO fixme replace with LoadLibrary/dlopen
{
    static LimeSDR_XTRXEntry limesdr_XTRXSupport; // self register on initialization
}

LimeSDR_XTRXEntry::LimeSDR_XTRXEntry() : DeviceRegistryEntry("LimeSDR_XTRX")
{
}

LimeSDR_XTRXEntry::~LimeSDR_XTRXEntry()
{
}

std::vector<DeviceHandle> LimeSDR_XTRXEntry::enumerate(const DeviceHandle &hint)
{
    std::vector<DeviceHandle> handles;
    DeviceHandle handle;
    handle.media = "PCI-E";

    std::vector<std::string> LimePCIePorts;
    FILE *lsPipe;
    lsPipe = popen("ls /dev/LimeXTRX*_control -d -1", "r");
    char tempBuffer[512];
    while(fscanf(lsPipe, "%s", tempBuffer) == 1)
        LimePCIePorts.push_back(tempBuffer);
    pclose(lsPipe);

    std::map<std::string, std::string> port_name_map = {
        {"LimeXTRX", GetDeviceName(LMS_DEV_LIMESDR_XTRX)}
    };

    for(auto port:LimePCIePorts)
    {
        if(access(port.c_str(), F_OK ) != -1 )
        {
            for (auto port_name:port_name_map)
            {
                size_t pos = port.find(port_name.first);
                if(pos != std::string::npos)
                {
                    std::string dev_nr(&port[pos+port_name.first.length()], &port[port.find("_")]);
                    handle.name = port_name.second + (dev_nr == "0" ? "" : " (" + dev_nr + ")");
                }
            }
            //handle.index++;
            handle.addr = port.substr(0, port.find("_"));
            handles.push_back(handle);
        }
    }
    return handles;
}

SDRDevice* LimeSDR_XTRXEntry::make(const DeviceHandle &handle)
{
    LitePCIe* control = new LitePCIe();
    LitePCIe* stream = new LitePCIe();
    try {
        std::string controlFile(handle.addr + "_control");
        control->Open(controlFile.c_str(), O_RDWR);

        std::string streamFile(handle.addr + "_trx0");
        stream->SetPathName(streamFile.c_str());

        return new LimeSDR_XTRX(control, stream);
    }
    catch ( std::runtime_error &e )
    {
        delete control;
        delete stream;
        char reason[256];
        sprintf(reason, "Unable to connect to device using handle(%s)", handle.serialize().c_str());
        throw std::runtime_error(reason);
    }
}
