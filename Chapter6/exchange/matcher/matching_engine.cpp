#include "matching_engine.h"

namespace Exchange {
  MatchingEngine::MatchingEngine(ClientRequestLFQueue *client_requests, ClientResponseLFQueue *client_responses,
                                 MEMarketUpdateLFQueue *market_updates)
      : incoming_requests_(client_requests), outgoing_ogw_responses_(client_responses), outgoing_md_updates_(market_updates),
        logger_("exchange_matching_engine.log") {
    for (size_t i = 0; i < ticker_order_book.size(); ++i) {
      ticker_order_book_[i] = new MEOrderBook(i, &logger_, this);
    }
  }

  MatchingEngine::~MatchingEngine() {
    run_ = false;

    using namespace std::literals::chrono_literals;
    std::this_thread::sleep_for(1s);

    
}
