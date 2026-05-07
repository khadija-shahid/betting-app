#include "MerkelMain.h"
#include "OrderBookEntry.h"
#include "Wallet.h"


#include <iostream>
#include <fstream>
#include <limits>
#include <iomanip>
#include <map>
#include <cmath>
#include <algorithm>
#include <vector>
#include <string>
#include <chrono>
#include <sstream>


void printOHLCtable(const std::vector<Candlestick>& candles);
std::string getSystemTimestamp()
{
    using namespace std::chrono;
    auto now = system_clock::now();
    auto s = time_point_cast<seconds>(now);
    std::time_t t = system_clock::to_time_t(s);

    std::tm tm{};
#if defined(_WIN32)
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif

    char buf[20];
    std::strftime(buf, sizeof(buf), "%Y/%m/%d %H:%M", &tm);
    return std::string(buf);
}

static std::string nowTimestamp()
{
    using namespace std::chrono;

    auto now = system_clock::now();
    auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

    std::time_t t = system_clock::to_time_t(now);
    std::tm tm{};
#if defined(_WIN32) || defined(_WIN64)
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif

    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y/%m/%d %H:%M:%S", &tm);

    std::string out(buf);
    out += ".";
    out += std::to_string((int)ms.count()); // milliseconds
    return out;
}

static void ensureTransactionsHeader()
{
    std::ifstream in("transactions.csv");
    if (!in.good())
    {
        std::ofstream out("transactions.csv");
        out << "username,type,product,price,amount,timestamp\n";
        return;
    }

    in.seekg(0, std::ios::end);
    if (in.tellg() == 0)
    {
        std::ofstream out("transactions.csv", std::ios::app);
        out << "username,type,product,price,amount,timestamp\n";
    }
}

static void appendTransactionCSV(const std::string& username,
                                 const std::string& type,
                                 const std::string& product,
                                 double price,
                                 double amount,
                                 const std::string& timestamp)
{
    ensureTransactionsHeader();

    std::ofstream file("transactions.csv", std::ios::app);
    if (!file.is_open()) return;

    file << username << ","
         << type << ","
         << product << ","
         << price << ","
         << amount << ","
         << timestamp
         << "\n";
}

MerkelMain::MerkelMain()
: orderBook{"20200601.csv"}
{
    
}

void MerkelMain::init() {
    currentTime = orderBook.getEarliestTime();
    std::cout << "Current timestamp: " << currentTime << "\n\n";
    mainLoop();
}

// =======================================================
// Menu
// =======================================================
void MerkelMain::printMenu() {
    std::cout << "\n========== ACCOUNT MENU ==========\n";
    std::cout << " 100: Register new user\n";
    std::cout << " 101: Login\n";
    std::cout << " 102: Reset password\n";

    std::cout << "\n========== MAIN MENU ==========\n";
    std::cout << " 1 : Make an ask (sell)\n";
    std::cout << " 2 : Make a bid (buy)\n";
    std::cout << " 3 : Move to next timeframe\n";
    std::cout << " 4 : Market stats\n";
    std::cout << " 5 : Print wallet\n";
    std::cout << " 6 : Deposit money\n";
    std::cout << " 7 : Withdraw money\n";
    std::cout << " 8 : Transaction history\n";
    std::cout << " 9 : Candlesticks\n";
    std::cout << " 10: Simulate trades\n";
    std::cout << " 11: User activity summary\n";
    std::cout << " 0 : Exit\n";
    std::cout << "================================\n";
}




