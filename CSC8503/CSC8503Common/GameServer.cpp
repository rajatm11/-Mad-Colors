#include "GameServer.h"
#include "GameWorld.h"
#include <iostream>

using namespace NCL;
using namespace CSC8503;

GameServer::GameServer(int onPort, int maxClients)	{
	port		= onPort;
	clientMax	= maxClients;
	clientCount = 0;
	netHandle	= nullptr;
	//threadAlive = false;

	Initialise();
}

GameServer::~GameServer()	{
	Shutdown();
}

void GameServer::Shutdown() {
	SendGlobalPacket(BasicNetworkMessages::Shutdown);

	//threadAlive = false;
	//updateThread.join();

	enet_host_destroy(netHandle);
	netHandle = nullptr;
}

bool GameServer::Initialise() {
	ENetAddress address;
	address.host = ENET_HOST_ANY;
	address.port = port;

	netHandle = enet_host_create(&address, clientMax, 1, 0, 0);

	if (!netHandle) {
		string msg = __FUNCTION__ " failed to create network handle!";
		Debug::PrintToConsole(msg);
		return false;
	}
	//threadAlive		= true;
	//updateThread	= std::thread(&GameServer::ThreadedUpdate, this);

	return true;
}

bool GameServer::SendGlobalPacket(int msgID) {
	GamePacket packet;
	packet.type = msgID;

	return SendGlobalPacket(packet);
}

bool GameServer::SendGlobalPacket(GamePacket& packet) {
	ENetPacket* dataPacket = enet_packet_create(&packet, packet.GetTotalSize(), 0);
	enet_host_broadcast(netHandle, 0, dataPacket);
	return true;
}

bool GameServer::SendPacketToPeer(int client, const GamePacket& packet)
{
	ENetPeer* clientPeer = &netHandle->peers[client];
	ENetPacket* dataPacket = enet_packet_create(&packet, packet.GetTotalSize(), 0);
	enet_peer_send(clientPeer, 0, dataPacket);
	return true;
}

void GameServer::UpdateServer() {
	if (!netHandle) {
		return;
	}

	ENetEvent event;
	while (enet_host_service(netHandle, &event, 0) > 0)	{
		int type	= event.type;
		ENetPeer* p = event.peer;

		int peer = p->incomingPeerID;

		//std::cout << "\nevent.peer :: " << event.peer << "\tincomingPeerID :: " << peer<<"\n";

		if (type == ENetEventType::ENET_EVENT_TYPE_CONNECT) {
			string msg = "Server: New client connected";
			Debug::PrintToConsole(msg);
			NewPlayerPacket player(peer);
			SendGlobalPacket(player);
			SendPacketToPeer(peer, PlayerIDPacket(peer, -1));
		}
		else if (type == ENetEventType::ENET_EVENT_TYPE_DISCONNECT) {
			string msg = "Server: A client has disconnected";
			Debug::PrintToConsole(msg);
			PlayerDisconnectPacket player(peer);
			SendGlobalPacket(player);
			SendPacketToPeer(peer, DisconnectConfirmPacket(peer));
		}
		else if (type == ENetEventType::ENET_EVENT_TYPE_RECEIVE) {
			GamePacket* packet = (GamePacket*)event.packet->data;
			ProcessPacket(packet, peer);
		}
		enet_packet_destroy(event.packet);
	}
}

//void GameServer::ThreadedUpdate() {
//	while (threadAlive) {
//		UpdateServer();
//	}
//}

//Second networking tutorial stuff

void GameServer::SetGameWorld(GameWorld &g) {
	gameWorld = &g;
}