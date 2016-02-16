#include "CalibrationCache.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <sys/stat.h>
using namespace std;

std::string CalibrationCache::cachePath = "";
static const char* limeSuiteDirName = ".limesuite";
static const char* cacheFilename = "LMS7002M_cache_values.txt";

int CalibrationCache::instanceCount = 0;
list<CalibrationCache::DCIQValues> CalibrationCache::dciq_cache = std::list<CalibrationCache::DCIQValues>();
list<CalibrationCache::VCOValues> CalibrationCache::vco_cache = std::list<CalibrationCache::VCOValues>();

CalibrationCache::CalibrationCache()
{
    if(instanceCount == 0)
    {
        std::string limeSuiteDir;
        std::string homeDir = getenv("HOME");
        //check if HOME variable is set
        if(homeDir.size() == 0)
        {
            printf("HOME variable is not set\n");
            homeDir = "/tmp"; //home not defined move to temp
        }
        limeSuiteDir = homeDir + "/" + limeSuiteDirName;
        //check if limesuite directory exists
        struct stat info;
        if( stat( limeSuiteDir.c_str(), &info ) != 0 )
        {
            printf("creating directory %s\n", limeSuiteDir.c_str());
            //create directory
            const int dir_err = mkdir(limeSuiteDir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
            if (-1 == dir_err)
                printf("Error creating directory %s\n", limeSuiteDir.c_str());
        }
        cachePath = limeSuiteDir+"/"+cacheFilename;
        printf("LMS7002M values cache at %s\n", cachePath.c_str());
    }
    LoadFromFile(cachePath.c_str());
    ++instanceCount;
}

CalibrationCache::~CalibrationCache()
{
    --instanceCount;
    if(instanceCount == 0)
        SaveToFile(cachePath.c_str());
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
