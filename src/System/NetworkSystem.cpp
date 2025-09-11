#include "System/NetworkSystem.h"

#include "Core/Event.h"
#include "Core/EventDispatcher.h"

#include <iostream>

NetworkSystem::NetworkSystem(const SystemContext& context)
    : assetManager(context.assetManager),
      eventDispatcher(context.eventDispatcher),
      registry(context.registry),
      world(context.world) {
  sendChatHandle = eventDispatcher->Subscribe<SendChatEvent>(
      [this](SendChatEvent e) { SendChat(e.message); });
}

void NetworkSystem::Update(float deltatime) {}

void NetworkSystem::SendChat(std::shared_ptr<std::string> chat){
  std::cout<<*chat.get()<<std::endl;
}

NetworkSystem::~NetworkSystem() = default;