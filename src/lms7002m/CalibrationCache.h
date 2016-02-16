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
    struct VCOValues
    {
        VCOValues() : tx(0), vco(0), csw(0){};
        int16_t tx;
        int64_t freq;
        int16_t vco;
        int16_t csw;
        std::string toString() const
        {
            std::stringstream ss;
            const char* spacer = "\t";
            ss << tx << spacer << freq << spacer << vco << spacer << csw;
            return ss.str();
        }
    };
    struct DCIQValues
    {
        DCIQValues() : tx(0), freq(0), dcI(0), dcQ(0), phaseCorr(0), gainI(0), gainQ(0){};
        int16_t tx;
        int64_t freq;
        int16_t dcI;
        int16_t dcQ;
        int16_t phaseCorr;
        int16_t gainI;
        int16_t gainQ;

        std::string toString() const
        {
            std::stringstream ss;
            const char* spacer = "\t";
            ss << tx << spacer << freq << spacer << dcI << spacer << dcQ
                    << spacer << phaseCorr << spacer << gainI << spacer << gainQ << spacer;
            return ss.str();
        }
    };

    CalibrationCache();
    ~CalibrationCache();

    int InsertVCO_CSW(uint32_t boardId, double frequency, uint8_t channel, bool transmitter, int vco, int csw);
    int GetVCO_CSW(uint32_t boardId, double frequency, uint8_t channel, bool transmitter, int *vco, int *csw);

    bool GetDCIQValues(bool tx, const int64_t freq, DCIQValues *dest);
    void SetDCIQValues(const DCIQValues &values);

    int SaveToFile(const char* filename);
    int LoadFromFile(const char* filename);

protected:
    int initializeDatabase();

    static std::string cachePath;
    static int instanceCount;
    static std::list<DCIQValues> dciq_cache;
    static sqlite3 *db;
};

}

#endif // CALIBRATION_CACHE_H
