#pragma once

#include <string>

namespace lime {

const std::string strFormat(const char* format, ...);

template<class T> class DeltaVariable
{
  public:
    DeltaVariable(T init)
        : mValue(init)
        , mLastValue(0){};
    void set(T val) { mValue = val; }
    void add(T val) { mValue += val; }
    T delta() { return mValue - mLastValue; } // value change since last reset
    T value() const { return mValue; }
    void checkpoint() { mLastValue = mValue; }

  private:
    T mValue;
    T mLastValue;
};

} // namespace lime
