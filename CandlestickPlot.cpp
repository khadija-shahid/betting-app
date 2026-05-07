#include "CandlestickPlot.h"
#include <iostream>
#include <iomanip>
#include <algorithm>

void plotCandlesticksASCII(const std::vector<Candlestick>& candles,
                           std::size_t maxToShow)
{
    if (candles.empty()) {
        std::cout << "No candlestick data available.\n";
        return;
    }

    std::size_t count = candles.size();
    std::size_t startIndex = 0;

    if (count > maxToShow) {
        startIndex = count - maxToShow;
    }

    // --------------------------------------------------
    // Find overall min/max for scaling
    // --------------------------------------------------
    double globalMin = candles[startIndex].low;
    double globalMax = candles[startIndex].high;

    for (std::size_t i = startIndex; i < count; ++i) {
        globalMin = std::min(globalMin, candles[i].low);
        globalMax = std::max(globalMax, candles[i].high);
    }

    double range = globalMax - globalMin;
    if (range <= 0.0) range = 1.0;

    // --------------------------------------------------
    // Print unified OHLC summary table header
    // --------------------------------------------------
    std::cout << "\n===== OHLC SUMMARY TABLE =====\n";
    std::cout << "DATE       | "
              << "OPEN        HIGH        LOW         CLOSE\n";
    std::cout << "-------------------------------------------------------------\n";

    std::cout << std::fixed << std::setprecision(2);

    // --------------------------------------------------
    // Print each candlestick row
    // --------------------------------------------------
    for (std::size_t i = startIndex; i < count; ++i) {
        const auto& c = candles[i];

        // Label based on granularity (YYYY-MM or YYYY-MM-DD)
        std::string label = c.startTime.substr(0, 10);

        std::cout << label << " | "
                  << "OPEN: "  << std::setw(8) << c.open  << "  "
                  << "HIGH: "  << std::setw(8) << c.high  << "  "
                  << "LOW: "   << std::setw(8) << c.low   << "  "
                  << "CLOSE: " << std::setw(8) << c.close
                  << "\n";
    }
}
