#pragma once

#include "common/macros.h"
#include "common/types.h"
#include "common/logging.h"

#include "exchange/order_server/client_response.h"

#include "market_order_book.h"

using namespace Common;

namespace Trading {
  struct PositionInfo {
    int32_t position_ = 0;
    double real_pnl_ = 0, unreal_pnl_ = 0, total_pnl_ = 0;
    std::array<double, sideToIndex(Side::MAX) + 1> open_vwap_;
    Qty volume_ = 0;
    const BBO *bbo_ = nullptr;

    auto toString() const {
      std::stringstream ss;
      ss << "Position{"
         << "pos:" << position_
         << " u-pnl:" << unreal_pnl_
         << " r-pnl:" << real_pnl_
         << " t-pnl:" << total_pnl_
         << " vol:" << qtyToString(volume_)
         << " vwaps:[" << (position_ ? open_vwap_.at(sideToIndex(Side::BUY)) / std::abs(position_) : 0)
         << "X" << (position_ ? open_vwap_.at(sideToIndex(Side::SELL)) / std::abs(position_) : 0)
         << "] "
         << (bbo_ ? bbo_->toString() : "") << "}";

    return ss.str();
    }

    auto addFill(const Exchange::MEClientResponse *client_response, Logger *logger) noexcept {
      const auto old_position = position_;
      const auto side_index = sideToIndex(client_response->side_);
      const auto opp_side_index = sideToIndex(client_response->side_ == Side::BUY ? Side::SELL : Side::BUY);
      const auto side_value = sideToValue(client_response->side_);
      position_ += client_response->exec_qty_ * side_value;
      volume_ += client_response->exec_qty_;

      if (old_position * sideToValue(client_response->side_) >= 0) { // opened / increased position.
        open_vwap_[side_index] += (client_response->price_ * client_response->exec_qty_);
      } else { // decreased position.
        const auto opp_side_vwap = open_vwap_[opp_side_index] / std::abs(old_position);
        open_vwap_[opp_side_index] = opp_side_vwap * std::abs(position_);
        real_pnl_ += std::min(static_cast<int32_t>(client_response->exec_qty_), std::abs(old_position)) *
                     (opp_side_vwap - client_response->price_) * sideToValue(client_response->side_);
        if (position_ * old_position < 0) { // flipped position to opposite sign
