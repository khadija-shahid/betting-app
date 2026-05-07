#pragma once

#include <string>
#include <vector>
#include "OrderBook.h"
#include "Wallet.h"
#include "UserManager.h"
#include "Candlestick.h"

class MerkelMain {
public:
    MerkelMain();
    void init();   

private:
    // ===================== MENU & CORE =====================
    void printMenu();
    void printHelp();
    void printMarketStats();
    void printProducts();
    void printWallet();
    void mainLoop();
    void gotoNextTimeFrame();
    void enterAsk();
    void enterBid();
    void showTransactionHistory();
    void printProductSummary();
    void showUserActivitySummary();

    // ===================== MINUTE CANDLESTICKS =====================
    // Build minute-based OHLC data
    std::vector<Candlestick> buildMinuteCandlesticks(const std::string& product);
     std::vector<OrderBookEntry>
    getTransactionOrdersForProduct(const std::string& product);
    // Candlestick table
    void printMinuteCandlestickTable(const std::vector<Candlestick>& candles,
                                     int startIndex, int endIndex);

    // (Graph output removed: coursework requires OHLC table only)

    // Timeframe selection menu
    void handleCandlestickTimeframe(const std::string& product);

    // Synthetic candlestick generator (if CSV has few minutes)
    void generateSyntheticCandles(std::vector<Candlestick>& candles, int targetCount);

    // ===================== DATA MEMBERS =====================
    OrderBook orderBook;
    Wallet wallet;
    std::string currentTime;

    // User authentication
    UserManager userManager = UserManager("users.csv");
    bool userLoggedIn = false;

    // ===================== CSV LOGGING =====================
    void logTransaction(const std::string& type,
                        const std::string& product,
                        double price,
                        double amount);

    // Wallet operations
    void depositMoney();
    void withdrawMoney();

    // ===================== TRADE SIMULATION =====================
    void simulateTradingActivity();
};
