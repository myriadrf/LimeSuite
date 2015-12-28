/**
@file   LMScomms.h
@author Lime Microsystems (www.limemicro.com)
@brief  Class for handling data transmission to LMS boards
*/

#ifndef LMS_COMMS_H
#define LMS_COMMS_H

#include "IConnection.h"
#include "LMS64CProtocol.h"

//! JB TODO, remove this compatibility layer later
//! A lot of code is written around LMScomms *
//! Replace with sed 's/LMScomms/IConnection/g'

/*!
 * Currently this calls replaces the wrapping done by connection manager.
 * Its also the interface used by the wx gui application for now.
 */
class LMScomms : public LMS64CProtocol
{
public:
    LMScomms(void):
        _actual(nullptr)
    {
        return;
    }

    //! Set the underlying connection
    void setConnection(IConnection *conn)
    {
        _actual = dynamic_cast<LMS64CProtocol *>(conn);
    }

    /*******************************************************************
     * Forwarding calls that only work when an active connection is set
     ******************************************************************/

    eConnectionType GetType(void)
    {
        return (_actual == nullptr)? CONNECTION_UNDEFINED : _actual->GetType();
    }

    bool IsOpen(void)
    {
        return (_actual == nullptr)? false : _actual->IsOpen();
    }

    int Write(const unsigned char *buffer, int length, int timeout_ms = 0)
    {
        if (_actual == nullptr) return -1;
        return _actual->Write(buffer, length, timeout_ms);
    }

    int Read(unsigned char *buffer, int length, int timeout_ms = 0)
    {
        if (_actual == nullptr) return -1;
        return _actual->Read(buffer, length, timeout_ms);
    }

private:
    LMS64CProtocol *_actual;
};

#endif // LMS_COMMS_H

