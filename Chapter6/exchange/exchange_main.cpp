#include <csignal>

#include "matcher/matching_engine.h"

Common::Logger* logger = nullptr;
Exchange::MatchingEngine* matching_engine = nullptr;

void signal_handler(int) {
  using namespace std::literals::chrono_literals;
  std::this
