#pragma oncea

#include <functional>

#include "socket_utils.h"
#include "logging.h"

namespace Common {
  /// Size of our send and receive buffers in bytes.
  constexpr size_t TCPBufferSize = 64 * 1024 * 1024;

  struct TCPSocket {
    explicit TCPSocket(Logger &logger)
        : logger_(logger) {
      outbound_data_.resize(TCPBufferSize);
      inbound_data_.resize(TCPBufferSize);
    }




    /// File descriptor for the socket.
    int socket_fd_ = -1;

    /// Send and receive buffers and trackers for read/write indices.
    std::vector<char> outbound_data_;
    size_t next_send_valid_index_ = 0; 
    std::vector<char> inbound_data_;
    size_t next_recv_valid_index_ = 0; 

    /// Socket attributes.
    struct sockaddr_in socket_attrib_{};

    /// Function wrapper to callback when there is data to be processed.
    std::function<void(TCPSocket *s, Nanos rx_time)> recv_callback_ = nullptr;

    std::string time_str_;
    Logger &logger_;
  };
}
