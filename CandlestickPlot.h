#pragma once
#include <vector>
#include "Candlestick.h"

void plotCandlesticksASCII(const std::vector<Candlestick>& candles,
                           std::size_t maxToShow = 20);
