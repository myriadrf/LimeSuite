#ifdef __unix__
#include <unistd.h>
#endif

#include "LitePCIe.h"
#include "LimeSDR_5GRadio.h"

#include <fstream>
#include <map>
#include <unistd.h>
#include <fcntl.h>

using namespace lime;

void __loadLimeSDR_5GRadio(void) //TODO fixme replace with LoadLibrary/dlopen
{
    static LimeSDR_5GRadioEntry limesdr_5GRadioSupport; // self register on initialization
}

LimeSDR_5GRadioEntry::LimeSDR_5GRadioEntry() : DeviceRegistryEntry("LimeSDR_5GRadio")
{
}

LimeSDR_5GRadioEntry::~LimeSDR_5GRadioEntry()
{
}

std::vector<DeviceHandle> LimeSDR_5GRadioEntry::enumerate(const DeviceHandle &hint)
{
    std::vector<DeviceHandle> handles;
    DeviceHandle handle;
    handle.media = "PCI-E";

    std::vector<std::string> LimePCIePorts;
    FILE *lsPipe;
    lsPipe = popen("ls /dev/Lime5GRadio*_control -d -1", "r");
    char tempBuffer[512];
    while(fscanf(lsPipe, "%s", tempBuffer) == 1)
        LimePCIePorts.push_back(tempBuffer);
    pclose(lsPipe);

    std::map<std::string, std::string> port_name_map = {
        {"Lime5GRadio", "LimeSDR-5GRadio"}
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

SDRDevice* LimeSDR_5GRadioEntry::make(const DeviceHandle &handle)
{
    LitePCIe* control = new LitePCIe();
    std::vector<LitePCIe*> txStreams(3);
    std::vector<LitePCIe*> rxStreams(3);
    try {
        std::string controlFile(handle.addr + "_control");
        control->Open(controlFile.c_str(), O_RDWR);

        std::string streamFile("");
        for (int i=0; i<3; ++i)
        {
            char portName[128];
            sprintf(portName, "%s_trx%i", handle.addr.c_str(), i);
            rxStreams[i] = new LitePCIe();
            rxStreams[i]->SetPathName(portName);

            sprintf(portName, "%s_write%i", handle.addr.c_str(), i);
            txStreams[i] = new LitePCIe();
            txStreams[i]->SetPathName(portName);
        }
        return new LimeSDR_5GRadio(control, std::move(rxStreams), std::move(txStreams));
    }
    catch ( std::runtime_error &e )
    {
        delete control;
        char reason[256];
        sprintf(reason, "Unable to connect to device using handle(%s): %s", handle.serialize().c_str(), e.what());
        throw std::runtime_error(reason);
    }
}
