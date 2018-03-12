#include <WinSock2.h>

#include "Session.h"

CNetSession::CNetSession()
{
	_Mode = MODE_NONE;
	_SendFlag = false;
	_SendCount = NULL;
	_IOCount = NULL;
	_LogOutFlag = NULL;
	_AuthToGameFlag = NULL;
	_ClientID = NULL;
	_Sock = INVALID_SOCKET;
	_RecvQ.Clear();
	

}

CNetSession::~CNetSession()
{

}

void CNetSession::SendPacket(CPacket *pPacket)
{
	_SendQ.Enqueue(pPacket);
	InterlockedIncrement(&_SendCount);
	return;
}

void CNetSession::Disconnect()
{
	shutdown(_Sock, SD_BOTH);
	return;
}

long CNetSession::GetMode()
{
	return _Mode;
}

bool CNetSession::SetMode_Game()
{
	if (MODE_AUTH != _Mode)
		return false;

	InterlockedExchange(&_Mode, MODE_AUTH_TO_GAME);
	return true;
}
