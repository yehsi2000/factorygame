#include "System/InventorySystem.h"

#include <algorithm>
#include <memory>

#include "Commands/InventoryCommand.h"
#include "Components/InventoryComponent.h"
#include "Core/CommandQueue.h"
#include "Core/Event.h"
#include "Core/EventDispatcher.h"
#include "Core/Registry.h"

InventorySystem::InventorySystem(const SystemContext &context)
    : registry(context.registry),
      eventDispatcher(context.eventDispatcher),
      commandQueue(context.commandQueue) {
  addEventHandle = eventDispatcher->Subscribe<ItemAddEvent>(
      [this](const ItemAddEvent &e) { this->AddItem(e); });

  consumeEventHandle = eventDispatcher->Subscribe<ItemConsumeEvent>(
      [this](const ItemConsumeEvent &e) { this->ConsumeItem(e); });

  moveEventHandle = eventDispatcher->Subscribe<ItemMoveEvent>(
      [this](const ItemMoveEvent &e) { this->MoveItem(e); });
}

void InventorySystem::AddItem(const ItemAddEvent &e) {
  if (e.amount == 0 || !registry->HasComponent<InventoryComponent>(e.target))
    return;

  return commandQueue->Enqueue(
      std::make_unique<InventoryCommand>(e.target, e.item, e.amount));
}

void InventorySystem::ConsumeItem(const ItemConsumeEvent &e) {
  if (e.amount == 0 || !registry->HasComponent<InventoryComponent>(e.target))
    return;

  auto &inv = registry->GetComponent<InventoryComponent>(e.target);

  if (std::find_if(inv.items.begin(), inv.items.end(), [&e](const auto &p) {
        return p.first == e.item;
      }) == inv.items.end())
    return;
  commandQueue->Enqueue(
      std::make_unique<InventoryCommand>(e.target, e.item, -e.amount));
}

void InventorySystem::MoveItem(const ItemMoveEvent &e) {
  if (e.amount <= 0) return;
  if (!registry->HasComponent<InventoryComponent>(e.source)) return;
  auto &source_inv = registry->GetComponent<InventoryComponent>(e.source);
  auto &dest_inv = registry->GetComponent<InventoryComponent>(e.dest);

  if (std::find_if(source_inv.items.begin(), source_inv.items.end(),
                   [&e](const auto &p) { return p.first == e.item; }) ==
      source_inv.items.end())
    return;
  commandQueue->Enqueue(
      std::make_unique<InventoryCommand>(e.dest, e.item, e.amount, e.source));
}

InventorySystem::~InventorySystem() = default;