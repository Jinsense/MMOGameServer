#include <WinSock2.h>
#include <windows.h>
#include <wchar.h>
#include <Ws2tcpip.h>
#include <Mstcpip.h>
#include <process.h>
#include <time.h>
#include <iostream>

using namespace std;

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
	InitializeSRWLock(&_Srwlock);
	CPacket::MemoryPoolInit();
}

CMMOServer::~CMMOServer()
{
	delete[] _pSessionArray;
}

bool CMMOServer::Start(WCHAR *szListenIP, int iPort, int iWorkerThread, bool bEnableNagle, BYTE byPacketCode, BYTE byPacketKey1, BYTE byPacketKey2)
{
	setlocale(LC_ALL, "Korean");
	
	CPacket::Init(byPacketCode, byPacketKey1, byPacketKey2);

	for (int i = 0; i < _iMaxSession; i++)
		_BlankSessionStack.Push(i);

	WSADATA Data;
	WSAStartup(MAKEWORD(2, 2), &Data);
	_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
	_ListenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	struct sockaddr_in Server_addr;
	ZeroMemory(&Server_addr, sizeof(Server_addr));
	Server_addr.sin_family = AF_INET;
	InetPton(AF_INET, szListenIP, &Server_addr.sin_addr);
	Server_addr.sin_port = htons(iPort);
	setsockopt(_ListenSocket, IPPROTO_TCP, TCP_NODELAY, (char*)bEnableNagle, sizeof(bEnableNagle));
	tcp_keepalive tcpkl;
	DWORD dwResult;
	tcpkl.onoff = 1;				// KEEPALIVE ON
	tcpkl.keepalivetime = 10000;	// 10초 마다 KEEPALIVE 신호를 보내겠다. (윈도우 기본은 2시간)
	tcpkl.keepaliveinterval = 1000;	// keepalive 신호를 보내고 응답이 없으면 1초마다 재 전송하겠다. (ms tcp 는 10회 재시도 한다)
	WSAIoctl(_ListenSocket, SIO_KEEPALIVE_VALS, &tcpkl, sizeof(tcp_keepalive), 0, 0, &dwResult, NULL, NULL);

	if (false == ::bind(_ListenSocket, (sockaddr*)&Server_addr, sizeof(Server_addr)))
		return false;

	if (false == listen(_ListenSocket, SOMAXCONN))
		return false;

	if (false == CreateThread())
		return false;
	
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
	CMMOServer *pAcceptThread = (CMMOServer*)pParam;
	if (NULL == pAcceptThread)
	{
		wprintf(L"[MMOServer :: AcceptThread] Init Error\n");
		return false;
	}
	return true;
}

bool CMMOServer::AcceptThread_update()
{

	return true;
}

unsigned __stdcall CMMOServer::AuthThread(void *pParam)
{
	CMMOServer *pAuthThread = (CMMOServer*)pParam;
	if (NULL == pAuthThread)
	{
		wprintf(L"[MMOServer :: AuthThread] Init Error\n");
		return false;
	}
	return true;
}
bool CMMOServer::AuthThread_update()
{

	return true;
}

unsigned __stdcall CMMOServer::GameUpdateThread(void *pParam)
{
	CMMOServer *pGameUpdateThread = (CMMOServer*)pParam;
	if (NULL == pGameUpdateThread)
	{
		wprintf(L"[MMOServer :: GameUpdateThread] Init Error\n");
		return false;
	}
	return true;
}

bool CMMOServer::GameUpdateThread_update()
{

	return true;
}

unsigned __stdcall CMMOServer::IOCPWorkerThread(void *pParam)
{
	CMMOServer *pIOCPWorkerThread = (CMMOServer*)pParam;
	if (NULL == pIOCPWorkerThread)
	{
		wprintf(L"[MMOServer :: IOCPWorkerThread] Init Error\n");
		return false;
	}
	return true;
}

bool CMMOServer::IOCPWorkerThread_update()
{

	return true;
}

unsigned __stdcall CMMOServer::SendThread(void *pParam)
{
	CMMOServer *pSendThread = (CMMOServer*)pParam;
	if (NULL == pSendThread)
	{
		wprintf(L"[MMOServer :: SendThread] Init Error\n");
		return false;
	}
	return true;
}

bool CMMOServer::SendThread_update()
{

	return true;
}