void MerkelMain::mainLoop() {
    bool running = true;

    while (running) {
        printMenu();

        int choice;
        std::cin >> choice;

        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(9999, '\n');
            std::cout << "Invalid input.\n";
            continue;
        }

        switch (choice) {

        case 100: {
            std::string fullname, email, password;
            std::cout << "Enter full name: ";
            std::getline(std::cin >> std::ws, fullname);

            std::cout << "Enter email: ";
            std::getline(std::cin >> std::ws, email);

            std::cout << "Enter password: ";
            std::getline(std::cin >> std::ws, password);

          if (userManager.registerUser(fullname, email, password)) {
    userLoggedIn = true;
    wallet = Wallet();
    std::string username =
        userManager.getLoggedInUser().username;

    wallet.loadFromCSV("wallet.csv", username);

    std::cout << "Logged in as: " << username << "\n";
}

            break;
        }

       case 101: {
            std::string inputUsername, password;

            std::cout << "Enter username: ";
            std::getline(std::cin >> std::ws, inputUsername);

            std::cout << "Enter password: ";
            std::getline(std::cin >> std::ws, password);

            if (userManager.loginUser(inputUsername, password))
            {
                userLoggedIn = true;

                // Reset in-memory wallet then load this user's wallet data
                wallet = Wallet();
                std::string loggedUser = userManager.getLoggedInUser().username;

                if (wallet.existsInCSV("wallet.csv", loggedUser))
                {
                    wallet.loadFromCSV("wallet.csv", loggedUser);
                }
                else
                {
                    // First login: seed a default balance then persist
                    wallet.insertCurrency("BTC", 10);
                    wallet.insertCurrency("USDT", 1000);
                    wallet.saveToCSV("wallet.csv", loggedUser);
                }

                std::cout << "Logged in as: " << loggedUser << "\n";
            }
            break;
        }


 case 102: {
    auto usernames = userManager.getAllUsernames();

    if (usernames.empty()) {
        std::cout << "No users found.\n";
        break;
    }

    std::cout << "\n=== SELECT USER TO RESET PASSWORD ===\n";
    for (int i = 0; i < (int)usernames.size(); ++i)
        std::cout << i << ") " << usernames[i] << "\n";

    int idx;
    std::cout << "Choose user index: ";
    std::cin >> idx;

    if (idx < 0 || idx >= (int)usernames.size()) {
        std::cout << "Invalid selection.\n";
        break;
    }

    std::string email, newPassword;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::cout << "Enter email (must be @gmail.com): ";
    std::getline(std::cin, email);

    std::cout << "Enter new password: ";
    std::getline(std::cin, newPassword);

    userManager.resetPassword(usernames[idx], email, newPassword);
    break;
}



        default:

            if (!userLoggedIn && choice != 0) {
                std::cout << "You must log in first.\n";
                break;
            }
            else if (choice == 1) enterAsk();
            else if (choice == 2) enterBid();
            else if (choice == 3) gotoNextTimeFrame();
            else if (choice == 4) printMarketStats();
            else if (choice == 5) printWallet();
            else if (choice == 6) depositMoney();
            else if (choice == 7) withdrawMoney();
            else if (choice == 8) showTransactionHistory();
            else if (choice == 9) handleCandlestickTimeframe("");
            else if (choice == 10) simulateTradingActivity();
            else if (choice == 11) showUserActivitySummary();
            else if (choice == 0) {
                std::cout << "Exiting...\n";
                running = false;
            }
            break;
        }
    }
}


// =======================================================
// Wallet
// =======================================================
void MerkelMain::printWallet() {
    std::cout << wallet.toString() << "\n";
}

// =======================================================
// Market stats 
// =======================================================
void MerkelMain::printMarketStats()
{
    std::cout << "\n=== MARKET STATS (" << currentTime << ") ===\n";

    for (const std::string& product : orderBook.getKnownProducts())
    {
        std::cout << "Product: " << product << "\n";

        std::vector<OrderBookEntry> asks = orderBook.getOrders(product, currentTime, OrderType::ask);
        std::cout << "Asks seen: " << asks.size() << "\n";

        if (!asks.empty())
        {
            double maxAsk = asks[0].price;
            double minAsk = asks[0].price;
            for (const auto& a : asks)
            {
                maxAsk = std::max(maxAsk, a.price);
                minAsk = std::min(minAsk, a.price);
            }
            std::cout << "Max ask: " << maxAsk << "\n";
            std::cout << "Min ask: " << minAsk << "\n";
        }
        else
        {
            std::cout << "Max ask: N/A\n";
            std::cout << "Min ask: N/A\n";
        }

        std::cout << "\n";
    }
}

