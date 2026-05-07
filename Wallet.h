#pragma once

#include <map>
#include <string>
#include "OrderBookEntry.h"

class Wallet {
public:
    Wallet() = default;

    // Add some amount of a currency to the wallet
    void insertCurrency(const std::string& type, double amount);

    // Check if wallet has at least this amount of currency
    bool containsCurrency(const std::string& type, double amount) const;

    // Try to remove amount from wallet. Return true on success.
    bool removeCurrency(const std::string& type, double amount);

    bool existsInCSV(const std::string& filename,
                 const std::string& username);


    // Return a human-readable string summarising wallet contents
    std::string toString() const;

    // Check if the wallet can afford to place this order
    bool canFulfillOrder(const OrderBookEntry& order) const;

    // Update wallet after a sale (asksale or bidsale)
    void processSale(const OrderBookEntry& sale);

    // Load wallet data for a specific user from CSV
    void loadFromCSV(const std::string& filename,
                     const std::string& username);

    // Save wallet data for a specific user to CSV
    void saveToCSV(const std::string& filename,
                   const std::string& username);

private:
    std::map<std::string, double> currencies;
};
