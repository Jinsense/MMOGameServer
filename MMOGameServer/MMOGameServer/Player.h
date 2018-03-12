#ifndef _GAMESERVER_PLAYER_PLAYER_H_
#define _GAMESERVER_PLAYER_PLAYER_H_

#include "Session.h"

class CPlayer : public CNetSession
{
public:
	CPlayer();
	~CPlayer();

	void OnAuth_ClientJoin();
	void OnAuth_ClientLeave();
	void OnAuth_Packet(CPacket *pPacket);
	void OnGame_ClientJoin();
	void OnGame_ClientLeave();
	void OnGame_Packet(CPacket *pPacket);
	void OnGame_ClientRelease();




};

#endif