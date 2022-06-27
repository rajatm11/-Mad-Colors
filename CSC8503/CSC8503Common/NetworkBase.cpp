#include "NetworkBase.h"
#include <enet/enet.h>
#include <iostream>
#include "Debug.h"

NetworkBase::NetworkBase()	{
	netHandle = nullptr;
}

NetworkBase::~NetworkBase()	{
	if (netHandle) {
		enet_host_destroy(netHandle);
	}
}

void NetworkBase::Initialise() {
	enet_initialize();
}

void NetworkBase::Destroy() {
	enet_deinitialize();
}

bool NetworkBase::ProcessPacket(GamePacket* packet, int peerID) {
	PacketHandlerIterator firstHandler;
	PacketHandlerIterator lastHandler;

	bool canHandle = GetPacketHandlers(packet->type, firstHandler, lastHandler);

	if (canHandle) {
		for (auto i = firstHandler; i != lastHandler; ++i) {
			i->second->ReceivePacket(packet->type, packet, peerID);
		}
		return true;
	}
	std::string msg = __FUNCTION__ " no handler for packet type " + packet->type;
	Debug::PrintToConsole(msg);
	return false;
}