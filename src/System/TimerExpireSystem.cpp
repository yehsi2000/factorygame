#include "System/TimerExpireSystem.h"

#include "Components/TimerComponent.h"
#include "Event.h"
#include "GEngine.h"
#include "Registry.h"

void TimerExpireSystem::Update() {
  auto view = engine->GetRegistry()->view<TimerExpiredTag>();
  for (auto entity : view) {
    auto& tag = engine->GetRegistry()->GetComponent<TimerExpiredTag>(entity);

    switch (tag.expiredId) {
      case TimerId::Interact:
        engine->GetDispatcher()->Publish(std::make_shared<InteractEvent>());
        break;
      default:
        break;
    }
    // 처리 후 태그 제거
    engine->GetRegistry()->RemoveComponent<TimerExpiredTag>(entity);
  }
}