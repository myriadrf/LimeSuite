#pragma once

namespace lime {

enum Dir
{
    Rx = 0,
    Tx = 1
};

struct Range
{
    Range(double a, double b, double step) : min(a), max(b), step(step){};
    double min;
    double max;
    double step;
};

}
