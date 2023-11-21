#pragma once

namespace lime {

enum class TRXDir : bool { Rx = 0, Tx = 1 };

struct Range {
    Range(double a, double b, double step)
        : min(a)
        , max(b)
        , step(step){};
    double min;
    double max;
    double step;
};

} // namespace lime
