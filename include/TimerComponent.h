#include <functional>

struct TimerComponent {
  double remaining;
  double duration;
  bool bIsRepeating;
  std::function<void()> onExpire;
  TimerComponent(double time, bool bRepeat, std::function<void()> callback) {
    remaining = time;
    duration = time;
    bIsRepeating = bRepeat;
    onExpire = std::move(callback);
  }
};