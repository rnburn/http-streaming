#include "timer_event.h"

#include <stdexcept>

#include "utility.h"

#include <event2/event.h>

namespace lightstep {
//------------------------------------------------------------------------------
// constructor
//------------------------------------------------------------------------------
TimerEvent::TimerEvent(const EventBase& event_base,
                       std::chrono::microseconds interval,
                       CallbackFunction callback, void* context)
    : interval_{interval}, callback_{callback}, context_{context} {
  event_ = event_new(event_base.libevent_handle(), -1, EV_PERSIST,
                     &TimerEvent::LibeventCallback, static_cast<void*>(this));
  if (event_ == nullptr) {
    throw std::runtime_error{"evtimer_new failed"};
  }

  auto tv = ToTimeval(interval_);
  auto rcode = event_add(event_, &tv);
  if (rcode != 0) {
    event_free(event_);
    throw std::runtime_error{"evtimer_add failed"};
  }
}

//------------------------------------------------------------------------------
// destructor
//------------------------------------------------------------------------------
TimerEvent::~TimerEvent() noexcept { event_free(event_); }

//------------------------------------------------------------------------------
// Reset
//------------------------------------------------------------------------------
void TimerEvent::Reset() {
  auto rcode = event_del(event_);
  if (rcode != 0) {
    throw std::runtime_error{"failed to delete event"};
  }
  auto tv = ToTimeval(interval_);
  rcode = event_add(event_, &tv);
  if (rcode != 0) {
    throw std::runtime_error{"failed to Reset event"};
  }
}

//------------------------------------------------------------------------------
// LibeventCallback
//------------------------------------------------------------------------------
void TimerEvent::LibeventCallback(int /*socket*/, short /*what*/,
                                  void* context) noexcept {
  auto& timer_event = *static_cast<TimerEvent*>(context);
  timer_event.callback_(timer_event.context_);
}
}  // namespace lightstep
