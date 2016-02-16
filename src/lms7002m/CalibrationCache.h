#ifndef CALIBRATION_CACHE_H
#define CALIBRATION_CACHE_H

#include <stdint.h>
#include <list>
#include <sstream>
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

    bool GetDCIQValues(bool tx, const int64_t freq, DCIQValues *dest);
    void SetDCIQValues(const DCIQValues &values);

    bool GetVCOValues(bool tx, const int64_t freq, VCOValues *dest);
    void SetVCOValues(const VCOValues &values);

    int SaveToFile(const char* filename);
    int LoadFromFile(const char* filename);

protected:
    static std::string cachePath;
    static int instanceCount;
    static std::list<DCIQValues> dciq_cache;
    static std::list<VCOValues> vco_cache;
};

#endif // CALIBRATION_CACHE_H
