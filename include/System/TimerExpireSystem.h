#ifndef SYSTEM_TIMEREXPIRESYSTEM_
#define SYSTEM_TIMEREXPIRESYSTEM_

class GEngine;

class TimerExpireSystem {
  GEngine* engine;

 public:
  TimerExpireSystem(GEngine* e) : engine(e) {}
  void Update();
};

#endif /* SYSTEM_TIMEREXPIRESYSTEM_ */
