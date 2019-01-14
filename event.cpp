#include "event.h"

#include <stdexcept>

#include "utility.h"

#include <event2/event.h>

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------------------------
Event::Event(EventBase& event_base, int file_descriptor, short what,
             Callback callback, void* context) {
  event_ = event_new(event_base.libevent_handle(), file_descriptor, what | EV_PERSIST,
                     callback, static_cast<void*>(context));
  if (event_ == nullptr) {
    throw std::runtime_error{"evtimer_new failed"};
  }
  auto rcode = event_add(event_, nullptr);
  if (rcode != 0) {
    event_free(event_);
    throw std::runtime_error{"event_add failed"};
  }
}

//--------------------------------------------------------------------------------------------------
// destructor
//--------------------------------------------------------------------------------------------------
Event::~Event() noexcept { event_free(event_); }
}  // namespace lightstep
