#include <WinSock2.h>
#include <windows.h>
#include <wchar.h>
#include <Ws2tcpip.h>
#include <Mstcpip.h>
#include <process.h>
#include <time.h>
#include <chrono>
#include <iostream>




using namespace std;

#include "MMOGameServer.h"

CMMOServer::CMMOServer(int iMaxSession) : _iMaxSession(iMaxSession)
{

	timeBeginPeriod(1);

	_bShutdown = false;
	_bShutdownListen = false;
	_ListenSocket = INVALID_SOCKET;
	_bEnableNagle = false;
	_iWorkerThread = WORKER_THREAD_MAX;

	ZeroMemory(_szListenIP, sizeof(_szListenIP));
	_iListenPort = NULL;
	_byCode = NULL;
	_iClientIDCnt = 0;
	_hAcceptThread = NULL;
	_hAuthThread = NULL;
	for (auto iCnt = 0; iCnt < WORKER_THREAD_MAX; iCnt++)
		_hIOCPWorkerThread[iCnt] = NULL;
	_hIOCP = NULL;
	_hSendThread = NULL;
	_pSessionArray = new CNetSession*[_iMaxSession];
	_pMemoryPool_ConnectInfo = new CMemoryPool<CLIENT_CONNECT_INFO>();
	InitializeSRWLock(&_Srwlock);
	CPacket::MemoryPoolInit();
	_pLog = _pLog->GetInstance();

	_Monitor_AcceptTotal = NULL;
	_Monitor_AcceptSocket = NULL;
	_Monitor_SessionAllMode = NULL;
	_Monitor_SessionAuthMode = NULL;
	_Monitor_SessionGameMode = NULL;
	_Monitor_Counter_AuthUpdate = NULL;
	_Monitor_Counter_GameUpdate = NULL;
	_Monitor_Counter_Recv = NULL;
	_Monitor_Counter_Send = NULL;
	_Monitor_Counter_PacketSend = NULL;
}

CMMOServer::~CMMOServer()
{
	timeEndPeriod(1);
	delete[] _pSessionArray;
}

bool CMMOServer::Start(WCHAR *szListenIP, int iPort, int iWorkerThread, bool bEnableNagle, BYTE byPacketCode, BYTE byPacketKey1, BYTE byPacketKey2)
{
	int iRetval = 0;
	setlocale(LC_ALL, "Korean");
	
	CPacket::Init(byPacketCode, byPacketKey1, byPacketKey2);
	_byCode = byPacketCode;

	for (int i = 0; i < _iMaxSession; i++)
		_BlankSessionStack.Push(i);

	WSADATA Data;
	if (0 != WSAStartup(MAKEWORD(2, 2), &Data))
		return false;
	_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
	if (NULL == _hIOCP)
		return false;
	_ListenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == _ListenSocket)
		return false;

	struct sockaddr_in Server_addr;
	ZeroMemory(&Server_addr, sizeof(Server_addr));
	Server_addr.sin_family = AF_INET;
	InetPton(AF_INET, (PCWSTR)szListenIP, &Server_addr.sin_addr);
	Server_addr.sin_port = htons(iPort);
	if (0 != setsockopt(_ListenSocket, IPPROTO_TCP, TCP_NODELAY, (char*)&bEnableNagle, sizeof(bEnableNagle)))
	{
		int LastError = WSAGetLastError();
		wprintf(L"[Sock opt Error : %d\n", LastError);
		return false;
	}
	
	tcp_keepalive tcpkl;
	DWORD dwResult;
	tcpkl.onoff = 1;				// KEEPALIVE ON
	tcpkl.keepalivetime = 10000;	// 10초 마다 KEEPALIVE 신호를 보내겠다. (윈도우 기본은 2시간)
	tcpkl.keepaliveinterval = 1000;	// keepalive 신호를 보내고 응답이 없으면 1초마다 재 전송하겠다. (ms tcp 는 10회 재시도 한다)
	WSAIoctl(_ListenSocket, SIO_KEEPALIVE_VALS, &tcpkl, sizeof(tcp_keepalive), 0, 0, &dwResult, NULL, NULL);
	
	iRetval == ::bind(_ListenSocket, (sockaddr*)&Server_addr, sizeof(Server_addr));
	if (SOCKET_ERROR == iRetval)
	{
		int LastError = WSAGetLastError();
		wprintf(L"[Sock opt Error : %d\n", LastError);
		return false;
	}

	iRetval == listen(_ListenSocket, SOMAXCONN);
	if (SOCKET_ERROR == iRetval)
	{
		int LastError = WSAGetLastError();
		wprintf(L"[Sock opt Error : %d\n", LastError);
		return false;
	}

	CreateThread();
	return true;
}

