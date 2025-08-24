#include "System/TimerExpireSystem.h"

#include "Commands/ResourceMineCommand.h"
#include "Components/AssemblingMachineComponent.h"
#include "Components/MiningDrillComponent.h"
#include "Components/PlayerStateComponent.h"
#include "Components/TimerComponent.h"
#include "Core/Event.h"
#include "Core/GEngine.h"
#include "Core/Registry.h"
#include "Core/TimerManager.h"

void TimerExpireSystem::Update() {
  Registry* reg = engine->GetRegistry();
  auto view = reg->view<TimerExpiredTag>();
  for (auto entity : view) {
    TimerId expiredId = reg->GetComponent<TimerExpiredTag>(entity).expiredId;
    // Remove the tag first
    engine->GetRegistry()->RemoveComponent<TimerExpiredTag>(entity);

    // Find the handle for the expired timer
    TimerHandle handle = INVALID_TIMER_HANDLE;
    auto& timerComp = reg->GetComponent<TimerComponent>(entity);
    handle = timerComp.timers[static_cast<int>(expiredId)];
    
    if (handle != INVALID_TIMER_HANDLE) {
      TimerInstance* timer = engine->GetTimerManager()->GetTimer(handle);
      if (timer) {
        // Cleanup timer first
        if (timer->isRepeating) {
          timer->elapsed = 0.0f;
        } else {
          timerComp.timers[static_cast<int>(expiredId)] =
              INVALID_TIMER_HANDLE;
          engine->GetTimerManager()->DestroyTimer(handle);
        }

        switch (expiredId) {
          // Mining from player or drill
          case TimerId::Mine:
            if (reg->HasComponent<PlayerStateComponent>(entity)) {
              const auto& stat =
                  reg->GetComponent<PlayerStateComponent>(entity);
              engine->GetCommandQueue()->Enqueue(
                  std::make_unique<ResourceMineCommand>(
                      entity, stat.interactingEntity));
            } else if (reg->HasComponent<MiningDrillComponent>(entity)) {
              const auto& drill =
                  reg->GetComponent<MiningDrillComponent>(entity);
              engine->GetCommandQueue()->Enqueue(
                  std::make_unique<ResourceMineCommand>(entity,
                                                        drill.oreEntity));
            }
            break;

          // Assembling machine done crafting
          case TimerId::AssemblingMachineCraft:
            if (reg->HasComponent<AssemblingMachineComponent>(entity)) {
              engine->GetDispatcher()->Publish(
                  AssemblyCraftOutputEvent{entity});
            }
            break;

          default:
            break;
        }
      }
    }
  }
}
