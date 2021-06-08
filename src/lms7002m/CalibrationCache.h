#ifndef CALIBRATION_CACHE_H
#define CALIBRATION_CACHE_H

#include <stdint.h>
#include <list>
#include <sstream>
#include <sqlite3.h>
namespace lime
{

class CalibrationCache
{
public:
    CalibrationCache();
    ~CalibrationCache();

    int InsertVCO_CSW(uint32_t boardId, double frequency, uint8_t channel, bool transmitter, int vco, int csw);
    int GetVCO_CSW(uint32_t boardId, double frequency, uint8_t channel, bool transmitter, int *vco, int *csw);

    int InsertDC_IQ(uint32_t boardId, double frequency, uint8_t channel, bool transmitter, int band_lna, int dcI, int dcQ, int gainI, int gainQ, int phaseOffset);
    int GetDC_IQ(uint32_t boardId, double frequency, uint8_t channel, bool transmitter, int band_lna, int *dcI, int *dcQ, int *gainI, int *gainQ, int *phaseOffset);
    int GetDC_IQ_Interp(uint32_t boardId, double frequency, uint8_t channel, bool transmitter, int band_lna, int *dcI, int *dcQ, int *gainI, int *gainQ, int *phaseOffset);

    int InsertFilter_RC(uint32_t boardId, double bandwidth, uint8_t channel, bool transmitter, int filter_id, int rcal, int ccal, int cfb = 0);
    int GetFilter_RC(uint32_t boardId, double bandwidth, uint8_t channel, bool transmitter, int filter_id, int *rcal, int *ccal, int *cfb = nullptr);

protected:
    int initializeDatabase();

    static std::string cachePath;
    static int instanceCount;
    static sqlite3 *db;
};

}

#endif // CALIBRATION_CACHE_H
