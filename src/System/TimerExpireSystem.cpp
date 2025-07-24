#include "System/TimerExpireSystem.h"

#include "Components/TimerComponent.h"
#include "Event.h"
#include "GEngine.h"
#include "Registry.h"

void TimerExpireSystem::Update() {
  auto view = engine->GetRegistry()->view<TimerExpiredTag>();
  for (auto entity : view) {
    auto& tag = engine->GetRegistry()->GetComponent<TimerExpiredTag>(entity);

    switch (tag.expiredId) {  // ID 기반 로직 (하드코딩 or 맵핑)
      case TimerId::Interact:
        engine->GetDispatcher()->Publish(std::make_shared<InteractEvent>());
        break;
      default:
        break;
    }
    // 처리 후 태그 제거 (다음 프레임에 다시 트리거 안 되게)
    engine->GetRegistry()->RemoveComponent<TimerExpiredTag>(entity);
  }
}