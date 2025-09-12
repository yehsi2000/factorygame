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
      packetQueue(context.packetQueue), // Incoming packets
      sendQueue(context.serverSendQueue), // Outgoing packets (server-specific)
      server(context.server) {
  sendChatHandle =
      eventDispatcher->Subscribe<SendChatEvent>([this](SendChatEvent e) {
        Broadcast(util::ChatBroadcastPacket(e.message));
      });
}

void ServerNetworkSystem::Update(float deltatime) {
  PacketPtr packet;
  while (packetQueue->TryPop(packet)) {
    uint8_t* p = packet.get();
    std::size_t packetSize;
    PACKET packetId;

    util::GetHeader(p, packetId, packetSize);

    switch (packetId) {
      case PACKET::CHAT_CLIENT:
        clientid_t clientId = util::Read64BigEnd(p);
        
        char* msgStart = reinterpret_cast<char*>(p);
        size_t msgSize = packetSize - sHeaderAndId;

        eventDispatcher->Publish(
            NewChatEvent(std::make_shared<std::string>(msgStart, msgSize)));

        Broadcast(std::move(packet));
        break;
    }
  }
}

void ServerNetworkSystem::Unicast(clientid_t clientId, PacketPtr packet) {
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