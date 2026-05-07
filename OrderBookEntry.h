#pragma once
#include <string>

enum class OrderType {
    bid,
    ask,
    asksale,
    bidsale
};

class OrderBookEntry {
public:
    double price;
    double amount;
    std::string timestamp;
    std::string product;
    OrderType orderType;
    std::string username;

    OrderBookEntry(double price,
                   double amount,
                   const std::string& timestamp,
                   const std::string& product,
                   OrderType orderType,
                   const std::string& username = "dataset");
};
