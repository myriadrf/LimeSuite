#include "CalibrationCache.h"
#include "ErrorReporting.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <sys/stat.h>
#include <vector>
#include <sstream>
#include <ciso646>
#include <cmath>
#ifndef __unix__
    #include <Windows.h>
    #include <Shlobj.h>
#endif
using namespace std;
using namespace lime;

std::string CalibrationCache::cachePath = "";
static const char* limeSuiteDirName = ".limesuite";
static const char* cacheFilename = "LMS7002M_cache_values.db";

int CalibrationCache::instanceCount = 0;
sqlite3* CalibrationCache::db = nullptr;

static inline double linearInterp(double x, double x0, double y0, double x1, double y1)
{
    double a = (x - x0)/(x1 - x0);
    return y0 + (y1 - y0)*a;
}

CalibrationCache::CalibrationCache()
{
    if(instanceCount == 0)
    {
        std::string limeSuiteDir;
        std::string homeDir;
#ifdef __unix__
        homeDir = getenv("HOME");
        //check if HOME variable is set
        if (homeDir.size() == 0)
        {
            printf("HOME variable is not set\n");
            homeDir = "/tmp"; //home not defined move to temp
        }
#else
        CHAR path[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_PROFILE, NULL, 0, path)) != S_OK)
        {
            printf("HOME variable is not set\n");
            GetTempPathA(MAX_PATH, path); //home not defined move to temp
        }
        homeDir = path;
