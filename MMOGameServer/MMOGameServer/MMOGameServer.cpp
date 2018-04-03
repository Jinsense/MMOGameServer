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

CMMOServer::CMMOServer(int iMaxSession) : _iMaxSession(iMaxSession), _AccpetSocketQueue(50000)
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

	_Monitor_Counter_RecvAvr = NULL;
	_Monitor_Counter_SendAvr = NULL;
	_Monitor_Counter_AcceptThreadAvr = NULL;
	_Monitor_Counter_SendThreadAvr = NULL;
	_Monitor_Counter_AuthThreadAvr = NULL;
	_Monitor_Counter_GameThreadAvr = NULL;

	_AccpetSocketQueue.Clear();
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
	tcpkl.keepalivetime = 10000;	// 10�� ���� KEEPALIVE ��ȣ�� �����ڴ�. (������ �⺻�� 2�ð�)
	tcpkl.keepaliveinterval = 1000;	// keepalive ��ȣ�� ������ ������ ������ 1�ʸ��� �� �����ϰڴ�. (ms tcp �� 10ȸ ��õ� �Ѵ�)
	WSAIoctl(_ListenSocket, SIO_KEEPALIVE_VALS, &tcpkl, sizeof(tcp_keepalive), 0, 0, &dwResult, NULL, NULL);
	
	iRetval = ::bind(_ListenSocket, (sockaddr*)&Server_addr, sizeof(Server_addr));
	if (SOCKET_ERROR == iRetval)
	{
		int LastError = WSAGetLastError();
		wprintf(L"[Sock opt Error : %d\n", LastError);
		return false;
	}

	iRetval = listen(_ListenSocket, SOMAXCONN);
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
	//	IOCP ���
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

void CMMOServer::StartRecvPost(int Index)
{
	DWORD Flag = 0;
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

	if (SOCKET_ERROR == WSARecv(_pSessionArray[Index]->_ClientInfo.Sock, Buf, NumOfBuf, NULL, &Flag, &_pSessionArray[Index]->_RecvOver, NULL))
	{
		int LastError = WSAGetLastError();
		if (ERROR_IO_PENDING != LastError)
		{
			if (true != SessionAcquireFree(Index))
			{
				_pLog->Log(L"Error", LOG_SYSTEM, L"Recv SocketError - Code %d", LastError);
				shutdown(_pSessionArray[Index]->_ClientInfo.Sock, SD_BOTH);
			}
		}
	}
	return;
}

void CMMOServer::RecvPost(int Index)
{
	if (false == SessionAcquireLock(Index))
		return;

	CNetSession *pSession = _pSessionArray[Index];
	DWORD Flag = 0;
	ZeroMemory(&pSession->_RecvOver, sizeof(pSession->_RecvOver));
	
	WSABUF Buf[2];
	DWORD FreeSize = pSession->_RecvQ.GetFreeSize();
	DWORD NotBrokenPushSize = pSession->_RecvQ.GetNotBrokenPushSize();
	if (0 == FreeSize && 0 == NotBrokenPushSize)
	{
		if (true == SessionAcquireFree(Index))
		{
			_pLog->Log(L"Error", LOG_SYSTEM, L"RecvPost - RecvQ Full [Index : %d]", Index);
			shutdown(pSession->_ClientInfo.Sock, SD_BOTH);
		}
	}

	int NumOfBuf = (NotBrokenPushSize < FreeSize) ? 2 : 1;

	Buf[0].buf = pSession->_RecvQ.GetWriteBufferPtr();
	Buf[0].len = NotBrokenPushSize;

	if (2 == NumOfBuf)
	{
		Buf[1].buf = pSession->_RecvQ.GetBufferPtr();
		Buf[1].len = FreeSize - NotBrokenPushSize;
	}

	if (SOCKET_ERROR == WSARecv(pSession->_ClientInfo.Sock, Buf, NumOfBuf, NULL, &Flag, &pSession->_RecvOver, NULL))
	{
		int LastError = WSAGetLastError();
		if (ERROR_IO_PENDING != LastError)
		{
//			_pLog->Log(L"Debug", LOG_SYSTEM, L"Recv SocketError - Code %d", LastError);
			if (true != SessionAcquireFree(Index))
			{
				_pLog->Log(L"Error", LOG_SYSTEM, L"Recv SocketError - Code %d", LastError);
				shutdown(pSession->_ClientInfo.Sock, SD_BOTH);
			}
		}
	}
	return;
}

