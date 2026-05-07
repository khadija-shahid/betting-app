#include "OrderBook.h"
#include "CSVReader.h"
#include <set>
#include <algorithm>

OrderBook::OrderBook(const std::string& filename) {
    orders = CSVReader::readCSV(filename);
}

void OrderBook::insertOrder(const OrderBookEntry& order) {
    orders.push_back(order);
}

std::vector<std::string> OrderBook::getKnownProducts() const {
    std::set<std::string> productSet;

    for (const auto& e : orders)
        productSet.insert(e.product);

    return std::vector<std::string>(productSet.begin(), productSet.end());
}

std::string OrderBook::getEarliestTime() const {
    if (orders.empty()) return "";
    std::string earliest = orders[0].timestamp;

    for (const auto& e : orders)
        if (e.timestamp < earliest)
            earliest = e.timestamp;

    return earliest;
}

std::string OrderBook::getNextTime(const std::string& currentTime) const {
    std::string nextTime;

    for (const auto& e : orders) {
        if (e.timestamp > currentTime) {
            if (nextTime.empty() || e.timestamp < nextTime)
                nextTime = e.timestamp;
        }
    }

    if (nextTime.empty())
        return getEarliestTime();

    return nextTime;
}

std::vector<OrderBookEntry> OrderBook::getOrders(
    const std::string& product,
    const std::string& timestamp,
    OrderType orderType
)

 const {
    std::vector<OrderBookEntry> result;

    for (const auto& e : orders) {
        if (e.product == product &&
            e.timestamp == timestamp &&
            e.orderType == orderType) {
            result.push_back(e);
        }
    }

    return result;
}

// 🔥 NEW FUNCTION — REAL FIX FOR CANDLESTICKS
std::vector<OrderBookEntry> OrderBook::getAllOrdersForProduct(const std::string& product) const {
    std::vector<OrderBookEntry> result;

    for (const auto& e : orders)
        if (e.product == product)
            result.push_back(e);

    // sort by timestamp (VERY IMPORTANT)
    std::sort(result.begin(), result.end(),
              [](const OrderBookEntry& a, const OrderBookEntry& b){
                  return a.timestamp < b.timestamp;
              });

    return result;
}

// Matching engine stays unchanged
std::vector<OrderBookEntry> OrderBook::matchAsksToBids(
    const std::string& product,
    const std::string& timestamp
) const {
    std::vector<OrderBookEntry> sales;

    std::vector<OrderBookEntry> asks;
    std::vector<OrderBookEntry> bids;

    for (const auto& e : orders) {
        if (e.product == product && e.timestamp == timestamp) {
            if (e.orderType == OrderType::ask)
                asks.push_back(e);
            else if (e.orderType == OrderType::bid)
                bids.push_back(e);
        }
    }

    if (asks.empty() || bids.empty())
        return sales;

    std::sort(asks.begin(), asks.end(),
              [](const OrderBookEntry& a, const OrderBookEntry& b){
                  return a.price < b.price;
              });

    std::sort(bids.begin(), bids.end(),
              [](const OrderBookEntry& a, const OrderBookEntry& b){
                  return a.price > b.price;
              });

    for (auto& ask : asks) {
        for (auto& bid : bids) {
            if (bid.amount <= 0) continue;
            if (ask.amount <= 0) break;

            if (bid.price >= ask.price) {
                double tradedAmount = std::min(ask.amount, bid.amount);
                if (tradedAmount <= 0) continue;

                double tradePrice = ask.price;

                // Create per-user sale records (seller + buyer). This matches the
                // starter-code idea where wallet updates apply only to the user
                // whose order participated in the trade.
                OrderBookEntry saleSeller{
                    tradePrice,
                    tradedAmount,
                    timestamp,
                    product,
                    OrderType::asksale,
                    ask.username
                };

                OrderBookEntry saleBuyer{
                    tradePrice,
                    tradedAmount,
                    timestamp,
                    product,
                    OrderType::bidsale,
                    bid.username
                };

                sales.push_back(saleSeller);
                sales.push_back(saleBuyer);

                ask.amount -= tradedAmount;
                bid.amount -= tradedAmount;
            }
        }
    }

    return sales;
}
