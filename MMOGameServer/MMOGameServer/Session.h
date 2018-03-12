#ifndef _GAMESERVER_PLAYER_SESSION_H_
#define _GAMESERVER_PLAYER_SESSION_H_

#include <Windows.h>

#include "Packet.h"
#include "RingBuffer.h"
#include "LockFreeQueue.h"

class CNetSession
{
public:
	enum en_SESSION_MODE
	{
		MODE_NONE = 1,
		MODE_AUTH = 2,
		MODE_AUTH_TO_GAME = 3,
		MODE_GAME = 4,
		MODE_LOGOUT_IN_AUTH = 5,
		MODE_LOGOUT_IN_GAME = 6,
		MODE_WAIT_LOGOUT = 7,
	};

	CNetSession();
	~CNetSession();

	virtual void OnAuth_ClientJoin() = 0;
	virtual void OnAuth_ClientLeave() = 0;
	virtual void OnAuth_Packet(CPacket *pPacket) = 0;
	virtual void OnGame_ClientJoin() = 0;
	virtual void OnGame_ClientLeave() = 0;
	virtual void OnGame_Packet(CPacket *pPacket) = 0;
	virtual void OnGame_ClientRelease() = 0;

	void	SendPacket(CPacket *pPacket);
	void	Disconnect();

	long	GetMode();
	bool	SetMode_Game();
private:


public:
	//	기본적인 IOCP 네트워크 처리를 위한 변수
	//	socket, recv, send buff, overlapped, iocount 등

	unsigned __int64	_ClientID;
	SOCKET	_Sock;
	OVERLAPPED	_SendOver;
	OVERLAPPED	_RecvOver;
	CRingBuffer	_RecvQ;
	CLockFreeQueue<CPacket*>	_SendQ;
	IN_ADDR		_Info;

private:
	long	_Mode;
	long	_SendFlag;
	long	_SendCount;
	long	_IOCount;
	long	_LogOutFlag;
	long	_AuthToGameFlag;

};

#endif