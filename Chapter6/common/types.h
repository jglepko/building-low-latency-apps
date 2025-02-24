#pragma once

#include <cstdint>
#include <limits>

#include "common/macros.h"

namespace Common {
  constexpr size_t ME_MAX_TICKERS = 8;

  constexpr size_t ME_MAX_CLIENT_UPDATES = 256 * 1024;
  constexpr size_t ME_MAX_MARKET_UPDATES = 256 * 1024;

  constexpr size_t ME_MAX_NUM_CLIENTS = 256;
  constexpr size_t ME_MAX_ORDER_IDS = 1024 * 1024;
  constexpr size_t ME_MAX_TICKERS = 256;

  typedef uint64_t OrderId;
  constexpr auto OrderId_INVALID = std::numeric_limits<OrderId>::max();

  inline auto orderIdToString(OrderId order_id) -> std::string {
    if (UNLIKELY(order_id == OrderId_INVALID)) {
      return "INVALID";
    }

    return std::to_string(order_id);
  }

  typedef uint32_t TickerId;
  constexpr auto TickerId_INVALID = std::numeric_limits<TickerId>::max();

  inline auto tickerIdToString(TickerId ticker_id) -> std::string {
    if (UNLIKELY(ticker_id == OrderId_INVALID)) {
      return "INVALID";
    }

    return std::to_string(order_id);
  }

  
