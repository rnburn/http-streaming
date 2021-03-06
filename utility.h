#include <utility>
#include <chrono>

#include <sys/time.h>

namespace lightstep {
timeval ToTimeval(std::chrono::microseconds microseconds);

// FinalAction and finally
//
// Taken from
// https://github.com/Microsoft/GSL/blob/cea0d0ac2bd775f0fb4c7e357a089979370ae3cd/include/gsl/gsl_util
//
// FinalAction allows you to ensure something gets run at the end of a scope
template <class F>
class FinalAction {
 public:
  explicit FinalAction(F f) noexcept : f_(std::move(f)) {}

  FinalAction(FinalAction&& other) noexcept
      : f_(std::move(other.f_)), invoke_(other.invoke_) {
    other.invoke_ = false;
  }

  FinalAction(const FinalAction&) = delete;
  FinalAction& operator=(const FinalAction&) = delete;
  FinalAction& operator=(FinalAction&&) = delete;

  ~FinalAction() noexcept {
    if (invoke_) f_();
  }

 private:
  F f_;
  bool invoke_{true};
};

// finally() - convenience function to generate a FinalAction
template <class F>
FinalAction<F> finally(F&& f) noexcept {
  return FinalAction<F>(std::forward<F>(f));
}
} // namespace lightstep
