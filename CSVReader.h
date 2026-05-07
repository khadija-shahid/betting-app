#pragma once
#include <vector>
#include <string>
#include "OrderBookEntry.h"

class CSVReader {
public:
    static std::vector<OrderBookEntry> readCSV(std::string filename);
    static std::vector<std::string> tokenize(std::string line, char delimiter);
};
