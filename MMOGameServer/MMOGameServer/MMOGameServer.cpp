#include "MMOGameServer.h"

CMMOServer::CMMOServer(int iMaxSession) : _iMaxSession(iMaxSession)
{
	_bShutdown = false;
	_bShutdownListen = false;
	_ListenSocket = INVALID_SOCKET;
	_bEnableNagle = false;
	_iWorkerThread = WORKER_THREAD_MAX;

	ZeroMemory(_szListenIP, sizeof(_szListenIP));
	_iListenPort = NULL;
	_byPacketCode = NULL;
	_hAcceptThread = NULL;
	_hAuthThread = NULL;
	for (auto iCnt = 0; iCnt < WORKER_THREAD_MAX; iCnt++)
		_hIOCPWorkerThread[iCnt] = NULL;
	_hIOCP = NULL;
	_hSendThread = NULL;
	_pSessionArray = new CNetSession*[_iMaxSession];

}

CMMOServer::~CMMOServer()
{

}

bool CMMOServer::Start(WCHAR *szListenIP, int iPort, int iWorkerThread, bool bEnableNagle, BYTE byPacketCode, BYTE byPacketKey1, BYTE byPacketKey2)
{
	
	return true;
}

bool CMMOServer::Stop()
{

	return true;
}

void CMMOServer::SetSessionArray(int iArrayIndex, CNetSession *pSession)
{

	return;
}

bool CMMOServer::CreateThread()
{

	return true;
}

bool CMMOServer::CreateIOCP_Socket(SOCKET Socket, ULONG_PTR Key)
{

	return true;
}

void CMMOServer::SendPacket_GameAll(CPacket *pPacket, unsigned __int64 ExcludeClientID = 0)
{

	return;
}

void CMMOServer::SendPacket(CPacket *pPacket, unsigned __int64 ClientID)
{

	return;
}

void CMMOServer::Error(int ErrorCode, WCHAR *szFormatStr, ...)
{

	return;
}

void CMMOServer::ProcAuth_Accept()
{
	return;
}

void CMMOServer::ProcAuth_Packet()
{

	return;
}

void CMMOServer::ProcAuth_Logout()
{

	return;
}

void CMMOServer::ProcGame_AuthToGame()
{

	return;
}

void CMMOServer::ProcGame_Packet()
{

	return;
}

void CMMOServer::ProcGame_Logout()
{

	return;
}

void CMMOServer::ProcGame_Release()
{

	return;
}

unsigned __stdcall CMMOServer::AcceptThread(void *pParam)
{

	return;
}

bool CMMOServer::AcceptThread_update()
{

	return true;
}

unsigned __stdcall CMMOServer::AuthThread(void *pParam)
{

	return;
}
bool CMMOServer::AuthThread_update()
{

	return true;
}

unsigned __stdcall CMMOServer::GameUpdateThread(void *pParam)
{

	return;
}

bool CMMOServer::GameUpdateThread_update()
{

	return true;
}

unsigned __stdcall CMMOServer::IOCPWorkerThread(void *pParam)
{

	return;
}

bool CMMOServer::IOCPWorkerThread_update()
{

	return true;
}

unsigned __stdcall CMMOServer::SendThread(void *pParam)
{

	return;
}

bool CMMOServer::SendThread_update()
{

	return true;
}