void CMMOServer::SendPost(int Index)
{
	CNetSession *pSession = _pSessionArray[Index];
	if (true == InterlockedCompareExchange(&pSession->_SendFlag, true, false))
		return;

	if (0 == pSession->_SendQ.GetUseSize())
//	if (0 == _pSessionArray[Index]->_SendQ.GetUseCount())
	{
		InterlockedExchange(&pSession->_SendFlag, false);
		return;
	}

	if (CNetSession::MODE_AUTH != pSession->_Mode && CNetSession::MODE_GAME != pSession->_Mode)
	{
		InterlockedExchange(&pSession->_SendFlag, false);
		return;
	}

//_Monitor_Counter_Send++;

	WSABUF Buf[WSABUF_MAX];
	CPacket *pPacket;
	int BufNum;
	int UseSize = (pSession->_SendQ.GetUseSize() / sizeof(CPacket*));
//	int UseSize = (_pSessionArray[Index]->_SendQ.GetUseCount());
	if (UseSize > WSABUF_MAX)
	{
		BufNum = WSABUF_MAX;
		pSession->_iSendPacketCnt = WSABUF_MAX;

		_Monitor_Counter_Send+= WSABUF_MAX;
		for (int j = 0; j < WSABUF_MAX; j++)
		{
			pSession->_SendQ.Dequeue((char*)&pPacket, sizeof(CPacket*));
//			pSession->_SendQ.Dequeue(pPacket);
			pSession->_CompleteSendPacket.Enqueue((char*)&pPacket, sizeof(CPacket*));
	//		pSession->_CompleteSendPacket.Enqueue(pPacket);
			Buf[j].buf = pPacket->GetBufferPtr();
			Buf[j].len = pPacket->GetPacketSize();
		}
	}
	else
	{
		BufNum = UseSize;
		pSession->_iSendPacketCnt = UseSize;

		_Monitor_Counter_Send += UseSize;
		for (int j = 0; j < UseSize; j++)
		{
			pSession->_SendQ.Dequeue((char*)&pPacket, sizeof(CPacket*));
//			pSession->_SendQ.Dequeue(pPacket);
			pSession->_CompleteSendPacket.Enqueue((char*)&pPacket, sizeof(CPacket*));
//			pSession->_CompleteSendPacket.Enqueue(pPacket);
			Buf[j].buf = pPacket->GetBufferPtr();
			Buf[j].len = pPacket->GetPacketSize();
		}
	}
	if (false == SessionAcquireLock(Index))
	{
		InterlockedExchange(&pSession->_SendFlag, false);
		return;
	}
	ZeroMemory(&pSession->_SendOver, sizeof(pSession->_SendOver));
	if (SOCKET_ERROR == WSASend(pSession->_ClientInfo.Sock, Buf, BufNum, NULL, 0, &pSession->_SendOver, NULL))
	{
		int LastError = WSAGetLastError();
//		_pLog->Log(L"Debug", LOG_SYSTEM, L"Send SocketError - Code %d", LastError);
		if (ERROR_IO_PENDING != LastError)
		{
			SessionAcquireFree(Index);
			_pLog->Log(L"Error", LOG_SYSTEM, L"Send SocketError - Code %d", LastError);
			shutdown(pSession->_ClientInfo.Sock, SD_BOTH);
			InterlockedExchange(&pSession->_SendFlag, false);
			return;

		}
	}
	return;
}

void CMMOServer::CompleteRecv(int Index, DWORD Trans)
{
	CNetSession *pSession = _pSessionArray[Index];
	pSession->_RecvQ.Enqueue(Trans);

	while (0 < pSession->_RecvQ.GetUseSize())
	{
		if (sizeof(CPacket::st_PACKET_HEADER) > pSession->_RecvQ.GetUseSize())
		{
			RecvPost(Index);
			return;
		}

		CPacket::st_PACKET_HEADER _Header;
		pSession->_RecvQ.Peek((char*)&_Header, sizeof(CPacket::st_PACKET_HEADER));
	
		if (CNetSession::BUF < _Header.shLen)
		{
			shutdown(pSession->_ClientInfo.Sock, SD_BOTH);
			return;
		}
	
		if (sizeof(CPacket::st_PACKET_HEADER) + _Header.shLen > pSession->_RecvQ.GetUseSize())
		{
			RecvPost(Index);
			return;
		}

		if (_byCode != _Header.byCode)
		{
			shutdown(pSession->_ClientInfo.Sock, SD_BOTH);
			return;
		}

		CPacket *pPacket = CPacket::Alloc();
		pSession->_RecvQ.Dequeue((char*)pPacket->GetBufferPtr(), _Header.shLen + sizeof(CPacket::st_PACKET_HEADER));
		pPacket->PushData(_Header.shLen + sizeof(CPacket::st_PACKET_HEADER));
		if (false == pPacket->DeCode(&_Header))
		{
			shutdown(pSession->_ClientInfo.Sock, SD_BOTH);
			pPacket->Free();
			return;
		}
		pPacket->m_header.byCode = _Header.byCode;
		pPacket->m_header.shLen = _Header.shLen;
		pPacket->m_header.RandKey = _Header.RandKey;
		pPacket->m_header.CheckSum = _Header.CheckSum;
		pPacket->PopData(sizeof(CPacket::st_PACKET_HEADER));

		pSession->_CompleteRecvPacket.Enqueue(pPacket);

		_Monitor_Counter_Recv++;
	}
	RecvPost(Index);
	return;
}

