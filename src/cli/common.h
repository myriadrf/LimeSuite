#include <VersionInfo.h>
#include <SystemResources.h>
#include "lime/LimeSuite.h"
#include "limesuite/DeviceHandle.h"
#include "limesuite/SDRDevice.h"
#include <iostream>
#include <cstdlib>
#include <ciso646>
#include <getopt.h>
#include <string>
#include <cstdlib>
#include <fstream>
#include "Logger.h"
#include "LMS64CProtocol.h"
#include "limesuite/DeviceRegistry.h"
#include "limesuite/LMS7002M.h"
#include <vector>
#include <stdint.h>
#include <iostream>
#include <iomanip>

void processRaw (std::string rawdata, uint32_t devAddr, bool debug);

