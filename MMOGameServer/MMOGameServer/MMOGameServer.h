#ifndef _GAMESERVER_GAMESERVER_MMOGAMESERVER_H_
#define _GAMESERVER_GAMESERVER_MMOGAMESERVER_H_

#pragma comment(lib, "ws2_32")
#pragma comment(lib, "winmm.lib")

#include "Packet.h"
#include "RingBuffer.h"
#include "MemoryPool.h"
#include "LockFreeStack.h"
#include "LockFreeQueue.h"
#include "Player.h"
#include "Log.h"
#include "Dump.h"

#define		WORKER_THREAD_MAX		10
#define		WSABUF_MAX				200
#define		AUTH_MAX				5
#define		GAME_MAX				3
#define		RELEASE_MAX				100

class CMMOServer
{
public:
	CMMOServer(int iMaxSession);
	virtual ~CMMOServer();

	bool Start(WCHAR *szListenIP, int iPort, int iWorkerThread, bool bEnableNagle, BYTE byPacketCode, BYTE byPacketKey1, BYTE byPacketKey2);
	bool Stop();

	//	�ܺο��� ���ǰ�ü ����
	void SetSessionArray(int iArrayIndex, CNetSession *pSession);

	//	���� ������ ����
	bool CreateThread();

	//	Socket IOCP ���
	bool CreateIOCP_Socket(SOCKET Socket, ULONG_PTR Key);

	//	��Ŷ ������, ��ü���� ���
	void SendPacket_GameAll(CPacket *pPacket, unsigned __int64 ExcludeClientID);

	//	��Ŷ ������, Ư�� Ŭ���̾�Ʈ
	void SendPacket(CPacket *pPacket, unsigned __int64 ClientID);

	bool SessionAcquireLock(int Index);
	bool SessionAcquireFree(int Index);
private:
	void Error(int ErrorCode, WCHAR *szFormatStr, ...);
	void StartRecvPost(int Index);
	void RecvPost(int Index);
	void SendPost(int Index);
	void CompleteRecv(int Index, DWORD Trans);
	void CompleteSend(int Index, DWORD Trans);

	//	Auth, Game �������� ó���Լ�
	void ProcAuth_Accept();
	void ProcAuth_LogoutInAuth();
	void ProcAuth_Logout();
	void ProcAuth_AuthToGame();

	void ProcGame_AuthToGame();
	void ProcGame_LogoutInGame();
	void ProcGame_Logout();
	void ProcGame_Release();

	//	������ �Լ�
	static unsigned int __stdcall	AcceptThread(void *pParam)
	{
		CMMOServer *pAcceptThread = (CMMOServer*)pParam;
		if (NULL == pAcceptThread)
		{
			wprintf(L"[MMOServer :: AcceptThread] Init Error\n");
			return false;
		}
		pAcceptThread->AcceptThread_update();
		return true;
	}

	bool AcceptThread_update();

	static unsigned int __stdcall	AuthThread(void *pParam)
	{
		CMMOServer *pAuthThread = (CMMOServer*)pParam;
		if (NULL == pAuthThread)
		{
			wprintf(L"[MMOServer :: AuthThread] Init Error\n");
			return false;
		}
		pAuthThread->AuthThread_update();
		return true;
	}
	bool AuthThread_update();

	static unsigned __stdcall	GameUpdateThread(void *pParam)
	{
		CMMOServer *pGameUpdateThread = (CMMOServer*)pParam;
		if (NULL == pGameUpdateThread)
		{
			wprintf(L"[MMOServer :: GameUpdateThread] Init Error\n");
			return false;
		}
		pGameUpdateThread->GameUpdateThread_update();
		return true;
	}
	bool GameUpdateThread_update();


	static unsigned __stdcall	IOCPWorkerThread(void *pParam)
	{
		CMMOServer *pIOCPWorkerThread = (CMMOServer*)pParam;
		if (NULL == pIOCPWorkerThread)
		{
			wprintf(L"[MMOServer :: IOCPWorkerThread] Init Error\n");
			return false;
		}
		pIOCPWorkerThread->IOCPWorkerThread_update();
		return true;
	}
	bool IOCPWorkerThread_update();

	static unsigned __stdcall	SendThread(void *pParam)
	{
		CMMOServer *pSendThread = (CMMOServer*)pParam;
		if (NULL == pSendThread)
		{
			wprintf(L"[MMOServer :: SendThread] Init Error\n");
			return false;
		}
		pSendThread->SendThread_update();
		return true;
	}
	bool SendThread_update();

private:
	virtual void OnConnectionRequest() = 0;
	
	//	AUTH ��� ������Ʈ �̺�Ʈ ����ó����
	virtual void OnAuth_Update() = 0;
	
	//	GAME ��� ������Ʈ �̺�Ʈ ����ó����
	virtual void OnGame_Update() = 0;

	virtual void OnError(int iErrorCode, WCHAR *szError) = 0;

public:
	const int _iMaxSession;

protected:
	bool _bShutdown;
	bool _bShutdownListen;

private:
	SOCKET _ListenSocket;

	BYTE _byCode;
	bool _bEnableNagle;
	int _iWorkerThread;

	WCHAR _szListenIP[16];
	int _iListenPort;
	unsigned __int64 _iClientIDCnt;

	HANDLE _hAcceptThread;

	CLockFreeQueue<CLIENT_CONNECT_INFO *>	_AccpetSocketQueue;	//	�ű����� Socket ť
	CMemoryPool<CLIENT_CONNECT_INFO>		*_pMemoryPool_ConnectInfo;

	//	Auth ��
	HANDLE	_hAuthThread;

	CLockFreeStack<int>	_BlankSessionStack;		//	�� ���� index

	//	GameUpdate ��
	HANDLE	_hGameUpdateThread;

	//	IOCP ��
	HANDLE	_hIOCPWorkerThread[WORKER_THREAD_MAX];
	HANDLE	_hIOCP;

	//	Send ��
	HANDLE	_hSendThread;

	CNetSession	**_pSessionArray;
	SRWLOCK		_Srwlock;
public:
	long long	_Monitor_AcceptTotal;			//	Accept �� Ƚ��
	long		_Monitor_AcceptSocket;			//	1�� �� Accept Ƚ��
	long		_Monitor_SessionAllMode;		//	��ü ������ ��
	long		_Monitor_SessionAuthMode;		//	AuthMode ������ ��
	long		_Monitor_SessionGameMode;		//	GameMode ������ ��

	long		_Monitor_Counter_AuthUpdate;	//	1�� �� AuthThread ���� Ƚ��
	long		_Monitor_Counter_GameUpdate;	//	1�� �� GameThread ���� Ƚ��
	long		_Monitor_Counter_Recv;			//	1�� �� Recv ���� Ƚ��
	long		_Monitor_Counter_Send;			//	1�� �� Send ���� Ƚ��
	long		_Monitor_Counter_PacketSend;	//	1�� �� SendThread ���� Ƚ��

	CSystemLog	*_pLog;

	
};

#endif