void CMMOServer::CompleteSend(int Index, DWORD Trans)
{
	CPacket *pPacket[WSABUF_MAX];
	CNetSession *pSession = _pSessionArray[Index];
	int Num = pSession->_iSendPacketCnt;

	pSession->_CompleteSendPacket.Peek((char*)&pPacket, sizeof(CPacket*) * Num);
	for (int i = 0; i < Num; i++)
	{
//		pSession->_CompleteSendPacket.Dequeue((char*)&pPacket, sizeof(CPacket*));
//		pSession->_CompleteSendPacket.Dequeue(pPacket);
		pPacket[i]->Free();
		pSession->_CompleteSendPacket.Dequeue(sizeof(CPacket*));
	}
	pSession->_iSendPacketCnt -= Num;
	InterlockedExchange(&pSession->_SendFlag, false);

	return;
}

void CMMOServer::ProcAuth_Accept()
{
	_Monitor_SessionAuthMode++;

	// �ű� ������ ó��
	CLIENT_CONNECT_INFO *pInfo;
	_AccpetSocketQueue.Dequeue((char*)&pInfo, sizeof(CLIENT_CONNECT_INFO*));
//	_AccpetSocketQueue.Dequeue(pInfo);

	int Index = NULL;
	_BlankSessionStack.Pop(Index);

	CNetSession* pSession = _pSessionArray[Index];
	pSession->_iArrayIndex = Index;
	pSession->_Mode = CNetSession::MODE_AUTH;
	pSession->_AuthToGameFlag = false;
	pSession->_iSendPacketCnt = NULL;
	pSession->_iSendPacketSize = NULL;
	pSession->_LogOutFlag = false;
	pSession->_RecvQ.Clear();
	InterlockedIncrement(&pSession->_IOCount);

	pSession->_ClientInfo.ClientID = pInfo->ClientID;
	strcpy_s(pSession->_ClientInfo.IP, sizeof(pSession->_ClientInfo.IP), pInfo->IP);
	pSession->_ClientInfo.Port = pInfo->Port;
	pSession->_ClientInfo.Sock = pInfo->Sock;

	_pMemoryPool_ConnectInfo->Free(pInfo);

	CreateIOCP_Socket(pSession->_ClientInfo.Sock, Index);
	pSession->OnAuth_ClientJoin();
	StartRecvPost(Index);
	return;
}

void CMMOServer::ProcAuth_LogoutInAuth()
{
	//	LogOutFlag�� true�� ������ LOGOUT_IN_AUTH ���� ����
	for (int i = 0; i < _iMaxSession; i++)
	{
		CNetSession *pSession = _pSessionArray[i];
		if (CNetSession::MODE_AUTH == pSession->_Mode && true == pSession->_LogOutFlag)
		{
			pSession->_Mode = CNetSession::MODE_LOGOUT_IN_AUTH;
			_Monitor_SessionAuthMode--;
		}
	}
	return;
}

void CMMOServer::ProcAuth_Logout()
{
	//	LOGOUT_IN_AUTH ����� ������ SendFlag�� 0�� ���
	//	WAIT_LOGOUT���� ��� ����
	for (int i = 0; i < _iMaxSession; i++)
	{
		CNetSession *pSession = _pSessionArray[i];
		if (CNetSession::MODE_LOGOUT_IN_AUTH == pSession->_Mode && false == pSession->_SendFlag)
		{
			pSession->_Mode = CNetSession::MODE_WAIT_LOGOUT;
			pSession->OnAuth_ClientLeave();
		}
	}
	return;
}

void CMMOServer::ProcAuth_AuthToGame()
{
	//	���� �迭�� ���鼭 AuthToGame ������ true�̸�,
	//	Auth ����� ������ AUTH_TO_GAME ���� ����
	for (int i = 0; i < _iMaxSession; i++)
	{
		CNetSession *pSession = _pSessionArray[i];
		if (CNetSession::MODE_AUTH == pSession->_Mode && pSession->_AuthToGameFlag)
		{
			pSession->_Mode = CNetSession::MODE_AUTH_TO_GAME;
			pSession->OnAuth_ClientLeave();
			_Monitor_SessionAuthMode--;
		}
	}
	return;
}

