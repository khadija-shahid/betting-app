#pragma once
#include <string>

struct Candlestick {
    double open;
    double high;
    double low;
    double close;
    std::string startTime;
    std::string endTime;

    Candlestick() {}

    Candlestick(double o,
                double h,
                double l,
                double c,
                const std::string& start,
                const std::string& end)
        : open(o), high(h), low(l), close(c),
          startTime(start), endTime(end) {}
};
