#pragma once

#include <vector>
#include <string>
#include "OrderBookEntry.h"

class OrderBook {
public:
    // Load all orders from CSV in constructor
    OrderBook(const std::string& filename);

    // Add new order
    void insertOrder(const OrderBookEntry& order);

    // List of all products
    std::vector<std::string> getKnownProducts() const;

    // Earliest timestamp
    std::string getEarliestTime() const;

    // Next timestamp after currentTime
    std::string getNextTime(const std::string& currentTime) const;

    // Orders matching exact timestamp + product + type
    std::vector<OrderBookEntry> getOrders(
        const std::string& product,
        const std::string& timestamp,
        OrderType orderType
    ) const;

    // NEW: Get ALL orders in the CSV for this product
    std::vector<OrderBookEntry> getAllOrdersForProduct(const std::string& product) const;

    // Match engine
    std::vector<OrderBookEntry> matchAsksToBids(
        const std::string& product,
        const std::string& timestamp
    ) const;

private:
    std::vector<OrderBookEntry> orders;
};
