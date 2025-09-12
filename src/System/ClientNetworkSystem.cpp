#include "System/ClientNetworkSystem.h"

#include <iostream>
#include <memory>

#include "Core/Event.h"
#include "Core/EventDispatcher.h"
#include "Core/Packet.h"
#include "Core/Socket.h"
#include "Util/PacketUtil.h"

ClientNetworkSystem::ClientNetworkSystem(const SystemContext& context)
    : assetManager(context.assetManager),
      eventDispatcher(context.eventDispatcher),
      registry(context.registry),
      world(context.world),
      timerManager(context.timerManager),
      packetQueue(context.packetQueue), // Incoming packets
      sendQueue(context.clientSendQueue), // Outgoing packets (client-specific)
      connectionSocket(context.socket),
      clientID(context.clientID) {
  sendChatHandle = eventDispatcher->Subscribe<SendChatEvent>(
      [this](SendChatEvent e) { SendMessage(e.message); });
}

void ClientNetworkSystem::Update(float deltatime) {
  PacketPtr packet;
  while (packetQueue->TryPop(packet)) {
    PacketHeader* header = reinterpret_cast<PacketHeader*>(packet.get());
    switch (header->packet_id) {
      case CHAT_BROADCAST:
        eventDispatcher->Publish(NewChatEvent(std::make_shared<std::string>(
            reinterpret_cast<uint8_t*>(packet.get()) + sPacketHeader,
            header->packet_size - sPacketHeader)));
        break;
    }
  }
  PacketPtr outgoingPacket;
  while (sendQueue->TryPop(outgoingPacket)) { // Pop PacketPtr directly
    PacketHeader* header =
        reinterpret_cast<PacketHeader*>(outgoingPacket.get());
    connectionSocket->Send(outgoingPacket.get(), header->packet_size);
  }
}

void ClientNetworkSystem::SendMessage(std::shared_ptr<std::string> message) {
  SendRequest request;
  request.type = ESendType::REQUEST;
  request.targetClientId = clientID;
  PacketPtr packet = std::make_unique<uint8_t[]>(sHeaderAndId + message->size());

  uint8_t* p = packet.get();
  util::WriteHeader(p, PACKET::CHAT_CLIENT, sHeaderAndId + message->size());

  util::Write64BigEnd(p, clientID);

  memcpy(p, message->c_str(), message->size());
  // No SendRequest wrapper needed, push PacketPtr directly
  sendQueue->Push(std::move(packet));
}

ClientNetworkSystem::~ClientNetworkSystem() = default;