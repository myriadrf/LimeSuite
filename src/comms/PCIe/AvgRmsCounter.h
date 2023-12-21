#ifndef LIME_AVGRMSCOUNTER_H
#define LIME_AVGRMSCOUNTER_H

#include <cstdint>

namespace lime {

/** @brief A helper class for calculating the Average and the Root Mean Square */
class AvgRmsCounter
{
  public:
    AvgRmsCounter();

    /**
      @brief Adds the given value to the accumulators.
      
      @param value The value to add.
     */
    void Add(double value);

    /**
      @brief Gets the results of the counters and resets them.
      
      @param[out] avg The average of all the submitted numbers.
      @param[out] rms The Root Mean Square Average of all the submitted numbers.
     */
    void GetResult(double& avg, double& rms);

    /**
      @brief Gets the current minimum value and resets it.
      
      @return The minimum value recorded.
     */
    double Min();

    /**
      @brief Gets the current maximum value and resets it.
      
      @return The maximum value recorded.
     */
    double Max();
  private:
    int32_t counter;
    double avgAccumulator;
    double rmsAccumulator;
    double min;
    double max;
};

} // namespace lime

#endif // LIME_AVGRMSCOUNTER_H