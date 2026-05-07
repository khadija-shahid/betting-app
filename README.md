# Trading App

A C++ command-line application that simulates a cryptocurrency trading environment. Users can log in, view live order book data, place bids and asks, and track their wallet balance. The app parses real market data from CSV files, renders candlestick charts, and manages transaction history — built as an object-oriented programming project using core OOP principles.

---

## Features

- **User authentication** — register and log in via a CSV-backed user management system
- **Order book** — view and interact with real bid/ask market data parsed from CSV
- **Place trades** — enter bids and asks across multiple cryptocurrency products
- **Wallet management** — deposit, withdraw, and track your balance across sessions
- **Candlestick charts** — view minute-based OHLC (Open, High, Low, Close) tables per product
- **Transaction history** — all trades are logged with timestamps to `transactions.csv`
- **Market stats** — view product summaries and user activity reports
- **Time simulation** — step through market timeframes to simulate live trading

---

## File Structure

```
betting-app/
├── main.cpp                  # Entry point
├── MerkelMain.h / .cpp       # Core app loop, menu, and trading logic
├── OrderBook.h / .cpp        # Order book parsing and matching
├── OrderBookEntry.h / .cpp   # Individual order data model
├── CSVReader.h / .cpp        # CSV file parser for market data
├── Candlestick.h / .cpp      # OHLC candlestick data model
├── CandlestickPlot.h / .cpp  # Candlestick table renderer
├── Wallet.h / .cpp           # Wallet balance and operations
├── User.h / .cpp             # User data model
├── UserManager.h / .cpp      # User registration and login
├── users.csv                 # Stored user accounts
├── wallet.csv                # Wallet balances
├── transactions.csv          # Trade history log
└── 20200601.csv              # Market data (not tracked in git)
```

---

## How to Run

### Prerequisites

- A C++ compiler supporting C++11 or later (e.g. `g++`, `clang++`)
- A terminal (Linux, macOS, or Windows with WSL/MinGW)

### Compile

```bash
g++ -std=c++11 -o betting-app main.cpp MerkelMain.cpp OrderBook.cpp OrderBookEntry.cpp CSVReader.cpp Candlestick.cpp CandlestickPlot.cpp Wallet.cpp User.cpp UserManager.cpp
```

### Run

```bash
./betting-app
```

---

## Usage

Once running, you will be prompted to log in or register. After authentication, use the numbered menu to:

1. View market stats
2. Enter a bid
3. Enter an ask
4. View your wallet
5. Go to the next time frame
6. View candlestick data
7. View transaction history

---

## Built With

- **C++11** — core language
- **Standard Library** — `<fstream>`, `<vector>`, `<map>`, `<chrono>`, `<algorithm>`
- **CSV files** — used for persistent storage of users, wallets, and transactions

---

## Author

Built as part of an Object-Oriented Programming coursework project.
