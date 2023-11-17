#pragma once

#include <chrono>
#include <string>
#include <vector>
#include <stdint.h>
#include <iostream>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <signal.h>
#include <getopt.h>

#include "limesuite/DeviceHandle.h"
#include "limesuite/DeviceRegistry.h"
#include "limesuite/SDRDevice.h"

lime::SDRDevice* ConnectUsingNameHint(const std::string& name);
