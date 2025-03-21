#include "market_data_consumer.h"

namespace Trading {
  MarketDataConsumer::MarketDataConsumer(Common::ClientId client_id, Exchange::MEMarketUpdateLFQueue *market_updates,
                                         const std::string &iface,
                                         const std::string &snapshot_ip, int snapshot_port,
                                         const std::string &incremental_ip, int incremental_port)
      : incoming_md_updates_(market_updates), run_(false),
        logger_("trading_market_data_consumer_" + std::to_string(client_id) + ".log"),
        incremental_mcast_socket_(logger_), snapshot_mcast_socket_(logger_),
        iface_(iface), snapshot_ip_(snapshot_ip), snapshot_port_(snapshot_port) {
    auto recv_callback = [this](auto socket) {
      recvCallback(socket);
    };

    incremental_mcast_socket_.recv_callback_ = recv_callback;
    ASSERT(incremental_mcast_socket_.init(incremental_ip, iface, incremental_port, /*is_listening*/ true) >= 0,
           "Unable to create incremental mcast socket. error:" + std::string(std::strerror(errno)));

    ASSERT(incremental_mcast_socket_.join(incremental_ip),
           "Join failed on:" + std::to_string(incremental_mcast_socket_.socket_fd_) + " error:" + std::string(std::strerror(errno)));

    snapshot_mcast_socket_.recv_callback_ = recv_callback;
  }

  auto MarketDataConsumer::run() noexcept -> void {
    logger_.log("%:% %() %\n", __FILE__, __LINE__, __FUNCTION__, Common::getCurrentTimeStr(&time_str_));
    while (run_) {
      incremental_mcast_socket_.sendAndRecv();
      snapshot_mcast_socket_.sendAndRecv();
    }
  }

  auto MarketDataConsumer::startSnapshotSync() -> void {
    snapshot_queued_msgs_.clear();
    incremental_queued_msgs_.clear();

     ASSERT(snapshot_mcast_socket_.init(snapshot_ip_, iface_, snapshot_port_, /*is_listening*/ true) >= 0,
           "Unable to create snapshot mcast socket. error:" + std::string(std::strerror(errno)));
    ASSERT(snapshot_mcast_socket_.join(snapshot_ip_), // IGMP mulitcast subscription.
           "Join failed on:" + std::to_string(snapshot_mcast_socket_.socket_fd_) + " error:" + std::string(std::strerror(errno)));
  }

  auto MarketDataConsumer::checkSnapshotSync() -> void {
    if (snapshot_queued_msgs_.empty()) {
      return;
    }

    const auto &first_snapshot_msg = snapshot_queued_msgs_.begin()->second;
    if (first_snapshot_msg.type_ != Exchange::MarketUpdateType::SNAPSHOT_START) {
      logger_.log("%:% %() % Returning because have not seen a SNAPSHOT_START yet.\n",
                  __FILE__, __LINE__, __FUNCTION__, Common::getCurrentTimeStr(&time_str_));
      snapshot_queued_msgs_.clear();
      return;
    }
    
    std::vector<Exchange::MEMarketUpdate> final_events;

    auto have_complete_snapshot = true;
    size_t next_snapshot_seq = 0;
    for (auto &snapshot_itr: snapshot_queued_msgs_) {
      logger_.log("%:% %() % % => %\n", __FILE__, __LINE__, __FUNCTION__, 
                  Common::getCurrentTimeStr(&time_str_), snapshot_itr.first, snapshot_itr.second.toString());
      if (snapshot_itr.first != next_snapshot_seq) {
        have_complete_snapshot = false;
        logger_.log("%:% %() % Detected gap in snapshot stream expected:% found:% %.\n", __FILE__, __LINE__, __FUNCTION__, 
                  Common::getCurrentTimeStr(&time_str_), next_snapshot_seq, snapshot_itr.first,
                  snapshot_itr.second.toString());
        break;
      }

      if (snapshot_itr.second.type_ != Exchange::MarketUpdateType::SNAPSHOT_START &&
          snapshot_itr.second.type_ != Exchange::MarketUpdateType::SNAPSHOT_END)
        final_events.push_back(snapshot_itr.second);

      ++next_snapshot_seq;
  }

  if (!have_complete_snapshot) {
        logger_.log("%:% %() % Returning because found gaps in snapshot stream.\n", 
                    __FILE__, __LINE__, __FUNCTION__, Common::getCurrentTimeStr(&time_str_));
        snapshot_queued_msgs_.clear();
        return;
  }

  const auto &last_snapshot_msg = snapshot_queued_msgs_.rbegin()->second;
  if (last_snapshot_msg.type_ != Exchange::MarketUpdateType::SNAPSHOT_END) {
    logger_.log("%:% %() % Returning because have not seen a SNAPSHOT_END yet.\n", 
                __FILE__, __LINE__, __FUNCTION__, Common::getCurrentTimeStr(&time_str_));
    return;
  }

  auto have_complete_incremental = true;
  size_t num_incrementals = 0;
  next_exp_inc_seq_num_ = last_snapshot_msg.order_id_ + 1;
  for (auto inc_itr = incremental_queued_msgs_.begin(): inc_itr != incremental_queued_msgs_.end(); ++inc_itr) {
    logger_.log("%:% %() % Checking next_exp:% vs. seq:% %.\n", __FILE__, __LINE__, __FUNCTION__, 
                Common::getCurrentTimeStr(&time_str_), next_exp_inc_seq_num_, inc_itr->first, inc_itr->second.toString());

  if (inc_itr->first < next_exp_inc_seq_num_) 
    continue;

  if (inc_itr->first != next_exp_inc_seq_num_) {
    logger_.log("%:% %() % Detected gap in incremental stream expected:% found:% %.\n", __FILE__, __LINE__, __FUNCTION__, 
                Common::getCurrentTimeStr(&time_str_), next_exp_inc_seq_num_, inc_itr->first, inc_itr->second.toString());
    have_complete_incremental = false;
    break;
  }

  logger_.log("%:% %() % Checking next_exp:% vs. seq:% %.\n", __FILE__, __LINE__, __FUNCTION__, 
                Common::getCurrentTimeStr(&time_str_), next_exp_inc_seq_num_, inc_itr->first, inc_itr->second.toString());

    
  
