#include "CalibrationCache.h"
#include <fstream>
#include <sstream>
#include <iostream>
using namespace std;

#define CACHE_LOCATION "/tmp/LMS7002M_cache_values.txt"

int CalibrationCache::instanceCount = 0;
list<CalibrationCache::DCIQValues> CalibrationCache::dciq_cache = std::list<CalibrationCache::DCIQValues>();
list<CalibrationCache::VCOValues> CalibrationCache::vco_cache = std::list<CalibrationCache::VCOValues>();

CalibrationCache::CalibrationCache()
{
    printf("LMS7002M values cache at %s\n", CACHE_LOCATION);
    if(instanceCount == 0)
        LoadFromFile(CACHE_LOCATION);
    ++instanceCount;
}

CalibrationCache::~CalibrationCache()
{
    --instanceCount;
    if(instanceCount == 0)
        SaveToFile(CACHE_LOCATION);
}

bool CalibrationCache::GetDCIQValues(bool tx, const int64_t freq, DCIQValues* dest)
{
    for(auto i : dciq_cache)
    {
        if(freq == i.freq && tx == i.tx)
        {
            if(dest)
                *dest = i;
            return true;
        }
    }
    return false;
}

void CalibrationCache::SetDCIQValues(const DCIQValues &values)
{
    for(auto i : dciq_cache)
    {
        if(values.freq == i.freq && values.tx == i.tx)
        {
            i = values;
            return;
        }
    }
    dciq_cache.push_back(values);
}

bool CalibrationCache::GetVCOValues(bool tx, const int64_t freq, VCOValues* dest)
{
    for(auto i : vco_cache)
    {
        if(freq == i.freq && tx == i.tx)
        {
            if(dest)
                *dest = i;
            return true;
        }
    }
    return false;
}

void CalibrationCache::SetVCOValues(const VCOValues &values)
{
    for(auto i : vco_cache)
    {
        if(values.freq == i.freq && values.tx == i.tx)
        {
            i = values;
            return;
        }
    }
    vco_cache.push_back(values);
}

int CalibrationCache::SaveToFile(const char* filename)
{
    ofstream fout;
    fout.open(filename);
    const char* spacer = "\t";
    fout << "[vco]" << endl;
    for(auto i : vco_cache)
    {
        fout << i.toString() << endl;
    }
    fout << "[dciq]" << endl;
    for(auto i : dciq_cache)
    {
        fout << i.toString() << endl;
    }
    fout.close();
}

/** @brief loads calibration values from file
    @return number of entries loaded
*/
int CalibrationCache::LoadFromFile(const char* filename)
{
    dciq_cache.clear();
    ifstream fin;
    fin.open(filename);
    std::string line;
    if(line[0] != '[')
        getline(fin,line);
    if(line == "[vco]")
    {
        vco_cache.clear();
        while(getline( fin, line ))
        {
            if(line[0] == '[')
                break;
            stringstream ss;
            ss.str(line);
            VCOValues v;
            ss >> v.tx;
            ss >> v.freq;
            ss >> v.vco;
            ss >> v.csw;
            vco_cache.push_back(v);
        }
    }
    if(line[0] != '[')
        getline(fin,line);
    if(line == "[dciq]")
    {
        dciq_cache.clear();
        while(getline( fin, line ))
        {
            if(line[0] == '[')
                break;
            stringstream ss;
            ss.str(line);
            DCIQValues v;
            ss >> v.tx;
            ss >> v.freq;
            ss >> v.dcI;
            ss >> v.dcQ;
            ss >> v.phaseCorr;
            ss >> v.gainI;
            ss >> v.gainQ;
            dciq_cache.push_back(v);
        }
    }
    fin.close();
    return dciq_cache.size()+vco_cache.size();
}
