//
// Created by petr on 9/23/20.
//

#include "stacktrace_exception.h"
#include <utility>
#undef BACKWARD_HAS_BFD
#define BACKWARD_HAS_BFD 1
#include "backward.hpp"
#include "range/v3/view/enumerate.hpp"

using namespace backward;
using namespace std::string_literals;
using namespace std::string_view_literals;

stacktrace_exception::stacktrace_exception(std::string_view message) {
  constexpr auto STACKTRACE_SKIP_N = 4;
  const auto traces = get_trace(STACKTRACE_SKIP_N);
  auto ss = std::stringstream{};
  if (!message.empty()) { ss << fmt::format("An exception occurred: {}\n", message); }
  const auto CAUSED_BY = "Caused by:\n"sv;
  ss << CAUSED_BY;

  const auto padding = std::string(CAUSED_BY.size(), ' ');
  for (const auto &[idx, trace] : ranges::views::enumerate(traces)) {
    ss << fmt::format("{}#{} {} ({}:{})\n", padding, idx, trace.location, trace.filename,
                      trace.line_n);
  }
  what_stacktrace = ss.str();
}

const char *stacktrace_exception::what() const noexcept { return what_stacktrace.c_str(); }


std::vector<trace_data> get_trace(std::size_t skip_n) {
  auto result = std::vector<trace_data>{};
  auto stackTrace = StackTrace{};
  auto resolver = TraceResolver{};
  stackTrace.load_here();
  resolver.load_stacktrace(stackTrace);
  result.reserve(stackTrace.size());
  for (std::size_t i = skip_n; i < stackTrace.size(); ++i) {
    const ResolvedTrace trace = resolver.resolve(stackTrace[i]);
    result.emplace_back(trace.source.filename, trace.object_function, trace.source.line);
  }

  return result;
}
trace_data::trace_data(std::string filename, std::string location, uint32_t line_n)
    : filename(std::move(filename)), location(std::move(location)), line_n(line_n) {}

invalid_argument_exception::invalid_argument_exception(const std::string_view &message)
    : stacktrace_exception(message) {}
