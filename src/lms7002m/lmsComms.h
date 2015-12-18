/**
@file   LMScomms.h
@author Lime Microsystems (www.limemicro.com)
@brief  Class for handling data transmission to LMS boards
*/

#ifndef LMS_COMMS_H
#define LMS_COMMS_H

#include "IConnection.h"

//! JB TODO, remove this compatibility layer later
//! A lot of code is written around LMScomms *
//! Replace with sed 's/LMScomms/IConnection/g'

class LMScomms : public IConnection
{
public:
    LMScomms(){}
};

#endif // LMS_COMMS_H

