#include "OrderBookEntry.h"

OrderBookEntry::OrderBookEntry(double _price,
                               double _amount,
                               const std::string& _timestamp,
                               const std::string& _product,
                               OrderType _orderType,
                               const std::string& _username)
    : price{_price},
      amount{_amount},
      timestamp{_timestamp},
      product{_product},
      orderType{_orderType},
      username{_username} {}
