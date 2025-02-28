#pragma once

#include <sstream>

#include "common/types.h"

using namespace Common;

namespace Exchange {
  enum class MarketUpdateType : uint8_t {
    INVALID = 0,
    CLEAR = 1,
    ADD = 2,
    MODIFY = 3,
    CANCEL = 4,
    TRADE = 5,
    SNAPSHOT_START = 6,
    SNAPSHOT_END = 7
  };

  inline std::string 