#endif
        limeSuiteDir = homeDir + "/" + limeSuiteDirName;
        //check if limesuite directory exists
        struct stat info;
        if( stat( limeSuiteDir.c_str(), &info ) != 0 )
        {
            printf("creating directory %s\n", limeSuiteDir.c_str());
            //create directory
#ifdef __unix__
            const int dir_err = mkdir(limeSuiteDir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
            if (-1 == dir_err)
                printf("Error creating directory %s\n", limeSuiteDir.c_str());
#else
            CreateDirectoryA(limeSuiteDir.c_str(), NULL);
#endif
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

cmd.push_back(
"CREATE TABLE LMS7002M_DC_IQ(\
    boardID INTEGER,\
    frequency INTEGER,\
    channel INTEGER,\
    transmitter BOOLEAN,\
    band_lna INTEGER,\
    dcI INTEGER,\
    dcQ INTEGER,\
    gainI INTEGER,\
    gainQ INTEGER,\
    phaseOffset INTEGER,\
    PRIMARY KEY (boardID, frequency, channel, transmitter, band_lna));"
    );

cmd.push_back(
"CREATE TABLE LMS7002M_FILTER_RC(\
    boardID INTEGER,\
    bandwidth INTEGER,\
    channel INTEGER,\
    transmitter BOOLEAN,\
    filter_id INTEGER,\
    rcal INTEGER,\
    ccal INTEGER,\
    cfb INTEGER,\
    PRIMARY KEY (boardID, bandwidth, channel, transmitter, filter_id));"
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
"VALUES ( " << boardId << "," << std::llrint(frequency) << "," << (int)channel << "," << (transmitter?1:0) << "," <<vco<<","<<csw<<");";

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
    QueryVCO_CSW *vco_csw = (QueryVCO_CSW*)vco_csw_pair;
    if(vco_csw != nullptr)
    {
        vco_csw->vco = argv[0] != nullptr ? std::stoi(argv[0]) : 0;
        vco_csw->csw = argv[1] != nullptr ? std::stoi(argv[1]) : 128;
        vco_csw->found = true;
        return 0;
    }
    return 1;
};

    QueryVCO_CSW vco_csw_pair;

    char* zErrMsg = 0;
    stringstream query;
    query << "SELECT vco, csw FROM LMS7002M_VCO where "<<
"boardID="<<boardId<<
" AND frequency="<<std::llrint(frequency)<<
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

int CalibrationCache::InsertDC_IQ(uint32_t boardId, double frequency, uint8_t channel, bool transmitter, int band_lna, int dcI, int dcQ, int gainI, int gainQ, int phaseOffset)
{
    char* zErrMsg = 0;
    stringstream query;
    query <<
"INSERT OR REPLACE INTO LMS7002M_DC_IQ (boardID, frequency, channel, transmitter, band_lna, dcI, dcQ, gainI, gainQ, phaseOffset) " <<
"VALUES ( " << boardId << "," << std::llrint(frequency) << "," << (int)channel << "," << (transmitter?1:0) << "," << band_lna << ", " <<
dcI<<","<<dcQ<<","<<gainI<<","<<gainQ<<","<<phaseOffset<<");";

    int rc = sqlite3_exec(db, query.str().c_str(), nullptr, 0, &zErrMsg);
    if( rc != SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        return -1;
    }
    return 0;
}

int CalibrationCache::GetDC_IQ(uint32_t boardId, double frequency, uint8_t channel, bool transmitter, int band_lna, int *dcI, int *dcQ, int *gainI, int *gainQ, int *phaseOffset)
{
    struct QueryDC_IQ
    {
        QueryDC_IQ() : dcI(0), dcQ(0), gainI(0), gainQ(0), phaseOffset(0), found(false){};
        int dcI;
        int dcQ;
        int gainI;
        int gainQ;
        int phaseOffset;
        bool found;
    };

    auto lambda_callback = [](void *dc_iq_data, int argc, char **argv, char **azColName)
    {
        QueryDC_IQ *data = (QueryDC_IQ*)dc_iq_data;
        if(data != nullptr)
        {
            data->dcI = argv[0] != nullptr ? std::stoi(argv[0]) : 0;
            data->dcQ = argv[1] != nullptr ? std::stoi(argv[1]) : 0;
            data->gainI = argv[2] != nullptr ? std::stoi(argv[2]) : 0;
            data->gainQ = argv[3] != nullptr ? std::stoi(argv[3]) : 0;
            data->phaseOffset = argv[4] != nullptr ? std::stoi(argv[4]) : 0;
            data->found = true;
            return 0;
        }
        return 1;
    };

    QueryDC_IQ queryResults;

    char* zErrMsg = 0;
    stringstream query;
    query << "SELECT dcI, dcQ, gainI, gainQ, phaseOffset FROM LMS7002M_DC_IQ where "<<
"boardID="<<boardId<<
" AND frequency="<<std::llrint(frequency)<<
" AND channel="<<(int)channel<<
" AND transmitter="<<(transmitter?1:0)<<
" AND band_lna="<<band_lna<<
";";

    int rc = sqlite3_exec(db, query.str().c_str(), lambda_callback, &queryResults, &zErrMsg);
    if( rc != SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        return -1;
    }
    if(not queryResults.found)
        return ReportError("GetDC_IQ(%g MHz, ch=%d, tx=%d): cannot find match", frequency/1e6, int(channel), transmitter);
    if(dcI)
        *dcI = queryResults.dcI;
    if(dcQ)
        *dcQ = queryResults.dcQ;
    if(gainI)
        *gainI = queryResults.gainI;
    if(gainQ)
        *gainQ = queryResults.gainQ;
    if(phaseOffset)
        *phaseOffset = queryResults.phaseOffset;
    return 0;
}

int CalibrationCache::GetDC_IQ_Interp(uint32_t boardId, double frequency, uint8_t channel, bool transmitter, int band_lna, int *dcI, int *dcQ, int *gainI, int *gainQ, int *phaseOffset)
{
    std::vector<double> closeFreqs;

    auto lambda_callback = [](void *data, int argc, char **argv, char **azColName)
    {
        std::vector<double> *data_freqs = (std::vector<double>*)data;
        if(data != nullptr)
        {
            for (size_t i = 0; i < (size_t)argc; i++)
            {
                if (argv[i] == nullptr) continue;
                data_freqs->push_back(double(std::stoll(argv[i])));
            }
            return 0;
        }
        return 1;
    };

    char* zErrMsg = 0;
    stringstream query;
    query << "SELECT min(frequency) as freq FROM LMS7002M_DC_IQ where "<<
"boardID="<<boardId<<
" AND frequency >= "<<std::llrint(frequency)<<
" AND frequency < "<<std::llrint(frequency + 1e6)<<
" AND channel="<<(int)channel<<
" AND transmitter="<<(transmitter?1:0)<<
" AND band_lna="<<band_lna<<
" UNION SELECT max(frequency) as freq FROM LMS7002M_DC_IQ where "<<
"boardID="<<boardId<<
" AND frequency <= "<<std::llrint(frequency)<<
" AND frequency > "<<std::llrint(frequency - 1e6)<<
" AND channel="<<(int)channel<<
" AND transmitter="<<(transmitter?1:0)<<
" AND band_lna="<<band_lna<<
";";

    int rc = sqlite3_exec(db, query.str().c_str(), lambda_callback, &closeFreqs, &zErrMsg);
    if( rc != SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        ReportError("SQL error: %s", zErrMsg);
        sqlite3_free(zErrMsg);
        return -1;
    }

    //found only one match, but its very close within margin
    if (closeFreqs.size() == 1 and std::abs(closeFreqs.front()-frequency) <= 100)
    {
        return GetDC_IQ(boardId, closeFreqs.front(), channel, transmitter, band_lna, dcI, dcQ, gainI, gainQ, phaseOffset);
    }

    //otherwise check for two results to perform interp
    if (closeFreqs.size() != 2) return ReportError(
        "GetDC_IQ_Interp(%g MHz, ch=%d, tx=%d): no matches between [%g, %g] MHz",
        frequency/1e6, int(channel), transmitter, frequency/1e6-1, frequency/1e6+1);

    //perform interpolation
    double f0 = closeFreqs[0];
    double f1 = closeFreqs[1];
    int dcI0, dcQ0, gainI0, gainQ0, phaseOffset0;
    int dcI1, dcQ1, gainI1, gainQ1, phaseOffset1;

    rc = GetDC_IQ(boardId, f0, channel, transmitter, band_lna, &dcI0, &dcQ0, &gainI0, &gainQ0, &phaseOffset0);
    if (rc != 0) return rc;

    rc = GetDC_IQ(boardId, f1, channel, transmitter, band_lna, &dcI1, &dcQ1, &gainI1, &gainQ1, &phaseOffset1);
    if (rc != 0) return rc;

    *dcI = std::rint(linearInterp(frequency, f0, dcI0, f1, dcI1));
    *dcQ = std::rint(linearInterp(frequency, f0, dcQ0, f1, dcQ1));
    *gainI = std::rint(linearInterp(frequency, f0, gainI0, f1, gainI1));
    *gainQ = std::rint(linearInterp(frequency, f0, gainQ0, f1, gainQ1));
    *phaseOffset = std::rint(linearInterp(frequency, f0, phaseOffset0, f1, phaseOffset1));

    return 0;
}

int CalibrationCache::InsertFilter_RC(uint32_t boardId, double bandwidth, uint8_t channel, bool transmitter, int filter_id, int rcal, int ccal, int cfb)
{
    char* zErrMsg = 0;
    stringstream query;
    query <<
"INSERT OR REPLACE INTO LMS7002M_FILTER_RC (boardID, bandwidth, channel, transmitter, filter_id, rcal, ccal, cfb) " <<
"VALUES ( " << boardId << "," << std::llrint(bandwidth) << "," << (int)channel << "," << (transmitter?1:0) << "," << filter_id << ", " <<
rcal<<","<<ccal<<","<<cfb<<");";

    int rc = sqlite3_exec(db, query.str().c_str(), nullptr, 0, &zErrMsg);
    if( rc != SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        return -1;
    }
    return 0;
}

int CalibrationCache::GetFilter_RC(uint32_t boardId, double bandwidth, uint8_t channel, bool transmitter, int filter_id, int *rcal, int *ccal, int *cfb)
{
    struct QueryFilter_RC
    {
        QueryFilter_RC() : rcal(0), ccal(0), cfb(0), found(false){};
        int rcal;
        int ccal;
        int cfb;
        bool found;
    };

    auto lambda_callback = [](void *filter_rc_data, int argc, char **argv, char **azColName)
    {
        QueryFilter_RC *data = (QueryFilter_RC*)filter_rc_data;
        if(data != nullptr)
        {
            data->rcal = argv[0] != nullptr ? std::stoi(argv[0]) : 0;
            data->ccal = argv[1] != nullptr ? std::stoi(argv[1]) : 0;
            data->cfb = argv[2] != nullptr ? std::stoi(argv[2]) : 0;
            data->found = true;
            return 0;
        }
        return 1;
    };

    QueryFilter_RC queryResults;

    char* zErrMsg = 0;
    stringstream query;
    query << "SELECT rcal, ccal, cfb FROM LMS7002M_FILTER_RC where "<<
"boardID="<<boardId<<
" AND bandwidth="<<std::llrint(bandwidth)<<
" AND channel="<<(int)channel<<
" AND transmitter="<<(transmitter?1:0)<<
" AND filter_id="<<filter_id<<
";";

    int rc = sqlite3_exec(db, query.str().c_str(), lambda_callback, &queryResults, &zErrMsg);
    if( rc != SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        return -1;
    }
    if(not queryResults.found)
        return -1;
    if(rcal)
        *rcal = queryResults.rcal;
    if(ccal)
        *ccal = queryResults.ccal;
    if(cfb)
        *cfb = queryResults.cfb;
    return 0;
}
