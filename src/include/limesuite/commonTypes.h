#pragma once

namespace lime {

enum class TRXDir : bool { Rx, Tx };

struct Range {
    constexpr Range(double min = 0, double max = 0, double step = 0)
        : min(min)
        , max(max)
        , step(step){};
    double min;
    double max;
    double step;
};

/**
  @brief Class for counting the difference since last time the value was set.
  @tparam T The type of the value to hold.
 */
template<class T> class DeltaVariable
{
  public:
    constexpr DeltaVariable(T init)
        : mValue(init)
        , mLastValue(0){};
    constexpr void set(T val) { mValue = val; }
    constexpr void add(T val) { mValue += val; }
    constexpr T delta() const { return mValue - mLastValue; } // value change since last reset
    constexpr T value() const { return mValue; }
    constexpr void checkpoint() { mLastValue = mValue; }

  private:
    T mValue;
    T mLastValue;
};

} // namespace lime
