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
	//	���� �迭�� ���鼭 AUTH ��� ������ Ȯ���Ͽ�
	//	�ش� ������ CompleteRecvPacket�� ó�� ��û
	//	�⺻ ��Ŷ 1�� ó��, ��Ȳ�� ���� N�� ó��

	return;
}

void CGameServer::OnGame_Packet()
{
	
	return;
}

void CGameServer::OnAuth_Update()
{
	//	Ŭ���̾�Ʈ�� ��û(��Ŷ����)�ܿ� �⺻������ �׽�
	//	ó���Ǿ�� �� ������ �κ� ����
	
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
















