// =======================================================
// Timeframe progression + trade execution 
// =======================================================
void MerkelMain::gotoNextTimeFrame()
{
    std::cout << "\nAdvancing to next timeframe...\n";

    const std::string currentUser = userManager.getLoggedInUser().username;

    // First, execute any matches in the CURRENT timeframe.
    std::vector<OrderBookEntry> executedForUser;
    for (const std::string& product : orderBook.getKnownProducts())
    {
        std::vector<OrderBookEntry> sales = orderBook.matchAsksToBids(product, currentTime);

        for (const auto& sale : sales)
        {
            if (sale.username != currentUser)
                continue;

            wallet.processSale(sale);
            executedForUser.push_back(sale);

            if (sale.orderType == OrderType::asksale)
                appendTransactionCSV(currentUser, "ASK_SALE", product, sale.price, sale.amount, currentTime);
            else if (sale.orderType == OrderType::bidsale)
                appendTransactionCSV(currentUser, "BID_SALE", product, sale.price, sale.amount, currentTime);
        }
    }

    // Show what was actually processed 
    if (executedForUser.empty())
    {
        std::cout << "No matched trades were executed for your orders in this timeframe.\n";
    }
    else
    {
        std::cout << "Executed trades:\n";
        std::cout << std::left
                  << std::setw(8)  << "Side"
                  << std::setw(12) << "Product"
                  << std::setw(12) << "Price"
                  << std::setw(12) << "Amount"
                  << "Timestamp" << "\n";
        std::cout << std::string(60, '-') << "\n";

        for (const auto& s : executedForUser)
        {
            const char* side = (s.orderType == OrderType::bidsale) ? "BUY" : "SELL";
            std::cout << std::left
                      << std::setw(8)  << side
                      << std::setw(12) << s.product
                      << std::setw(12) << s.price
                      << std::setw(12) << s.amount
                      << s.timestamp
                      << "\n";
        }
        std::cout << std::string(60, '-') << "\n";
    }

    // Persist wallet after processing.
    wallet.saveToCSV("wallet.csv", currentUser);

    // Now advance the market time.
    currentTime = orderBook.getNextTime(currentTime);
    std::cout << "Current timestamp: " << currentTime << "\n";
}

// =======================================================
// Ask & Bid
// =======================================================
void MerkelMain::enterAsk()
{
    auto products = orderBook.getKnownProducts();

    std::cout << "\n=== AVAILABLE PRODUCTS ===\n";
    for (const auto& p : products)
        std::cout << "  " << p << "\n";

    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::string product;
    std::cout << "Enter product to SELL (e.g. BTC/USDT): ";
    std::getline(std::cin, product);

    if (std::find(products.begin(), products.end(), product) == products.end())
    {
        std::cout << "Invalid product.\n";
        return;
    }

    double price, amount;

    std::cout << "Price: ";
    std::cin >> price;

    std::cout << "Amount: ";
    std::cin >> amount;

    // market timeframe, and must be wallet-fulfillable before being placed.
    OrderBookEntry order(
        price,
        amount,
        currentTime,
        product,
        OrderType::ask,
        userManager.getLoggedInUser().username
    );

    if (!wallet.canFulfillOrder(order))
    {
        std::cout << "Wallet has insufficient funds for this ask.\n";
        return;
    }

    orderBook.insertOrder(order);

   appendTransactionCSV(
    userManager.getLoggedInUser().username,
    "ASK",
    product,
    price,
    amount,
    currentTime  
);

    std::cout << "Ask submitted for " << product << ".\n";
}


void MerkelMain::enterBid()
{
    auto products = orderBook.getKnownProducts();

    std::cout << "\n=== AVAILABLE PRODUCTS ===\n";
    for (const auto& p : products)
        std::cout << "  " << p << "\n";

    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::string product;
    std::cout << "Enter product to BUY (e.g. BTC/USDT): ";
    std::getline(std::cin, product);

    if (std::find(products.begin(), products.end(), product) == products.end())
    {
        std::cout << "Invalid product.\n";
        return;
    }

    double price, amount;

    std::cout << "Price: ";
    std::cin >> price;

    std::cout << "Amount: ";
    std::cin >> amount;

    OrderBookEntry order(
        price,
        amount,
        currentTime,
        product,
        OrderType::bid,
        userManager.getLoggedInUser().username
    );

    if (!wallet.canFulfillOrder(order))
    {
        std::cout << "Wallet has insufficient funds for this bid.\n";
        return;
    }

    orderBook.insertOrder(order);

   appendTransactionCSV(
    userManager.getLoggedInUser().username,
    "BID",
    product,
    price,
    amount,
    currentTime
);

    std::cout << "Bid submitted for " << product << ".\n";
}

