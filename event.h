#pragma once

#include "event_base.h"

struct event;

namespace lightstep {
class Event {
 public:
  using Callback = void (*)(int socket, short what, void* context);

  Event(EventBase& event_base, int file_descriptor, short what,
        Callback callback, void* context);

  ~Event() noexcept;

 private:
  event* event_;
};
} // namespace lightstep
