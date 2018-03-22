#include <time.h>
#include <process.h>

#include "GameServer.h"

CGameServer::CGameServer(int iMaxSession) : CMMOServer(iMaxSession)
{
	_bMonitor = true;
	_hMonitorThread = NULL;
	_pPlayer = new CPlayer[iMaxSession];
	for (int i = 0; i < iMaxSession; i++)
		SetSessionArray(i, (CNetSession*)&_pPlayer[i]);
}

CGameServer::~CGameServer()
{
	delete[] _pPlayer;
}

void CGameServer::OnConnectionRequest()
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
	//	GAME 모드의 Update 처리
	//	클라이언트의 요청 (패킷수신) 외에 기본적으로 항시
	//	처리되어야 할 게임 컨텐츠 부분 로직

	return;
}

void CGameServer::OnError(int iErrorCode, WCHAR *szError)
{

	return;
}

bool CGameServer::MonitorInit()
{
	_hMonitorThread = (HANDLE)_beginthreadex(NULL, 0, &MonitorThread, (LPVOID)this, 0, NULL);
	return true;
}

bool CGameServer::MonitorOnOff()
{
	if (_bMonitor == true)
		_bMonitor = false;
	else
		_bMonitor = true;
	return _bMonitor;
}

bool CGameServer::MonitorThread_update()
{
	struct tm *pTime = new struct tm;
	time_t Timer;
	Timer = time(NULL);
	localtime_s(pTime, &Timer);

	int year = pTime->tm_year + 1900;
	int month = pTime->tm_mon + 1;
	int day = pTime->tm_mday;
	int hour = pTime->tm_hour;
	int min = pTime->tm_min;
	int sec = pTime->tm_sec;

	while (!_bShutdown)
	{
		Sleep(1000);
		Timer = time(NULL);
		localtime_s(pTime, &Timer);
		if (true == _bMonitor)
		{
			wprintf(L"\n\n");
			wprintf(L"////////////////////////////////////////////////////////////////////////////////\n");
			wprintf(L"	ServerStart : %d/%d/%d %d:%d:%d\n\n", year, month, day, hour, min, sec);
			wprintf(L"////////////////////////////////////////////////////////////////////////////////\n");
			wprintf(L"	%d/%d/%d %d:%d:%d\n\n", pTime->tm_year + 1900, pTime->tm_mon + 1,
				pTime->tm_mday, pTime->tm_hour, pTime->tm_min, pTime->tm_sec);

			wprintf(L"	SessionALL		:	%d\n", _Monitor_SessionAllMode);
			wprintf(L"	SessionAuth 		:	%d\n", _Monitor_SessionAuthMode);
			wprintf(L"	SessionGame		:	%d\n\n", _Monitor_SessionGameMode);
			
			wprintf(L"	Recv TPS		:	%d\n", _Monitor_Counter_Recv);
			wprintf(L"	Send TPS		:	%d\n\n", _Monitor_Counter_Send);

			wprintf(L"	AcceptTotal		:	%I64d\n\n", _Monitor_AcceptTotal);

			wprintf(L"	Accept Thread FPS	:	%d\n", _Monitor_AcceptSocket);
			wprintf(L"	Send   Thread FPS	:	%d\n", _Monitor_Counter_PacketSend);
			wprintf(L"	Auth   Thread FPS	:	%d\n", _Monitor_Counter_AuthUpdate);
			wprintf(L"	Game   Thread FPS	:	%d\n\n", _Monitor_Counter_GameUpdate);

			wprintf(L"	MemoryPool Alloc	:	%d\n", CPacket::GetAllocPool());
			wprintf(L"	Alloc / Free		:	%d\n", CPacket::_UseCount);
		}
		_Monitor_Counter_Recv = 0;
		_Monitor_Counter_Send = 0;
		_Monitor_AcceptSocket = 0;
		_Monitor_Counter_PacketSend = 0;
		_Monitor_Counter_AuthUpdate = 0;
		_Monitor_Counter_GameUpdate = 0;
	}
	return true;
}












