void MerkelMain::depositMoney() {
    double amount;
    std::cout << "Deposit USDT amount: ";
    std::cin >> amount;

    if (amount > 0) {
        wallet.insertCurrency("USDT", amount);
        wallet.saveToCSV("wallet.csv",
            userManager.getLoggedInUser().username);

        appendTransactionCSV(
    userManager.getLoggedInUser().username,
    "DEPOSIT",
    "USDT",
    1,
    amount,
    currentTime
);


        std::cout << "Deposit successful.\n";
    }
}


void MerkelMain::withdrawMoney() {
    double amount;
    std::cout << "Withdraw USDT amount: ";
    std::cin >> amount;

    if (!wallet.containsCurrency("USDT", amount)) {
        std::cout << "Insufficient funds.\n";
        return;
    }

    wallet.removeCurrency("USDT", amount);
    wallet.saveToCSV("wallet.csv",
        userManager.getLoggedInUser().username);

    appendTransactionCSV(
    userManager.getLoggedInUser().username,
    "WITHDRAW",
    "USDT",
    1,
    amount,
    currentTime
);


    std::cout << "Withdrawal successful.\n";
}


// =======================================================
// Print Products
// =======================================================
void MerkelMain::printProducts() {
    auto products = orderBook.getKnownProducts();
    for (const auto& p : products)
        std::cout << "  " << p << "\n";
}

void MerkelMain::showTransactionHistory()
{
    const std::string user = userManager.getLoggedInUser().username;

    struct Tx {
        std::string when;
        std::string type;
        std::string product;
        double price = 0.0;
        double amount = 0.0;
    };

    auto split = [](const std::string& s) {
        std::vector<std::string> parts;
        std::string cur;
        for (char c : s) {
            if (c == ',') { parts.push_back(cur); cur.clear(); }
            else cur.push_back(c);
        }
        parts.push_back(cur);
        return parts;
    };

    std::ifstream in("transactions.csv");
    if (!in.is_open()) {
        std::cout << "No transaction history file found.\n";
        return;
    }

    std::string line;
    std::getline(in, line); // header

    std::vector<Tx> all;
    while (std::getline(in, line)) {
        if (line.rfind(user + ",", 0) != 0)
            continue;

        auto c = split(line);
        // username,type,product,price,amount,timestamp
        if (c.size() < 6)
            continue;

        Tx t;
        t.type = c[1];
        t.product = c[2];
        t.when = c[5];
        try { t.price = std::stod(c[3]); } catch (...) { t.price = 0.0; }
        try { t.amount = std::stod(c[4]); } catch (...) { t.amount = 0.0; }
        all.push_back(t);
    }

    if (all.empty()) {
        std::cout << "No transactions recorded yet.\n";
        return;
    }

    auto printList = [&](const std::vector<Tx>& rows) {
        std::cout << "\n--- Transactions ---\n";
        std::cout << std::left
                  << std::setw(20) << "Timestamp"
                  << std::setw(10) << "Type"
                  << std::setw(12) << "Product"
                  << std::setw(12) << "Price"
                  << std::setw(12) << "Amount"
                  << "\n";
        std::cout << std::string(66, '-') << "\n";

        for (const auto& t : rows) {
            std::cout << std::left
                      << std::setw(20) << t.when
                      << std::setw(10) << t.type
                      << std::setw(12) << t.product
                      << std::setw(12) << t.price
                      << std::setw(12) << t.amount
                      << "\n";
        }
        std::cout << std::string(66, '-') << "\n";
    };

    std::cout << "\n=== Transaction History ===\n";
    std::cout << " 1) Show last 5\n";
    std::cout << " 2) Filter by product\n";
    std::cout << "Choice: ";

    int choice = 0;
    std::cin >> choice;
    if (std::cin.fail()) {
        std::cin.clear();
        std::cin.ignore(9999, '\n');
        std::cout << "Invalid input.\n";
        return;
    }

    std::vector<Tx> view;
    if (choice == 1) {
        int start = std::max(0, (int)all.size() - 5);
        for (int i = (int)all.size() - 1; i >= start; --i)
            view.push_back(all[i]);
        printList(view);
        return;
    }

    if (choice == 2) {
        auto products = orderBook.getKnownProducts();

        // clear the newline left by std::cin >> choice
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        std::cout << "Available products:\n";
        for (const auto& p : products)
            std::cout << "  " << p << "\n";

        std::string product;
        std::cout << "Enter product: ";
        std::getline(std::cin, product);

        for (int i = (int)all.size() - 1; i >= 0; --i)
            if (all[i].product == product)
                view.push_back(all[i]);

        if (view.empty())
            std::cout << "No transactions found for product: " << product << "\n";
        else
            printList(view);
        return;
    }

    std::cout << "Invalid choice.\n";
}

