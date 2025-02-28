#pragma once

#include <functional>

#include "common/thread_utils.h"
#include "common/macros.h"
#include "common/tcp_server.h"

#include "order_server/client_request.h"
#include "order_server/client_response.h"
#include "order_server/fifo_sequencer.h"

namespace Exchange {
  class OrderServer {
  public:
    OrderServer(clientRequestLFQueue *client_requests,
                   clientResponseLFQueue *client_responses,
                   MEMarketUpdateLFQueue *market_updates);

    ~OrderServer();

    auto start() -> void;

    auto stop() -> void;

    auto processClientRequest(const MEClientRequest *client_request) noexcept {
      auto order_book = ticker_order_book_[client_request->ticker_id_];
      switch (client_request->type_) {
        case ClientRequestType::NEW: {
          order_book->add(client_request->client_id_, client_request->order_id_, client_request->ticker_id_,
                          client_request->side_, client_request->price_, client_request->qty_);
        }
          break;

        default: {
          FATAL("Received invalid client-request-type:" + clientRequestTypeToString(client_request->type_));
        }
          break;
      }
    }

    auto sendClientResponse(const MEClientResponse *client_response) noexcept {
      logger_.log("%:% %() % Sending %\n", __FILE__, __LINE__, __FUNCTION__, Common::getCurrentTimeStr(&time_str_), client_response->toString());
      auto next_write = outgoing_ogw_responses_->getNextToWriteTo();
      *next_write = std::move(*client_response);
      outgoing_ogw_responses_->updateWriteIndex();
    }

    auto sendMarketUpdate(const MEMarketUpdate *market_update) noexcept {
      logger_.log("%:% %() % Sending %\n", __FILE__, __LINE__, __FUNCTION__, Common::getCurrentTimeStr(&time_str_), market_update->toString());
      auto next_write = outgoing_md_updates_->getNextToWriteTo();
      *next_write = *market_update;
      outgoing_md_updates_->updateWriteIndex();
    }

    auto run() noexcept {
      logger_.log("%:% %() %\n", __FILE__, __LINE__, __FUNCTION__, Common::getCurrentTimeStr(&time_str_));
      while (run_) {
        const auto me_client_request = incoming_requests_->getNextToRead();
        if (LIKELY(me_client_request)) {
          logger_.log("%:% %() % Processing %\n", __FILE__, __LINE__, __FUNCTION__, Common::getCurrentTimeStr(&time_str_),
                      me_client_request->toString());
          processClientRequest(me_client_request);
          incoming_requests_->updateReadIndex();
        }
      }
    }

    OrderServer() = delete;

    OrderServer(const OrderServer &) = delete;

    OrderServer(const OrderServer &&) = delete;

    OrderServer &operator=(const OrderServer &) = delete;

    OrderServer &operator=(const OrderServer &&) = delete;

  private:
    OrderBookHashMap ticker_order_book_;

    ClientRequestLFQueue *incoming_requests_ = nullptr;
    ClientResponseLFQueue *outgoing_ogw_responses_ = nullptr;
    MEMarketUpdateLFQueue *outgoing_md_updates_ = nullptr;

    volatile bool run_ = false;

    std::string time_str_;
    Logger logger_;
  };
}
