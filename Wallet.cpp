#include "Wallet.h"
#include "CSVReader.h"

#include <sstream>
#include <fstream>
#include <vector>
#include <iostream>

// =======================================================
// INTERNAL HELPERi
// =======================================================
static void ensureWalletCSV(const std::string& filename)
{
    std::ifstream in(filename);
    if (!in.good())
    {
        std::ofstream out(filename);
        out << "username,currency,amount\n";
        return;
    }

    in.seekg(0, std::ios::end);
    if (in.tellg() == 0)
    {
        std::ofstream out(filename);
        out << "username,currency,amount\n";
    }
}

// =======================================================
// Wallet core logic 
// =======================================================
void Wallet::insertCurrency(const std::string& type, double amount)
{
    if (amount < 0) return;
    currencies[type] += amount;
}

bool Wallet::containsCurrency(const std::string& type, double amount) const
{
    if (amount < 0) return false;

    auto it = currencies.find(type);
    if (it == currencies.end()) return false;

    return it->second >= amount;
}

bool Wallet::removeCurrency(const std::string& type, double amount)
{
    if (amount < 0) return false;

    auto it = currencies.find(type);
    if (it == currencies.end()) return false;
    if (it->second < amount) return false;

    it->second -= amount;
    return true;
}

std::string Wallet::toString() const
{
    std::ostringstream oss;
    oss << "Wallet contents:\n";

    if (currencies.empty()) {
        oss << "  (empty)\n";
        return oss.str();
    }

    for (const auto& pair : currencies)
        oss << "  " << pair.first << " : " << pair.second << "\n";

    return oss.str();
}

bool Wallet::canFulfillOrder(const OrderBookEntry& order) const
{
    auto tokens = CSVReader::tokenize(order.product, '/');
    if (tokens.size() != 2) return false;

    const std::string& base  = tokens[0];
    const std::string& quote = tokens[1];

    if (order.orderType == OrderType::ask)
        return containsCurrency(base, order.amount);

    if (order.orderType == OrderType::bid)
        return containsCurrency(quote, order.price * order.amount);

    return false;
}

void Wallet::processSale(const OrderBookEntry& sale)
{
    auto tokens = CSVReader::tokenize(sale.product, '/');
    if (tokens.size() != 2) return;

    const std::string& base  = tokens[0];
    const std::string& quote = tokens[1];

    if (sale.orderType == OrderType::asksale)
    {
        removeCurrency(base, sale.amount);
        insertCurrency(quote, sale.amount * sale.price);
    }
    else if (sale.orderType == OrderType::bidsale)
    {
        removeCurrency(quote, sale.amount * sale.price);
        insertCurrency(base, sale.amount);
    }
}

// =======================================================
// CSV PERSISTENCE (PER USER)
// =======================================================
void Wallet::loadFromCSV(const std::string& filename,
                         const std::string& username)
{
    currencies.clear();

    std::ifstream file(filename);
    if (!file.is_open()) return;

    std::string line;
    std::getline(file, line); // skip header

    while (std::getline(file, line))
    {
        std::stringstream ss(line);
        std::string user, currency, amountStr;

        std::getline(ss, user, ',');
        std::getline(ss, currency, ',');
        std::getline(ss, amountStr, ',');

        if (user == username)
            currencies[currency] = std::stod(amountStr);
    }
}

void Wallet::saveToCSV(const std::string& filename,
                       const std::string& username)
{
    ensureWalletCSV(filename);

    std::vector<std::string> lines;
    std::ifstream in(filename);

    // Keep all rows EXCEPT this user's
    if (in.is_open())
    {
        std::string line;
        std::getline(in, line); // skip header

        while (std::getline(in, line))
        {
            if (line.rfind(username + ",", 0) != 0)
                lines.push_back(line);
        }
        in.close();
    }

    std::ofstream out(filename);
    out << "username,currency,amount\n";

    for (const auto& l : lines)
        out << l << "\n";

    for (const auto& pair : currencies)
    {
        out << username << ","
            << pair.first << ","
            << pair.second << "\n";
    }
}

bool Wallet::existsInCSV(const std::string& filename,
                         const std::string& username)
{
    std::ifstream file(filename);
    if (!file.is_open()) return false;

    std::string line;
    while (std::getline(file, line))
    {
        if (line.rfind(username + ",", 0) == 0)
            return true;
    }
    return false;
}