std::vector<Candlestick>
MerkelMain::buildMinuteCandlesticks(const std::string& product)
{
    std::vector<OrderBookEntry> all =
        orderBook.getAllOrdersForProduct(product);
    auto txOrders = getTransactionOrdersForProduct(product);
    all.insert(all.end(), txOrders.begin(), txOrders.end());

    if (all.empty()) return {};
    std::stable_sort(
        all.begin(),
        all.end(),
        [](const OrderBookEntry& a, const OrderBookEntry& b)
        {
            return a.timestamp < b.timestamp;
        }
    );


    std::map<std::string, std::vector<OrderBookEntry>> grouped;

    for (auto& o : all) {
        std::string key = o.timestamp.substr(0, 16); // YYYY/MM/DD HH:MM
        grouped[key].push_back(o);
    }

    std::vector<Candlestick> candles;

    for (auto& pair : grouped) {
        auto& entries = pair.second;
        if (entries.empty()) continue;

        double open  = entries.front().price;
        double close = entries.back().price;
        double high  = open;
        double low   = open;

        for (auto& e : entries) {
            high = std::max(high, e.price);
            low  = std::min(low,  e.price);
        }

        candles.push_back(Candlestick(open, high, low, close,
                                      pair.first, pair.first));
    }

    return candles;
}

std::vector<OrderBookEntry>
MerkelMain::getTransactionOrdersForProduct(const std::string& product)
{
    std::vector<OrderBookEntry> out;
    std::ifstream file("transactions.csv");
    if (!file.is_open())
        return out;

    std::string line;
    std::getline(file, line); // skip header

    while (std::getline(file, line))
    {
        std::stringstream ss(line);
        std::string user, type, prod, priceStr, amountStr, timestamp;

        std::getline(ss, user, ',');
        std::getline(ss, type, ',');
        std::getline(ss, prod, ',');
        std::getline(ss, priceStr, ',');
        std::getline(ss, amountStr, ',');
        std::getline(ss, timestamp, ',');

        if (prod != product) continue;
        if (type != "ASK" && type != "BID") continue;

        double price = std::stod(priceStr);
        double amount = std::stod(amountStr);

        out.emplace_back(
            price,
            amount,
            timestamp,
            product,
            (type == "ASK") ? OrderType::ask : OrderType::bid,
            user
        );
    }

    return out;
}

