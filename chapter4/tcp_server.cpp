#include "tcp_server.h"

namespace Common {
  auto TCPServer::addToEpollList(TCPSocket *socket) {
    epoll_event ev{EPOLLET | EPOLLIN, {reinterpret_cast<void *>(socket)}};
    return !epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, socket->socket_fd_, &ev);
  }

  auto TCPServer::listen(const std::string &iface, int port) -> void {
    epoll_fd_ = epoll_create(1);
    ASSERT(epoll_fd_ >= 0, "epoll_create() failed error:" + std::string(std::strerror(errno)));

    ASSERT(listener_socket_.connect("", iface, port, true) >= 0,
           "Listener socket failed to connect. iface:" + iface + " port:" + std::to_string(port) + " error:" +
           std::string(std::strerror(errno)));

    ASSERT(addToEpollList(&listener_socket_), "epoll_ctl() failed. error:" + std::string(std::strerror(errno)));
  }

  auto TCPServer::sendAndRecv() noexcept -> void {
    auto recv = false;

    std::for_each(receive_sockets_.begin(), receive_sockets_.end(), [&recv](auto socket) {
      recv |= socket->sendAndRecv();
    });
  }

  auto TCPServer::poll() noexcept -> void {
    const int max_events = 1 + send_sockets_.size() + receive_sockets_.size();

    const int n = epoll_wait(epoll_fd_, events_, max_events, 0);
    bool have_new_connection = false;
    for (int i = 0; i < n; ++i) {
      const auto &event = events_[i];
      auto socket = reinterpret_cast<TCPSocket *>(event.data.ptr);

      // check for new connections.
      if (event.events & EPOLLIN) {
        if (socket == &listener_socket_) {
          logger_.log(%:% %() % EPOLLIN listener_socket:%\n", __FILE__, __LINE__, __FUNCTION__,
                      Common::getCurrentTimeStr(&time_str_), socket->socket_fd_);
          have_new_connection = true;
          continue;
        }
        logger_.log(
