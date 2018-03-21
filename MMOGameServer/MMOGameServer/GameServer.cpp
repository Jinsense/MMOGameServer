#include "GameServer.h"

CGameServer::CGameServer(int iMaxSession) : CMMOServer(iMaxSession)
{
	
}

CGameServer::~CGameServer()
{

}

void CGameServer::OnConnectionRequest()
{

	return;
}

void CGameServer::OnAuth_Packet()
{
	//	세션 배열을 돌면서 AUTH 모드 세션을 확인하여
	//	해당 세션의 CompleteRecvPacket을 처리 요청
	//	기본 패킷 1개 처리, 상황에 따라 N개 처리

	return;
}

void CGameServer::OnGame_Packet()
{
	
	return;
}

void CGameServer::OnAuth_Update()
{
	//	클라이언트의 요청(패킷수신)외에 기본적으로 항시
	//	처리되어야 할 컨텐츠 부분 로직
	
	return;
}

void CGameServer::OnGame_Update()
{

	return;
}

void CGameServer::OnError(int iErrorCode, WCHAR *szError)
{

	return;
}
















































