#pragma once

#include <array>
#include <sstream>
#include "common/types.h"

using namespace Common;

namespace Trading {
  enum class OMOrderState : uint8_t {
    INVALID = 0,
    PENDING_NEW = 1,
    LIVE = 2,
    PENDING_CANCEL = 3,
    DEAD = 4
  };

  inline auto OMOrderStateToString(OMOrderState side) -> std::string {
    switch (side) {
      case OMOrderState::PENDING_NEW:
        return "NEW";
      
      case ClientRequestType::INVALID:
        return "INVALID";
    }
    return "UNKNOWN";
  }

#pragma pack(push, 1)

  struct MEClientRequest {
    ClientRequestType type_ = ClientRequestType::INVALID;

    ClientId client_id_ = ClientId_INVALID;
    TickerId ticker_id_ = TickerId_INVALID;
    Orderid order_id_ = OrderId_INVALID;
    Side side_ = Side::INVALID;
    Price price_ = Price_INVALID;
    Qty qty_ = Qty_INVALID;

    auto toString() const {
      std::stringstream ss;
      ss << "MEClientRequest"
         << " ["
         << "type:" << clientRequestTypeToString(type_)
         << "client:" << clientIdToString(client_id_)
         << " ticker:" << tickerIdToString(ticker_id_)
         << " oid:" << orderIdToString(order_id_)
         << " side:" << sideToString(side_)
         << " qty:" << qtyToString(qty_)
         << " price:" << priceToString(price_)
         << "]";
    return ss.str();
    }
  };

  struct OMClientRequest {
    size_t seq_num_ = 0;
    MEClientRequest me_client_request_;

    auto toString() const {
      std::stringstream ss;
      ss << "OMClientRequest"
         << " ["
         << " seq:" << seq_num_
         << " " << me_client_request_.toString()
         << "]";
      return ss.str();
    }
  };

#pragma pack(pop)

  typedef LFQueue<MEClientRequest> ClientRequestLFQueue;
}
