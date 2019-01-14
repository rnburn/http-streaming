#include <cassert>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <random>
#include <sstream>
#include <iomanip>

#include <arpa/inet.h>
#include <unistd.h>

#include <event2/event.h>

#include "event.h"
#include "event_base.h"
#include "socket.h"
#include "timer_event.h"

using namespace lightstep;

static Socket connect() {
  Socket socket;
  ::sockaddr_in address = {};
  address.sin_family = AF_INET;
  address.sin_port = htons(9000);
  auto rcode = ::inet_pton(AF_INET, "127.0.0.1", &(address.sin_addr));
  assert(rcode == 1);
  rcode = ::connect(socket.file_descriptor(),
                    reinterpret_cast<sockaddr*>(&address), sizeof address);
  if (rcode != 0) {
    std::cerr << "connect failed: " << std::strerror(errno) << "\n";
    std::abort();
  }
  socket.SetNonblocking();
  socket.SetReuseAddress();
  return socket;
}

const char* const StreamHeader =
    "POST / HTTP/1.1\r\n"
    "Content-Type: text/plain\r\n"
    "Transfer-Encoding: chunked\r\n"
    "Connection: keep-alive\r\n\r\n";

struct Context {
  bool exit{false};
  EventBase* event_base;
  std::string stream{StreamHeader};
};

static std::string GenerateData() {
  static std::mt19937 rng{0};
  std::bernoulli_distribution digit_dist{0.5};
  std::uniform_int_distribution<size_t> size_dist{1, 10000000};
  auto size = size_dist(rng);
  std::string result(size, '0');
  for (int i=0; i<static_cast<int>(size); ++i) {
    if (digit_dist(rng)) {
      result[i] = '1';
    } 
  }
  return result;
}

static void OnTick(void* context_void) {
  auto& context = *static_cast<Context*>(context_void);
  if (context.exit) {
    if (context.stream.empty()) {
      context.event_base->LoopBreak();
    }
    return;
  }
  auto data = GenerateData();
  std::ostringstream oss;
  oss << std::hex << data.size() << "\r\n";
  context.stream.append(oss.str());
  context.stream.append(data);
  context.stream.append("\r\n");
}

static void OnFinish(int /*socket*/, short /*what*/, void* context_void) {
  auto& context = *static_cast<Context*>(context_void);
  // Add terminating characters for the stream.
  context.stream.append("0\r\n\r\n");
  context.exit = true;
}

static void OnSocketReady(int socket, short /*what*/, void* context_void) {
  auto& context = *static_cast<Context*>(context_void);
  if (context.stream.empty()) {
    return;
  }
  auto num_written =
      ::write(socket, static_cast<const void*>(context.stream.data()),
              context.stream.size());
  if (num_written <= 0) {
    std::cerr << "unexpected write return value: " << num_written << "\n";
    std::terminate();
  }
  std::cout << "write " << num_written << "/" << context.stream.size()
            << std::endl;
  context.stream.erase(0, num_written);
}

int main() {
  Context context;
  EventBase event_base;
  context.event_base = &event_base;

  TimerEvent ticker{event_base, std::chrono::milliseconds{10}, OnTick,
                    static_cast<void*>(&context)};

  event_base.OnTimeout(std::chrono::seconds{3}, OnFinish,
                       static_cast<void*>(&context));

  auto socket = connect();

  Event socket_write{event_base, socket.file_descriptor(), EV_WRITE,
                     OnSocketReady, static_cast<void*>(&context)};
  event_base.Dispatch();
  return 0;
}