bool CMMOServer::Stop()
{

	return true;
}

void CMMOServer::SetSessionArray(int iArrayIndex, CNetSession *pSession)
{
	_pSessionArray[iArrayIndex] = pSession;
	return;
}

bool CMMOServer::CreateThread()
{
	_hAcceptThread = (HANDLE)_beginthreadex(NULL, 0, &AcceptThread, (LPVOID)this, 0, NULL);
	for(int i = 0; i < WORKER_THREAD_MAX; i++)
		_hIOCPWorkerThread[i] = (HANDLE)_beginthreadex(NULL, 0, &IOCPWorkerThread, (LPVOID)this, 0, NULL);
	_hSendThread = (HANDLE)_beginthreadex(NULL, 0, &SendThread, (LPVOID)this, 0, NULL);
	_hAuthThread = (HANDLE)_beginthreadex(NULL, 0, &AuthThread, (LPVOID)this, 0, NULL);
	_hGameUpdateThread = (HANDLE)_beginthreadex(NULL, 0, &GameUpdateThread, (LPVOID)this, 0, NULL);
	return true;
}

bool CMMOServer::CreateIOCP_Socket(SOCKET Socket, ULONG_PTR Key)
{
	//	IOCP 등록
	CreateIoCompletionPort((HANDLE)Socket, _hIOCP, Key, 0);
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

bool CMMOServer::SessionAcquireLock(int Index)
{
	if (true == _pSessionArray[Index]->_LogOutFlag)
		return false;
	long IOCount = 0;
	IOCount = InterlockedIncrement(&_pSessionArray[Index]->_IOCount);
	if (1 == IOCount)
	{
		SessionAcquireFree(Index);
		return false;
	}
	return true;
}

bool CMMOServer::SessionAcquireFree(int Index)
{
	if (0 == InterlockedDecrement(&_pSessionArray[Index]->_IOCount))
	{
		_pSessionArray[Index]->_LogOutFlag = true;
		return false;
	}
	return true;
}

void CMMOServer::Error(int ErrorCode, WCHAR *szFormatStr, ...)
{

	return;
}

void CMMOServer::RecvPost(int Index)
{
	if (false == SessionAcquireLock(Index))
		return;

	ZeroMemory(&_pSessionArray[Index]->_RecvOver, sizeof(_pSessionArray[Index]->_RecvOver));
	
	WSABUF Buf[2];
	DWORD FreeSize = _pSessionArray[Index]->_RecvQ.GetFreeSize();
	DWORD NotBrokenPushSize = _pSessionArray[Index]->_RecvQ.GetNotBrokenPushSize();
	if (0 == FreeSize && 0 == NotBrokenPushSize)
	{
		if (true == SessionAcquireFree(Index))
		{
			_pLog->Log(L"Error", LOG_SYSTEM, L"RecvPost - RecvQ Full [Index : %d]", Index);
			shutdown(_pSessionArray[Index]->_ClientInfo.Sock, SD_BOTH);
		}
	}
	int NumOfBuf = (NotBrokenPushSize < FreeSize) ? 2 : 1;

	Buf[0].buf = _pSessionArray[Index]->_RecvQ.GetWriteBufferPtr();
	Buf[0].len = NotBrokenPushSize;

	if (2 == NumOfBuf)
	{
		Buf[1].buf = _pSessionArray[Index]->_RecvQ.GetBufferPtr();
		Buf[1].len = FreeSize - NotBrokenPushSize;
	}

	if (SOCKET_ERROR == WSARecv(_pSessionArray[Index]->_ClientInfo.Sock, Buf, NumOfBuf, NULL, NULL, &_pSessionArray[Index]->_RecvOver, NULL))
	{
		int LastError = WSAGetLastError();
		if (ERROR_IO_PENDING != LastError)
		{
			if(true == SessionAcquireFree(Index))
				shutdown(_pSessionArray[Index]->_ClientInfo.Sock, SD_BOTH);
		}
	}
	return;
}

void CMMOServer::SendPost(int Index)
{
	if (true == InterlockedCompareExchange(&_pSessionArray[Index]->_SendFlag, true, false))
		return;

	if (0 == _pSessionArray[Index]->_SendQ.GetUseCount())
	{
		InterlockedExchange(&_pSessionArray[Index]->_SendFlag, false);
		return;
	}

	if (CNetSession::MODE_AUTH != _pSessionArray[Index]->_Mode && CNetSession::MODE_GAME != _pSessionArray[Index]->_Mode)
	{
		InterlockedExchange(&_pSessionArray[Index]->_SendFlag, false);
		return;
	}

	WSABUF Buf[WSABUF_MAX];
	CPacket *pPacket;
	int BufNum;
	int UseSize = (_pSessionArray[Index]->_SendQ.GetUseCount());
	if (UseSize > WSABUF_MAX)
	{
		BufNum = WSABUF_MAX;
		_pSessionArray[Index]->_iSendPacketCnt = WSABUF_MAX;

		for (int j = 0; j < WSABUF_MAX; j++)
		{
			_pSessionArray[Index]->_SendQ.Dequeue(pPacket);
			_pSessionArray[Index]->_CompleteSendPacket.Enqueue(pPacket);
			Buf[j].buf = pPacket->GetBufferPtr();
			Buf[j].len = pPacket->GetPacketSize();
		}
	}
	else
	{
		BufNum = UseSize;
		_pSessionArray[Index]->_iSendPacketCnt = UseSize;

		for (int j = 0; j < UseSize; j++)
		{
			_pSessionArray[Index]->_SendQ.Dequeue(pPacket);
			_pSessionArray[Index]->_CompleteSendPacket.Enqueue(pPacket);
			Buf[j].buf = pPacket->GetBufferPtr();
			Buf[j].len = pPacket->GetPacketSize();
		}
	}
	if (false == SessionAcquireLock(Index))
	{
		InterlockedExchange(&_pSessionArray[Index]->_SendFlag, false);
		return;
	}
	ZeroMemory(&_pSessionArray[Index]->_SendOver, sizeof(_pSessionArray[Index]->_SendOver));
	if (SOCKET_ERROR == WSASend(_pSessionArray[Index]->_ClientInfo.Sock, Buf, BufNum, NULL, 0, &_pSessionArray[Index]->_SendOver, NULL))
	{
		int LastError = WSAGetLastError();
		if (ERROR_IO_PENDING != LastError)
		{
			if (true == SessionAcquireFree(Index))
			{
				shutdown(_pSessionArray[Index]->_ClientInfo.Sock, SD_BOTH);
				InterlockedExchange(&_pSessionArray[Index]->_SendFlag, false);
				return;
			}
		}
	}
	return;
}

void CMMOServer::CompleteRecv(int Index, DWORD Trans)
{
	_pSessionArray[Index]->_RecvQ.Enqueue(Trans);

	while (0 < _pSessionArray[Index]->_RecvQ.GetUseSize())
	{
		if (sizeof(CPacket::st_PACKET_HEADER) > _pSessionArray[Index]->_RecvQ.GetUseSize())
		{
			RecvPost(Index);
			return;
		}

		CPacket::st_PACKET_HEADER _Header;
		_pSessionArray[Index]->_RecvQ.Peek((char*)&_Header, sizeof(CPacket::st_PACKET_HEADER));
	
		if (CNetSession::BUF < _Header.shLen)
		{
			shutdown(_pSessionArray[Index]->_ClientInfo.Sock, SD_BOTH);
			return;
		}
	
		if (sizeof(CPacket::st_PACKET_HEADER) + _Header.shLen > _pSessionArray[Index]->_RecvQ.GetUseSize())
		{
			RecvPost(Index);
			return;
		}

		if (_byCode != _Header.byCode)
		{
			shutdown(_pSessionArray[Index]->_ClientInfo.Sock, SD_BOTH);
			return;
		}

		CPacket *pPacket = CPacket::Alloc();
		_pSessionArray[Index]->_RecvQ.Dequeue((char*)pPacket->GetBufferPtr(), _Header.shLen + sizeof(CPacket::st_PACKET_HEADER));
		pPacket->PushData(_Header.shLen + sizeof(CPacket::st_PACKET_HEADER));
		if (false == pPacket->DeCode(&_Header))
		{
			shutdown(_pSessionArray[Index]->_ClientInfo.Sock, SD_BOTH);
			pPacket->Free();
			return;
		}
		pPacket->m_header.byCode = _Header.byCode;
		pPacket->m_header.shLen = _Header.shLen;
		pPacket->m_header.RandKey = _Header.RandKey;
		pPacket->m_header.CheckSum = _Header.CheckSum;
		pPacket->PopData(sizeof(CPacket::st_PACKET_HEADER));
		_pSessionArray[Index]->_CompleteRecvPacket.Enqueue(pPacket);
	}
	RecvPost(Index);
	return;
}

void CMMOServer::CompleteSend(int Index, DWORD Trans)
{
	CPacket *pPacket;
	for (int i = 0; i < _pSessionArray[Index]->_iSendPacketCnt; i++)
	{
		_pSessionArray[Index]->_CompleteSendPacket.Dequeue(pPacket);
		if (nullptr == pPacket)
			break;
		pPacket->Free();
	}
	_pSessionArray[Index]->_iSendPacketCnt = 0;
	InterlockedExchange(&_pSessionArray[Index]->_SendFlag, false);

	return;
}

void CMMOServer::ProcAuth_Accept()
{
	// 신규 접속자 처리
	CLIENT_CONNECT_INFO *pInfo;
	_AccpetSocketQueue.Dequeue(pInfo);

	int Index = NULL;
	_BlankSessionStack.Pop(Index);

	_pSessionArray[Index]->_iArrayIndex = Index;
	_pSessionArray[Index]->_Mode = CNetSession::MODE_AUTH;
	_pSessionArray[Index]->_AuthToGameFlag = false;
	_pSessionArray[Index]->_iSendPacketCnt = NULL;
	_pSessionArray[Index]->_iSendPacketSize = NULL;
	_pSessionArray[Index]->_LogOutFlag = false;
	_pSessionArray[Index]->_RecvQ.Clear();
	InterlockedIncrement(&_pSessionArray[Index]->_IOCount);

	//	클라이언트 정보가 제대로 복사되는지
	_pSessionArray[Index]->_ClientInfo = *pInfo;

	//	삭제되도 이상 없는지 ClientInfo가 이상 없는지
	_pMemoryPool_ConnectInfo->Free(pInfo);

	CreateIOCP_Socket(_pSessionArray[Index]->_ClientInfo.Sock, Index);
	_pSessionArray[Index]->OnAuth_ClientJoin();
	RecvPost(Index);
	return;
}

void CMMOServer::ProcAuth_LogoutInAuth()
{
	//	LogOutFlag가 true인 세션을 LOGOUT_IN_AUTH 모드로 변경
	for (int i = 0; i < _iMaxSession; i++)
	{
		if (CNetSession::MODE_AUTH == _pSessionArray[i]->_Mode && true == _pSessionArray[i]->_LogOutFlag)
			_pSessionArray[i]->_Mode = CNetSession::MODE_LOGOUT_IN_AUTH;
	}
	return;
}

void CMMOServer::ProcAuth_Logout()
{
	//	LOGOUT_IN_AUTH 모드의 세션이 SendFlag가 0인 경우
	//	WAIT_LOGOUT으로 모드 변경
	for (int i = 0; i < _iMaxSession; i++)
	{
		if (CNetSession::MODE_LOGOUT_IN_AUTH == _pSessionArray[i]->_Mode && false == _pSessionArray[i]->_SendFlag)
		{
			_pSessionArray[i]->_Mode = CNetSession::MODE_WAIT_LOGOUT;
			_pSessionArray[i]->OnAuth_ClientLeave();
		}
	}
	return;
}

void CMMOServer::ProcAuth_AuthToGame()
{
	//	세션 배열을 돌면서 AuthToGame 변수가 true이며,
	//	Auth 모드인 세션을 AUTH_TO_GAME 모드로 변경
	for (int i = 0; i < _iMaxSession; i++)
	{
		if (CNetSession::MODE_AUTH == _pSessionArray[i]->_Mode && true == _pSessionArray[i]->_AuthToGameFlag)
		{
			_pSessionArray[i]->_Mode = CNetSession::MODE_AUTH_TO_GAME;
			_pSessionArray[i]->OnAuth_ClientLeave();
		}
	}
	return;
}

void CMMOServer::ProcGame_AuthToGame()
{
	//	Game 모드로 전환
	//	세션 배열을 돌면서 AUTH_TO_GAME 모드의 세션을 GAME 모드로 변경한다
	//	나중에는 모드 변경 인원을 제한
	//	현재는 테스트를 위해 무제한으로 변경
	for (int i = 0; i < _iMaxSession; i++)
	{
		if (CNetSession::MODE_AUTH_TO_GAME == _pSessionArray[i]->_Mode)
			_pSessionArray[i]->_Mode = CNetSession::MODE_GAME;
	}
	return;
}

void CMMOServer::ProcGame_LogoutInGame()
{
	//	GAME 모드 LogoutFlag 처리
	//	세션 배열을 돌면서 GAME 모드에서 LogoutFlag가
	//	true인 세션을 LOGOUT_IN_GAME 모드로 변경
	for (int i = 0; i < _iMaxSession; i++)
	{
		if (CNetSession::MODE_GAME == _pSessionArray[i]->_Mode && true == _pSessionArray[i]->_LogOutFlag)
		{
			_pSessionArray[i]->_Mode = CNetSession::MODE_LOGOUT_IN_GAME;
		}
	}
	return;
}

void CMMOServer::ProcGame_Logout()
{
	//	LOGOUT_IN_GAME 모드 릴리즈 단계 돌입
	//	세션 배열을 돌면서 LOGOUT_IN_GAME 모드의 세션이
	//	SendFlag (WSASend 보내기 작업이 없다는 확인)가 0인 경우
	//	WAIT_LOGOUT으로 모드 변경
	//	virtual OnGame_ClientLeave 호출
	for (int i = 0; i < _iMaxSession; i++)
	{
		if (CNetSession::MODE_LOGOUT_IN_GAME == _pSessionArray[i]->_Mode && false == _pSessionArray[i]->_SendFlag)
		{
			_pSessionArray[i]->_Mode = CNetSession::MODE_WAIT_LOGOUT;
			_pSessionArray[i]->OnGame_ClientLeave();
		}
	}
	return;
}

void CMMOServer::ProcGame_Release()
{
	//	WAIT_LOGOUT 최종 릴리즈
	//	세션 배열을 돌면서 WAIT_LOGOUT 모드의 세션을 최종적으로 릴리즈 처리한다. 
	//	virtual OnGame_ClientRelease 호출
	//	NONE_MODE로 바꾸고 플래그 바꿈
	//	세션 초기화
	//	해당 세션 배열 Index를 BlankSessionStack에 넣음
	for (int i = 0; i < _iMaxSession; i++)
	{
		if (CNetSession::MODE_WAIT_LOGOUT == _pSessionArray[i]->_Mode)
		{
			_pSessionArray[i]->OnGame_ClientRelease();
			_pSessionArray[i]->MODE_NONE;
			_pSessionArray[i]->_iArrayIndex = NULL;
			_pSessionArray[i]->_iSendPacketCnt = NULL;
			_pSessionArray[i]->_iSendPacketSize = NULL;
			_pSessionArray[i]->_SendFlag = false;
			_pSessionArray[i]->_LogOutFlag = false;
			_pSessionArray[i]->_AuthToGameFlag = false;
			
			while (0 != _pSessionArray[i]->_SendQ.GetUseCount())
			{
				//	애초에 큐에 패킷이 남아있으면 안됨..
				CPacket *pPacket;
				_pSessionArray[i]->_SendQ.Dequeue(pPacket);
				pPacket->Free();
			}
			while (0 != _pSessionArray[i]->_CompleteRecvPacket.GetUseCount())
			{
				//	애초에 큐에 패킷이 남아있으면 안됨..
				CPacket *pPacket;
				_pSessionArray[i]->_CompleteRecvPacket.Dequeue(pPacket);
				pPacket->Free();
			}
			while (0 != _pSessionArray[i]->_CompleteSendPacket.GetUseCount())
			{
				//	애초에 큐에 패킷이 남아있으면 안됨..
				CPacket *pPacket;
				_pSessionArray[i]->_CompleteSendPacket.Dequeue(pPacket);
				pPacket->Free();
			}

			_pSessionArray[i]->_ClientInfo.ClientID = NULL;
			_pSessionArray[i]->_ClientInfo.Port = NULL;
			closesocket(_pSessionArray[i]->_ClientInfo.Sock);
			_pSessionArray[i]->_ClientInfo.Sock = INVALID_SOCKET;
			_BlankSessionStack.Push(i);
		}
	}
	return;
}

bool CMMOServer::AcceptThread_update()
{
	while (!_bShutdown)
	{
		SOCKADDR_IN ClientAddr;
		int AddrSize = sizeof(ClientAddr);
		SOCKET Sock = accept(_ListenSocket, (SOCKADDR*)&ClientAddr, &AddrSize);
		if (INVALID_SOCKET == Sock)
			continue;
		_Monitor_AcceptSocket++;
		_Monitor_AcceptTotal++;

		CLIENT_CONNECT_INFO *pInfo = _pMemoryPool_ConnectInfo->Alloc();
		//	IP가 제대로 저장되는지 체크할 것
		inet_ntop(AF_INET, &ClientAddr.sin_addr, pInfo->IP, sizeof(pInfo->IP));
		pInfo->Port = ClientAddr.sin_port;
		pInfo->Sock = Sock;
		pInfo->ClientID = _iClientIDCnt++;

		_AccpetSocketQueue.Enqueue(pInfo);
		InterlockedIncrement(&_Monitor_SessionAllMode);
	}
	return true;
}

bool CMMOServer::AuthThread_update()
{
	int Count;
	while (!_bShutdown)
	{
		Sleep(5);
		Count = 0;
		_Monitor_Counter_AuthUpdate++;

		while(0 != _AccpetSocketQueue.GetUseCount())
//		while (Count < AUTH_MAX)
		{
//			if (0 != _AccpetSocketQueue.GetUseCount())
			{
				ProcAuth_Accept();				
			}
//			Count++;
		}

		for (int i = 0; i < _iMaxSession; i++)
		{
			if (CNetSession::MODE_AUTH == _pSessionArray[i]->_Mode && 0 != _pSessionArray[i]->_CompleteRecvPacket.GetUseCount())
			{
				Count = 0;
				while (Count < AUTH_MAX)
				{
					if (0 == _pSessionArray[i]->_CompleteRecvPacket.GetUseCount())
						break;
					CPacket *pPacket;
					_pSessionArray[i]->_CompleteRecvPacket.Dequeue(pPacket);
					_pSessionArray[i]->OnAuth_Packet(pPacket);
					pPacket->Free();
					Count++;
				}
			}
		}
		OnAuth_Update();		
		ProcAuth_LogoutInAuth();
		ProcAuth_Logout();
		ProcAuth_AuthToGame();
	}
	return true;
}

bool CMMOServer::GameUpdateThread_update()
{
	int Count;
	while (!_bShutdown)
	{
		Sleep(1);
		Count = 0;
		_Monitor_Counter_GameUpdate++;
		ProcGame_AuthToGame();

		//	GAME 모드 세션들 패킷 처리
		//	세션 배열을 모두 돌면서 GAME 모드 세션을 확인하여
		//	해당 세션의 CompleteRecvPacket 처리요청
		//	virtual OnGame_Packet 호출
		//	기본 패킷 1개 처리, 상황에 따라 N개 처리
		for (int i = 0; i < _iMaxSession; i++)
		{
			if (CNetSession::MODE_GAME == _pSessionArray[i]->_Mode && 0 != _pSessionArray[i]->_CompleteRecvPacket.GetUseCount())
			{
				Count = 0;
				while (Count < GAME_MAX)
				{
					if (0 == _pSessionArray[i]->_CompleteRecvPacket.GetUseCount())
						break;
					CPacket *pPacket;
					_pSessionArray[i]->_CompleteRecvPacket.Dequeue(pPacket);
					_pSessionArray[i]->OnGame_Packet(pPacket);
					pPacket->Free();
					Count++;
				}
			}
		}
		OnGame_Update();
		ProcGame_LogoutInGame();
		ProcGame_Logout();
		ProcGame_Release();
	}
	return true;
}

bool CMMOServer::IOCPWorkerThread_update()
{
	DWORD Retval;

	while (!_bShutdown)
	{
		OVERLAPPED * pOver = NULL;
		int  *Index = NULL;
		DWORD Trans = 0;
		
		Retval = GetQueuedCompletionStatus(_hIOCP, &Trans, (PULONG_PTR)&Index, (LPWSAOVERLAPPED*)&pOver, INFINITE);

		if (nullptr == pOver)
		{
			if (nullptr == Index && 0 == Trans)
			{
				PostQueuedCompletionStatus(_hIOCP, 0, 0, 0);
			}
		}
		if (0 == Trans)
		{
			shutdown(_pSessionArray[*Index]->_ClientInfo.Sock, SD_BOTH);
		}
		else if (pOver == &_pSessionArray[*Index]->_RecvOver)
		{
			CompleteRecv(*Index, Trans);
		}
		else if (pOver == &_pSessionArray[*Index]->_SendOver)
		{
			CompleteSend(*Index, Trans);
		}

		SessionAcquireFree(*Index);
	}
	return true;
}

bool CMMOServer::SendThread_update()
{
	while (!_bShutdown)
	{
		Sleep(5);
		for (int i = 0; i < _iMaxSession; i++)
		{
			if (CNetSession::MODE_NONE == _pSessionArray[i]->_Mode || true == _pSessionArray[i]->_SendFlag || true == _pSessionArray[i]->_LogOutFlag ||
				CNetSession::MODE_LOGOUT_IN_AUTH == _pSessionArray[i]->_Mode || CNetSession::MODE_LOGOUT_IN_GAME == _pSessionArray[i]->_Mode ||
				CNetSession::MODE_WAIT_LOGOUT == _pSessionArray[i]->_Mode)
				continue;

			if (false == SessionAcquireLock(i))
				continue;
			
			SendPost(i);
			SessionAcquireLock(i);
		}
	}
	return true;
}