void CMMOServer::ProcGame_AuthToGame()
{
	//	Game ���� ��ȯ
	//	���� �迭�� ���鼭 AUTH_TO_GAME ����� ������ GAME ���� �����Ѵ�
	//	���߿��� ��� ���� �ο��� ����
	//	����� �׽�Ʈ�� ���� ���������� ����
	for (int i = 0; i < _iMaxSession; i++)
	{
		CNetSession *pSession = _pSessionArray[i];
		if (CNetSession::MODE_AUTH_TO_GAME == pSession->_Mode)
		{
			pSession->_Mode = CNetSession::MODE_GAME;
			_Monitor_SessionGameMode++;
		}
	}
	return;
}

void CMMOServer::ProcGame_LogoutInGame()
{
	//	GAME ��� LogoutFlag ó��
	//	���� �迭�� ���鼭 GAME ��忡�� LogoutFlag��
	//	true�� ������ LOGOUT_IN_GAME ���� ����
	for (int i = 0; i < _iMaxSession; i++)
	{
		CNetSession *pSession = _pSessionArray[i];
		if (CNetSession::MODE_GAME == pSession->_Mode && pSession->_LogOutFlag)
		{
			pSession->_Mode = CNetSession::MODE_LOGOUT_IN_GAME;
			_Monitor_SessionGameMode--;
		}
	}
	return;
}

void CMMOServer::ProcGame_Logout()
{
	//	LOGOUT_IN_GAME ��� ������ �ܰ� ����
	//	���� �迭�� ���鼭 LOGOUT_IN_GAME ����� ������
	//	SendFlag (WSASend ������ �۾��� ���ٴ� Ȯ��)�� 0�� ���
	//	WAIT_LOGOUT���� ��� ����
	//	virtual OnGame_ClientLeave ȣ��
	for (int i = 0; i < _iMaxSession; i++)
	{
		CNetSession *pSession = _pSessionArray[i];
		if (CNetSession::MODE_LOGOUT_IN_GAME == pSession->_Mode && false == pSession->_SendFlag)
		{
			pSession->_Mode = CNetSession::MODE_WAIT_LOGOUT;
			pSession->OnGame_ClientLeave();
		}
	}
	return;
}

