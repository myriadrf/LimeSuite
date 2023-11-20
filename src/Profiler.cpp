#include "Profiler.h"

#include <chrono>
#include <fstream>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "limesuite/config.h"

using namespace std;

namespace lime {

ProfilerCollection gAllProfilers;

auto startupTime = chrono::steady_clock::now();
double LIME_API GetTimeSinceStartup()
{
    return chrono::duration_cast<chrono::microseconds>(chrono::steady_clock::now() - startupTime).count();
}

ProfilerScope::ProfilerScope(Profiler* p, const std::string& title, uint8_t cat)
    : parent(p)
{
    evt.startNano = GetTimeSinceStartup();
    evt.name = title;
    evt.category = cat;
}

ProfilerScope::~ProfilerScope()
{
    evt.duration = GetTimeSinceStartup() - evt.startNano;
    parent->Add(evt);
}

Profiler::Profiler()
{
    mPID = getpid();
    pthread_t tid = pthread_self();
    mTID = tid;
    entries.reserve(10000);
}

Profiler::~Profiler()
{
    //GenerateChromeJSON();

    // Profiler a;
    // gAllProfilers.profilers.push_back(a);
    // Profiler &p = gAllProfilers.profilers.back();
    // p.entries = entries;
    // p.mTID = mTID;
    // p.mPID = mPID;
}

void Profiler::Done()
{
    gAllProfilers.profilers.push_back(this);
}

void Profiler::Add(const ProfilerEvent& s)
{
    entries.push_back(s);
}

void Profiler::GenerateChromeJSON()
{
}

ProfilerCollection::ProfilerCollection()
{
}

ProfilerCollection::~ProfilerCollection()
{
    GenerateChromeJSON();
    //for(int i=0; i<profilers.size(); ++i)
    //delete profilers[i];
}

void ProfilerCollection::GenerateChromeJSON()
{
    if (profilers.size() == 0)
        return;
    printf("Generating JSON, events: %li\n", profilers.size());
    std::ofstream o("perfTrace.json");
    o.write("[\n", 2);
    for (uint32_t j = 0; j < profilers.size(); ++j)
    {
        for (uint32_t i = 0; i < profilers[j]->entries.size(); ++i)
        {
            const ProfilerEvent& event = profilers[j]->entries[i];
            o << "{\n";
            o << "\"name\": \"" << event.name << "\",\n";
            o << "\"cat\": ";
            switch (event.category)
            {
            case 0:
                o << "\"gen\""
                  << ",\n";
                break;
            case 1:
                o << "\"rx\""
                  << ",\n";
                break;
            case 2:
                o << "\"tx\""
                  << ",\n";
                break;
            }
            o << "\"ph\": "
              << "\"X\""
              << ",\n";
            o << "\"ts\": " << event.startNano << ",\n";
            o << "\"dur\": " << event.duration << ",\n";
            o << "\"pid\": " << profilers[j]->mPID << ",\n";
            o << "\"tid\": " << profilers[j]->mTID << ",\n";
            o << "\"args\": "
              << "\"\"";
            o << "\n}," << std::endl;
        }
    }
    o.close();
}

} // namespace lime
