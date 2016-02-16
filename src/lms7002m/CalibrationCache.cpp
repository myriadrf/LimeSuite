#include "CalibrationCache.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <sys/stat.h>
#include <vector>
#include <sstream>
using namespace std;
using namespace lime;

std::string CalibrationCache::cachePath = "";
static const char* limeSuiteDirName = ".limesuite";
static const char* cacheFilename = "LMS7002M_cache_values.db";

int CalibrationCache::instanceCount = 0;
list<CalibrationCache::DCIQValues> CalibrationCache::dciq_cache = std::list<CalibrationCache::DCIQValues>();
sqlite3* CalibrationCache::db = nullptr;

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

        if(!ifstream(cachePath.c_str()))
        {
            initializeDatabase();
        }
        printf("LMS7002M values cache at %s\n", cachePath.c_str());
    }
    int rc = sqlite3_open(cachePath.c_str(), &db);
    if( rc )
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
    }
    ++instanceCount;
}

CalibrationCache::~CalibrationCache()
{
    --instanceCount;
    if(instanceCount == 0)
        sqlite3_close(db);
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

/** @brief Creates database tables
*/
int CalibrationCache::initializeDatabase()
{
    int rc = sqlite3_open(cachePath.c_str(), &db);
    if( rc )
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }

    vector<string> cmd;
    //create table
    cmd.push_back(
"CREATE TABLE LMS7002M_VCO(\
    boardID INTEGER,\
    frequency INTEGER,\
    channel INTEGER,\
    transmitter BOOLEAN,\
    VCO INTEGER,\
    CSW INTEGER,\
    PRIMARY KEY (boardID, frequency, channel, transmitter));"
    );

    char *zErrMsg = 0;
    for(auto command : cmd)
    {
        rc = sqlite3_exec(db, command.c_str(), nullptr, 0, &zErrMsg);
        if( rc != SQLITE_OK )
        {
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
            break;
        }
    }
    sqlite3_close(db);
    return 0;
}

int CalibrationCache::InsertVCO_CSW(uint32_t boardId, double frequency, uint8_t channel, bool transmitter, int vco, int csw)
{
    char* zErrMsg = 0;
    stringstream query;
    query <<
"INSERT OR REPLACE INTO LMS7002M_VCO (boardID, frequency, channel, transmitter, vco, csw) " <<
"VALUES ( " << boardId << "," << frequency << "," << (int)channel << "," << (transmitter?1:0) << "," <<vco<<","<<csw<<");";

    int rc = sqlite3_exec(db, query.str().c_str(), nullptr, 0, &zErrMsg);
    if( rc != SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        return -1;
    }
    return 0;
}

int CalibrationCache::GetVCO_CSW(uint32_t boardId, double frequency, uint8_t channel, bool transmitter, int *vco, int *csw)
{
    struct QueryVCO_CSW
    {
        QueryVCO_CSW() : vco(0), csw(0), found(false){};
        int vco;
        int csw;
        bool found;
    };

auto lambda_callback = [](void *vco_csw_pair, int argc, char **argv, char **azColName)
{
    printf("callback active\n");
    QueryVCO_CSW *vco_csw = (QueryVCO_CSW*)vco_csw_pair;
    if(vco_csw != nullptr)
    {
        vco_csw->vco = argv[0] != nullptr ? std::stoi(argv[0]) : 0;
        vco_csw->csw = argv[1] != nullptr ? std::stoi(argv[1]) : 128;
        vco_csw->found = true;
        printf("Found\n");
        return 0;
    }
    return 1;
};



    QueryVCO_CSW vco_csw_pair;

    char* zErrMsg = 0;
    stringstream query;
    query << "SELECT vco, csw FROM LMS7002M_VCO where "<<
"boardID="<<boardId<<
" AND frequency="<<frequency<<
" AND channel="<<(int)channel<<
" AND transmitter="<<(transmitter?1:0)<<";";

    int rc = sqlite3_exec(db, query.str().c_str(), lambda_callback, &vco_csw_pair, &zErrMsg);
    if( rc != SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        return -1;
    }
    if(not vco_csw_pair.found)
        return -1;
    if(vco)
        *vco = vco_csw_pair.vco;
    if(csw)
        *csw = vco_csw_pair.csw;
    return 0;
}