void CMMOServer::ProcGame_Release()
{
	//	WAIT_LOGOUT ���� ������
	//	���� �迭�� ���鼭 WAIT_LOGOUT ����� ������ ���������� ������ ó���Ѵ�. 
	//	virtual OnGame_ClientRelease ȣ��
	//	NONE_MODE�� �ٲٰ� �÷��� �ٲ�
	//	���� �ʱ�ȭ
	//	�ش� ���� �迭 Index�� BlankSessionStack�� ����
	for (int i = 0; i < _iMaxSession; i++)
	{
		CNetSession *pSession = _pSessionArray[i];
		if (CNetSession::MODE_WAIT_LOGOUT == pSession->_Mode)
		{
			pSession->OnGame_ClientRelease();
			pSession->_Mode = CNetSession::MODE_NONE;
			pSession->_iArrayIndex = NULL;
			pSession->_iSendPacketCnt = NULL;
			pSession->_iSendPacketSize = NULL;
			pSession->_SendFlag = false;
			pSession->_LogOutFlag = false;
			pSession->_AuthToGameFlag = false;
			
			while (0 != pSession->_SendQ.GetUseSize())
//			while (0 != _pSessionArray[i]->_SendQ.GetUseCount())
			{
				//	���ʿ� ť�� ��Ŷ�� ���������� �ȵ�..
				CPacket *pPacket;
				pSession->_SendQ.Dequeue((char*)&pPacket, sizeof(CPacket*));
//				_pSessionArray[i]->_SendQ.Dequeue(pPacket);
				pPacket->Free();
			}
			while (0 != pSession->_CompleteRecvPacket.GetUseCount())
			{
				//	���ʿ� ť�� ��Ŷ�� ���������� �ȵ�..
				CPacket *pPacket;
				pSession->_CompleteRecvPacket.Dequeue(pPacket);
				pPacket->Free();
			}
			while (0 != pSession->_CompleteSendPacket.GetUseSize())
//			while (0 != _pSessionArray[i]->_CompleteSendPacket.GetUseCount())
			{
				//	���ʿ� ť�� ��Ŷ�� ���������� �ȵ�..
				CPacket *pPacket;
				pSession->_CompleteSendPacket.Dequeue((char*)&pPacket, sizeof(CPacket*));
//				_pSessionArray[i]->_CompleteSendPacket.Dequeue(pPacket);
				pPacket->Free();
			}

			pSession->_ClientInfo.ClientID = NULL;
			pSession->_ClientInfo.Port = NULL;
			closesocket(pSession->_ClientInfo.Sock);
			pSession->_ClientInfo.Sock = INVALID_SOCKET;

			InterlockedDecrement(&_Monitor_SessionAllMode);

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
		//	IP�� ����� ����Ǵ��� üũ�� ��
		inet_ntop(AF_INET, &ClientAddr.sin_addr, pInfo->IP, sizeof(pInfo->IP));
		pInfo->Port = ClientAddr.sin_port;
		pInfo->Sock = Sock;
		pInfo->ClientID = _iClientIDCnt++;

//		_AccpetSocketQueue.Enqueue(pInfo);
		_AccpetSocketQueue.Enqueue((char*)&pInfo, sizeof(CLIENT_CONNECT_INFO*));
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

		while (_AccpetSocketQueue.GetUseSize())
//		while(_AccpetSocketQueue.GetUseCount())
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
			CNetSession *pSession = _pSessionArray[i];
			if (CNetSession::MODE_AUTH == pSession->_Mode && 0 != pSession->_CompleteRecvPacket.GetUseCount())
			{
				Count = 0;
				while (Count < AUTH_MAX)
				{
					if (0 == pSession->_CompleteRecvPacket.GetUseCount())
						break;
					CPacket *pPacket;
					pSession->_CompleteRecvPacket.Dequeue(pPacket);
					pSession->OnAuth_Packet(pPacket);
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
		Sleep(0);
		Count = 0;
		_Monitor_Counter_GameUpdate++;
		ProcGame_AuthToGame();

		//	GAME ��� ���ǵ� ��Ŷ ó��
		//	���� �迭�� ��� ���鼭 GAME ��� ������ Ȯ���Ͽ�
		//	�ش� ������ CompleteRecvPacket ó����û
		//	virtual OnGame_Packet ȣ��
		//	�⺻ ��Ŷ 1�� ó��, ��Ȳ�� ���� N�� ó��
		for (int i = 0; i < _iMaxSession; i++)
		{
			CNetSession *pSession = _pSessionArray[i];
			if (CNetSession::MODE_GAME == pSession->_Mode && 0 != pSession->_CompleteRecvPacket.GetUseCount())
			{
				Count = 0;
				while (Count < GAME_MAX)
				{
					if (0 == pSession->_CompleteRecvPacket.GetUseCount())
						break;
					CPacket *pPacket;
					pSession->_CompleteRecvPacket.Dequeue(pPacket);
					pSession->OnGame_Packet(pPacket);
					pPacket->Free();
//					SessionAcquireLock(i);
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
		int  Index = _iMaxSession + 1;
		DWORD Trans = 0;
		
		Retval = GetQueuedCompletionStatus(_hIOCP, &Trans, (PULONG_PTR)&Index, (LPWSAOVERLAPPED*)&pOver, INFINITE);
		CNetSession *pSession = _pSessionArray[Index];
		if (nullptr == pOver)
		{
			if (_iMaxSession < Index && 0 == Trans)
			{
				PostQueuedCompletionStatus(_hIOCP, 0, 0, 0);
			}
		}
		else if (0 == Trans)
		{
			shutdown(pSession->_ClientInfo.Sock, SD_BOTH);
		}
		else if (pOver == &pSession->_RecvOver)
		{
			CompleteRecv(Index, Trans);
		}
		else if (pOver == &pSession->_SendOver)
		{
			CompleteSend(Index, Trans);
		}

		SessionAcquireFree(Index);
	}
	return true;
}

bool CMMOServer::SendThread_update()
{
	while (!_bShutdown)
	{
		Sleep(0);
		for (int i = 0; i < _iMaxSession; i++)
		{
			CNetSession *pSession = _pSessionArray[i];
			if (CNetSession::MODE_NONE == pSession->_Mode || pSession->_SendFlag || pSession->_LogOutFlag ||
				CNetSession::MODE_LOGOUT_IN_AUTH == pSession->_Mode || CNetSession::MODE_LOGOUT_IN_GAME == pSession->_Mode ||
				CNetSession::MODE_WAIT_LOGOUT == pSession->_Mode)
				continue;

//			if (false == SessionAcquireLock(i))
//				continue;
			
			SendPost(i);
//			SessionAcquireFree(i);
		}
		_Monitor_Counter_PacketSend++;
	}
	return true;
}

