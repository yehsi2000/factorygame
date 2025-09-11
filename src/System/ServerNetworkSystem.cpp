#include "System/ServerNetworkSystem.h"

#include <iostream>
#include <memory>

#include "Core/Event.h"
#include "Core/EventDispatcher.h"
#include "Core/Packet.h"
#include "Core/Server.h"
#include "Core/ThreadSafeQueue.h"
#include "Util/PacketUtil.h"

ServerNetworkSystem::ServerNetworkSystem(const SystemContext& context)
    : assetManager(context.assetManager),
      eventDispatcher(context.eventDispatcher),
      registry(context.registry),
      world(context.world),
      timerManager(context.timerManager),
      packetQueue(context.packetQueue),
      sendQueue(context.sendQueue),
      server(context.server) {
  sendChatHandle = eventDispatcher->Subscribe<SendChatEvent>(
      [this](SendChatEvent e) { Broadcast(util::ChatBroadcastPacket(e.message)); });
}

void ServerNetworkSystem::Update(float deltatime) {
  PacketPtr packet;
  while (packetQueue->TryPop(packet)) {
    PacketHeader* header = reinterpret_cast<PacketHeader*>(packet.get());
    switch (header->packet_id) {
      case CHAT_CLIENT:
        eventDispatcher->Publish(NewChatEvent(std::make_shared<std::string>(
            reinterpret_cast<char*>(packet.get()) + sizeof(PacketHeader) + sizeof(uintptr_t),
            header->packet_size - sizeof(PacketHeader) - sizeof(uintptr_t))));
        Broadcast(std::move(packet));
        break;
    }
  }
}

void ServerNetworkSystem::Unicast(uintptr_t clientId, PacketPtr packet) {
  SendRequest request;
  request.type = ESendType::UNICAST;
  request.targetClientId = clientId;
  request.packet = std::move(packet);
  sendQueue->Push(std::move(request));
  server->StartSend();
}

void ServerNetworkSystem::Broadcast(PacketPtr packet) {
  SendRequest request;
  request.type = ESendType::BROADCAST;
  request.targetClientId = 0;
  request.packet = std::move(packet);
  sendQueue->Push(std::move(request));
  server->StartSend();
}
ServerNetworkSystem::~ServerNetworkSystem() = default;