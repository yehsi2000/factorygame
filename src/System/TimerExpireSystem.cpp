#include "System/TimerExpireSystem.h"

#include "Commands/ResourceMineCommand.h"
#include "Components/AssemblingMachineComponent.h"
#include "Components/MiningDrillComponent.h"
#include "Components/PlayerStateComponent.h"
#include "Components/TimerComponent.h"
#include "Core/CommandQueue.h"
#include "Core/Event.h"
#include "Core/EventDispatcher.h"
#include "Core/Registry.h"
#include "Core/TimerManager.h"

TimerExpireSystem::TimerExpireSystem(const SystemContext& context)
    : eventDispatcher(context.eventDispatcher),
      commandQueue(context.commandQueue),
      registry(context.registry),
      timerManager(context.timerManager) {}

void TimerExpireSystem::Update() {
  auto view = registry->view<TimerExpiredTag>();
  for (auto entity : view) {
    TimerId expiredId =
        registry->GetComponent<TimerExpiredTag>(entity).expiredId;
    // Remove the tag first
    registry->RemoveComponent<TimerExpiredTag>(entity);

    // Find the handle for the expired timer
    TimerHandle handle = INVALID_TIMER_HANDLE;
    auto& timerComp = registry->GetComponent<TimerComponent>(entity);
    handle = timerComp.timers[static_cast<int>(expiredId)];

    if (handle != INVALID_TIMER_HANDLE) {
      TimerInstance* timer = timerManager->GetTimer(handle);
      if (timer) {
        // Cleanup timer first
        if (timer->bIsRepeating) {
          timer->elapsed = 0.0f;
        } else {
          timerComp.timers[static_cast<int>(expiredId)] = INVALID_TIMER_HANDLE;
          timerManager->DestroyTimer(handle);
        }

        switch (expiredId) {
          // Mining from player or drill
          case TimerId::Mine:
            // Player mining Case
            if (registry->HasComponent<PlayerStateComponent>(entity)) {
              const auto& stat =
                  registry->GetComponent<PlayerStateComponent>(entity);

              commandQueue->Enqueue(std::make_unique<ResourceMineCommand>(
                  entity, stat.interactingEntity));
            }
            // Drill mining case
            else if (registry->HasComponent<MiningDrillComponent>(entity)) {
              const auto& drill =
                  registry->GetComponent<MiningDrillComponent>(entity);

              commandQueue->Enqueue(std::make_unique<ResourceMineCommand>(
                  entity, drill.oreEntity));
            }
            break;

          // Assembling machine done crafting
          case TimerId::AssemblingMachineCraft:
            if (registry->HasComponent<AssemblingMachineComponent>(entity)) {
              eventDispatcher->Publish(AssemblyCraftOutputEvent{entity});
            }
            break;

          default:
            break;
        }
      }
    }
  }
}

TimerExpireSystem::~TimerExpireSystem() = default;
