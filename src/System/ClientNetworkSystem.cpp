#include "System/ClientNetworkSystem.h"

#include <iostream>
#include <memory>

#include "Core/Event.h"
#include "Core/EventDispatcher.h"
#include "Core/Socket.h"
#include "Core/Packet.h"


ClientNetworkSystem::ClientNetworkSystem(const SystemContext& context)
    : assetManager(context.assetManager),
      eventDispatcher(context.eventDispatcher),
      registry(context.registry),
      world(context.world),
      timerManager(context.timerManager),
      packetQueue(context.packetQueue),
      sendQueue(context.sendQueue),
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
            reinterpret_cast<char*>(packet.get()) + sizeof(PacketHeader),
            header->packet_size - sizeof(PacketHeader))));
        break;
    }
  }
  SendRequest sendreq;
  while (sendQueue->TryPop(sendreq)) {
    switch (sendreq.type) {
      case ESendType::SERVER:
        PacketHeader* header = reinterpret_cast<PacketHeader*>(sendreq.packet.get());
        connectionSocket->Send(sendreq.packet.get(), header->packet_size);
        break;
    }
  }
}

void ClientNetworkSystem::SendMessage(std::shared_ptr<std::string> message) {
  SendRequest request;
  request.type = ESendType::SERVER;
  request.targetClientId = 0;

  PacketPtr packet = std::make_unique<char[]>(
      sizeof(PacketHeader) + sizeof(clientID) + message->size());

  PacketHeader* header = reinterpret_cast<PacketHeader*>(packet.get());
  header->packet_id = PACKET::CHAT_CLIENT;
  header->packet_size =
      sizeof(PacketHeader) + sizeof(clientID) + message->size();

  char* data = packet.get() + sizeof(PacketHeader);
  memcpy(data, &clientID, sizeof(clientID));

  data += sizeof(clientID);

  memcpy(data, message->c_str(), message->size());
  request.packet = std::move(packet);

  sendQueue->Push(std::move(request));
}

ClientNetworkSystem::~ClientNetworkSystem() = default;