// =======================================================
// Timeframe Handler
// =======================================================
void MerkelMain::handleCandlestickTimeframe(const std::string&)
{
    auto products = orderBook.getKnownProducts();
    if (products.empty())
    {
        std::cout << "No products available.\n";
        return;
    }

    // -----------------------------
    // SHOW PRODUCT LIST
    // -----------------------------
    std::cout << "\n=== AVAILABLE PRODUCTS ===\n";
    for (const auto& p : products)
        std::cout << "  " << p << "\n";

    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::string product;
    std::cout << "Enter product (e.g. BTC/USDT): ";
    std::getline(std::cin, product);

    if (std::find(products.begin(), products.end(), product) == products.end())
    {
        std::cout << "Invalid product.\n";
        return;
    }

    auto orders = orderBook.getAllOrdersForProduct(product);
    if (orders.empty())
    {
        std::cout << "No data for product.\n";
        return;
    }

    // =====================================================
    // DEFAULT: YEARLY SUMMARY 
    // =====================================================
    std::map<std::string, std::vector<OrderBookEntry>> askBuckets, bidBuckets;

for (const auto& o : orderBook.getAllOrdersForProduct(product))
{
    std::string yearKey = o.timestamp.substr(0, 4);

    if (o.orderType == OrderType::ask)
        askBuckets[yearKey].push_back(o);
    else if (o.orderType == OrderType::bid)
        bidBuckets[yearKey].push_back(o);
}

auto txOrders = getTransactionOrdersForProduct(product);

for (auto o : txOrders)
{
    o.timestamp = "2026" + o.timestamp.substr(4);

    if (o.orderType == OrderType::ask)
        askBuckets["2026"].push_back(o);
    else if (o.orderType == OrderType::bid)
        bidBuckets["2026"].push_back(o);
}



    auto buildCandles = [](auto& buckets)
    {
        std::vector<Candlestick> out;

        for (auto& pair : buckets)
        {
            auto& v = pair.second;
            std::stable_sort(v.begin(), v.end(),
                 [](const auto& a, const auto& b) {
                     return a.timestamp < b.timestamp;
                 });


            double open = v.front().price;
            double close = v.back().price;
            double high = open, low = open;

            for (auto& e : v)
            {
                high = std::max(high, e.price);
                low  = std::min(low,  e.price);
            }

            out.emplace_back(open, high, low, close,
                             pair.first, pair.first);
        }
        return out;
    };

    auto askYearly = buildCandles(askBuckets);
    auto bidYearly = buildCandles(bidBuckets);

    std::cout << "\n(YEARLY summary)\n";

    std::cout << "\nProduct: " << product << " | ASKS | YEARLY\n";
    printOHLCtable(askYearly);

    std::cout << "\nProduct: " << product << " | BIDS | YEARLY\n";
    printOHLCtable(bidYearly);

    // =====================================================
    // OPTIONAL FILTER 
    // =====================================================
    std::cout << "\nView another range?\n";
    std::cout << "1) Daily (minute candlesticks)\n";
    std::cout << "2) Monthly\n";
    std::cout << "0) Exit\n";
    std::cout << "Choice: ";

    int choice;
    std::cin >> choice;

    if (choice == 0) return;

    // =====================================================
    // DAILY → MINUTE CANDLESTICKS
    // =====================================================
    if (choice == 1)
    {
        auto candles = buildMinuteCandlesticks(product);
        if (candles.empty())
        {
            std::cout << "No minute data available.\n";
            return;
        }

        int minutes;
        std::cout << "How many previous minutes? ";
        std::cin >> minutes;

        if (std::cin.fail() || minutes <= 0)
        {
            std::cout << "Invalid number.\n";
            return;
        }

        int total = candles.size();
        int start = std::max(0, total - minutes);

        printMinuteCandlestickTable(candles, start, total - 1);
        return;
    }

    // =====================================================
    // MONTHLY SUMMARY
    // =====================================================
    if (choice == 2)
    {
        askBuckets.clear();
        bidBuckets.clear();

        for (const auto& o : orders)
        {
            std::string monthKey = o.timestamp.substr(0, 7); // YYYY/MM

            if (o.orderType == OrderType::ask)
                askBuckets[monthKey].push_back(o);
            else if (o.orderType == OrderType::bid)
                bidBuckets[monthKey].push_back(o);
        }

        auto askMonthly = buildCandles(askBuckets);
        auto bidMonthly = buildCandles(bidBuckets);

        std::cout << "\nProduct: " << product << " | ASKS | MONTHLY\n";
        printOHLCtable(askMonthly);

        std::cout << "\nProduct: " << product << " | BIDS | MONTHLY\n";
        printOHLCtable(bidMonthly);
    }
}



