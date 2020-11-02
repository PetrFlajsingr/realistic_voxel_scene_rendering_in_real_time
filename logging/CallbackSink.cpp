//
// Created by petr on 11/2/20.
//

#include "CallbackSink.h"
#include <mutex>
#include <spdlog/details/pattern_formatter.h>

void CallbackSink::log(const spdlog::details::log_msg &msg) {
  spdlog::memory_buf_t formatted;
  formatter->format(msg, formatted);
  callback(std::string(formatted.data(), formatted.size()));
}
void CallbackSink::flush() {}
void CallbackSink::set_pattern(const std::string &pattern) {
  formatter = std::unique_ptr<spdlog::formatter>(new spdlog::pattern_formatter(pattern));
}
void CallbackSink::set_formatter(std::unique_ptr<spdlog::formatter> sink_formatter) {
  formatter = std::move(sink_formatter);
}
