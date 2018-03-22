#ifndef _GAMESERVER_GAMESERVER_GAMESERVER_H_
#define _GAMESERVER_GAMESERVER_GAMESERVER_H_

#include "MMOGameServer.h"

class CGameServer : public CMMOServer
{
public:
	CGameServer(int iMaxSession);
	~CGameServer();

	void OnConnectionRequest();
	void OnAuth_Update();
	void OnGame_Update();
	void OnError(int iErrorCode, WCHAR *szError);

	bool MonitorInit();
	bool MonitorOnOff();
	static unsigned int __stdcall MonitorThread(void *pParam)
	{
		CGameServer *pMonitorThread = (CGameServer*)pParam;
		if (NULL == pMonitorThread)
		{
			wprintf(L"[GameServer :: MonitorThread] Init Error\n");
			return false;
		}
		pMonitorThread->MonitorThread_update();
		return true;
	}
	bool MonitorThread_update();

private:
	bool	_bMonitor;
	CPlayer *_pPlayer;
	HANDLE	_hMonitorThread;
};

#endif
