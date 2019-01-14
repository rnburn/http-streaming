#include "utility.h"

namespace lightstep {
//------------------------------------------------------------------------------
// ToTimeval
//------------------------------------------------------------------------------
timeval ToTimeval(std::chrono::microseconds microseconds) {
  timeval result;
  auto num_microseconds = microseconds.count();
  const size_t microseconds_in_second = 1000000;
  result.tv_sec =
      static_cast<time_t>(num_microseconds / microseconds_in_second);
  result.tv_usec =
      static_cast<suseconds_t>(num_microseconds % microseconds_in_second);
  return result;
}
} // namespace lightstep
