#include <WinSock2.h>

#include "Session.h"

CNetSession::CNetSession() : _RecvQ(BUF), _SendQ(BUF), _CompleteSendPacket(BUF)
{
	_Mode = MODE_NONE;
	_iArrayIndex = NULL;
	_RecvQ.Clear();
	_SendQ.Clear();
	_iSendPacketCnt = NULL;
	_iSendPacketSize = NULL;
	_SendFlag = false;
	_IOCount = NULL;
	_LogOutFlag = false;
	_AuthToGameFlag = false;
}

CNetSession::~CNetSession()
{

}

void CNetSession::SendPacket(CPacket *pPacket)
{
	pPacket->AddRef();
	pPacket->EnCode();
	_SendQ.Enqueue((char*)&pPacket, sizeof(CPacket*));
	if (nullptr == pPacket)
		g_CrashDump->Crash();
//	_SendQ.Enqueue(pPacket);
	return;
}

void CNetSession::Disconnect()
{
	shutdown(_ClientInfo.Sock, SD_BOTH);
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

	_Mode = MODE_AUTH_TO_GAME;
	return true;
}