// =======================================================
// Print Table
// =======================================================
void MerkelMain::printMinuteCandlestickTable(const std::vector<Candlestick>& candles,
                                             int startIndex, int endIndex)
{
    std::cout << "\n==================== MINUTE CANDLES ====================\n\n";

std::cout << std::left
          << std::setw(20) << "TIME"
          << std::setw(18) << "OPEN"
          << std::setw(18) << "HIGH"
          << std::setw(18) << "LOW"
          << std::setw(18) << "CLOSE"
          << "\n-------------------------------------------------------------------------------\n";

std::cout << std::fixed << std::setprecision(6);  // ← SET ONCE

for (int i = startIndex; i <= endIndex; ++i)
{
    const Candlestick& c = candles[i];

    std::cout << std::left
              << std::setw(20) << c.startTime
              << std::setw(18) << c.open
              << std::setw(18) << c.high
              << std::setw(18) << c.low
              << std::setw(18) << c.close
              << "\n";
}

std::cout << "-------------------------------------------------------------------------------\n";

}


// =======================================================
// Simulation
// =======================================================
void MerkelMain::simulateTradingActivity()
{
    std::cout << "\n--- Simulating Trading Activity ---\n";
    std::cout << "This will create 5 ASKs + 5 BIDs for EACH product using dataset time.\n";

    const std::string currentUser = userManager.getLoggedInUser().username;
    auto products = orderBook.getKnownProducts();

    for (const auto& product : products)
    {
        std::cout << "\nProduct: " << product << "\n";

        // --------------------------------------------------
        // MARKET SNAPSHOT (DATASET TIME)
        // --------------------------------------------------
        auto asks = orderBook.getOrders(product, currentTime, OrderType::ask);
        auto bids = orderBook.getOrders(product, currentTime, OrderType::bid);

        double minAsk = 0.0, maxAsk = 0.0;
        double minBid = 0.0, maxBid = 0.0;

        if (!asks.empty())
        {
            minAsk = maxAsk = asks[0].price;
            for (const auto& a : asks)
            {
                minAsk = std::min(minAsk, a.price);
                maxAsk = std::max(maxAsk, a.price);
            }
        }

        if (!bids.empty())
        {
            minBid = maxBid = bids[0].price;
            for (const auto& b : bids)
            {
                minBid = std::min(minBid, b.price);
                maxBid = std::max(maxBid, b.price);
            }
        }

        std::cout << "Market snapshot:\n";
        std::cout << "  Ask range : " << minAsk << " → " << maxAsk << "\n";
        std::cout << "  Bid range : " << minBid << " → " << maxBid << "\n";

        // --------------------------------------------------
        // REFERENCE PRICE (SAFE FALLBACK LOGIC)
        // --------------------------------------------------
        double referencePrice = 1.0;
        if (minAsk > 0.0 && maxBid > 0.0)
            referencePrice = (minAsk + maxBid) / 2.0;
        else if (minAsk > 0.0)
            referencePrice = minAsk;
        else if (maxBid > 0.0)
            referencePrice = maxBid;

        // --------------------------------------------------
        // CREATE SIMULATED ORDERS
        // --------------------------------------------------
        for (int i = 1; i <= 5; ++i)
        {
            double amount = 0.1; // wallet-safe amount
            double priceOffset = 0.002 * i; // deterministic variation

            double askPrice = referencePrice * (1.01 + priceOffset);
            double bidPrice = referencePrice * (0.99 - priceOffset);

            std::cout << "\nOrder set " << i << ":\n";
            std::cout << "  Sell check → requires " << amount << " "
                      << product.substr(0, product.find('/')) << "\n";
            std::cout << "  Buy check  → requires "
                      << (bidPrice * amount) << " "
                      << product.substr(product.find('/') + 1) << "\n";

            // ---------------- ASK (SELL) ----------------
            OrderBookEntry ask(
                askPrice,
                amount,
                currentTime,
                product,
                OrderType::ask,
                currentUser
            );

            if (wallet.canFulfillOrder(ask))
            {
                orderBook.insertOrder(ask);
                appendTransactionCSV(currentUser, "ASK", product, askPrice, amount, currentTime);
            }

            // ---------------- BID (BUY) ----------------
            OrderBookEntry bid(
                bidPrice,
                amount,
                currentTime,
                product,
                OrderType::bid,
                currentUser
            );

            if (wallet.canFulfillOrder(bid))
            {
                orderBook.insertOrder(bid);
                appendTransactionCSV(currentUser, "BID", product, bidPrice, amount, currentTime);
            }
        }

        // --------------------------------------------------
        // MATCH & EXECUTE TRADES
        // --------------------------------------------------
        auto sales = orderBook.matchAsksToBids(product, currentTime);

        for (const auto& sale : sales)
        {
            if (sale.username != currentUser)
                continue;

            wallet.processSale(sale);

            if (sale.orderType == OrderType::asksale)
                appendTransactionCSV(currentUser, "ASK_SALE", product, sale.price, sale.amount, currentTime);
            else if (sale.orderType == OrderType::bidsale)
                appendTransactionCSV(currentUser, "BID_SALE", product, sale.price, sale.amount, currentTime);
        }
    }

    wallet.saveToCSV("wallet.csv", currentUser);
    std::cout << "\nSimulation complete. Orders executed and wallet updated.\n";
}


