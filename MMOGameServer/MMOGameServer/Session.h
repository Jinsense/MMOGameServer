#ifndef _GAMESERVER_PLAYER_SESSION_H_
#define _GAMESERVER_PLAYER_SESSION_H_

#include <Windows.h>
#include <iostream>

#include "Packet.h"
#include "RingBuffer.h"
#include "LockFreeQueue.h"
#include "CommonProtocol.h"

typedef struct CLIENT_CONNECT_INFO
{
	//	IP, Port, Socket, ClientID ����
	char IP[20] = { 0, };
	int Port;
	SOCKET Sock;
	unsigned __int64	ClientID;

	CLIENT_CONNECT_INFO()
	{
		Port = NULL;
		ClientID = NULL;
		Sock = INVALID_SOCKET;
	}
}CLIENTINFO;

class CNetSession
{
public:
	enum en_SESSION_BUF
	{
		BUF = 4000,
	};

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
	//	�⺻���� IOCP ��Ʈ��ũ ó���� ���� ����
	//	socket, recv, send buff, overlapped, iocount ��

	en_SESSION_MODE	_Mode;		//	������ ���¸��
	int		_iArrayIndex;		//	Session �迭�� �ڱ� �ε���

	CLIENT_CONNECT_INFO _ClientInfo;
	OVERLAPPED	_SendOver;
	OVERLAPPED	_RecvOver;
	CRingBuffer	_RecvQ;
	CRingBuffer _SendQ;
//	CLockFreeQueue<CPacket*>	_SendQ;
	CLockFreeQueue<CPacket*>	_CompleteRecvPacket;
	CRingBuffer _CompleteSendPacket;
//	CLockFreeQueue<CPacket*>	_CompleteSendPacket;

	int		_iSendPacketCnt;
	int		_iSendPacketSize;

	long	_SendFlag; 
	long	_IOCount;

	bool	_LogOutFlag;
	bool	_AuthToGameFlag;

};

#endif