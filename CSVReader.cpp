#include "CSVReader.h"
#include <iostream>
#include <fstream>
#include <sstream>

std::vector<OrderBookEntry> CSVReader::readCSV(std::string filename)
{
    std::vector<OrderBookEntry> entries;
    std::ifstream file{ filename };
    std::string line;

    if (!file.is_open()) {
        std::cout << "Failed to open file: " << filename << std::endl;
        return entries;
    }

    // Skip header row
    std::getline(file, line);

    while (std::getline(file, line))
    {
        if (line.size() == 0)
            continue;

        std::vector<std::string> tokens = tokenize(line, ',');

        // Expected format:
        // timestamp, product, orderType, price, amount
        if (tokens.size() != 5)
            continue;

        try {
            double price  = std::stod(tokens[3]);
            double amount = std::stod(tokens[4]);

            OrderType type;
            if (tokens[2] == "bid")
                type = OrderType::bid;
            else if (tokens[2] == "ask")
                type = OrderType::ask;
            else
                continue;

            OrderBookEntry entry{
                price,
                amount,
                tokens[0],  // timestamp
                tokens[1],  // product
                type,
                "dataset"   // default username
            };

            entries.push_back(entry);
        }
        catch (...) {
            // Ignore malformed rows
            continue;
        }
    }

    std::cout << "CSVReader::readCSV read "
              << entries.size()
              << " entries" << std::endl;

    return entries;
}

std::vector<std::string> CSVReader::tokenize(std::string line, char delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    std::stringstream ss(line);

    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }

    return tokens;
}
