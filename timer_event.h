#pragma once

#include <chrono>

#include "event_base.h"

struct event;

namespace lightstep {
class TimerEvent {
 public:
  using CallbackFunction = void (*)(void* context);

  template <class Rep, class Period>
  TimerEvent(const EventBase& event_base,
             std::chrono::duration<Rep, Period> interval,
             CallbackFunction callback, void* context)
      : TimerEvent{
            event_base,
            std::chrono::duration_cast<std::chrono::microseconds>(interval),
            callback, context} {}

  TimerEvent(const EventBase& event_base, std::chrono::microseconds interval,
             CallbackFunction callback, void* context);

  ~TimerEvent() noexcept;

  TimerEvent(TimerEvent&& other) = delete;
  TimerEvent(const TimerEvent&) = delete;

  TimerEvent& operator=(TimerEvent&& other) = delete;
  TimerEvent& operator=(const TimerEvent&) = delete;

  void Reset();

 private:
  event* event_;
  std::chrono::microseconds interval_;
  CallbackFunction callback_;
  void* context_;

  static void LibeventCallback(int /*socket*/, short /*what*/,
                               void* context) noexcept;
};
}  // namespace lightstep
