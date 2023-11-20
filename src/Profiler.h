#ifndef LIME_PROFILER_H
#define LIME_PROFILER_H

#include <stdint.h>
#include <string>
#include <sys/types.h>
#include <vector>

namespace lime {

class Profiler;

struct ProfilerEvent {
  public:
    int64_t startNano;
    int32_t duration;
    uint8_t category;
    std::string name;
};

class ProfilerScope
{
  public:
    ProfilerScope() = delete;
    ProfilerScope(Profiler* p, const std::string& title, uint8_t cat = 0);
    ~ProfilerScope();

    Profiler* parent;
    ProfilerEvent evt;
};

class Profiler
{
  public:
    Profiler();
    ~Profiler();

    void Add(const ProfilerEvent& s);
    void Done();

    pid_t mPID;
    pid_t mTID;

    std::vector<ProfilerEvent> entries;
    void GenerateChromeJSON();
};

class ProfilerCollection
{
  public:
    ProfilerCollection();
    ~ProfilerCollection();

    void GenerateChromeJSON();

    std::vector<Profiler*> profilers;
};

double GetTimeSinceStartup();

} // namespace lime

#endif // LIME_PROFILER_H
