#include "System/InventorySystem.h"

#include <algorithm>
#include <memory>

#include "Commands/InventoryCommand.h"
#include "Core/Event.h"
#include "Core/GEngine.h"

InventorySystem::InventorySystem(GEngine *engine)
    : engine(engine),
      addEventHandle(engine->GetDispatcher()->Subscribe<ItemAddEvent>(
          [this](const ItemAddEvent &e) { this->AddItem(e); })),
      consumeEventHandle(engine->GetDispatcher()->Subscribe<ItemConsumeEvent>(
          [this](const ItemConsumeEvent &e) { this->ConsumeItem(e); })),
      moveEventHandle(engine->GetDispatcher()->Subscribe<ItemMoveEvent>(
          [this](const ItemMoveEvent &e) { this->MoveItem(e); })) {}

void InventorySystem::AddItem(const ItemAddEvent &e) {
  if (e.amount == 0) return;
  Registry *reg = engine->GetRegistry();
  if (!reg->HasComponent<InventoryComponent>(e.target)) return;
  CommandQueue *cq = engine->GetCommandQueue();
  cq->Enqueue(std::make_unique<InventoryCommand>(e.target, e.item, e.amount));
}

void InventorySystem::ConsumeItem(const ItemConsumeEvent &e) {
  if (e.amount == 0) return;
  Registry *reg = engine->GetRegistry();
  if (!reg->HasComponent<InventoryComponent>(e.target)) return;
  InventoryComponent &inv = reg->GetComponent<InventoryComponent>(e.target);

  if (std::find_if(inv.items.begin(), inv.items.end(), [&e](const auto &p) {
        return p.first == e.item;
      }) == inv.items.end())
    return;

  CommandQueue *cq = engine->GetCommandQueue();
  cq->Enqueue(std::make_unique<InventoryCommand>(e.target, e.item, -e.amount));
}

void InventorySystem::MoveItem(const ItemMoveEvent &e) {
  if (e.amount <= 0) return;
  Registry *reg = engine->GetRegistry();
  if (!reg->HasComponent<InventoryComponent>(e.source)) return;
  InventoryComponent &source_inv =
      reg->GetComponent<InventoryComponent>(e.source);
  InventoryComponent &dest_inv = reg->GetComponent<InventoryComponent>(e.dest);

  if (std::find_if(source_inv.items.begin(), source_inv.items.end(),
                   [&e](const auto &p) { return p.first == e.item; }) ==
      source_inv.items.end())
    return;

  CommandQueue *cq = engine->GetCommandQueue();
  cq->Enqueue(std::make_unique<InventoryCommand>(e.dest, e.item, e.amount, e.source));
}
