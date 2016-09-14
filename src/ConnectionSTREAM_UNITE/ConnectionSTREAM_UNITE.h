/**
    @file ConnectionSTREAM_UNITE.h
    @author Lime Microsystems
    @brief Implementation of STREAM+UNITE board combination connection.
*/

#pragma once
#include <ConnectionRegistry.h>
#include <ConnectionHandle.h>
#include <ConnectionEVB7COM/ConnectionEVB7COM.h>
#include <ConnectionSTREAM/ConnectionSTREAM.h>

namespace lime
{
class ConnectionSTREAM_UNITE : public virtual ConnectionSTREAM
{
public:
    ConnectionSTREAM_UNITE(void* ctx, const unsigned index, const int vid=-1, const int pid=-1, const char* comPortName=nullptr);
    virtual ~ConnectionSTREAM_UNITE(void);

    virtual DeviceInfo GetDeviceInfo(void);
    virtual int TransactSPI(const int addr, const uint32_t *writeData, uint32_t *readData, const size_t size);
    virtual int DeviceReset(void);

protected:
    ConnectionEVB7COM *comPort;
};

class ConnectionSTREAM_UNITEEntry : public virtual ConnectionSTREAMEntry
{
public:
    ConnectionSTREAM_UNITEEntry(void);
    ~ConnectionSTREAM_UNITEEntry(void);
    std::vector<ConnectionHandle> enumerate(const ConnectionHandle& hint);
    IConnection* make(const ConnectionHandle& handle);
};

}