// =======================================================
// OHLC SUMMARY TABLE 
// =======================================================
void printOHLCtable(const std::vector<Candlestick>& candles)
{
    if (candles.empty())
    {
        std::cout << "No data available.\n";
        return;
    }

    std::cout << std::left
              << std::setw(12) << "DATE"
              << std::setw(14) << "OPEN"
              << std::setw(14) << "HIGH"
              << std::setw(14) << "LOW"
              << std::setw(14) << "CLOSE"
              << "\n--------------------------------------------------------------\n";

    for (const auto& c : candles)
    {
        std::cout << std::left
                  << std::setw(12) << c.startTime
                  << std::setw(14) << c.open
                  << std::setw(14) << c.high
                  << std::setw(14) << c.low
                  << std::setw(14) << c.close
                  << "\n";
    }

    std::cout << "--------------------------------------------------------------\n";
}

void MerkelMain::showUserActivitySummary()
{
    std::ifstream file("transactions.csv");
    if (!file.is_open()) {
        std::cout << "No transaction data found.\n";
        return;
    }

    std::string currentUser = userManager.getLoggedInUser().username;
    std::string line;

    int askCount = 0;
    int bidCount = 0;
    double totalSpent = 0.0;

    std::map<std::string, int> askPerProduct;
    std::map<std::string, int> bidPerProduct;

    // Skip header
    std::getline(file, line);

    while (std::getline(file, line))
    {
        std::stringstream ss(line);
        std::string user, type, product, priceStr, amountStr, timestamp;

        std::getline(ss, user, ',');
        std::getline(ss, type, ',');
        std::getline(ss, product, ',');
        std::getline(ss, priceStr, ',');
        std::getline(ss, amountStr, ',');
        std::getline(ss, timestamp, ',');

        if (user != currentUser) continue;

        double price = std::stod(priceStr);
        double amount = std::stod(amountStr);

        if (type == "ASK") {
            askCount++;
            askPerProduct[product]++;
        }
        else if (type == "BID") {
            bidCount++;
            bidPerProduct[product]++;
            totalSpent += price * amount;
        }
    }

    file.close();

    // ================= OUTPUT =================
    std::cout << "\n=== USER ACTIVITY SUMMARY ===\n";
    std::cout << "User: " << currentUser << "\n\n";

    std::cout << "Total ASKs placed: " << askCount << "\n";
    std::cout << "Total BIDs placed: " << bidCount << "\n";
    std::cout << "Total USDT spent (BIDs): " << totalSpent << "\n";

    std::cout << "\n--- Breakdown by Product ---\n";
    for (const auto& p : askPerProduct)
        std::cout << p.first << " | ASKs: " << p.second << "\n";

    for (const auto& p : bidPerProduct)
        std::cout << p.first << " | BIDs: " << p.second << "\n";

    std::cout << "============================\n\n";
}
